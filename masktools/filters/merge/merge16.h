#ifndef __Mt_Merge16_H__
#define __Mt_Merge16_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge16 {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight);

extern Processor *merge16_c_stacked;
extern Processor *merge16_luma_420_c_stacked;

extern Processor *merge16_sse2_stacked;
extern Processor *merge16_sse4_1_stacked;

extern Processor *merge16_luma_420_sse2_stacked;
extern Processor *merge16_luma_420_ssse3_stacked; 
extern Processor *merge16_luma_420_sse4_1_stacked;

extern Processor *merge16_c_interleaved;
extern Processor *merge16_luma_420_c_interleaved;
extern Processor *merge16_luma_422_c_interleaved;

extern Processor *merge16_sse2_interleaved;
extern Processor *merge16_sse4_1_interleaved;

extern Processor *merge16_luma_420_sse2_interleaved;
extern Processor *merge16_luma_420_ssse3_interleaved; 
extern Processor *merge16_luma_420_sse4_1_interleaved;

extern Processor *merge16_luma_422_sse2_interleaved;
extern Processor *merge16_luma_422_ssse3_interleaved;
extern Processor *merge16_luma_422_sse4_1_interleaved;

class Merge16 : public MaskTools::Filter
{

   bool use_luma;
   ProcessorList<Processor> processors;
   ProcessorList<Processor> chroma_processors;

protected:

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        if (use_luma && (nPlane > 0)) {
          if (!(width_ratios[1][C] == 1 && width_ratios[1][C] == 1)) {
            // 420 or 422
            chroma_processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
                    frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
                    frames[1].plane(0).data(), frames[1].plane(0).pitch(),
                    dst.width(), dst.height());
            }
            else {
                processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
                    frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
                    frames[1].plane(0).data(), frames[1].plane(0).pitch(),
                    dst.width(), dst.height());
            }
        }
        else {
            processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
                frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
                frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
                dst.width(), dst.height());
        }
    }

public:
   Merge16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      use_luma = parameters["luma"].toBool();

      bool is420 = width_ratios[1][C] == 2 && height_ratios[1][C] == 2;
      bool is422 = width_ratios[1][C] == 2 && height_ratios[1][C] == 1;
     
      // PF: for greyscale: graceful fallback to chromaless operation
      if (plane_counts[C] == 1) {
        use_luma = false;
        operators[1] = operators[2] = NONE;
      }

      if (use_luma) {
        /*
        if ((width_ratios[1][C] != 2 || height_ratios[1][C] != 2) && (width_ratios[1][C] != 1 || height_ratios[1][C] != 1)) {
        error = "\"luma\" is unsupported in 422";
        return;
        }
        */
          auto c1 = childs[0]->colorspace();
          auto c2 = childs[1]->colorspace();
          if ((width_ratios[1][c1] != width_ratios[1][c2]) || (height_ratios[1][c1] != height_ratios[1][c2])) {
              error = "clips should have identical colorspace";
              return;
          }

          /* if "luma" is set, we force the chroma processing. Much more handy */
          operators[1] = operators[2] = PROCESS;
          /* no need to change U/V default processing, because of in place filter */
      } else if (operators[1] == PROCESS || operators[2] == PROCESS) {
          auto mask_colorspace = childs[2]->colorspace();
          if (plane_counts[mask_colorspace] == 1) {
              error = "Mask cannot be greyscale when chroma mode is PROCESS";
          }
      }

      if (parameters["stacked"].toBool()) {
          if (is422 && use_luma) {
            error = "stacked and use_luma on 422 not supported";
            return;
          }
          /* add the processors */
          processors.push_back( Filtering::Processor<Processor>( merge16_c_stacked, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
          processors.push_back( Filtering::Processor<Processor>( merge16_sse2_stacked, Constraint( CPU_SSE2, 1, 1, 1, 1 ), 1 ) );
          processors.push_back( Filtering::Processor<Processor>( merge16_sse4_1_stacked, Constraint( CPU_SSE4_1, 1, 1, 1, 1 ), 2 ) );

          /* add the chroma processors */
          chroma_processors.push_back( Filtering::Processor<Processor>( merge16_luma_420_c_stacked, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
          chroma_processors.push_back( Filtering::Processor<Processor>( merge16_luma_420_sse2_stacked, Constraint( CPU_SSE2, 1, 1, 1, 1 ), 1 ) );
          chroma_processors.push_back( Filtering::Processor<Processor>( merge16_luma_420_ssse3_stacked, Constraint( CPU_SSSE3, 1, 1, 1, 1 ), 2 ) );
          chroma_processors.push_back( Filtering::Processor<Processor>( merge16_luma_420_sse4_1_stacked, Constraint( CPU_SSE4_1, 1, 1, 1, 1 ), 3 ) );
      } else {
          /* add the processors */
          processors.push_back( Filtering::Processor<Processor>( merge16_c_interleaved, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
          processors.push_back( Filtering::Processor<Processor>( merge16_sse2_interleaved, Constraint( CPU_SSE2, 1, 1, 1, 1 ), 1 ) );
          processors.push_back( Filtering::Processor<Processor>( merge16_sse4_1_interleaved, Constraint( CPU_SSE4_1, 1, 1, 1, 1 ), 2 ) );

          /* add the chroma processors */
          // used only for 420 and 422
          if (is420) {
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_420_c_interleaved, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_420_sse2_interleaved, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_420_ssse3_interleaved, Constraint(CPU_SSSE3, 1, 1, 1, 1), 2));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_420_sse4_1_interleaved, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
          }
          else { // 422
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_422_c_interleaved, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_422_sse2_interleaved, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_422_ssse3_interleaved, Constraint(CPU_SSSE3, 1, 1, 1, 1), 2));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge16_luma_422_sse4_1_interleaved, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
          }
      }
   }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_merge16";

      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( false, "luma" ) );
      signature.add( Parameter( false, "stacked" ) );

      return add_defaults( signature );
   }
};

} } } } // namespace Merge, Filters, MaskTools, Filtering

#endif