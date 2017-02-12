#ifndef __Common_Avs2x_Clip_H__
#define __Common_Avs2x_Clip_H__

#include "filter.h"
#include "../common/utils/utils.h"

#if defined(FILTER_AVS_25)
#include "avisynth-2_5.h"
#elif defined(FILTER_AVS_26)
#include <windows.h>
#include <avisynth.h>
#else
#error FILTER_AVS_2x not defined
#endif

namespace Filtering { namespace Avisynth2x {

/* plane conversion */
template<typename T> Plane<T> ConvertTo(const PVideoFrame& frame, int nPlane, int nPixelSize);

template<> static inline Plane<Byte> ConvertTo<Byte>(const PVideoFrame &frame, int nPlane, int nPixelSize)
{
   return Plane<Byte>( frame->GetWritePtr( nPlane ), frame->GetPitch( nPlane ), frame->GetRowSize( nPlane ), frame->GetHeight( nPlane ), nPixelSize, frame->GetHeight(nPlane));
}
template<> static inline Plane<const Byte> ConvertTo<const Byte>(const PVideoFrame &frame, int nPlane, int nPixelSize)
{
   return Plane<const Byte>( frame->GetReadPtr( nPlane ), frame->GetPitch( nPlane ), frame->GetRowSize( nPlane ), frame->GetHeight( nPlane ), nPixelSize, frame->GetHeight(nPlane));
}

#if defined(FILTER_AVS_25)
template<typename T> Plane<T> ConvertInterleavedTo(const PVideoFrame& frame, int nPlane);

template<> static inline Plane<Byte> ConvertInterleavedTo<Byte>(const PVideoFrame &frame, int nPlane)
{
   const int nWidth = frame->GetRowSize() / (nPlane ? 2 : 1);
   const int nHeight = frame->GetHeight();
   const ptrdiff_t nPitch = frame->GetPitch();
   return Plane<Byte>( frame->GetWritePtr() + (nPlane == 2 ? nPitch * 3 / 4 : nPlane == 1 ? nPitch / 2 : 0), nPitch, nWidth, nHeight );
}

template<> static inline Plane<const Byte> ConvertInterleavedTo<const Byte>(const PVideoFrame &frame, int nPlane)
{
   const int nWidth = frame->GetRowSize() / (nPlane ? 2 : 1);
   const int nHeight = frame->GetHeight();
   const ptrdiff_t nPitch = frame->GetPitch();
   return Plane<const Byte>( frame->GetReadPtr() + (nPlane == 2 ? nPitch * 3 / 4 : nPlane == 1 ? nPitch / 2 : 0), nPitch, nWidth, nHeight );
}
#endif

/* colorspace conversion */
Colorspace AVSColorspaceToColorspace(int pixel_type)
{
   switch ( pixel_type )
   {
   case VideoInfo::CS_I420: return COLORSPACE_I420;
   case VideoInfo::CS_YV12: return COLORSPACE_YV12;
#if defined(FILTER_AVS_26)
   case VideoInfo::CS_YV16: return COLORSPACE_YV16;
   case VideoInfo::CS_YV24: return COLORSPACE_YV24;
   case VideoInfo::CS_Y8  : return COLORSPACE_Y8;

   case VideoInfo::CS_YV411: return COLORSPACE_YV411; // new from 2.2.1

   // new color spaces from v2.2.0
   case VideoInfo::CS_Y10: return COLORSPACE_Y10;
   case VideoInfo::CS_YUV420P10: return COLORSPACE_YUV420P10;
   case VideoInfo::CS_YUV422P10: return COLORSPACE_YUV422P10;
   case VideoInfo::CS_YUV444P10: return COLORSPACE_YUV444P10;

   case VideoInfo::CS_Y12: return COLORSPACE_Y12;
   case VideoInfo::CS_YUV420P12: return COLORSPACE_YUV420P12;
   case VideoInfo::CS_YUV422P12: return COLORSPACE_YUV422P12;
   case VideoInfo::CS_YUV444P12: return COLORSPACE_YUV444P12;

   case VideoInfo::CS_Y14: return COLORSPACE_Y14;
   case VideoInfo::CS_YUV420P14: return COLORSPACE_YUV420P14;
   case VideoInfo::CS_YUV422P14: return COLORSPACE_YUV422P14;
   case VideoInfo::CS_YUV444P14: return COLORSPACE_YUV444P14;

   case VideoInfo::CS_Y16: return COLORSPACE_Y16;
   case VideoInfo::CS_YUV420P16: return COLORSPACE_YUV420P16;
   case VideoInfo::CS_YUV422P16: return COLORSPACE_YUV422P16;
   case VideoInfo::CS_YUV444P16: return COLORSPACE_YUV444P16;

   case VideoInfo::CS_Y32: return COLORSPACE_Y32;
   case VideoInfo::CS_YUV420PS: return COLORSPACE_YUV420PS;
   case VideoInfo::CS_YUV422PS: return COLORSPACE_YUV422PS;
   case VideoInfo::CS_YUV444PS: return COLORSPACE_YUV444PS;

   case VideoInfo::CS_RGBP: return COLORSPACE_RGBP8;
   case VideoInfo::CS_RGBP10: return COLORSPACE_RGBP10;
   case VideoInfo::CS_RGBP12: return COLORSPACE_RGBP12;
   case VideoInfo::CS_RGBP14: return COLORSPACE_RGBP14;
   case VideoInfo::CS_RGBP16: return COLORSPACE_RGBP16;
   case VideoInfo::CS_RGBPS: return COLORSPACE_RGBPS;

#else if defined(FILTER_AVS_25)
   case VideoInfo::CS_YUY2: return COLORSPACE_YV16;
#endif

   default: return COLORSPACE_NONE;
   }
}

static const int PlaneOrder[] = { PLANAR_Y, PLANAR_U, PLANAR_V };
static const int PlaneOrderRGB[] = { PLANAR_G, PLANAR_B, PLANAR_R };

/* clip class, adapted to avs25 */
class Clip : public Filtering::Clip {

    ::PClip pclip;
    IScriptEnvironment *env;

public:

    Clip(const ::PClip &pclip) : pclip(pclip), env(NULL)
    {
        VideoInfo vi = pclip->GetVideoInfo();
        nWidth = vi.width;
        nHeight = vi.height;
        nFrames = vi.num_frames;
        C = AVSColorspaceToColorspace(vi.pixel_type);
    }
    
    virtual ~Clip()
    {
    }

    template<typename T> Frame<T> ConvertTo(const PVideoFrame& frame)
    {
        Plane<T> planes[3];
#if defined(FILTER_AVS_25)
        if ( C == COLORSPACE_YV16 )
        {
            for ( int i = 0; i < plane_counts[C]; i++ )
                planes[i] = Avisynth2x::ConvertInterleavedTo<T>( frame, i );
        }
        else
#endif
        {
            int bit_depth = bit_depths[C];
            int nPixelSize = bit_depth == 8 ? 1 : ((bit_depth <= 16) ? 2 : 4); // uint8_t, uint16_t, float
            for (int i = 0; i < plane_counts[C]; i++)
                planes[i] = Avisynth2x::ConvertTo<T>(frame, planes_isRGB[C] ? PlaneOrderRGB[i] : PlaneOrder[i], nPixelSize);
        }
        return plane_counts[C] == 1 ? Frame<T>(planes[0]) : Frame<T>(planes, C);
    }


#if 0
   virtual Frame<Byte> get_frame(int n, IScriptEnvironment *env)
   {
     PVideoFrame frame = pclip->GetFrame(n, env);
     return ConvertTo<Byte>(frame);
   }
    
   virtual Frame<const Byte> get_const_frame(int n, IScriptEnvironment *env)
   {
       /* PF: nasty phenomenon, only happened in speficic circumstanced/timing.
       1.)We are getting the frame
       2.)Extract the plane ReadPtr-s with ConvertTo
       3.)Then the destructor of the local variable PVideoFrame frame is called on exit.
       Thus the pointers that has been just requested are no longer valid, that PVideoFrame area may be reused.
       */
       PVideoFrame frame = pclip->GetFrame(clip<int>(n, 0, nFrames - 1), env);
       return ConvertTo<const Byte>(frame);
   }
#endif
   /* not used
   virtual Frame<Byte> get_frame(int n, PVideoFrame &frame_out, IScriptEnvironment *env)
   {
     frame_out = pclip->GetFrame(n, env);
     return ConvertTo<Byte>(frame_out);
   }
   */

   virtual Frame<const Byte> get_const_frame(int n, PVideoFrame &frame_out, IScriptEnvironment *env)
   {
     // PF 170211: Issue: volatile pointers
     // return the frame, or else plane pointers stored in ConvertTo are not valid anymore
     // after function exit
     frame_out = pclip->GetFrame(clip<int>(n, 0, nFrames - 1), env);
     return ConvertTo<const Byte>(frame_out);
   }
};

} } // namespace Avisynth2x, Common

#endif
