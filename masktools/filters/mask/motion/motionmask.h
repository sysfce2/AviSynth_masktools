#ifndef __Mt_Motionmask_H__
#define __Mt_Motionmask_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Motion {

typedef bool (Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nMotionThreshold, int nSceneChange, int nSceneChangeValue, int nWidth, int nHeight);
typedef bool (Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nMotionThreshold, int nSceneChange, int nSceneChangeValue, int nWidth, int nHeight);
typedef bool (Processor32)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, Float nLowThreshold, Float nHighThreshold, Float nMotionThreshold, int nSceneChange, Float nSceneChangeValue_f, int nWidth, int nHeight);

extern Processor *mask_c;
extern Processor *mask_sse2;
extern Processor *mask_asse2;

extern Processor16 *mask10_16_c;
extern Processor16 *mask10_16_sse2;
extern Processor16 *mask10_16_asse2;

extern Processor32 *mask32_c;
extern Processor32 *mask32_sse2;
extern Processor32 *mask32_asse2;

class MotionMask : public MaskTools::Filter
{
   int nLowThresholds[3];
   int nHighThresholds[3];
   int nMotionThreshold;

   float nLowThresholds_f[3];
   float nHighThresholds_f[3];
   float nMotionThreshold_f;

   ProcessorList<Processor> processors;
   ProcessorList<Processor16> processors16;
   ProcessorList<Processor32> processors32;

   int nSceneChange;
   
   int nSceneChangeValue;
   Float nSceneChangeValue_f;

   int bits_per_pixel;

protected:

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        if (bits_per_pixel == 8) {
          if (nPlane == 0)
            nSceneChange = processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              nLowThresholds[nPlane], nHighThresholds[nPlane], nMotionThreshold, 0, nSceneChangeValue, dst.width(), dst.height()) ? 3 : 2;
          else
            processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              nLowThresholds[nPlane], nHighThresholds[nPlane], nMotionThreshold, nSceneChange, nSceneChangeValue, dst.width(), dst.height());
        }
        else if (bits_per_pixel <= 16) {
          if (nPlane == 0)
            nSceneChange = processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              nLowThresholds[nPlane], nHighThresholds[nPlane], nMotionThreshold, 0, nSceneChangeValue, dst.width(), dst.height()) ? 3 : 2;
          else
            processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              nLowThresholds[nPlane], nHighThresholds[nPlane], nMotionThreshold, nSceneChange, nSceneChangeValue, dst.width(), dst.height());
        }
        else {
          if (nPlane == 0)
            nSceneChange = processors32.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              nLowThresholds_f[nPlane], nHighThresholds_f[nPlane], nMotionThreshold_f, 0, nSceneChangeValue_f, dst.width(), dst.height()) ? 3 : 2;
          else
            processors32.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              nLowThresholds_f[nPlane], nHighThresholds_f[nPlane], nMotionThreshold_f, nSceneChange, nSceneChangeValue_f, dst.width(), dst.height());
        }
    }

public:
   MotionMask(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
     bits_per_pixel = bit_depths[C];
     bool isFloat = bits_per_pixel == 32;


     if (!isFloat) {
       // default value of 10 scaled by bit depth
       int nLow0, nLow1;
       int nHigh0, nHigh1;
       int max_pixel_value = (1 << bits_per_pixel) - 1;
       nLow0 = parameters["thY1"].is_defined() ? parameters["thY1"].toInt() : (10 << (bits_per_pixel - 8));
       nLow1 = parameters["thC1"].is_defined() ? parameters["thC1"].toInt() : (10 << (bits_per_pixel - 8));
       nHigh0 = parameters["thY2"].is_defined() ? parameters["thY2"].toInt() : (10 << (bits_per_pixel - 8));
       nHigh1 = parameters["thC2"].is_defined() ? parameters["thC2"].toInt() : (10 << (bits_per_pixel - 8));

       nMotionThreshold = parameters["thT"].is_defined() ? parameters["thT"].toInt() : (10 << (bits_per_pixel - 8));
       nMotionThreshold = min(max(nMotionThreshold, 0), max_pixel_value);

       nLowThresholds[0] = min(max(nLow0, 0), max_pixel_value);
       nLowThresholds[1] = nLowThresholds[2] = min(max(nLow1, 0), max_pixel_value);
       nHighThresholds[0] = min(max(nHigh0, 0), max_pixel_value);
       nHighThresholds[1] = nHighThresholds[2] = min(max(nHigh1, 0), max_pixel_value);

       // value copied into the whole frame when scenechange is detected
       nSceneChangeValue = parameters["scvalue"].is_defined() ? parameters["scvalue"].toInt() : 0;
       nSceneChangeValue = min(max(nSceneChangeValue, 0), max_pixel_value);
     }
     else {
       float nLow0, nLow1;
       float nHigh0, nHigh1;
       nLow0 = parameters["thY1"].is_defined() ? (float)parameters["thY1"].toFloat() : (10.0f / 255);
       nLow1 = parameters["thC1"].is_defined() ? (float)parameters["thC1"].toFloat() : (10.0f / 255);
       nHigh0 = parameters["thY2"].is_defined() ? (float)parameters["thY2"].toFloat() : (10.0f / 255);
       nHigh1 = parameters["thC2"].is_defined() ? (float)parameters["thC2"].toFloat() : (10.0f / 255);
       
       nMotionThreshold_f = parameters["thT"].is_defined() ? (float)parameters["thC2"].toFloat() : (10.0f / 255);

       nLowThresholds_f[0] = nLow0;
       nLowThresholds_f[1] = nLowThresholds_f[2] = nLow1;
       nHighThresholds_f[0] = nHigh0;
       nHighThresholds_f[1] = nHighThresholds_f[2] = nHigh1;

       nSceneChangeValue_f = (Float)(parameters["scvalue"].toFloat());
     }


      /* add the processors */
      switch (bits_per_pixel) {
      case 8:
        processors.push_back(Filtering::Processor<Processor>(mask_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(mask_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        processors.push_back(Filtering::Processor<Processor>(mask_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
        break;
      case 10:
      case 12:
      case 14:
      case 16:
        processors16.push_back(Filtering::Processor<Processor16>(mask10_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors16.push_back(Filtering::Processor<Processor16>(mask10_16_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        processors16.push_back(Filtering::Processor<Processor16>(mask10_16_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
        break;
      case 32:
        processors32.push_back(Filtering::Processor<Processor32>(mask32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors32.push_back(Filtering::Processor<Processor32>(mask32_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1)); // only sad is sse2
        processors32.push_back(Filtering::Processor<Processor32>(mask32_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
        break;
      }
      }

   InputConfiguration &input_configuration() const { return InPlaceTemporalOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_motion";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(10.0f, "thY1"));
      signature.add(Parameter(10.0f, "thY2"));
      signature.add(Parameter(10.0f, "thC1"));
      signature.add(Parameter(10.0f, "thC2"));
      signature.add(Parameter(10.0f, "thT"));
      signature.add(Parameter(0.0f, "sc_value"));

      return add_defaults( signature );
   }

};

} } } } }

#endif