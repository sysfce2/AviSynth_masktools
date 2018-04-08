#include <stdint.h>

void lut_cuda(int bits_per_pixel, int num_input,
   uint8_t *pDst, const uint8_t * const *pSrc, int pitch, int width, int height, const void* lut, PNeoEnv env);
