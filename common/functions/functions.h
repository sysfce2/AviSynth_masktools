#ifndef __Common_BaseFunctions_H__
#define __Common_BaseFunctions_H__

#include "EnvCommon.h"
#include "../utils/utils.h"
#include "../constraints/constraints.h"

namespace Filtering { namespace Functions {
 
void memset_plane(Byte *ptr, ptrdiff_t pitch, int width, int height, Byte value, IScriptEnvironment* env);
void memset_plane_16(Byte *ptr, ptrdiff_t pitch, int width, int height, Word value, IScriptEnvironment* env);
void memset_plane_32(Byte *ptr, ptrdiff_t pitch, int width, int height, float value, IScriptEnvironment* env);

void copy_plane(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int rowsize, int height, IScriptEnvironment* env);

void memset_plane_cuda(Byte *ptr, ptrdiff_t pitch, int width, int height, Byte value, IScriptEnvironment* env);
void memset_plane_16_cuda(Byte *ptr, ptrdiff_t pitch, int width, int height, Word value, IScriptEnvironment* env);
void memset_plane_32_cuda(Byte *ptr, ptrdiff_t pitch, int width, int height, float value, IScriptEnvironment* env);

void copy_plane_cuda(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int rowsize, int height, IScriptEnvironment* env);

CpuFlags get_cpu_flags();


}
}

#endif
