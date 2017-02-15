#ifndef __Mt_Inpand_H__
#define __Mt_Inpand_H__

#include "../morphologic.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inpand {


extern Processor *inpand_square_c;
extern Processor *inpand_square_sse2;
extern Processor *inpand_square_asse2;

extern Processor *inpand_horizontal_c;
extern Processor *inpand_horizontal_sse2;
extern Processor *inpand_horizontal_asse2;

extern Processor *inpand_vertical_c;
extern Processor *inpand_vertical_sse2;
extern Processor *inpand_vertical_asse2;

extern Processor *inpand_both_c;
extern Processor *inpand_both_sse2;
extern Processor *inpand_both_asse2;

extern Processor *inpand_custom_c;

/* 16 bit */
extern StackedProcessor *inpand_square_stacked_c;
extern StackedProcessor *inpand_horizontal_stacked_c;
extern StackedProcessor *inpand_vertical_stacked_c;
extern StackedProcessor *inpand_both_stacked_c;
extern StackedProcessor *inpand_custom_stacked_c;

extern Processor16 *inpand_square_native_c;
extern Processor16 *inpand_horizontal_native_c;
extern Processor16 *inpand_vertical_native_c;
extern Processor16 *inpand_both_native_c;
extern Processor16 *inpand_custom_native_c;

class Inpand : public Morphologic::MorphologicFilter
{
public:
  Inpand(const Parameters&parameters) : Morphologic::MorphologicFilter(parameters)
  {
    int bits_per_pixel = bit_depths[C];
    bool isStacked = parameters["stacked"].toBool();
    if (isStacked)
      bits_per_pixel = 16;
    /* add the processors */
    if (parameters["mode"].toString() == "square")
    {
      if (bits_per_pixel == 8) {
        processors.push_back(Filtering::Processor<Processor>(inpand_square_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(inpand_square_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 3));
        processors.push_back(Filtering::Processor<Processor>(inpand_square_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 4));
      }
      else if (isStacked) {
        stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_square_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      else if (bits_per_pixel <= 16) {
        processors16.push_back(Filtering::Processor<Processor16>(inpand_square_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
    }
    else if (parameters["mode"].toString() == "horizontal")
    {
      if (bits_per_pixel == 8) {
        processors.push_back(Filtering::Processor<Processor>(inpand_horizontal_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(inpand_horizontal_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 3));
        processors.push_back(Filtering::Processor<Processor>(inpand_horizontal_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 4));
      }
      else if (isStacked) {
        stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_horizontal_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      else if (bits_per_pixel <= 16) {
        processors16.push_back(Filtering::Processor<Processor16>(inpand_horizontal_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
    }
    else if (parameters["mode"].toString() == "vertical")
    {
      if (bits_per_pixel == 8) {
        processors.push_back(Filtering::Processor<Processor>(inpand_vertical_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(inpand_vertical_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 3));
        processors.push_back(Filtering::Processor<Processor>(inpand_vertical_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 4));
      }
      else if (isStacked) {
        stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_vertical_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      else if (bits_per_pixel <= 16) {
        processors16.push_back(Filtering::Processor<Processor16>(inpand_vertical_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
    }
    else if (parameters["mode"].toString() == "both")
    {
      if (bits_per_pixel == 8) {
        processors.push_back(Filtering::Processor<Processor>(inpand_both_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(inpand_both_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 3));
        processors.push_back(Filtering::Processor<Processor>(inpand_both_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 4));
      }
      else if (isStacked) {
        stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_both_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      else if (bits_per_pixel <= 16) {
        processors16.push_back(Filtering::Processor<Processor16>(inpand_both_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
    }
    else
    {
      if (bits_per_pixel == 8) {
        processors.push_back(Filtering::Processor<Processor>(inpand_custom_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      else if (isStacked) {
        stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_custom_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      else if (bits_per_pixel <= 16) {
        processors16.push_back(Filtering::Processor<Processor16>(inpand_custom_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      }
      FillCoordinates(parameters["mode"].toString());
    }
  }

  static Signature Inpand::filter_signature()
  {
    Signature signature = "mt_inpand";

    signature.add(Parameter(TYPE_CLIP, ""));
    signature.add(Parameter(TYPE_FLOAT, "thY")); // overwritten to default 255..65535 in morphologic.h
    signature.add(Parameter(TYPE_FLOAT, "thC"));
    signature.add(Parameter(String("square"), "mode"));
    signature.add(Parameter(false, "stacked"));

    return add_defaults(signature);
  }
};

} } } } } // namespace Inpand, Morphologic, Filter, MaskTools, Filtering

#endif