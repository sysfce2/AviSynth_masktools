#ifndef __Common_Avs2x_Filter_H__
#define __Common_Avs2x_Filter_H__

#include "EnvCommon.h"
#include "params.h"
#include "../common/constraints/constraints.h"
#include <avs/cpuid.h>

namespace Filtering { namespace Avisynth2x {

static CpuFlags AvsToInternalCpuFlags(int avsCpuFlags) {
  int flags = CPU_NONE;
  if (avsCpuFlags & CPUF_AVX2) flags |= CPU_AVX2;
  if (avsCpuFlags & CPUF_AVX) flags |= CPU_AVX;
  if (avsCpuFlags & CPUF_SSE4_2) flags |= CPU_SSE4_2;
  if (avsCpuFlags & CPUF_SSE4_1) flags |= CPU_SSE4_1;
  if (avsCpuFlags & CPUF_SSSE3) flags |= CPU_SSSE3;
  if (avsCpuFlags & CPUF_SSE3) flags |= CPU_SSE3;
  if (avsCpuFlags & CPUF_SSE2) flags |= CPU_SSE2;
  return flags;
}

static int GetDeviceType(const ::PClip& clip)
{
    int devtypes = (clip->GetVersion() >= 5) ? clip->SetCacheHints(CACHE_GET_DEV_TYPE, 0) : 0;
    if (devtypes == 0) {
        return DEV_TYPE_CPU;
    }
    return devtypes;
}

template<class T>
class Filter : public GenericVideoFilter
{
    T _filter;
    Signature signature;
    int inputConfigSize; // to prevent MT static init problems with /Zc:threadSafeInit- settings for WinXP

    static AVSValue __cdecl _create(AVSValue args, void *user_data, IScriptEnvironment *env)
    {
        UNUSED(user_data);
        return new Filter<T>(args[0].AsClip(), GetParameters(args, T::filter_signature(), env), env);
    }
public:
    Filter(::PClip child, const Parameters &parameters, IScriptEnvironment *env) : _filter(parameters, AvsToInternalCpuFlags(env->GetCPUFlags())), GenericVideoFilter(child), signature(T::filter_signature())
    {
        inputConfigSize = (::IsCUDA(env) ? _filter.input_configuration() : _filter.input_configuration_cuda()).size();
        // When the above line is missing, problems kick in _filter.get_frame(n, destination, env)
        // Why: "input_configuration" has static initializer that has problems in multithreaded environment
        // when the XP compatible /Zc:threadSafeInit- switch is used for compiling in Visual Studio
        // https://docs.microsoft.com/en-us/cpp/build/reference/zc-threadsafeinit-thread-safe-local-static-initialization
        // In non-threadSafeInit mode (XP) when get_frame is called in multithreaded environment,
        // the initialization is started in thread#1 and at specific timing conditions (e.g. debug mode is not OK) is not finished yet when
        // thread#2 also calls into input_configuration().size().
        // Real life problems: the proper size value is "2", but in thread#2 still "0" is reported!
        // This resulted in zero sized local PVideoFrame array to be allocated, but later, when the initialization
        // is finished in an other thread, size() turnes into "2". It needs only some 1/10000th seconds, but the problem is there by then.
        // This zero sized array is then indexed with the proper size of "2" from 0..1 -> Access Violation
        // Debuglog: Masktools2 Getframe #1
        //  Masktools2 Getframe #0
        //  Masktools2 Getframe 0, clipcount = 0 // should be 2!!
        //  Masktools2 Getframe 0, clipcount2 = 2 // meanwhile the init was done in the background, we get 2 which is correct
        //  Masktools2 Getframe 1, clipcount = 2
        if (_filter.is_error())
        {
            env->ThrowError((signature.getName() + " : " + _filter.get_error()).c_str());
        }
    }

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env)
    {
        bool is_in_place = !::IsCUDA(env) && _filter.is_in_place();
        PVideoFrame dst = is_in_place ? child->GetFrame(n, env) : env->NewVideoFrame(vi);

        if (is_in_place) {
            env->MakeWritable(&dst);
        }

        Frame<Byte> destination = dynamic_cast<Clip *>((Filtering::Clip *)_filter.get_childs()[0].get())->ConvertTo<Byte>(dst);

        _filter.get_frame(n, destination, env);

        return dst;
    }

    int __stdcall SetCacheHints(int cachehints, int frame_range) override {
        if (cachehints == CACHE_GET_DEV_TYPE && _filter.is_cuda_available()) {
            return GetDeviceType(child) & (DEV_TYPE_CPU | DEV_TYPE_CUDA);
        }
        else if (cachehints == CACHE_GET_MTMODE) {
            return MT_NICE_FILTER;
        }
        return 0;
    }

    static void create(IScriptEnvironment *env)
    {
#ifdef MT_DUAL_SIGNATURES
      String signature1 = SignatureToString(T::filter_signature(), true); // integer
      env->AddFunction(T::filter_signature().getName().c_str(), signature1.c_str(), _create, NULL);
      // order is important! 
      String signature2 = SignatureToString(T::filter_signature(), false); // float
      if (signature1 != signature2)
        env->AddFunction(T::filter_signature().getName().c_str(), signature2.c_str(), _create, NULL);
      // compatibility: alternative parameter list, int and float
      // or else Avisynth would find the function from another, earlier loaded masktools2 version
#else
      String signature = SignatureToString(T::filter_signature(), false); // float
      env->AddFunction(T::filter_signature().getName().c_str(), signature.c_str(), _create, NULL);
#endif
    }
};

} } // namespace Avisynth2x, Filtering

#endif
