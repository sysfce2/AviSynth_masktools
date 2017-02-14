#ifndef __Mt_Average16_H__
#define __Mt_Average16_H__
#if 0

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace Average16 {

typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, int nOrigHeight);

Processor16 average16_stacked_c;
Processor16 average16_stacked_sse2;

Processor16 average16_native_c;
Processor16 average16_native_sse2;

class Average16 : public MaskTools::Filter
{

   ProcessorList<Processor16> processors16;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
   {
      UNUSED(n);
      processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(), frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), dst.width(), dst.height(), dst.origheight());
   }

public:
    Average16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
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
      
        if (isStacked) {
            /* add the processors */
            processors16.push_back(Filtering::Processor<Processor16>(average16_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors16.push_back(Filtering::Processor<Processor16>(average16_stacked_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        } else {
          processors16.push_back(Filtering::Processor<Processor16>(average16_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors16.push_back(Filtering::Processor<Processor16>(average16_native_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        }
    }

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_average16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }

};

} } } } }

#endif
#endif