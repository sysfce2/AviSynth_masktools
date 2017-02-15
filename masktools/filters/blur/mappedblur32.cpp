#include "mappedblur.h"
#include "../../filters/mask/functions32.h"

using namespace Filtering;

namespace Filtering { namespace MaskTools { namespace Filters { namespace Blur {

inline Float convolution_all_32_c(Float a11, Float a21, Float a31, Float a12, Float a22, Float a32, Float a13, Float a23, Float a33, const Float matrix[10], Float nLowThreshold, Float nHighThreshold)
{
   Float nSum = a22 * matrix[4];

   UNUSED(nHighThreshold);

   if ( abs(a11 - a22) < nLowThreshold ) nSum = nSum + a11 * matrix[0]; else nSum = nSum + a22 * matrix[0];
   if ( abs(a21 - a22) < nLowThreshold ) nSum = nSum + a21 * matrix[1]; else nSum = nSum + a22 * matrix[1];
   if ( abs(a31 - a22) < nLowThreshold ) nSum = nSum + a31 * matrix[2]; else nSum = nSum + a22 * matrix[2];
   if ( abs(a12 - a22) < nLowThreshold ) nSum = nSum + a12 * matrix[3]; else nSum = nSum + a22 * matrix[3];
   if ( abs(a32 - a22) < nLowThreshold ) nSum = nSum + a32 * matrix[5]; else nSum = nSum + a22 * matrix[5];
   if ( abs(a13 - a22) < nLowThreshold ) nSum = nSum + a13 * matrix[6]; else nSum = nSum + a22 * matrix[6];
   if ( abs(a23 - a22) < nLowThreshold ) nSum = nSum + a23 * matrix[7]; else nSum = nSum + a22 * matrix[7];
   if ( abs(a33 - a22) < nLowThreshold ) nSum = nSum + a33 * matrix[8]; else nSum = nSum + a22 * matrix[8];

   return (nSum + matrix[9] / 2)/ matrix[9]; // no clipping
}

inline Float convolution_below_32_c(Float a11, Float a21, Float a31, Float a12, Float a22, Float a32, Float a13, Float a23, Float a33, const Float matrix[10], Float nLowThreshold, Float nHighThreshold)
{
   Float nSum = a22 * matrix[4];
   Float nSumCoeff = matrix[4];

   UNUSED(nHighThreshold);

   if ( abs(a11 - a22) < nLowThreshold ) { nSum = nSum + a11 * matrix[0]; nSumCoeff = nSumCoeff + matrix[0]; }
   if ( abs(a21 - a22) < nLowThreshold ) { nSum = nSum + a21 * matrix[1]; nSumCoeff = nSumCoeff + matrix[1]; }
   if ( abs(a31 - a22) < nLowThreshold ) { nSum = nSum + a31 * matrix[2]; nSumCoeff = nSumCoeff + matrix[2]; }
   if ( abs(a12 - a22) < nLowThreshold ) { nSum = nSum + a12 * matrix[3]; nSumCoeff = nSumCoeff + matrix[3]; }
   if ( abs(a32 - a22) < nLowThreshold ) { nSum = nSum + a32 * matrix[5]; nSumCoeff = nSumCoeff + matrix[5]; }
   if ( abs(a13 - a22) < nLowThreshold ) { nSum = nSum + a13 * matrix[6]; nSumCoeff = nSumCoeff + matrix[6]; }
   if ( abs(a23 - a22) < nLowThreshold ) { nSum = nSum + a23 * matrix[7]; nSumCoeff = nSumCoeff + matrix[7]; }
   if ( abs(a33 - a22) < nLowThreshold ) { nSum = nSum + a33 * matrix[8]; nSumCoeff = nSumCoeff + matrix[8]; }

   if ( nSumCoeff ) return (nSum + nSumCoeff / 2) / nSumCoeff; // no clipping
   else return a22;
}

class Thresholds {
   const Float *pThreshold;
   ptrdiff_t nPitch;
public:
   Thresholds(const Float *pThreshold, ptrdiff_t nPitch) :
   pThreshold(pThreshold), nPitch(nPitch / sizeof(Float))
   {
   }

   ptrdiff_t minpitch() const { return nPitch; }
   ptrdiff_t maxpitch() const { return nPitch; }
   void nextRow() { pThreshold += nPitch; }
   Float min(int x) const { return pThreshold[x]; }
   Float max(int x) const { return pThreshold[x]; }
};

template<Filters::Mask::Operator op>
void mask32_t(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, const Float *pMask, ptrdiff_t nMaskPitch, const Float matrix[10], int nWidth, int nHeight)
{
   Thresholds thresholds(pMask, nMaskPitch);
   Filters::Mask::generic32_c<op, Thresholds>(pDst, nDstPitch, pSrc, nSrcPitch, thresholds, matrix, nWidth, nHeight);
}

Processor32 *mapped_below_32_c = &mask32_t<convolution_below_32_c>;
Processor32 *mapped_all_32_c = &mask32_t<convolution_all_32_c>;

} } } }