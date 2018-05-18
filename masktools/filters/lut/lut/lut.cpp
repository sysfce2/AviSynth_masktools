#include "lut.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Single::lut_c(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, const Byte lut[256])
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++) {
            dstp[x] = lut[dstp[x]];
        }
        dstp += dst_pitch;
    }
}

void Filtering::MaskTools::Filters::Lut::Single::realtime8_c(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, Parser::Context &ctx)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      dstp[x] = ctx.compute_byte_x(dstp[x]);
    }
    dstp += dst_pitch;
  }
}

template<int bits_per_pixel>
void realtime16_t_c(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, Parser::Context &ctx)
{
  const Word max_pixel_value = (1 << bits_per_pixel) - 1;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      Word pixel = reinterpret_cast<uint16_t *>(dstp)[x] ;
      if (bits_per_pixel != 16) pixel = min(pixel, max_pixel_value);

      Word result = ctx.compute_word_x<bits_per_pixel>(pixel);

      reinterpret_cast<uint16_t *>(dstp)[x] = result;
    }
    dstp += dst_pitch;
  }
}

void Filtering::MaskTools::Filters::Lut::Single::realtime32_c(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, bool chroma, Parser::Context &ctx)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++) {
      reinterpret_cast<Float *>(dstp)[x] = ctx.compute_float_x(reinterpret_cast<Float *>(dstp)[x], chroma);
    }
    dstp += dst_pitch;
  }
}

template void realtime16_t_c<10>(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<12>(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<14>(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, Parser::Context &ctx);
template void realtime16_t_c<16>(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, Parser::Context &ctx);

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Single {
ProcessorCtx *realtime10_c = &realtime16_t_c<10>;
ProcessorCtx *realtime12_c = &realtime16_t_c<12>;
ProcessorCtx *realtime14_c = &realtime16_t_c<14>;
ProcessorCtx *realtime16_c = &realtime16_t_c<16>;
} } } } }

