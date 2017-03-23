#ifndef __Mt_MorphologicFilter_H__
#define __Mt_MorphologicFilter_H__

#include "../../common/base/filter.h"
#include "../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic {

typedef void (Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);
typedef void (StackedProcessor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, int nOrigHeight);
typedef void (Processor16)(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, int nOrigHeight);
typedef void (Processor32)(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, Float nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);

class MorphologicFilter : public MaskTools::Filter
{
    int nMaxDeviations[3];
    float nMaxDeviations_f[3];
    int *coordinates_list;
    int coordinates_count;

    MorphologicFilter(const MorphologicFilter &filter);

    int bits_per_pixel;
    bool isStacked;

protected:

    ProcessorList<Processor> processors;
    ProcessorList<StackedProcessor> stackedProcessors;
    ProcessorList<Processor16> processors16;
    ProcessorList<Processor32> processors32;

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
      UNUSED(n);
      if (bits_per_pixel == 8) {
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          nMaxDeviations[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height());
      }
      else if (isStacked) {
        stackedProcessors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          nMaxDeviations[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height() / 2, dst.origheight()); // stacked: /2
      }
      else if(bits_per_pixel <= 16) {
        processors16.best_processor(constraints[nPlane])(reinterpret_cast<Word*>((Byte*)dst.data()), dst.pitch() / sizeof(uint16_t), /* /2: word sized */
          reinterpret_cast<const Word*>((const Byte*)(frames[0].plane(nPlane).data())), frames[0].plane(nPlane).pitch() / sizeof(uint16_t),
          nMaxDeviations[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height(), dst.origheight());
      }
      else {
        processors32.best_processor(constraints[nPlane])((Float *)dst.data(), dst.pitch() / sizeof(Float),
          (Float *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch() / sizeof(Float),
          nMaxDeviations_f[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height());
      }
    }

    void FillCoordinates(const String &coordinates)
    {
        auto coeffs = Parser::getDefaultParser().parse(coordinates, " (),;.").getExpression();
        coordinates_count = coeffs.size();
        coordinates_list = new int[coordinates_count];
        int i = 0;

        while (!coeffs.empty())
        {
            coordinates_list[i++] = int(coeffs.front().getValue(0, 0, 0));
            coeffs.pop_front();
        }
    }

public:
    MorphologicFilter(const Parameters &parameters, CpuFlags cpuFlags) : MaskTools::Filter(parameters, FilterProcessingType::CHILD, (CpuFlags)cpuFlags), coordinates_list(NULL), coordinates_count(0)
    {
      isStacked = parameters["stacked"].toBool();
      bits_per_pixel = bit_depths[C];

      if (isStacked && bits_per_pixel != 8) {
        error = "Stacked specified for a non-8 bit clip";
        return;
      }
      
      if (isStacked)
        bits_per_pixel = 16;

      bool isFloat = bits_per_pixel == 32;
      bool fullscale = planes_isRGB[C];
      String scalemode = parameters["paramscale"].toString();

      int thY, thC;
      float thY_f, thC_f;

      const char *errortxt = "invalid parameter: paramscale. Use i8, i10, i12, i14, i16, f32 for scale or none/empty to disable scaling";

      // defaults
      thY_f = thC_f = 1.0;
      thY = thC = (1 << bits_per_pixel) - 1;

      // Y threshold
      if (parameters["thY"].is_defined()) {
        thY_f = (float)parameters["thY"].toFloat();
        if (!ScaleParam(scalemode, thY_f, bits_per_pixel, thY_f, thY, fullscale))
        {
          error = errortxt;
          return;
        }
      }

      // chroma threshold
      if (parameters["thC"].is_defined()) {
        thC_f = (float)parameters["thC"].toFloat();
        if (!ScaleParam(scalemode, thC_f, bits_per_pixel, thC_f, thC, fullscale))
        {
          error = errortxt;
          return;
        }
      }

      if (isFloat) {
        nMaxDeviations_f[0] = thY_f;
        nMaxDeviations_f[1] = nMaxDeviations_f[2] = thC_f;
      }
      else {
        nMaxDeviations[0] = thY;
        nMaxDeviations[1] = nMaxDeviations[2] = thC;
      }

    }

    ~MorphologicFilter()
    {
        delete[] coordinates_list;
        coordinates_list = NULL;
    }

    InputConfiguration &input_configuration() const { return OneFrame(); }
};

} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif