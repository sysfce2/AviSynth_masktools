#ifndef __Mt_Inflate16_H__
#define __Mt_Inflate16_H__
#if 0
#include "../morphologic16.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 { namespace Inflate16 {

extern StackedProcessor *inflate_stacked_c;
extern Processor16 *inflate_native_c;

class Inflate16 : public Morphologic16::MorphologicFilter16
{
public:
    Inflate16(const Parameters &parameters) : Morphologic16::MorphologicFilter16(parameters)
    {
        if (parameters["stacked"].toBool()) {
            /* add the processors */
            stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inflate_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        } else {
            processors16.push_back(Filtering::Processor<Processor16>(inflate_native_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        }
    }

    static Signature Inflate16::filter_signature()
    {
        Signature signature = "mt_inflate16";

        signature.add( Parameter(TYPE_CLIP, "") );
        signature.add( Parameter(65535, "thY") );
        signature.add( Parameter(65535, "thC") );
        signature.add( Parameter( false, "stacked" ) );

        return add_defaults( signature );
    }
};

} } } } } // namespace Inflate, Morphologic, Filter, MaskTools, Filtering
#endif
#endif