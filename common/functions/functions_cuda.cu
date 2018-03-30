#include "functions.h"
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

using namespace Filtering;

template <typename vpixel_t, typename pixel_t>
__global__ void kl_fill(vpixel_t* dst, pixel_t v, int width, int height, int pitch)
{
   int x = threadIdx.x + blockIdx.x * blockDim.x;
   int y = threadIdx.y + blockIdx.y * blockDim.y;

   if (x < width && y < height) {
      dst[x + y * pitch] = VHelper<vpixel_t>::make(v);
   }
}

void Functions::memset_plane_cuda(Byte *ptr, ptrdiff_t pitch, int width, int height, Byte value, IScriptEnvironment* env)
{
   UNUSED(env);
   int p4 = (int)pitch >> 2;
   int w4 = (int)width >> 2;
   dim3 threads(32, 16);
   dim3 blocks(nblocks(w4, threads.x), nblocks(height, threads.y));
   kl_fill<<<blocks, threads>>>((uchar4*)ptr, value, w4, height, p4);
   DEBUG_SYNC;
}

void Functions::memset_plane_16_cuda(Byte *ptr, ptrdiff_t pitch, int width, int height, Word value, IScriptEnvironment* env)
{
   UNUSED(env);
   int p4 = (int)pitch >> 3;
   int w4 = (int)width >> 2;
   dim3 threads(32, 16);
   dim3 blocks(nblocks(w4, threads.x), nblocks(height, threads.y));
   kl_fill << <blocks, threads >> >((ushort4*)ptr, value, w4, height, p4);
   DEBUG_SYNC;
}

void Functions::memset_plane_32_cuda(Byte *ptr, ptrdiff_t pitch, int width, int height, float value, IScriptEnvironment* env)
{
   UNUSED(env);
   int p4 = (int)pitch >> 4;
   int w4 = (int)width >> 2;
   dim3 threads(32, 16);
   dim3 blocks(nblocks(w4, threads.x), nblocks(height, threads.y));
   kl_fill << <blocks, threads >> >((float4*)ptr, value, w4, height, p4);
   DEBUG_SYNC;
}

template <typename pixel_t>
__global__ void kl_copy(
   pixel_t* dst, int dst_pitch, const pixel_t* __restrict__ src, int src_pitch, int width, int height)
{
   int x = threadIdx.x + blockIdx.x * blockDim.x;
   int y = threadIdx.y + blockIdx.y * blockDim.y;

   if (x < width && y < height) {
      dst[x + y * dst_pitch] = src[x + y * src_pitch];
   }
}

void Functions::copy_plane_cuda(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int rowsize, int height, IScriptEnvironment* env)
{
   UNUSED(env);
   int sp4 = (int)src_pitch >> 2;
   int dp4 = (int)dst_pitch >> 2;
   int w4 = rowsize >> 2;
   dim3 threads(32, 16);
   dim3 blocks(nblocks(w4, threads.x), nblocks(height, threads.y));
   kl_copy << <blocks, threads >> >((uchar4*)pDst, dp4, (uchar4*)pSrc, sp4, w4, height);
   DEBUG_SYNC;
}
