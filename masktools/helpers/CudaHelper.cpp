
#include "DeviceLocalData.cpp"

void OnCudaError(cudaError_t err) {
#if 1 // デバッグ用（本番は取り除く）
   printf("[CUDA Error] %s (code: %d)\n", cudaGetErrorString(err), err);
#endif
}
