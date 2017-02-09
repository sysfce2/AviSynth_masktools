#ifndef __Mt_Deflate16_H__
#define __Mt_Deflate16_H__

#include "../morphologic16.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 { namespace Deflate16 {

extern StackedProcessor *deflate_stacked_c;
extern InterleavedProcessor *deflate_interleaved_c;

class Deflate16 : public Morphologic16::MorphologicFilter16
{
public:
    Deflate16(const Parameters &parameters) : Morphologic16::MorphologicFilter16(parameters)
    {      
        if (parameters["stacked"].toBool()) {
            /* add the processors */
            stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(deflate_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        } else {
            interleavedProcessors.push_back(Filtering::Processor<InterleavedProcessor>(deflate_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
/* sample from 8 bit deflate:
processors.push_back(Filtering::Processor<Processor>(deflate_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
processors.push_back(Filtering::Processor<Processor>(deflate_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 2));
*/
        }
    }

    static Signature Deflate16::filter_signature()
    {
        Signature signature = "mt_deflate16";

        signature.add( Parameter(TYPE_CLIP, "") );
        signature.add( Parameter(65535, "thY") );
        signature.add( Parameter(65535, "thC") );
        signature.add( Parameter( false, "stacked" ) );

        return add_defaults( signature );
    }
};

} } } } } // namespace Deflate, Morphologic, Filter, MaskTools, Filtering

#endif