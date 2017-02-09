#ifndef __Common_Plane_H__
#define __Common_Plane_H__

#include "../utils/utils.h"

namespace Filtering {

template<typename T>
class Plane {

protected:

   T *pPixel;
   ptrdiff_t nPitch;
   int nWidth;
   int nHeight;
   int nPixelSize; // 1 for 8 bit, 2 for 10-16 bit, 4 for float

public:

   Plane() : pPixel(NULL), nPitch(0), nWidth(0), nHeight(0), nPixelSize(1) {}
   Plane(T *pPixel, ptrdiff_t nPitch, int nWidth, int nHeight, int nPixelSize) : pPixel(pPixel), nPitch(nPitch), nWidth(nWidth), nHeight(nHeight), nPixelSize(nPixelSize) { }

   Plane(const Plane<const T> &plane) : pPixel(plane.data()), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height()), nPixelSize(plane.pixelsize())
   {
   }

   T* data() const { return pPixel; }
   ptrdiff_t pitch() const { return nPitch; }
   int width() const { return nWidth; }
   int height() const { return nHeight; }
   int pixelsize() const { return nPixelSize; }

   Plane<T> offset(int x = 0, int y = 0, int w = 0, int h = 0) const 
   { 
     return Plane<T>( pPixel + x * nPixelSize + y * nPitch, nPitch, w, h, nPixelSize ); // byte pointers->x*pixelsize
   }
   void print() const { Filtering::print( LOG_DEBUG, "plane 0x%x (%ix%i:%i) t%i\n", pPixel, nWidth, nHeight, nPitch, GetCurrentThreadId() ); }

};

} // namespace Filtering

#endif

