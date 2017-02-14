#ifndef __Mt_Edgemask_H__
#define __Mt_Edgemask_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Edge {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight);
typedef void(Processor16)(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight);
typedef void(Processor32)(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, const Float matrix[10], Float nLowThreshold, Float nHighThreshold, int nWidth, int nHeight);

extern Processor *convolution_c;
extern Processor *convolution_sse2;

extern Processor *sobel_c;
extern Processor *sobel_sse2;
extern Processor *sobel_ssse3;

extern Processor *roberts_c;
extern Processor *roberts_sse2;
extern Processor *roberts_ssse3;

extern Processor *laplace_c;
extern Processor *laplace_sse2;
extern Processor *laplace_ssse3;

extern Processor *cartoon_c;
extern Processor *cartoon_sse2;

extern Processor *half_prewitt_c;
extern Processor *half_prewitt_sse2;
extern Processor *half_prewitt_ssse3;

extern Processor *prewitt_c;
extern Processor *prewitt_sse2;
extern Processor *prewitt_ssse3;

extern Processor *morpho_c;
extern Processor *morpho_sse2;

// uint16_t
#define DEFINE_EXTERNS(name) \
extern Processor16 *name##_10_c; \
extern Processor16 *name##_12_c; \
extern Processor16 *name##_14_c; \
extern Processor16 *name##_16_c;

DEFINE_EXTERNS(convolution)
DEFINE_EXTERNS(sobel)
DEFINE_EXTERNS(roberts)
DEFINE_EXTERNS(laplace)
DEFINE_EXTERNS(cartoon)
DEFINE_EXTERNS(half_prewitt)
DEFINE_EXTERNS(prewitt)
DEFINE_EXTERNS(morpho)

#undef DEFINE_EXTERNS

// float
extern Processor32 *convolution_32_c;
extern Processor32 *sobel_32_c;
extern Processor32 *roberts_32_c;
extern Processor32 *laplace_32_c;
extern Processor32 *cartoon_32_c;
extern Processor32 *half_prewitt_32_c;
extern Processor32 *prewitt_32_c;
extern Processor32 *morpho_32_c;


class EdgeMask : public MaskTools::Filter
{
   int nLowThresholds[3];
   int nHighThresholds[3];

   float nLowThresholds_f[3];
   float nHighThresholds_f[3];

   Short matrix[10];
   Float matrix_f[10];

   ProcessorList<Processor> processors;
   ProcessorList<Processor16> processors16;
   ProcessorList<Processor32> processors32;

protected:

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        if (bit_depths[C] == 8) {
          processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            matrix, nLowThresholds[nPlane], nHighThresholds[nPlane], dst.width(), dst.height());
        }
        else if(bit_depths[C] <= 16) {
          processors16.best_processor(constraints[nPlane])((Word *)dst.data(), dst.pitch(),
            (Word *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            matrix, nLowThresholds[nPlane], nHighThresholds[nPlane], dst.width(), dst.height());
        }
        else { // float
          processors32.best_processor(constraints[nPlane])((Float *)dst.data(), dst.pitch(),
            (Float *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            matrix_f, nLowThresholds_f[nPlane], nHighThresholds_f[nPlane], dst.width(), dst.height());
        }
    }

public:
   EdgeMask(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::CHILD )
   {
     int bits_per_pixel = bit_depths[C];
     bool isFloat = bits_per_pixel == 32;

     if (!isFloat) {
       // default value of 10 scaled by bit depth
       int nLow0, nLow1;
       int nHigh0, nHigh1;
       int max_pixel_value = (1 << bits_per_pixel) - 1;
       nLow0 = parameters["thY1"].is_defined() ? parameters["thY1"].toInt() : (10 << (bits_per_pixel - 8));
       nLow1 = parameters["thC1"].is_defined() ? parameters["thC1"].toInt() : (10 << (bits_per_pixel - 8));
       nHigh0 = parameters["thY2"].is_defined() ? parameters["thY2"].toInt() : (10 << (bits_per_pixel - 8));
       nHigh1 = parameters["thC2"].is_defined() ? parameters["thC2"].toInt() : (10 << (bits_per_pixel - 8));

       nLowThresholds[0] = min(max(nLow0, 0), max_pixel_value);
       nLowThresholds[1] = nLowThresholds[2] = min(max(nLow1, 0), max_pixel_value);
       nHighThresholds[0] = min(max(nHigh0, 0), max_pixel_value);
       nHighThresholds[1] = nHighThresholds[2] = min(max(nHigh1, 0), max_pixel_value);
     }
     else {
       float nLow0, nLow1;
       float nHigh0, nHigh1;
       nLow0 = parameters["thY1"].is_defined() ? (float)parameters["thY1"].toFloat() : (10.0f / 255);
       nLow1 = parameters["thC1"].is_defined() ? (float)parameters["thC1"].toFloat() : (10.0f / 255);
       nHigh0 = parameters["thY2"].is_defined() ? (float)parameters["thY2"].toFloat() : (10.0f / 255);
       nHigh1 = parameters["thC2"].is_defined() ? (float)parameters["thC2"].toFloat() : (10.0f / 255);

       nLowThresholds_f[0] = nLow0;
       nLowThresholds_f[1] = nLowThresholds_f[2] = nLow1;
       nHighThresholds_f[0] = nHigh0;
       nHighThresholds_f[1] = nHighThresholds_f[2] = nHigh1;
     }

      /* add the processors */
      if ( parameters["mode"].toString() == "sobel" )
      {
         print(LOG_DEBUG, "Edge : using sobel detector");
         switch (bits_per_pixel) {
         case 8:
           processors.push_back(Filtering::Processor<Processor>(sobel_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           processors.push_back(Filtering::Processor<Processor>(sobel_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
           processors.push_back(Filtering::Processor<Processor>(sobel_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
           break;
         case 10: 
           processors16.push_back(Filtering::Processor<Processor16>(sobel_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 12:
           processors16.push_back(Filtering::Processor<Processor16>(sobel_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 14:
           processors16.push_back(Filtering::Processor<Processor16>(sobel_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 16:
           processors16.push_back(Filtering::Processor<Processor16>(sobel_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 32: 
           processors32.push_back(Filtering::Processor<Processor32>(sobel_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         }
      }
      else if ( parameters["mode"].toString() == "roberts" )
      {
         print(LOG_DEBUG, "Edge : using roberts detector");
         switch(bits_per_pixel) {
         case 8: 
           processors.push_back(Filtering::Processor<Processor>(roberts_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           processors.push_back(Filtering::Processor<Processor>(roberts_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
           processors.push_back(Filtering::Processor<Processor>(roberts_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
           break;
         case 10:
           processors16.push_back(Filtering::Processor<Processor16>(roberts_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 12:
           processors16.push_back(Filtering::Processor<Processor16>(roberts_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 14:
           processors16.push_back(Filtering::Processor<Processor16>(roberts_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 16:
           processors16.push_back(Filtering::Processor<Processor16>(roberts_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 32: 
           processors32.push_back(Filtering::Processor<Processor32>(roberts_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         }
      }
      else if (parameters["mode"].toString() == "laplace")
      {
        print(LOG_DEBUG, "Edge : using laplace detector");
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(laplace_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(laplace_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
          processors.push_back(Filtering::Processor<Processor>(laplace_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
          break;
        case 10: 
          processors16.push_back(Filtering::Processor<Processor16>(laplace_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(laplace_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(laplace_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(laplace_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32: 
          processors32.push_back(Filtering::Processor<Processor32>(laplace_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else if (parameters["mode"].toString() == "cartoon")
      {
        print(LOG_DEBUG, "Edge : using cartoon detector");
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(cartoon_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(cartoon_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
          break;
        case 10: 
          processors16.push_back(Filtering::Processor<Processor16>(cartoon_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(cartoon_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(cartoon_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(cartoon_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32: 
          processors32.push_back(Filtering::Processor<Processor32>(cartoon_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else if (parameters["mode"].toString() == "min/max")
      {
        print(LOG_DEBUG, "Edge : using morphologic detector");
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(morpho_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(morpho_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
          break;
        case 10: 
          processors16.push_back(Filtering::Processor<Processor16>(morpho_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(morpho_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(morpho_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(morpho_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32: 
          processors32.push_back(Filtering::Processor<Processor32>(morpho_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else if (parameters["mode"].toString() == "prewitt")
      {
        print(LOG_DEBUG, "Edge : using prewitt detector");
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(prewitt_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(prewitt_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
          processors.push_back(Filtering::Processor<Processor>(prewitt_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
          break;
        case 10: 
          processors16.push_back(Filtering::Processor<Processor16>(prewitt_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(prewitt_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(prewitt_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(prewitt_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32: 
          processors32.push_back(Filtering::Processor<Processor32>(prewitt_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else if (parameters["mode"].toString() == "hprewitt")
      {
        print(LOG_DEBUG, "Edge : using hprewitt detector");
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(half_prewitt_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(half_prewitt_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
          processors.push_back(Filtering::Processor<Processor>(half_prewitt_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
          break;
        case 10: 
          processors16.push_back(Filtering::Processor<Processor16>(half_prewitt_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(half_prewitt_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(half_prewitt_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(half_prewitt_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32:
          processors32.push_back(Filtering::Processor<Processor32>(half_prewitt_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else
      {
         print(LOG_DEBUG, "Edge : using custom detector");
         auto coeffs = Parser::getDefaultParser().parse(parameters["mode"].toString(), " ").getExpression();
         bool isAsmOk = true;
         if (isFloat) {
           memset(matrix_f, 0, sizeof(matrix_f));
           Float nNegative = 0, nPositive = 0;
           for (int i = 0; i < 9; i++)
           {
             Float coefficient;

             if (!coeffs.size())
             {
               error = "invalid mode";
               return;
             }

             print(LOG_DEBUG, "%i\n", coeffs.size());
             matrix_f[i] = coefficient = static_cast<Float>(coeffs.front().getValue(0, 0, 0));

             nNegative = nNegative + (coefficient < 0 ? -coefficient : 0);
             nPositive = nPositive + (coefficient > 0 ? +coefficient : 0);
             coeffs.pop_front();
           }
           Float nSum = max<Float>(nNegative, nPositive);

           if (coeffs.size())
             nSum = static_cast<Float>(coeffs.front().getValue(0, 0, 0));

           /* disable asm if sum of coefficients indicates a risk of overflow */
#if 0
           // float: no overflow
           if (nNegative > 128 || nPositive > 128)
             isAsmOk = false;
#endif

           /* find the upper power of 2, if possible */
           int i;
           for (i = 0; i < 15; i++)
           {
             if ((1 << i) >= nSum)
             {
               if (!coeffs.size())
                 nSum = (Float)(1 << i);
               break;
             }
           }

           matrix_f[9] = static_cast<Float>(nSum);

           if (i >= 15)
             error = "Too high divisor, please specify a divisor between 1 and 32767";
           if (1 << i != matrix_f[9])
             isAsmOk = false;
           if (matrix_f[9] <= 0)
             error = "Divisor must be positive";
         }
         else { // integer matrix
           memset(matrix, 0, sizeof(matrix));
           int nNegative = 0, nPositive = 0;
           for (int i = 0; i < 9; i++)
           {
             Short coefficient;

             if (!coeffs.size())
             {
               error = "invalid mode";
               return;
             }

             print(LOG_DEBUG, "%i\n", coeffs.size());
             matrix[i] = coefficient = static_cast<Short>(coeffs.front().getValue(0, 0, 0));

             nNegative += coefficient < 0 ? -coefficient : 0;
             nPositive += coefficient > 0 ? +coefficient : 0;
             coeffs.pop_front();
           }
           int nSum = max<int>(nNegative, nPositive);

           if (coeffs.size())
             nSum = static_cast<Short>(coeffs.front().getValue(0, 0, 0));

           /* disable asm if sum of coefficients indicates a risk of overflow */
           if (nNegative > 128 || nPositive > 128)
             isAsmOk = false;

           /* find the upper power of 2, if possible */
           int i;
           for (i = 0; i < 15; i++)
           {
             if ((1 << i) >= nSum)
             {
               if (!coeffs.size())
                 nSum = 1 << i;
               break;
             }
           }

           matrix[9] = static_cast<Short>(nSum);

           if (i >= 15)
             error = "Too high divisor, please specify a divisor between 1 and 32767";
           if (1 << i != matrix[9])
             isAsmOk = false;
           if (matrix[9] <= 0)
             error = "Divisor must be positive";
         }

         print(LOG_DEBUG, "Edge : %3i %3i %3i\n"
                        "       %3i %3i %3i\n"
                        "       %3i %3i %3i\n"
                        "normalize: %3i", matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9]);
         switch(bits_per_pixel) {
         case 8: 
           processors.push_back(Filtering::Processor<Processor>(convolution_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           if (isAsmOk)
           {
             processors.push_back(Filtering::Processor<Processor>(convolution_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 2));
           }
           break;
         case 10: 
           processors16.push_back(Filtering::Processor<Processor16>(convolution_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 12:
           processors16.push_back(Filtering::Processor<Processor16>(convolution_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 14:
           processors16.push_back(Filtering::Processor<Processor16>(convolution_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 16:
           processors16.push_back(Filtering::Processor<Processor16>(convolution_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         case 32:
           processors32.push_back(Filtering::Processor<Processor32>(convolution_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           break;
         }
      }
   }

   InputConfiguration &input_configuration() const { return OneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_edge"; // common for any bit depths

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("sobel"), "mode"));
      signature.add(Parameter(TYPE_FLOAT, "thY1")); // default 10 must be scaled + prepare float support
      signature.add(Parameter(TYPE_FLOAT, "thY2"));
      signature.add(Parameter(TYPE_FLOAT, "thC1"));
      signature.add(Parameter(TYPE_FLOAT, "thC2"));

      return add_defaults( signature );
   }

};

} } } } }

#endif