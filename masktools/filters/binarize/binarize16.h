#ifndef __Mt_Binarize16_H__
#define __Mt_Binarize16_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize16 {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, Word nThreshold, int nWidth, int nHeight, int nOrigHeight);

#define DEFINE_PROCESSOR(name) \
extern Processor *binarize_##name##_stacked_16_c; \
extern Processor *binarize_##name##_stacked_16_sse2; \
extern Processor *binarize_##name##_native_10_c; \
extern Processor *binarize_##name##_native_10_sse2; \
extern Processor *binarize_##name##_native_12_c; \
extern Processor *binarize_##name##_native_12_sse2; \
extern Processor *binarize_##name##_native_14_c; \
extern Processor *binarize_##name##_native_14_sse2; \
extern Processor *binarize_##name##_native_16_c; \
extern Processor *binarize_##name##_native_16_sse2;

DEFINE_PROCESSOR(upper);
DEFINE_PROCESSOR(lower);
DEFINE_PROCESSOR(0_x);
DEFINE_PROCESSOR(t_x);
DEFINE_PROCESSOR(x_0);
DEFINE_PROCESSOR(x_t);
DEFINE_PROCESSOR(t_0);
DEFINE_PROCESSOR(0_t);
DEFINE_PROCESSOR(x_255);
DEFINE_PROCESSOR(t_255);
DEFINE_PROCESSOR(255_x);
DEFINE_PROCESSOR(255_t);

#undef DEFINE_PROCESSOR

class Binarize16 : public MaskTools::Filter
{
    Word nThreshold;
    ProcessorList<Processor> processors;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        UNUSED(frames);
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(), nThreshold, dst.width(), dst.height(), dst.origheight());
    }

    bool isMode(const char *mode) {
        return parameters["mode"].is_defined() && parameters["mode"].toString() == mode;
    }

public:
  Binarize16(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::INPLACE)
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

    if (parameters["threshold"].is_defined()) {
      nThreshold = convert<Word, int>(parameters["threshold"].toInt());
    }
    else {
      if(isStacked)
        nThreshold = 32768; // 16 bit stacked half
      else
        nThreshold = (1 << (bits_per_pixel - 1)); // bit-depth adaptive default value
    }

#define SET_MODE(mode) \
    if (isStacked) { \
        processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_stacked_16_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
        processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_stacked_16_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
    } else { \
        switch(bit_depths[C]) { \
        case 10: processors.push_back(Filtering::Processor<Processor>(binarize_##mode##_native_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); \
                 processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_native_10_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
                 break; \
        case 12: processors.push_back(Filtering::Processor<Processor>(binarize_##mode##_native_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); \
                 processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_native_12_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
                 break; \
        case 14: processors.push_back(Filtering::Processor<Processor>(binarize_##mode##_native_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); \
                 processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_native_14_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
                 break; \
        case 16: processors.push_back(Filtering::Processor<Processor>(binarize_##mode##_native_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); \
                 processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_native_16_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
                 break; \
        }\
    }

        if (isMode("0 x")) { SET_MODE(0_x); } // e.g. binarize_0_x_10_native_c instead of binarize_0_x_native_c
        else if (isMode("t x")) { SET_MODE(t_x); }
        else if (isMode("x 0")) { SET_MODE(x_0); }
        else if (isMode("x t")) { SET_MODE(x_t); }
        else if (isMode("t 0")) { SET_MODE(t_0); }
        else if (isMode("0 t")) { SET_MODE(0_t); }
        else if (isMode("x 255")) { SET_MODE(x_255); }
        else if (isMode("t 255")) { SET_MODE(t_255); }
        else if (isMode("255 x")) { SET_MODE(255_x); }
        else if (isMode("255 t")) { SET_MODE(255_t); }
        else if ((parameters["mode"].is_defined() && (parameters["mode"].toString() == "upper" || parameters["mode"].toString() == "0 255")) || (!parameters["mode"].is_defined() && parameters["upper"].toBool())) {
            SET_MODE(upper);
        } else {
            SET_MODE(lower);
        }

#undef SET_MODE
    }

    InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

    static Signature filter_signature()
    {
        Signature signature = "mt_binarize16";
        signature.setValidBitdepth(10,16);

        signature.add(Parameter(TYPE_CLIP, ""));
        signature.add(Parameter(32768, "threshold")); // default is autocalculated for current bit depth as seen above
        signature.add(Parameter(false, "upper"));
        signature.add(Parameter(String("lower"), "mode"));
        signature.add(Parameter(false, "stacked"));
        //signature.add(Parameter(16, "bits")); // PF test

        return add_defaults( signature );
    }
};

} } } } // namespace Binarize, Filter, MaskTools, Filtering

#endif