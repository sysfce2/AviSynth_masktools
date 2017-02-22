#include "luts.h"
#include "../functions.h"

using namespace Filtering;

//similar template to lutf
template<bool realtime, class T>
static void custom_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pLut, Parser::Context *ctx, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, const String &mode)
{
   T new_value( mode );
   for ( int j = 0; j < nHeight; j++ )
   {
      for ( int i = 0; i < nWidth; i++ )
      {
         new_value.reset();
         for ( int k = 0; k < nCoordinates; k+=2 )
         {
            int x = pCoordinates[k] + i;
            int y = pCoordinates[k+1] + j;

            if ( x < 0 ) x = 0;
            if ( x >= nWidth ) x = nWidth - 1;
            if ( y < 0 ) y = 0;
            if ( y >= nHeight ) y = nHeight - 1;
            if (realtime)
              new_value.add(ctx->compute_byte(pDst[i], pSrc[x + (y - j) * nSrcPitch]));
            else
              new_value.add(pLut[(pDst[i] << 8) + pSrc[x + (y - j) * nSrcPitch]]);
         }
         pDst[i] = new_value.finalize();
      }
      pSrc += nSrcPitch;
      pDst += nDstPitch;
   }
}

//similar template to lutf
template<bool realtime, int bits_per_pixel, class T>
static void custom16_c(Byte *pDst8, ptrdiff_t nDstPitch, const Byte *pSrc8, ptrdiff_t nSrcPitch, const Byte *pLut, Parser::Context *ctx, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, const String &mode)
{
  T new_value(mode);

  const uint16_t *pSrc = reinterpret_cast<const uint16_t *>(pSrc8);
  uint16_t *pDst = reinterpret_cast<uint16_t *>(pDst8);
  nSrcPitch /= sizeof(uint16_t);
  nDstPitch /= sizeof(uint16_t);
  const uint16_t max_pixel_value = (1 << bits_per_pixel) - 1;

  for (int j = 0; j < nHeight; j++)
  {
    for (int i = 0; i < nWidth; i++)
    {
      new_value.reset();
      for (int k = 0; k < nCoordinates; k += 2)
      {
        int x = pCoordinates[k] + i;
        int y = pCoordinates[k + 1] + j;

        if (x < 0) x = 0;
        if (x >= nWidth) x = nWidth - 1;
        if (y < 0) y = 0;
        if (y >= nHeight) y = nHeight - 1;

        if (realtime) {
          if (bits_per_pixel == 16) // no clamp
            new_value.add(ctx->compute_word(pDst[i], pSrc[x + (y - j) * nSrcPitch], -1, -1, bits_per_pixel));
          else
            new_value.add(min(ctx->compute_word(pDst[i], pSrc[x + (y - j) * nSrcPitch], -1, -1, bits_per_pixel), max_pixel_value));
        }
        else {
          Word X = pDst[i];
          Word Y = pSrc[x + (y - j) * nSrcPitch];
          if (bits_per_pixel < 16) {
            X = min(X, max_pixel_value);
            Y = min(Y, max_pixel_value);
          }
          new_value.add(reinterpret_cast<const uint16_t *>(pLut)[(X << bits_per_pixel) + Y]);
        }
      }
      pDst[i] = new_value.finalize(); // cannot overflow
    }
    pSrc += nSrcPitch;
    pDst += nDstPitch;
  }
}

//similar template to lutf
template<class T>
static void custom32_c(Byte *pDst8, ptrdiff_t nDstPitch, const Byte *pSrc8, ptrdiff_t nSrcPitch, const Byte *pLut, Parser::Context *ctx, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, const String &mode)
{
  UNUSED(pLut);
  T new_value(mode);

  const float *pSrc = reinterpret_cast<const float *>(pSrc8);
  float *pDst = reinterpret_cast<float *>(pDst8);
  nSrcPitch /= sizeof(float);
  nDstPitch /= sizeof(float);

  for (int j = 0; j < nHeight; j++)
  {
    for (int i = 0; i < nWidth; i++)
    {
      new_value.reset();
      for (int k = 0; k < nCoordinates; k += 2)
      {
        int x = pCoordinates[k] + i;
        int y = pCoordinates[k + 1] + j;

        if (x < 0) x = 0;
        if (x >= nWidth) x = nWidth - 1;
        if (y < 0) y = 0;
        if (y >= nHeight) y = nHeight - 1;
        // float is always realtime
        new_value.add(ctx->compute_float(pDst[i], pSrc[x + (y - j) * nSrcPitch], -1, -1, 32));
      }
      pDst[i] = new_value.finalize();
    }
    pSrc += nSrcPitch;
    pDst += nDstPitch;
  }
}
namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Spatial {

Processor *processors_array[NUM_MODES] = MPROCESSOR_SINGLE( custom_c, false );
Processor *processors_10_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, false, 10);
Processor *processors_12_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, false, 12);
Processor *processors_14_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, false, 14);
Processor *processors_16_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, false, 16);

Processor *processors_realtime_8_array[NUM_MODES] = MPROCESSOR_SINGLE(custom_c, true);
Processor *processors_realtime_10_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, true, 10);
Processor *processors_realtime_12_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, true, 12);
Processor *processors_realtime_14_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, true, 14);
Processor *processors_realtime_16_array[NUM_MODES] = MPROCESSOR16_SINGLE(custom16_c, true, 16);
Processor *processors_realtime_32_array[NUM_MODES] = MPROCESSOR32_SINGLE(custom32_c);

} } } } }

