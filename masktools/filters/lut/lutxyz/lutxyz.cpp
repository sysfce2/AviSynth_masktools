#include "lutxyz.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Trial::lut_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, const Byte *lut)
{
   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = lut[(pDst[x]<<16) + (pSrc1[x]<<8) + (pSrc2[x])];
      pDst += nDstPitch;
      pSrc1 += nSrc1Pitch;
      pSrc2 += nSrc2Pitch;
   }
}

void Filtering::MaskTools::Filters::Lut::Trial::realtime8_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      dstp[x] = ctx.compute_byte(dstp[x], srcp[x], srcp2[x]);
    }
    dstp += dst_pitch;
    srcp += nSrcPitch;
    srcp2 += nSrc2Pitch;
  }
}

template<int bits_per_pixel>
static void realtime16_t_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx)
{
  const Word max_pixel_value = (1 << bits_per_pixel) - 1;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      Word pixelX = reinterpret_cast<uint16_t *>(dstp)[x];
      if (bits_per_pixel != 16) pixelX = min(pixelX, max_pixel_value); // clamp input below 16 bit
      Word pixelY = reinterpret_cast<const uint16_t *>(srcp)[x];
      if (bits_per_pixel != 16) pixelY = min(pixelY, max_pixel_value); // clamp input below 16 bit
      Word pixelZ = reinterpret_cast<const uint16_t *>(srcp2)[x];
      if (bits_per_pixel != 16) pixelZ = min(pixelZ, max_pixel_value); // clamp input below 16 bit

      Word result = ctx.compute_word_xyz<bits_per_pixel>(pixelX, pixelY, pixelZ);

      reinterpret_cast<uint16_t *>(dstp)[x] = result;
    }
    dstp += dst_pitch;
    srcp += nSrcPitch;
    srcp2 += nSrc2Pitch;
  }
}

void Filtering::MaskTools::Filters::Lut::Trial::realtime32_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      reinterpret_cast<Float *>(dstp)[x] = ctx.compute_float(reinterpret_cast<Float *>(dstp)[x], reinterpret_cast<const Float *>(srcp)[x], reinterpret_cast<const Float *>(srcp2)[x], -1.0 /*n/a*/, 32);
    }
    dstp += dst_pitch;
    srcp += nSrcPitch;
    srcp2 += nSrc2Pitch;
  }
}

template void realtime16_t_c<10>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<12>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<14>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<16>(Byte *dstp, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *srcp2, ptrdiff_t nSrc2Pitch, int width, int height, Parser::Context &ctx);

namespace Filtering {
  namespace MaskTools {
    namespace Filters {
      namespace Lut {
        namespace Trial {
          ProcessorCtx *realtime10_c = &realtime16_t_c<10>;
          ProcessorCtx *realtime12_c = &realtime16_t_c<12>;
          ProcessorCtx *realtime14_c = &realtime16_t_c<14>;
          ProcessorCtx *realtime16_c = &realtime16_t_c<16>;
        }
      }
    }
  }
}


