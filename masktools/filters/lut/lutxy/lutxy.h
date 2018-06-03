#ifndef __Mt_Lutxy_H__
#define __Mt_Lutxy_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"
#include <mutex>

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Dual {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, const Byte lut[65536]);
typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, const Word *lut);
typedef void(ProcessorCtx)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Parser::Context &ctx);
typedef void(ProcessorCtx32)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, bool chroma, Parser::Context &ctx);

Processor lut_c;
extern Processor16 *lut10_c;
extern Processor16 *lut12_c;
extern Processor16 *lut14_c;
extern Processor16 *lut16_c;

ProcessorCtx realtime8_c;
extern ProcessorCtx *realtime10_c;
extern ProcessorCtx *realtime12_c;
extern ProcessorCtx *realtime14_c;
extern ProcessorCtx *realtime16_c;
ProcessorCtx32 realtime32_c;


class Lutxy : public MaskTools::Filter
{
   // re-use repeated luts, like in lutxyz
   struct Lut {
     bool used;
     Byte *ptr;
   };

   Lut luts[4+1];
   
   static Byte *calculateLut(const std::deque<Filtering::Parser::Symbol> &expr, int bits_per_pixel, String scale_inputs, bool clamp_float) {
     Parser::Context ctx(expr, scale_inputs, clamp_float);
     int pixelsize = bits_per_pixel == 8 ? 1 : 2; // byte / uint16_t
     size_t buffer_size = ((size_t)1 << bits_per_pixel) * ((size_t)1 << bits_per_pixel) *pixelsize;
     Byte *lut = new Byte[buffer_size];

     switch (bits_per_pixel) {
     case 8:
       for (int x = 0; x < 256; x++)
         for (int y = 0; y < 256; y++)
           lut[(x << 8) + y] = ctx.compute_byte_xy(x, y);
       break;
     case 10:
       for (int x = 0; x < 1024; x++)
         for (int y = 0; y < 1024; y++)
           reinterpret_cast<Word *>(lut)[(x << 10) + y] = ctx.compute_word_xy<10>(x, y);
       break;
     case 12:
       for (int x = 0; x < 4096; x++)
         for (int y = 0; y < 4096; y++)
           reinterpret_cast<Word *>(lut)[(x << 12) + y] = ctx.compute_word_xy<12>(x, y);
       break;
     case 14:
       for (int x = 0; x < 16384; x++)
         for (int y = 0; y < 16384; y++)
           reinterpret_cast<Word *>(lut)[(x << 14) + y] = ctx.compute_word_xy<14>(x, y);
       break;
     case 16:
       // 64bit only
       for (int x = 0; x < 65536; x++)
         for (int y = 0; y < 65536; y++)
           reinterpret_cast<Word *>(lut)[((size_t)x << 16) + y] = ctx.compute_word_xy<16>(x, y);
       break;
     }
     return lut;
   }

   // for realtime
   std::deque<Filtering::Parser::Symbol> *parsed_expressions[4];

   Processor *processor;
   Processor16 *processor16;
   ProcessorCtx *processorCtx;
   ProcessorCtx *processorCtx16;
   ProcessorCtx32 *processorCtx32;
   int bits_per_pixel;
   bool realtime;
   String scale_inputs;
   bool clamp_float;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[4], const Constraint constraints[4]) override
    {
        UNUSED(n);
        UNUSED(constraints);
        if (realtime) {
          // thread safety
          Parser::Context ctx(*parsed_expressions[nPlane], scale_inputs, clamp_float);
          if (bits_per_pixel <= 16)
            processorCtx(dst.data(), dst.pitch(), frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), dst.width(), dst.height(), ctx);
          else {
            const bool chroma = ((nPlane == 1 || nPlane == 2) && !planes_isRGB[C]);
            processorCtx32(dst.data(), dst.pitch(), frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), dst.width(), dst.height(), chroma, ctx); // extra parameter
          }
        }
        else if (bits_per_pixel == 8)
          processor(dst.data(), dst.pitch(), frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), dst.width(), dst.height(), luts[nPlane].ptr);
        else if (bits_per_pixel <= 16)
          processor16(dst.data(), dst.pitch(), frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), dst.width(), dst.height(), (Word *)luts[nPlane].ptr);
    }

public:
   Lutxy(const Parameters &parameters, CpuFlags cpuFlags) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE, (CpuFlags)cpuFlags)
   {
      for (int i = 0; i < 4; i++) {
        parsed_expressions[i] = nullptr;
      }

      for (int i = 0; i < 4+1; ++i) {
        luts[i].used = false;
        luts[i].ptr = nullptr;
      }

      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr", "aExpr" };

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y);

      bits_per_pixel = bit_depths[C];
      realtime = parameters["realtime"].toBool();
      scale_inputs = parameters["scale_inputs"].toString();
      clamp_float = parameters["clamp_float"].toBool();

      // 14, 16 bit and float: default realtime, 
      // hardcore users on x64 with 8GB memory and plenty of time can override 16 bit lutxy with realtime=false :)
      // lut sizes
      // 10 bits: 2 MBytes (2*1024*1024) per expression 
      // 12 bits: 32 MBytes (2*4096*4096) per expression 
      // 14 bits: 512 MBytes (2*16384*16384) per expression 
      // 16 bits: 8 GBytes (2*65536*65536) per expression 
      if (bits_per_pixel >= 14 && !parameters["realtime"].is_defined()) {
        realtime = true;
      }

      if (bits_per_pixel == 32)
        realtime = true;

      if (bits_per_pixel == 16) {
        if ((uint64_t)std::numeric_limits<size_t>::max() <= 0xFFFFFFFFull && bits_per_pixel == 16) {
          realtime = true; // not even possible a real 16 bit lutxy on 32 bit environment
        }
      }

      /* compute the luts */
      for ( int i = 0; i < 4; i++ )
      {
          if (operators[i] != PROCESS) {
              continue;
          }

          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              operators[i] = NONE; //inplace
              continue;
          }

          bool customExpressionDefined = false;
          if (parameters[expr_strs[i]].is_defined()) {
            parser.parse(parameters[expr_strs[i]].toString(), " ");
            customExpressionDefined = true;
          } else
            parser.parse(parameters["expr"].toString(), " ");

          // for check:
          Parser::Context ctx(parser.getExpression(), scale_inputs, clamp_float);

          if (!ctx.check())
          {
            error = "invalid expression in the lut";
            return;
          }

          if (realtime) {
            parsed_expressions[i] = new std::deque<Parser::Symbol>(parser.getExpression());

            switch (bits_per_pixel) {
            case 8: processorCtx = realtime8_c; break;
            case 10: processorCtx = realtime10_c; break;
            case 12: processorCtx = realtime12_c; break;
            case 14: processorCtx = realtime14_c; break;
            case 16: processorCtx = realtime16_c; break;
            case 32: processorCtx32 = realtime32_c; break;
            }
            continue;
          }

          // pure lut, no realtime

          // save memory, reuse luts, like in xyz
          if (customExpressionDefined) {
            luts[i].used = true;
            luts[i].ptr = calculateLut(parser.getExpression(), bits_per_pixel, scale_inputs, clamp_float);
          }
          else {
            if (luts[4].ptr == nullptr) {
              luts[4].used = true;
              luts[4].ptr = calculateLut(parser.getExpression(), bits_per_pixel, scale_inputs, clamp_float);
            }
            luts[i].ptr = luts[4].ptr;
          }

          switch (bits_per_pixel) {
          case 8:
            processor = lut_c;
            break;
          case 10:
            processor16 = lut10_c;
            break;
          case 12:
            processor16 = lut12_c;
            break;
          case 14:
            processor16 = lut14_c;
            break;
          case 16:
            // 64bit only
            processor16 = lut16_c;
            break;
          }
      }
   }

   ~Lutxy()
   {
     for (int i = 0; i < 4+1; ++i) {
       if (luts[i].used) {
         delete[] luts[i].ptr;
       }
     }
     for (int i = 0; i < 4; i++) {
       delete parsed_expressions[i];
     }
   }

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lutxy";

      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(String("x"), "expr", false));
      signature.add(Parameter(String("x"), "yExpr", false));
      signature.add(Parameter(String("x"), "uExpr", false));
      signature.add(Parameter(String("x"), "vExpr", false));

      add_defaults( signature );

      signature.add(Parameter(false, "realtime", false));
      signature.add(Parameter(String("x"), "aExpr", false));
      signature.add(Parameter(String("none"), "scale_inputs", false));
      signature.add(Parameter(false, "clamp_float", false));
      return signature;
   }
};

} } } } }

#endif