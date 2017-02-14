#ifndef __Mt_Clamp16_H__
#define __Mt_Clamp16_H__
#if 0
#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace Clamp16 {

typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pUpLimit, ptrdiff_t nUpLimitPitch, const Byte *pLowLimit, ptrdiff_t nLowLimitPitch, int nWidth, int nHeight, int nOrigHeight, int nOvershoot, int nUndershoot);

Processor16 clamp16_stacked_c;
extern Processor16 *clamp16_stacked_sse2;
extern Processor16 *clamp16_stacked_sse4_1;

extern Processor16 *clamp16_native_10_c;
extern Processor16 *clamp16_native_12_c;
extern Processor16 *clamp16_native_14_c;
extern Processor16 *clamp16_native_16_c;

extern Processor16 *clamp16_native_10_sse2;
extern Processor16 *clamp16_native_12_sse2;
extern Processor16 *clamp16_native_14_sse2;
extern Processor16 *clamp16_native_16_sse2;

extern Processor16 *clamp16_native_10_sse4_1;
extern Processor16 *clamp16_native_12_sse4_1;
extern Processor16 *clamp16_native_14_sse4_1;
extern Processor16 *clamp16_native_16_sse4_1;

class Clamp16 : public MaskTools::Filter
{

   int nOvershoot, nUndershoot;
   ProcessorList<Processor16> processors16;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
   {
      UNUSED(n);
      processors16.best_processor( constraints[nPlane] )( dst.data(), dst.pitch(), 
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), 
          frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(), 
          dst.width(), dst.height(), dst.origheight(), nOvershoot, nUndershoot );
   }

public:
    Clamp16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
    {
      bool isStacked = parameters["stacked"].toBool();
      int bits_per_pixel = bit_depths[C];

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
      
        nUndershoot = parameters["undershoot"].toInt();
        nOvershoot = parameters["overshoot"].toInt();

        /* add the processors */
        if (isStacked) {
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_stacked_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_stacked_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 2));
        } else {
          switch (bit_depths[C]) {
          case 10:
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_10_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_10_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 2));
            break;
          case 12:
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_12_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_12_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 2));
            break;
          case 14:
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_14_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_14_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 2));
            break;
          case 16:
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_16_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            processors16.push_back(Filtering::Processor<Processor16>(clamp16_native_16_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 2));
            break;
          }
        }
    }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_clamp16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(0, "overshoot"));
      signature.add(Parameter(0, "undershoot"));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }

};

} } } } }

#endif
#endif