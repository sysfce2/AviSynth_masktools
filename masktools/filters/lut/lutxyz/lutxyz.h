#ifndef __Mt_Lutxyz_H__
#define __Mt_Lutxyz_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Trial {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, const Byte *lut);
typedef void(ProcessorCtx)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, Parser::Context &ctx);
typedef void(ProcessorCtx32)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, bool chroma, Parser::Context &ctx);

Processor lut_c;

ProcessorCtx realtime8_c;
extern ProcessorCtx *realtime10_c;
extern ProcessorCtx *realtime12_c;
extern ProcessorCtx *realtime14_c;
extern ProcessorCtx *realtime16_c;
ProcessorCtx32 realtime32_c;

class Lutxyz : public MaskTools::Filter
{
    struct Lut {
        bool used;
        Byte *ptr;
    };

   Lut luts[4+1];

   static Byte *calculateLut(const std::deque<Filtering::Parser::Symbol> &expr, String scale_inputs, bool clamp_float) {
       Parser::Context ctx(expr, scale_inputs, clamp_float);
       Byte *lut = new Byte[256 * 256 * 256];

       for ( int x = 0; x < 256; x++ ) {
           for ( int y = 0; y < 256; y++ ) {
               for ( int z = 0; z < 256; z++ ) {
                   lut[(x<<16)+(y<<8)+z] = ctx.compute_byte_xyz(x, y, z); 
               }
           }
       }
       return lut;
   }

   // for realtime
   std::deque<Filtering::Parser::Symbol> *parsed_expressions[4];

   ProcessorCtx *processorCtx;
   ProcessorCtx *processorCtx16;
   ProcessorCtx32 *processorCtx32;
   int bits_per_pixel;
   bool realtime;
   String scale_inputs;
   bool clamp_float;
   int use_expr;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[4], const Constraint constraints[4]) override
    {
        UNUSED(n);
        UNUSED(constraints);
        if (realtime) {
          // thread safety
          Parser::Context ctx(*parsed_expressions[nPlane], scale_inputs, clamp_float);

          if (bits_per_pixel <= 16)
            processorCtx(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
              dst.width(), dst.height(), ctx);
          else {
            const bool chroma = ((nPlane == 1 || nPlane == 2) && !planes_isRGB[C]);
            processorCtx32(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
              dst.width(), dst.height(), chroma, ctx);
          }

        }
        else if (bits_per_pixel == 8) {
          lut_c(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            dst.width(), dst.height(), luts[nPlane].ptr);
        }
    }

public:
   Lutxyz(const Parameters &parameters, CpuFlags cpuFlags) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE, (CpuFlags)cpuFlags)
   {
      for (int i = 0; i < 4; i++) {
        parsed_expressions[i] = nullptr;
      }

      for (int i = 0; i < 4+1; ++i) {
          luts[i].used = false;
          luts[i].ptr = nullptr;
      }

      bits_per_pixel = bit_depths[C];
      realtime = parameters["realtime"].toBool();
      scale_inputs = parameters["scale_inputs"].toString();
      if (!checkValidScaleInputs(scale_inputs, error))
        return; // error message filled
      clamp_float = parameters["clamp_float"].toBool();

      if (bits_per_pixel > 8)
        realtime = true;

      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr", "aExpr" };

      // 2.2.15- decide on using external Expr filter
      // This part is duplicated in lut, lutxy, lutxyz, lutxyza
      use_expr = parameters["use_expr"].toInt();
      bool use_external_expr = false;
      if (use_expr < 0 || use_expr>2) {
        error = "invalid value for parameter 'use_expr'";
        return;
      }
      if (use_expr == 1 && bits_per_pixel > 8) // mode 1: use Expr when over 8 bits or lutxyza
        use_external_expr = true;
      if (use_expr == 2 && realtime) // mode 2: use Expr when masktools would use its own slow calculation
        use_external_expr = true;
      if (use_external_expr) {
        if (nXOffset != 0 || nYOffset != 0) {
          error = "X and Y offset must be zero when using 'use_expr'";
          return;
        }
        if (nWidth != nCoreWidth || nHeight != nCoreHeight) {
          error = "Cannot change original width or height when using 'use_expr'";
          return;
        }

        expr_clamp_float = clamp_float;
        // expr_need_process[4] and expr_list[4] is defined in filter level
        expr_scale_inputs = scale_inputs; // copy parameter
                                          // we pass whole frame for Expr, set expression for all planes
        for (int i = 0; i < 4; i++)
        {
          expr_need_process[i] = true;
          if (operators[i] == COPY) {
            expr_list[i] = "x"; // Expr will optimize it to 'copy plane'
          }
          else if (operators[i] == COPY_SECOND) {
            expr_list[i] = "y";
          }
          else if (operators[i] == COPY_THIRD) {
            expr_list[i] = "z";
          }
          else if (operators[i] == COPY_FOURTH) {
            expr_list[i] = "a";
          }
          else if (operators[i] == MEMSET) {
            expr_list[i] = std::to_string(operators[i].value_f()); // Expr will optimize it to 'fill plane'
          }
          else if (operators[i] == PROCESS) {
            // no expression for spec y/u/v/a plane, neither have we the jolly joker "expr"
            if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              expr_list[i] = "x"; // in Expr: no such thing as 'no process', copy 1st clip
              continue;
            }
            else if (parameters[expr_strs[i]].is_defined()) {
              expr_list[i] = parameters[expr_strs[i]].toString();
            }
            else {
              expr_list[i] = parameters["expr"].toString();
            }
          }
          else {
            expr_need_process[i] = false;
          }
        } // planes
      }
      else {

        Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y).addSymbol(Parser::Symbol::Z);

        /* compute the luts */
        for (int i = 0; i < 4; i++)
        {
          if (operators[i] != PROCESS) {
            continue;
          }

          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
            operators[i] = COPY_SECOND; //inplace
            continue;
          }

          bool customExpressionDefined = false;
          if (parameters[expr_strs[i]].is_defined()) {
            parser.parse(parameters[expr_strs[i]].toString(), " ");
            customExpressionDefined = true;
          }
          else
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

          if (customExpressionDefined) {
            luts[i].used = true;
            luts[i].ptr = calculateLut(parser.getExpression(), scale_inputs, clamp_float); // 8 bit always
          }
          else {
            if (luts[4].ptr == nullptr) {
              luts[4].used = true;
              luts[4].ptr = calculateLut(parser.getExpression(), scale_inputs, clamp_float);
            }
            luts[i].ptr = luts[4].ptr;
          }
        }
      }
   }

   ~Lutxyz()
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

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lutxyz";

      signature.add(Parameter(TYPE_CLIP, "", false));
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
      signature.add(Parameter(0, "use_expr", false));
      return signature;
   }
};

} } } } }

#endif