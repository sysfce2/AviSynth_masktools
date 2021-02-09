Normally masktools is built as a referenced github subproject of AvisynthCUDAFilters.

In order to successfully build masktools 'cuda' branch separately, 
some files should be copied here, into the common_cuda folder.

From: 
https://github.com/pinterf/AviSynthCUDAFilters/tree/master/common
(Originally by Nekopanda: https://github.com/nekopanda/AviSynthCUDAFilters/tree/master/common)

Files are shared between AviSynthCUDAFilters project items.

EnvCommon.h
Copy.h
Frame.h
DeviceLocalData.cpp
VectorFunctions.cuh
KMV.h
ReduceKernel.cuh
DeviceLocalData.h
DebugWriter.h
DebugWriter.cpp
CommonFunctions.h
Copy.cu