#ifndef __Mt_Logic16_H__
#define __Mt_Logic16_H__
#if 0
#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Logic16 {

typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, int nOrigHeight, Word nThresholdDestination, Word nThresholdSource);

#define DEFINE_PROCESSOR(name) \
   extern Processor16 *name##_stacked_c; \
   extern Processor16 *name##_native_c; \
   extern Processor16 *name##_stacked_sse2; \
   extern Processor16 *name##_native_sse2;

DEFINE_PROCESSOR(and);
DEFINE_PROCESSOR(or);
DEFINE_PROCESSOR(andn);
DEFINE_PROCESSOR(xor);

#define DEFINE_TRIPLE(mode) \
DEFINE_PROCESSOR(mode); \
DEFINE_PROCESSOR(mode##add); \
DEFINE_PROCESSOR(mode##sub)

#define DEFINE_NINE(mode) \
DEFINE_TRIPLE(mode); \
DEFINE_TRIPLE(add##mode); \
DEFINE_TRIPLE(sub##mode)

DEFINE_NINE(min);
DEFINE_NINE(max);

#undef DEFINE_TRIPLE
#undef DEFINE_NINE
#undef DEFINE_PROCESSOR

class Logic16 : public MaskTools::Filter
{

   ProcessorList<Processor16> processors16;
   int nThresholdDestination, nThresholdSource;

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[3], const Constraint constraints[3]) override
   {
       UNUSED(n);
       processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
           frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
           dst.width(), dst.height(), dst.origheight(), nThresholdDestination, nThresholdSource);
   }

public:
    Logic16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
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

#define SET_MODE(mode) \
    if (isStacked) { \
        processors16.push_back( Filtering::Processor<Processor16>( mode##_stacked_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
        processors16.push_back( Filtering::Processor<Processor16>( mode##_stacked_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
    } else { \
        processors16.push_back( Filtering::Processor<Processor16>( mode##_native_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
        processors16.push_back( Filtering::Processor<Processor16>( mode##_native_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
    }
        int nTh1 = parameters["th1"].toInt();
        int nTh2 = parameters["th2"].toInt();

        if (parameters["mode"].toString() == "and") {
            SET_MODE(and);
        } else if (parameters["mode"].toString() == "or") {
            SET_MODE(or);
        } else if (parameters["mode"].toString() == "xor") {
            SET_MODE(xor);
        } else if (parameters["mode"].toString() == "andn") {
            SET_MODE(andn);
        } else {
            bool isDstSub = nTh1 < 0;
            bool isDstAdd = nTh1 > 0;
            bool isSrcSub = nTh2 < 0;
            bool isSrcAdd = nTh2 > 0;

            nThresholdDestination = convert<Word, int>(abs<int>(nTh1)); // todo bits_per_pixel
            nThresholdSource = convert<Word, int>(abs<int>(nTh2));

            if (parameters["mode"].toString() == "min")
            {
                if (isDstAdd && isSrcAdd) { SET_MODE(addminadd); }
                else if (isDstAdd && isSrcSub) { SET_MODE(addminsub); }
                else if (isDstSub && isSrcAdd) { SET_MODE(subminadd); }
                else if (isDstSub && isSrcSub) { SET_MODE(subminsub); }
                else if (isDstAdd) { SET_MODE(addmin); }
                else if (isSrcAdd) { SET_MODE(minadd); }
                else if (isDstSub) { SET_MODE(submin); }
                else if (isSrcSub) { SET_MODE(minsub); }
                else { SET_MODE(min); }
            }
            else if (parameters["mode"].toString() == "max")
            {
                if (isDstAdd && isSrcAdd) { SET_MODE(addmaxadd); }
                else if (isDstAdd && isSrcSub) { SET_MODE(addmaxsub); }
                else if (isDstSub && isSrcAdd) { SET_MODE(submaxadd); }
                else if (isDstSub && isSrcSub) { SET_MODE(submaxsub); }
                else if (isDstAdd) { SET_MODE(addmax); }
                else if (isSrcAdd) { SET_MODE(maxadd); }
                else if (isDstSub) { SET_MODE(submax); }
                else if (isSrcSub) { SET_MODE(maxsub); }
                else { SET_MODE(max); }
            }
            else
                error = "\"mode\" must be either \"and\", \"or\", \"xor\", \"andn\", \"min\" or \"max\"";
        }
    }

#undef SET_MODE

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_logic16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String(""), "mode"));
      signature.add(Parameter(0, "th1"));
      signature.add(Parameter(0, "th2"));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }

};


} } } }
#endif
#endif