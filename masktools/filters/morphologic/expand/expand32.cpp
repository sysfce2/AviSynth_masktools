#include "expand.h"
#include "../functions32.h"

using namespace Filtering;

typedef Float (local_maximum_f)(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9);

static inline Float maximum_square(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9)
{
    Float nMax = a1;
    if ( a2 > nMax ) nMax = a2;
    if ( a3 > nMax ) nMax = a3;
    if ( a4 > nMax ) nMax = a4;
    if ( a5 > nMax ) nMax = a5;
    if ( a6 > nMax ) nMax = a6;
    if ( a7 > nMax ) nMax = a7;
    if ( a8 > nMax ) nMax = a8;
    if ( a9 > nMax ) nMax = a9;
    return nMax;
}

static inline Float maximum_horizontal(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9)
{
    Float nMax = a4;

    UNUSED(a1); UNUSED(a2); UNUSED(a3); UNUSED(a7); UNUSED(a8); UNUSED(a9); 

    if ( a5 > nMax ) nMax = a5;
    if ( a6 > nMax ) nMax = a6;
    return nMax;
}

static inline Float maximum_vertical(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9)
{
    Float nMax = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a4); UNUSED(a6); UNUSED(a7); UNUSED(a9); 

    if ( a5 > nMax ) nMax = a5;
    if ( a8 > nMax ) nMax = a8;
    return nMax;
}

static inline Float maximum_both(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9)
{
    Float nMax = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a7); UNUSED(a9); 

    if ( a4 > nMax ) nMax = a4;
    if ( a5 > nMax ) nMax = a5;
    if ( a6 > nMax ) nMax = a6;
    if ( a8 > nMax ) nMax = a8;
    return nMax;
}

template<local_maximum_f Maximum>
static inline Float maximumThresholded(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9, Float nMaxDeviation)
{
    Float nMaximum = Maximum(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( nMaximum - a5 > nMaxDeviation ) nMaximum = a5 + nMaxDeviation;
    return nMaximum;
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Expand {

    class NewValue32 {
        Float nMax;
        Float nMaxDeviation;
        Float nValue;
    public:
        NewValue32(Float nValue, Float nMaxDeviation) : nMax(-1.0), nMaxDeviation(nMaxDeviation), nValue(nValue) { }
        void add(Float _nValue) { if ( _nValue > nMax ) nMax = _nValue; }
        Float finalize() const { return nMax < 0 ? nValue : (nMax - nValue > nMaxDeviation ? nValue + nMaxDeviation : nMax); }
    };


Processor32 *expand_square_32_c = &MorphologicProcessor<Float>::generic_32_c<
    process_line_morpho_32_c<Border::Left, maximumThresholded<::maximum_square>>,
    process_line_morpho_32_c<Border::None, maximumThresholded<::maximum_square>>,
    process_line_morpho_32_c<Border::Right, maximumThresholded<::maximum_square>>
    >;

Processor32 *expand_horizontal_32_c = &MorphologicProcessor<Float>::generic_32_c<
    process_line_morpho_32_c<Border::Left, maximumThresholded<::maximum_horizontal>>,
    process_line_morpho_32_c<Border::None, maximumThresholded<::maximum_horizontal>>,
    process_line_morpho_32_c<Border::Right, maximumThresholded<::maximum_horizontal>>
    >;

Processor32 *expand_vertical_32_c = &MorphologicProcessor<Float>::generic_32_c<
    process_line_morpho_32_c<Border::Left, maximumThresholded<::maximum_vertical>>,
    process_line_morpho_32_c<Border::None, maximumThresholded<::maximum_vertical>>,
    process_line_morpho_32_c<Border::Right, maximumThresholded<::maximum_vertical>>
    >;

Processor32 *expand_both_32_c = &MorphologicProcessor<Float>::generic_32_c<
    process_line_morpho_32_c<Border::Left, maximumThresholded<::maximum_both>>,
    process_line_morpho_32_c<Border::None, maximumThresholded<::maximum_both>>,
    process_line_morpho_32_c<Border::Right, maximumThresholded<::maximum_both>>
    >;


Processor32 *expand_custom_32_c   = &generic_custom_32_c<NewValue32>;

} } } } }