#ifndef __Mt_Base_H__
#define __Mt_Base_H__

#include "../../../common/functions/functions.h"
#include "../../../common/params/params.h"
#include "../../../common/parser/parser.h"
#include "../../../common/utils/utils.h"
#include "../../common/params/params.h"
#include "../../common/clip/inputconfig.h"

namespace Filtering { namespace MaskTools { 

enum class FilterProcessingType {
    INPLACE,
    CHILD
};

class Filter {

protected:
    ClipArray childs;
    Parameters parameters;

    int nWidth, nHeight;
    Colorspace C;

    String error;
    CpuFlags flags;
    Operator operators[3];

    bool inPlace_;
    int nXOffset, nYOffset, nXOffsetUV, nYOffsetUV;
    int nCoreWidth, nCoreHeight, nCoreWidthUV, nCoreHeightUV;

   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[3], const Constraint constraints[3]) = 0;
   virtual InputConfiguration &input_configuration() const = 0;

   static Signature &add_defaults(Signature &signature)
   {
      signature.add( Parameter( 3.0f, "Y" ) );
      signature.add( Parameter( 1.0f, "U" ) );
      signature.add( Parameter( 1.0f, "V" ) );
      signature.add( Parameter( Value( String( "" ) ), "chroma" ) );
      signature.add( Parameter( 0, "offX" ) );
      signature.add( Parameter( 0, "offY" ) );
      signature.add( Parameter( -1, "w" ) );
      signature.add( Parameter( -1, "h" ) );
      signature.add( Parameter( true, "sse2" ) );
      signature.add( Parameter( true, "sse3" ) );
      signature.add( Parameter( true, "ssse3" ) );
      signature.add( Parameter( true, "sse4" ) );

      return signature;
   }

public:

    Filter(const Parameters &parameters, FilterProcessingType processingType) :
        parameters(parameters),
        flags(Functions::get_cpu_flags()),
        inPlace_(processingType == FilterProcessingType::INPLACE),
        nXOffset(parameters["offx"].toInt()),
        nYOffset(parameters["offy"].toInt()),
        nCoreWidth(parameters["w"].toInt()),
        nCoreHeight((parameters["stacked"].is_defined() && parameters["stacked"].toBool() && parameters["h"].toInt()>=0) ? (2 * parameters["h"].toInt()) : parameters["h"].toInt())
    {
        for (auto &param: parameters) {
            if (param.getType() == TYPE_CLIP) {
                childs.push_back(param.getValue().toClip());
            }
        }

        assert(!childs.empty());

        nWidth = childs[0]->width();
        nHeight = childs[0]->height();
        C = childs[0]->colorspace();

        operators[0] = Operator((float)parameters["Y"].toFloat()); // prepare, float for memset
        operators[1] = Operator((float)parameters["U"].toFloat());
        operators[2] = Operator((float)parameters["V"].toFloat());

        if (nXOffset < 0 || nXOffset > nWidth) nXOffset = 0;
        if (nYOffset < 0 || nYOffset > nHeight) nYOffset = 0;
        if (nXOffset + nCoreWidth  > nWidth  || nCoreWidth  < 0) nCoreWidth = nWidth - nXOffset;
        if (nYOffset + nCoreHeight > nHeight || nCoreHeight < 0) nCoreHeight = nHeight - nYOffset;

        if (parameters["chroma"].is_defined())
        {
            /* overrides chroma channel operators according to the "chroma" string */
            String chroma = parameters["chroma"].toString();

            if (chroma == "process")
                operators[1] = operators[2] = PROCESS;
            else if (chroma == "copy")
                operators[1] = operators[2] = COPY;
            else if (chroma == "copy first")
                operators[1] = operators[2] = COPY;
            else if (chroma == "copy second")
                operators[1] = operators[2] = COPY_SECOND;
            else if (chroma == "copy third")
                operators[1] = operators[2] = COPY_THIRD;
            else
                operators[1] = operators[2] = Operator(MEMSET, std::stof(chroma.c_str())); // atoi(chroma.c_str())
        }

        /* checks the operators */
        for (int i = 0; i < 3; i++)
        {
            if (operators[i] == COPY_THIRD && childs.size() < 3)
                operators[i] = COPY_SECOND;
            if (operators[i] == COPY_SECOND && childs.size() < 2)
                operators[i] = COPY;
        }

        if (is_in_place())
        {
            /* in place filters copy differently */
            for (int i = 0; i < 3; i++)
            {
                switch (operators[i].getMode())
                {
                case COPY: operators[i] = NONE; break;
                case COPY_SECOND: operators[i] = COPY; break;
                case COPY_THIRD: operators[i] = COPY_SECOND; break;
                }
            }
        }

        /* effective modes */
        print(LOG_DEBUG, "modes : %i %i %i\n", operators[0].getMode(), operators[1].getMode(), operators[2].getMode());

        /* cpu flags */
        if (!parameters["sse2"].toBool()) flags &= ~CPU_SSE2;
        if (!parameters["sse3"].toBool()) flags &= ~CPU_SSE3;
        if (!parameters["ssse3"].toBool()) flags &= ~CPU_SSSE3;
        if (!parameters["sse4"].toBool()) {
            flags &= ~CPU_SSE4_1;
            flags &= ~CPU_SSE4_2;
        }

        print(LOG_DEBUG, "using cpu flags : 0x%x\n", flags);

        /* chroma offsets and box */
        if (C != COLORSPACE_Y8 && C != COLORSPACE_Y10 && C != COLORSPACE_Y12 && C != COLORSPACE_Y14 && C != COLORSPACE_Y16 && C != COLORSPACE_Y32 && C != COLORSPACE_NONE)
        {
          // also for planar RGB, naming is a bit confusing yet
            nXOffsetUV = nXOffset / width_ratios[1][C];
            nYOffsetUV = nYOffset / height_ratios[1][C];
            nCoreWidthUV = (nXOffset + nCoreWidth) / width_ratios[1][C] - nXOffsetUV;
            nCoreHeightUV = (nYOffset + nCoreHeight) / height_ratios[1][C] - nYOffsetUV;
        }

        /* effective offset */
        print(LOG_DEBUG, "offset : %i %i, width x height : %i x %i\n", nXOffset, nYOffset, nCoreWidth, nCoreHeight);

        /* check the colorspace */
        if (C == COLORSPACE_NONE)
            error = "unsupported colorspace. masktools only support planar YUV colorspaces (YV12, YV16, YV24)";
    }

    void process_plane(int n, const Plane<Byte> &output_plane, int nPlane, const Constraint constraints[3], const Frame<const byte> frames[3])
    {
        bool isStacked = parameters["stacked"].is_defined() && parameters["stacked"].toBool();

        switch (operators[nPlane].getMode())
        {
        case COPY:
          if (isStacked) {
            // in two parts, there may be sub-window
            // msb
            Functions::copy_plane(output_plane.data(), output_plane.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              output_plane.width(), output_plane.height() / 2);
            // lsb
            Functions::copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
              frames[0].plane(nPlane).data() + frames[0].plane(nPlane).pitch() * frames[0].plane(nPlane).origheight() / 2, frames[0].plane(nPlane).pitch(),
              output_plane.width(), output_plane.height() / 2);
          }
          else {
            Functions::copy_plane(output_plane.data(), output_plane.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              output_plane.width()*output_plane.pixelsize(), output_plane.height()); // rowsize = width*pixelsize
          }
          break;
        case COPY_SECOND:
            if (isStacked) {
              // msb
              Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2);
              // lsb
              Functions::copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
                frames[1].plane(nPlane).data() + frames[1].plane(nPlane).pitch() * frames[1].plane(nPlane).origheight() / 2, frames[1].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2);
            } else {
              Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
                output_plane.width()*output_plane.pixelsize(), output_plane.height());
            }
            break;
        case COPY_THIRD:
            if (isStacked) {
              // msb
              Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2);
              // lsb
              Functions::copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
                frames[2].plane(nPlane).data() + frames[2].plane(nPlane).pitch() * frames[2].plane(nPlane).origheight() / 2, frames[2].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2);
            }
            else {
              Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
                output_plane.width()*output_plane.pixelsize(), output_plane.height());
            }
            break;
        case MEMSET:
            switch (output_plane.pixelsize()) {
            case 1:
              if (isStacked) {
                // in two parts, there may be sub-window
                Word val = static_cast<Word>(operators[nPlane].value());
                // msb
                Functions::memset_plane(output_plane.data(), output_plane.pitch(),
                  output_plane.width(), output_plane.height()/2,
                  static_cast<Byte>(val >> 8));
                // lsb
                Functions::memset_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
                  output_plane.width(), output_plane.height() / 2,
                  static_cast<Byte>(val & 0xFF));
              } else {
                Functions::memset_plane(output_plane.data(), output_plane.pitch(),
                  output_plane.width(), output_plane.height(), // memset needs real width
                  static_cast<Byte>(operators[nPlane].value()));
              }
              break;
            case 2: // 16 bit
              Functions::memset_plane_16(output_plane.data(), output_plane.pitch(),
                output_plane.width(), output_plane.height(),
                static_cast<Word>(operators[nPlane].value()));
              break;
            case 4: // 32 bit/float
              Functions::memset_plane_32(output_plane.data(), output_plane.pitch(),
                output_plane.width(), output_plane.height(),
                static_cast<float>(operators[nPlane].value_f()));
              break;
            }
        break;
        case PROCESS:
            process(n, output_plane, nPlane, frames, constraints);
            break;
        case NONE:
        default: break;
        }
    }

    virtual Frame<Byte> get_frame(int n, const Frame<Byte> &output_frame, IScriptEnvironment *env)
    {
        Frame<Byte> output = output_frame.offset(nXOffset, nYOffset, nCoreWidth, nCoreHeight);

        Frame<const Byte> frames[3];
        Constraint constraints[3];

        // PF 170211: Issue: volatile pointers
        // old: get_const_frame requested PVideoFrame frame = clip->GetFrame to a local variable, 
        //      the ConvertTo method then stored plane pointers obtained by frame->ReadPtr, then 
        //      the function returned and the frame's destructor was called.
        //      So the plane pointers just requested were no longer valid!
        //      As the memory was not referenced anymore, Avisynth reassigned the same area to the
        //      next frame for GetFrame. The third clip parameter was given the same
        //      memory are as the second clip used.
        // new: Child Avisynth frames (additional clip parameters of mt_xxx filters) now are stored in an array, 
        //      as PVideoFrame frames are reference counted, the read pointers
        //      are still valid here, and processing can be done with valid pointers
        //      Method get_const_frame returns the result PVideoFrame after clip's GetFrame() and 
        //           caller has to store it until the processing of the planes is done.
        // 
        // script failed: 
        /* blankclip(128, 1024, 1024, "yv24")
           horiz_gradient = masktools_mt_lutspa(expr = "x 255 *", u = -128, v = -128)
           vert_gradient = masktools_mt_lutspa(expr = "y 255 *", u = -128, v = -128).scriptclip("""tweak(bright=rand(255))""")
           masktools_mt_merge(vert_gradient, horiz_gradient, masktools_mt_lut(y = -255), true)
           #expected: mask = 255 : horiz_gradient
           #reality : mask = 255 results in full white instead of horiz_gradient
           #reason : horiz_gradient points to the same memory area as the mask, due to a bug in masktools b2.
           #         the second clip's video frame is released before the plane processing and
           #         under specific timing and circumstances the mask clip is getting the same video pointers
           #         as overlay clip.
         */
        int clipcount = int(input_configuration().size());
        PVideoFrame *tmp_videoframes = new PVideoFrame[clipcount]; // Fix#1 for "volatile pointers"

        for (int i = 0; i < int(input_configuration().size()); i++) {
            int childindex = input_configuration()[i].index();
            PClip &currentClip = childs[childindex];
            // Fix#2 for "volatile pointers": get back PVideoFrame GetFrame result
            frames[i] = currentClip->get_const_frame(n + input_configuration()[i].offset(), tmp_videoframes[i], env)
                .offset(nXOffset, nYOffset, nCoreWidth, nCoreHeight);
        }

        for (int i = 0; i < plane_counts[C]; i++) {
            constraints[i] = Constraint(flags, output.plane(i));
        }

        for (int i = 0; i < int(input_configuration().size()); i++) {
            for (int j = 0; j < plane_counts[frames[i].colorspace()]; j++) {
                constraints[j] = Constraint(constraints[j], frames[i].plane(j));
            }
        }

        for (int i = 0; i < plane_counts[C]; i++) {
            process_plane(n, output.plane(i), i, constraints, frames);
        }

        delete[] tmp_videoframes;

        return output_frame;
    }

    ClipArray &get_childs() { return childs; }

    String get_error() const { return error; }
    bool is_error() const { return !error.empty(); }
    bool is_in_place() const { return inPlace_; }

};

} } // namespace MaskTools, Filtering

#endif
