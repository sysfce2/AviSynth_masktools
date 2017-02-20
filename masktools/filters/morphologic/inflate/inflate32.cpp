#include "inflate.h"
#include "../functions32.h"
#include "../../../common/simd.h"

using namespace Filtering;

static inline Float meanMax(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9)
{
    Float nSum = (a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9) / 8.0f;
    return nSum > a5 ? nSum : a5;
}

static inline Float meanMaxThresholded(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9, Float nMaxDeviation)
{
    Float nMeanMax = meanMax(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( nMeanMax - a5 > nMaxDeviation ) nMeanMax = a5 + nMaxDeviation;
    return nMeanMax;
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inflate {

Processor32 *inflate_32_c = &MorphologicProcessor<Float>::generic_32_c<
    process_line_morpho_32_c<Border::Left, meanMaxThresholded>,
    process_line_morpho_32_c<Border::None, meanMaxThresholded>,
    process_line_morpho_32_c<Border::Right, meanMaxThresholded>
>;

} } } } }