#include "lutxy.h"
#include <type_traits>

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Dual::lut_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, const Byte lut[65536])
{
   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = lut[(pDst[x]<<8) + pSrc[x]];
      pDst += nDstPitch;
      pSrc += nSrcPitch;
   }
}


template<int bits_per_pixel>
static void lut16_t_c(Byte *dstp, ptrdiff_t nDstPitch, const Byte *srcp, ptrdiff_t nSrcPitch, int nWidth, int nHeight, const Word *lut)
{
  const Word max_pixel_value = (1 << bits_per_pixel) - 1;
  for (int y = 0; y < nHeight; y++)
  {
    for (int x = 0; x < nWidth; x++) {
      Word pixelX = reinterpret_cast<uint16_t *>(dstp)[x];
      if (bits_per_pixel != 16) pixelX = min(pixelX, max_pixel_value);
      Word pixelY = reinterpret_cast<const uint16_t *>(srcp)[x];
      if (bits_per_pixel != 16) pixelY = min(pixelY, max_pixel_value);
      typedef typename std::conditional< bits_per_pixel == 16 , size_t , int>::type safe_index_t;
      reinterpret_cast<uint16_t *>(dstp)[x] = lut[((safe_index_t)pixelX << bits_per_pixel) + pixelY];
    }
    dstp += nDstPitch;
    srcp += nSrcPitch;
  }
}

void Filtering::MaskTools::Filters::Lut::Dual::realtime8_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      dstp[x] = ctx.compute_byte_xy(dstp[x], srcp[x]);
    }
    dstp += dst_pitch;
    srcp += nSrcPitch;
  }
}

template<int bits_per_pixel>
static void realtime16_t_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx)
{
  const Word max_pixel_value = (1 << bits_per_pixel) - 1;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      Word pixelX = reinterpret_cast<uint16_t *>(dstp)[x];
      if (bits_per_pixel != 16) pixelX = min(pixelX, max_pixel_value); // clamp input below 16 bit
      Word pixelY = reinterpret_cast<const uint16_t *>(srcp)[x];
      if (bits_per_pixel != 16) pixelY = min(pixelY, max_pixel_value); // clamp input below 16 bit

      Word result = ctx.compute_word_xy<bits_per_pixel>(pixelX, pixelY);

      reinterpret_cast<uint16_t *>(dstp)[x] = result;
    }
    dstp += dst_pitch;
    srcp += nSrcPitch;
  }
}

void Filtering::MaskTools::Filters::Lut::Dual::realtime32_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      reinterpret_cast<Float *>(dstp)[x] = ctx.compute_float_xy(reinterpret_cast<Float *>(dstp)[x], reinterpret_cast<const Float *>(srcp)[x]);
    }
    dstp += dst_pitch;
    srcp += nSrcPitch;
  }
}

template void lut16_t_c<10>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, const Word* lut);
template void lut16_t_c<12>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, const Word* lut);
template void lut16_t_c<14>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, const Word* lut);
template void lut16_t_c<16>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, const Word* lut);

template void realtime16_t_c<10>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<12>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<14>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<16>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int width, int height, Parser::Context &ctx);

namespace Filtering {
  namespace MaskTools {
    namespace Filters {
      namespace Lut {
        namespace Dual {
          Processor16 *lut10_c = &lut16_t_c<10>;
          Processor16 *lut12_c = &lut16_t_c<12>;
          Processor16 *lut14_c = &lut16_t_c<14>;
          Processor16 *lut16_c = &lut16_t_c<16>;

          ProcessorCtx *realtime10_c = &realtime16_t_c<10>;
          ProcessorCtx *realtime12_c = &realtime16_t_c<12>;
          ProcessorCtx *realtime14_c = &realtime16_t_c<14>;
          ProcessorCtx *realtime16_c = &realtime16_t_c<16>;
        }
      }
    }
  }
}

