#ifndef __Mt_Inflate_H__
#define __Mt_Inflate_H__

#include "../morphologic.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inflate {

extern Processor *inflate_c;
extern Processor *inflate_sse2;
extern Processor *inflate_asse2;

/* 16 bit */
extern StackedProcessor *inflate_stacked_c;
extern Processor16 *inflate_native_c;

/*32 bit */
extern Processor32 *inflate32_c;

class Inflate : public Morphologic::MorphologicFilter
{
public:
  Inflate(const Parameters &parameters) : Morphologic::MorphologicFilter(parameters)
  {
    int bits_per_pixel = bit_depths[C];
    bool isStacked = parameters["stacked"].toBool();
    if (isStacked)
      bits_per_pixel = 16;
    /* add the processors */
    if (bits_per_pixel == 8) {
      processors.push_back(Filtering::Processor<Processor>(inflate_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(inflate_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
      processors.push_back(Filtering::Processor<Processor>(inflate_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 2));
    }
    else if (isStacked) {
      stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inflate_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
    }
    else if (bits_per_pixel <= 16){
      processors16.push_back(Filtering::Processor<Processor16>(inflate_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
    }
    else {
      processors32.push_back(Filtering::Processor<Processor32>(inflate32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
    }
  }

  static Signature Inflate::filter_signature()
  {
    Signature signature = "mt_inflate";

    signature.add(Parameter(TYPE_CLIP, ""));
    signature.add(Parameter(TYPE_FLOAT, "thY")); // overwritten to default 255..65535 in morphologic.h
    signature.add(Parameter(TYPE_FLOAT, "thC"));
    signature.add(Parameter(false, "stacked"));

    return add_defaults(signature);
  }
};

} } } } } // namespace Inflate, Morphologic, Filter, MaskTools, Filtering

#endif