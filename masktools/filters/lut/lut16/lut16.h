#ifndef __Mt_Lut16_H__
#define __Mt_Lut16_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Single16bit {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word lut[65536], int nOrigHeightForStacked);

Processor lut16_c;
Processor lut16_c_stacked;

class Lut16 : public MaskTools::Filter
{
   Word luts[3][65536]; // full size, even for 10 bits (avoid over addressing by invalid pixel values)
   Processor* processor;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        UNUSED(constraints);
        UNUSED(frames);
        processor(dst.data(), dst.pitch(), dst.width(), dst.height(), luts[nPlane], dst.origheight());
    }

public:
   Lut16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr" };

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X);

      bool isStacked = parameters["stacked"].toBool();
      int bits_per_pixel = bit_depths[C];
      int max_pixel_value = (1 << bits_per_pixel) - 1;

      if (isStacked && bits_per_pixel != 8) {
        error = "Stacked specified for a non-8 bit clip";
        return;
      }
      if (!isStacked && bits_per_pixel == 8) {
        error = "8 bit clip needs stacked=true";
        return;
      }
      if (bits_per_pixel == 32) {
        error = "32 bit float clip is not supported yet";
        return;
      }

      /* compute the luts */
      for ( int i = 0; i < 3; i++ )
      {
          if (operators[i] != PROCESS) {
              continue;
          }

          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              operators[i] = NONE; //inplace
              continue;
          }

          if ( parameters[expr_strs[i]].is_defined() ) 
              parser.parse(parameters[expr_strs[i]].toString(), " ");
          else
              parser.parse(parameters["expr"].toString(), " ");

          Parser::Context ctx(parser.getExpression());

          if ( !ctx.check() )
          {
              error = "invalid expression in the lut";
              return;
          }

          if (bits_per_pixel == 16 || bits_per_pixel == 8) { // real or stacked 16 bit
            for (int x = 0; x <= 65535; x++)
              luts[i][x] = ctx.compute_word(x, 0.0f);
          }
          else {
            for (int x = 0; x <= max_pixel_value; x++)
              luts[i][x] = min(ctx.compute_word(x, 0.0f),Word(max_pixel_value)); // clamp to 65535 is not enough for 10-16 bit
            for (int x = max_pixel_value; x < 65536; x++)
              luts[i][x] = Word(max_pixel_value);
          }
      }

      if (isStacked) {
          processor = lut16_c_stacked;
      } else {
          processor = lut16_c;
      }
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lut16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("x"), "expr"));
      signature.add(Parameter(String("x"), "yExpr"));
      signature.add(Parameter(String("x"), "uExpr"));
      signature.add(Parameter(String("x"), "vExpr"));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }
};


} } } } }

#endif