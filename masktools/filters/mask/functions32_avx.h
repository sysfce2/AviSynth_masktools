#ifndef __Mt_MaskFunctions32_AVX_H__
#define __Mt_MaskFunctions32_AVX_H__

#include "../../../common/utils/utils.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask {

typedef Float (Operator)(Float, Float, Float, Float, Float, Float, Float, Float, Float, const Float matrix[10], Float nLowThreshold, Float nHighThreshold);
typedef void (ProcessLineAvx)(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Float matrix[10], const __m256 &lowThresh, const __m256 &highThresh, int width);

template<Operator op, class T>
void generic32_as_avx_c(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, T &thresholds, const Float matrix[10], int nWidth, int nHeight)
{
  nDstPitch /= sizeof(float);
  nSrcPitch /= sizeof(float);
  const Float *pSrcp = pSrc - nSrcPitch;
  const Float *pSrcn = pSrc + nSrcPitch;

   /* top-left */
   pDst[0] = op(pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], matrix, thresholds.min(0), thresholds.max(0));

   /* top */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], matrix, thresholds.min(x), thresholds.max(x));

   /* top-right */
   pDst[nWidth-1] = op(pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));

   pDst  += nDstPitch;
   pSrcp += nSrcPitch;
   pSrc  += nSrcPitch;
   pSrcn += nSrcPitch;

   thresholds.nextRow();

   for ( int y = 1; y < nHeight - 1; y++ )
   {
      /* left */
      pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], matrix, thresholds.min(0), thresholds.max(0));

      /* center */
      for ( int x = 1; x < nWidth - 1; x++ )
         pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], matrix, thresholds.min(x), thresholds.max(x));

      /* right */
      pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));

      pDst  += nDstPitch;
      pSrcp += nSrcPitch;
      pSrc  += nSrcPitch;
      pSrcn += nSrcPitch;

      thresholds.nextRow();
   }

   /* bottom-left */
   pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], matrix, thresholds.min(0), thresholds.max(0));

   /* bottom */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], matrix, thresholds.min(x), thresholds.max(x));

   /* bottom-right */
   pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));
}

template<ProcessLineAvx process_line_left, ProcessLineAvx process_line, ProcessLineAvx process_line_right>
static void generic32_avx(Float *pDst0, ptrdiff_t nDstPitch, const Float *pSrc0, ptrdiff_t nSrcPitch, const Float matrix[10], float nLowThreshold, float nHighThreshold, int nWidth, int nHeight) {
    Byte *pDst = reinterpret_cast<Byte *>(pDst0);
    const Byte *pSrc = reinterpret_cast<const Byte *>(pSrc0);

    nWidth *= sizeof(float); // byte size for sse
  
    const Byte *pSrcp = pSrc - nSrcPitch; // prev
    const Byte *pSrcn = pSrc + nSrcPitch; // next
    auto low_thr_v = _mm256_set1_ps(nLowThreshold);
    auto high_thr_v = _mm256_set1_ps(nHighThreshold);

    size_t avx_width = (nWidth - sizeof(float) - 32) / 32 * 32 + 32;
    /* top-left */
    process_line_left(pDst, pSrc, pSrc, pSrcn, matrix, low_thr_v, high_thr_v, 32);
    /* top */
    process_line(pDst + 32, pSrc+32, pSrc+32, pSrcn+32, matrix, low_thr_v, high_thr_v, avx_width - 32);

    /* top-right */
    process_line_right(pDst + nWidth - 32, pSrc + nWidth - 32, pSrc + nWidth - 32, pSrcn + nWidth - 32, matrix, low_thr_v, high_thr_v, 32);

    pDst  += nDstPitch;
    pSrcp += nSrcPitch;
    pSrc  += nSrcPitch;
    pSrcn += nSrcPitch;

    for ( int y = 1; y < nHeight-1; y++ )
    {
        /* left */
        process_line_left(pDst, pSrcp, pSrc, pSrcn, matrix, low_thr_v, high_thr_v, 32);
        /* center */
        process_line(pDst + 32, pSrcp+32, pSrc+32, pSrcn+32, matrix, low_thr_v, high_thr_v, avx_width - 32);
        /* right */
        process_line_right(pDst + nWidth - 32, pSrcp + nWidth - 32, pSrc + nWidth - 32, pSrcn + nWidth - 32, matrix, low_thr_v, high_thr_v, 32);

        pDst  += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc  += nSrcPitch;
        pSrcn += nSrcPitch;
    }

    /* bottom-left */
    process_line_left(pDst, pSrcp, pSrc, pSrc, matrix, low_thr_v, high_thr_v, 32);
    /* bottom */
    process_line(pDst + 32, pSrcp+32, pSrc+32, pSrc+32, matrix, low_thr_v, high_thr_v, avx_width - 32);
    /* bottom-right */
    process_line_right(pDst + nWidth - 32, pSrcp + nWidth - 32, pSrc + nWidth - 32, pSrc + nWidth - 32, matrix, low_thr_v, high_thr_v, 32);
    _mm256_zeroupper();
}

} } } }

#endif