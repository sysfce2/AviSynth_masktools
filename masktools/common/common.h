#ifndef __Mt_COMMON_H__
#define __Mt_COMMON_H__

#include "../../common/utils/utils.h"

#ifndef MT_FORCEINLINE
#if defined(__clang__)
// Check clang first. clang-cl also defines __MSC_VER
// We set MSVC because they are mostly compatible
#   define CLANG
#if defined(_MSC_VER)
#   define MSVC
#   define MT_FORCEINLINE __attribute__((always_inline)) inline
#else
#   define MT_FORCEINLINE __attribute__((always_inline)) inline
#endif
#elif   defined(_MSC_VER)
#   define MSVC
#   define MSVC_PURE
#   define MT_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#   define GCC
#   define MT_FORCEINLINE __attribute__((always_inline)) inline
#else
#   define MT_FORCEINLINE inline
#   undef __forceinline
#   define __forceinline inline
#endif 

#endif

#if defined(GCC)
#include <stdlib.h>
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free(ptr) free(ptr)
#endif


#endif // __Mt_COMMON_H__
