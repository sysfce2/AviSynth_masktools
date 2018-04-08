#include <stdint.h>
#include <avisynth.h>
#include "CommonFunctions.h"
#include "VectorFunctions.cuh"

#ifndef NDEBUG
//#if 1
#define DEBUG_SYNC \
		CUDA_CHECK(cudaGetLastError()); \
      CUDA_CHECK(cudaDeviceSynchronize())
#else
#define DEBUG_SYNC
#endif

template<typename vpixel_t, typename pixel_t, int bits_per_pixel>
__global__ void kl_lut_x(vpixel_t* dst, const vpixel_t* __restrict__ src,
   int pitch4, int width4, int height, const pixel_t* __restrict__ lut, int mask)
{
   int x = threadIdx.x + blockIdx.x * blockDim.x;
   int y = threadIdx.y + blockIdx.y * blockDim.y;

   if (x < width4 && y < height) {
      auto X = src[x + y * pitch4];
      if (sizeof(pixel_t) == 1) {
         vpixel_t t = {
            lut[X.x],
            lut[X.y],
            lut[X.z],
            lut[X.w]
         };
         dst[x + y * pitch4] = t;
      }
      else { // == 2
         vpixel_t t = {
            lut[X.x & mask],
            lut[X.y & mask],
            lut[X.z & mask],
            lut[X.w & mask]
         };
         dst[x + y * pitch4] = t;
      }
   }
}

template<int bits_per_pixel>
__device__ int lut_index_xy(int x, int y) {
   return (x << bits_per_pixel) + y;
}

template<typename vpixel_t, typename pixel_t, int bits_per_pixel>
__global__ void kl_lut_xy(vpixel_t* dst,
   const vpixel_t* __restrict__ src0,
   const vpixel_t* __restrict__ src1,
   int pitch4, int width4, int height, const pixel_t* __restrict__ lut, int mask)
{
   int x = threadIdx.x + blockIdx.x * blockDim.x;
   int y = threadIdx.y + blockIdx.y * blockDim.y;

   if (x < width4 && y < height) {
      auto X = src0[x + y * pitch4];
      auto Y = src1[x + y * pitch4];
      if (sizeof(pixel_t) == 1) {
         vpixel_t t = {
            lut[lut_index_xy<bits_per_pixel>(X.x, Y.x)],
            lut[lut_index_xy<bits_per_pixel>(X.y, Y.y)],
            lut[lut_index_xy<bits_per_pixel>(X.z, Y.z)],
            lut[lut_index_xy<bits_per_pixel>(X.w, Y.w)]
         };
         dst[x + y * pitch4] = t;
      }
      else { // == 2
         vpixel_t t = {
            lut[lut_index_xy<bits_per_pixel>(X.x, Y.x) & mask],
            lut[lut_index_xy<bits_per_pixel>(X.y, Y.y) & mask],
            lut[lut_index_xy<bits_per_pixel>(X.z, Y.z) & mask],
            lut[lut_index_xy<bits_per_pixel>(X.w, Y.w) & mask]
         };
         dst[x + y * pitch4] = t;
      }
   }
}

template<int bits_per_pixel>
__device__ int lut_index_xyz(int x, int y, int z) {
   return (x << (bits_per_pixel * 2)) + (y << bits_per_pixel) + z;
}

template<typename vpixel_t, typename pixel_t, int bits_per_pixel>
__global__ void kl_lut_xyz(vpixel_t* dst,
   const vpixel_t* __restrict__ src0,
   const vpixel_t* __restrict__ src1,
   const vpixel_t* __restrict__ src2,
   int pitch4, int width4, int height, const pixel_t* __restrict__ lut, int mask)
{
   int x = threadIdx.x + blockIdx.x * blockDim.x;
   int y = threadIdx.y + blockIdx.y * blockDim.y;

   if (x < width4 && y < height) {
      auto X = src0[x + y * pitch4];
      auto Y = src1[x + y * pitch4];
      auto Z = src2[x + y * pitch4];
      if (sizeof(pixel_t) == 1) {
         vpixel_t t = {
            lut[lut_index_xyz<bits_per_pixel>(X.x, Y.x, Z.x)],
            lut[lut_index_xyz<bits_per_pixel>(X.y, Y.y, Z.y)],
            lut[lut_index_xyz<bits_per_pixel>(X.z, Y.z, Z.z)],
            lut[lut_index_xyz<bits_per_pixel>(X.w, Y.w, Z.w)]
         };
         dst[x + y * pitch4] = t;
      }
      else { // == 2
         vpixel_t t = {
            lut[lut_index_xyz<bits_per_pixel>(X.x, Y.x, Z.x) & mask],
            lut[lut_index_xyz<bits_per_pixel>(X.y, Y.y, Z.y) & mask],
            lut[lut_index_xyz<bits_per_pixel>(X.z, Y.z, Z.z) & mask],
            lut[lut_index_xyz<bits_per_pixel>(X.w, Y.w, Z.w) & mask]
         };
         dst[x + y * pitch4] = t;
      }
   }
}

template <typename vpixel_t, typename pixel_t, int bits_per_pixel>
void lut_cuda(int num_input, pixel_t *pDst, const pixel_t * const *pSrc, int pitch, int width, int height, const pixel_t* lut, PNeoEnv env)
{
   const int mask = (1 << bits_per_pixel) - 1;

   int width4 = width >> 2;
   int pitch4 = pitch >> 2;

   dim3 threads(16, 8);
   dim3 blocks(nblocks(width4, threads.x), nblocks(height, threads.y));

   switch (num_input) {
   case 1:
      kl_lut_x<vpixel_t, pixel_t, bits_per_pixel> << <blocks, threads >> > ((vpixel_t*)pDst,
         (const vpixel_t*)pSrc[0],
         pitch4, width4, height, lut, mask);
      DEBUG_SYNC;
      break;
   case 2:
      kl_lut_xy<vpixel_t, pixel_t, bits_per_pixel> << <blocks, threads >> > ((vpixel_t*)pDst,
         (const vpixel_t*)pSrc[0], (const vpixel_t*)pSrc[1],
         pitch4, width4, height, lut, mask);
      DEBUG_SYNC;
      break;
   case 3:
      kl_lut_xyz<vpixel_t, pixel_t, bits_per_pixel> << <blocks, threads >> > ((vpixel_t*)pDst,
         (const vpixel_t*)pSrc[0], (const vpixel_t*)pSrc[1], (const vpixel_t*)pSrc[2],
         pitch4, width4, height, lut, mask);
      DEBUG_SYNC;
      break;
   }
}

void lut_cuda_16(int bits_per_pixel, int num_input, uint16_t *pDst, const uint16_t * const *pSrc, int pitch, int width, int height, const uint16_t* lut, PNeoEnv env)
{
   switch (bits_per_pixel) {
   case 10: return lut_cuda<ushort4, uint16_t, 8>(num_input, pDst, pSrc, pitch, width, height, lut, env);
   case 12: return lut_cuda<ushort4, uint16_t, 8>(num_input, pDst, pSrc, pitch, width, height, lut, env);
   case 14: return lut_cuda<ushort4, uint16_t, 8>(num_input, pDst, pSrc, pitch, width, height, lut, env);
   case 16: return lut_cuda<ushort4, uint16_t, 8>(num_input, pDst, pSrc, pitch, width, height, lut, env);
   }
}

void lut_cuda(int bits_per_pixel, int num_input, uint8_t *pDst, const uint8_t * const *pSrc, int pitch, int width, int height, const void* lut, PNeoEnv env)
{
   if (bits_per_pixel == 8) {
      return lut_cuda<uchar4, uint8_t, 8>(num_input, pDst, pSrc, pitch, width, height, (const uint8_t*)lut, env);
   }
   else {
      return lut_cuda_16(bits_per_pixel, num_input,
         (uint16_t*)pDst, (const uint16_t * const *)pSrc, pitch, width, height, (const uint16_t*)lut, env);
   }
}
