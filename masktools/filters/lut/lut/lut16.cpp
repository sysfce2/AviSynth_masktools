#include "lut.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Single::lut16_c_native(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word lut[65536], int)
{
    for ( int y = 0; y < nHeight; y++ )
    {
        auto pDst16 = reinterpret_cast<Word*>(pDst);
        for ( int x = 0; x < nWidth; x+=1 )
            pDst16[x] = lut[pDst16[x]];
        pDst += nDstPitch;
    }
}

void Filtering::MaskTools::Filters::Lut::Single::lut16_c_stacked(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word lut[65536], int nOrigHeightForStacked)
{
    auto pLsb = pDst + nDstPitch * nOrigHeightForStacked / 2;

    for ( int y = 0; y < nHeight / 2; y++ )
    {
        for ( int x = 0; x < nWidth; x++ ) {
            Word value = lut[(pDst[x] << 8) + pLsb[x]];
            pDst[x] = value >> 8;
            pLsb[x] = value & 0xFF;
        }
        pDst += nDstPitch;
        pLsb += nDstPitch;
    }
}
