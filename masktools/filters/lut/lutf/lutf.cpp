#include "lutf.h"
#include "../functions.h"

using namespace Filtering;

template<bool realtime, class T>
static void frame_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t src_pitch, const Byte *lutp, Parser::Context *ctx, int width, int height)
{
    T processor("");

    processor.reset();

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            processor.add(dstp[i]);
        }
        dstp += dst_pitch;
    }

    dstp -= dst_pitch * height;

    // lutf: treat it as if the first frame was constant, result of avg/min/max, etc...
    if (realtime) {
      const Byte X = processor.finalize();
      for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
          dstp[i] = ctx->compute_byte(X,srcp[i]);
        }
        srcp += src_pitch;
        dstp += dst_pitch;
      }
    }
    else {
      const Byte *lut = lutp + (processor.finalize() << 8);

      for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
          dstp[i] = lut[srcp[i]];
        }
        srcp += src_pitch;
        dstp += dst_pitch;
      }
    }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Frame {

Processor *processors_array[NUM_MODES] = MPROCESSOR_SINGLE(frame_c, false);
//Processor *processors_array[NUM_MODES] = { &frame_c< false, Nonizer >, &frame_c< false, Averager<int> >, &frame_c< false, Minimizer >, &frame_c< false, Maximizer >, &frame_c< false, Deviater<int> >, &frame_c< false, Rangizer >, &frame_c< false, Medianizer >, &frame_c< false, MedianizerBetter<4> >, &frame_c< false, MedianizerBetter<6> >, &frame_c< false, MedianizerBetter<2> >, };

Processor *processorsCtx_array[NUM_MODES] = MPROCESSOR_SINGLE(frame_c, true);

} } } } }

