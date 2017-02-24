#include "merge.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge {

   static void merge32_avx_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
      const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
   {
      for (int y = 0; y < nHeight; ++y)
      {
         for (int x = 0; x < nWidth; ++x)
            reinterpret_cast<float *>(pDst)[x] = (1.0f - reinterpret_cast<const float *>(pMask)[x]) * reinterpret_cast<float *>(pDst)[x] + 
            reinterpret_cast<const float *>(pMask)[x] * reinterpret_cast<const float *>(pSrc1)[x];
         pDst += nDstPitch;
         pSrc1 += nSrc1Pitch;
         pMask += nSrc2Pitch;
      }
   }

   static void merge32_luma_420_avx_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
      const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
   {
      for (int y = 0; y < nHeight; ++y)
      {
         for (int x = 0; x < nWidth; ++x)
         {
           // 420: both width and height is halved, averaging from 4 pixels of full size mask
            const float nMask = 0.25f * (reinterpret_cast<const float *>(pMask)[x * 2] + reinterpret_cast<const float *>(pMask)[x * 2 + nSrc2Pitch] + 
                                         reinterpret_cast<const float *>(pMask)[x * 2 + 1] + reinterpret_cast<const float *>(pMask)[x * 2 + nSrc2Pitch + 1]);
            reinterpret_cast<float *>(pDst)[x] = (1.0f - nMask) * reinterpret_cast<float *>(pDst)[x] + nMask * reinterpret_cast<const float *>(pSrc1)[x];
         }
         pDst += nDstPitch;
         pSrc1 += nSrc1Pitch;
         pMask += nSrc2Pitch * 2;
      }
   }

   static void merge32_luma_422_avx_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
     const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
   {
     for (int y = 0; y < nHeight; ++y)
     {
       for (int x = 0; x < nWidth; ++x)
       {
         const float nMask = 0.5f * (reinterpret_cast<const float *>(pMask)[x * 2] + reinterpret_cast<const float *>(pMask)[x * 2 + 1]); // 422: only width is halved, averaging from two pixels of full size mask
         reinterpret_cast<float *>(pDst)[x] = (1.0f - nMask) * reinterpret_cast<float *>(pDst)[x] + nMask * reinterpret_cast<const float *>(pSrc1)[x];
       }
       pDst += nDstPitch;
       pSrc1 += nSrc1Pitch;
       pMask += nSrc2Pitch;
     }
   }

   template <MemoryMode mem_mode>
   MT_FORCEINLINE __m256 merge32_avx_core(Byte *pDst, const Byte *pSrc, const __m256& mask) {
      // ( 1- mask) * dst + mask * src = 
      // dst - dst * mask + src * mask = 
      // dst + mask * (src - dst)
      auto dst = simd256_load_ps<mem_mode>(pDst);
      auto src = simd256_load_ps<mem_mode>(pSrc);
      auto diff = _mm256_sub_ps(src, dst);
      auto tmp = _mm256_mul_ps(diff, mask); // (p2-p1)*mask
      return _mm256_add_ps(tmp, dst); // +dst
   }

   template <MemoryMode mem_mode>
   void merge32_avx_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
      const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
   {
      nWidth *= sizeof(Float);

      int wMod32 = (nWidth / 32) * 32;
      auto pDst_s = pDst;
      auto pSrc1_s = pSrc1;
      auto pMask_s = pMask;
      for (int j = 0; j < nHeight; ++j) {
         for (int i = 0; i < wMod32; i += 32) {
            auto mask = simd256_load_ps<mem_mode>(pMask + i);
            auto result = merge32_avx_core<mem_mode>(pDst + i, pSrc1 + i, mask);
            simd256_store_ps<mem_mode>(pDst + i, result);
         }
         pDst += nDstPitch;
         pSrc1 += nSrc1Pitch;
         pMask += nSrc2Pitch;
      }

      if (nWidth > wMod32) {
         merge32_avx_c(pDst_s + wMod32, nDstPitch, pSrc1_s + wMod32, nSrc1Pitch, pMask_s + wMod32, nSrc2Pitch, (nWidth - wMod32) / sizeof(Float), nHeight);
      }
      _mm256_zeroupper();
   }

   template <MemoryMode mem_mode>
   void merge32_luma_420_avx_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
     const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
   {
     nWidth *= sizeof(Float);

     int wMod32 = (nWidth / 32) * 32;
     auto pDst_s = pDst;
     auto pSrc1_s = pSrc1;
     auto pMask_s = pMask;
     auto vOneFourth = _mm256_set1_ps(0.25f);
     for (int j = 0; j < nHeight; ++j) {
       for (int i = 0; i < wMod32; i += 32) {
         _mm_prefetch(reinterpret_cast<const char*>(pMask) + i * 2 + 64, _MM_HINT_T0);
         _mm_prefetch(reinterpret_cast<const char*>(pMask) + nSrc2Pitch + i * 2 + 64, _MM_HINT_T0);
         // preparing mask
         auto src2_row1_t1 = simd256_load_ps<mem_mode>(pMask + i * 2);
         auto src2_row1_t2 = simd256_load_ps<mem_mode>(pMask + i * 2 + 32);
         auto src2_row2_t1 = simd256_load_ps<mem_mode>(pMask + nSrc2Pitch + i * 2);
         auto src2_row2_t2 = simd256_load_ps<mem_mode>(pMask + nSrc2Pitch + i * 2 + 32);
         auto tmp1 = _mm256_add_ps(src2_row1_t1, src2_row2_t1);
         auto tmp2 = _mm256_add_ps(src2_row1_t2, src2_row2_t2);
         auto sum = _mm256_add_ps(tmp1, tmp2);
         auto mask = _mm256_mul_ps(sum, vOneFourth);

         auto result = merge32_avx_core<mem_mode>(pDst + i, pSrc1 + i, mask);

         simd256_store_ps<mem_mode>(pDst + i, result);
       }
       pDst += nDstPitch;
       pSrc1 += nSrc1Pitch;
       pMask += nSrc2Pitch * 2;
     }
     if (nWidth > wMod32) {
       merge32_luma_420_avx_c(pDst_s + wMod32, nDstPitch, pSrc1_s + wMod32, nSrc1Pitch, pMask_s + wMod32 * 2, nSrc2Pitch, (nWidth - wMod32) / sizeof(Float), nHeight);
     }
     _mm256_zeroupper();
   }

   template <MemoryMode mem_mode>
   void merge32_luma_422_avx_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
     const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
   {
     nWidth *= sizeof(Float);

     int wMod32 = (nWidth / 32) * 32;
     auto pDst_s = pDst;
     auto pSrc1_s = pSrc1;
     auto pMask_s = pMask;
     auto vHalf = _mm256_set1_ps(0.5f);
     for (int j = 0; j < nHeight; ++j) {
       for (int i = 0; i < wMod32; i += 32) {
         _mm_prefetch(reinterpret_cast<const char*>(pMask) + i * 2 + 64, _MM_HINT_T0);
         // preparing mask
         auto mask_row_t1 = simd256_load_ps<mem_mode>(pMask + i * 2);
         auto mask_row_t2 = simd256_load_ps<mem_mode>(pMask + i * 2 + 32);
         auto sum = _mm256_add_ps(mask_row_t1, mask_row_t2);
         auto mask = _mm256_mul_ps(sum, vHalf);
         auto result = merge32_avx_core<mem_mode>(pDst + i, pSrc1 + i, mask);

         simd256_store_ps<mem_mode>(pDst + i, result);
       }
       pDst += nDstPitch;
       pSrc1 += nSrc1Pitch;
       pMask += nSrc2Pitch;
     }
     if (nWidth > wMod32) {
       merge32_luma_422_avx_c(pDst_s + wMod32, nDstPitch, pSrc1_s + wMod32, nSrc1Pitch, pMask_s + wMod32 * 2, nSrc2Pitch, (nWidth - wMod32) / sizeof(Float), nHeight);
     }
     _mm256_zeroupper();
   }


   Processor32 *merge32_avx = merge32_avx_t<MemoryMode::SSE2_UNALIGNED>;
   Processor32 *merge32_luma_420_avx = merge32_luma_420_avx_t<MemoryMode::SSE2_UNALIGNED>;
   Processor32 *merge32_luma_422_avx = merge32_luma_422_avx_t<MemoryMode::SSE2_UNALIGNED>;

} } } }