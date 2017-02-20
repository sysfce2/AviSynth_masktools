#include "deflate.h"
#include "../functions32.h"
#include "../../../common/simd.h"

using namespace Filtering;

static MT_FORCEINLINE Float meanMin(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9)
{
    Float nSum = (a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9) / 8.0f;
    return nSum < a5 ? nSum : a5;
}

static MT_FORCEINLINE Float meanMinThresholded(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6, Float a7, Float a8, Float a9, Float nMaxDeviation)
{
    Float nMeanMin = meanMin(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( a5 - nMeanMin > nMaxDeviation ) nMeanMin = a5 - nMaxDeviation;
    return nMeanMin;
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Deflate {

Processor32 *deflate_32_c = &MorphologicProcessor<Float>::generic_32_c<
    process_line_morpho_32_c<Border::Left, meanMinThresholded>,
    process_line_morpho_32_c<Border::None, meanMinThresholded>,
    process_line_morpho_32_c<Border::Right, meanMinThresholded>
>;

} } } } }