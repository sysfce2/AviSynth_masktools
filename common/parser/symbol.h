#ifndef __Mt_Symbol_H__
#define __Mt_Symbol_H__

#include "../utils/utils.h"
#include <deque>

//because ICC is smart enough on its own and force inlining actually makes it slower
#ifdef __INTEL_COMPILER
#define MT_FORCEINLINE inline
#else
#define MT_FORCEINLINE __forceinline
#endif

namespace Filtering { namespace Parser {

class Symbol {
public:
   typedef enum {
      NUMBER,
      OPERATOR,
      FUNCTION,
      TERNARY,
      VARIABLE_X,
      VARIABLE_Y,
      VARIABLE_Z,
      // 4th variable since v2.2.1
      VARIABLE_A,

      VARIABLE_BITDEPTH, // automatic silent parameter of the expression v2.2.1
      VARIABLE_SCRIPT_BITDEPTH, // base bit depth of the values to scale v2.2.2, settable with i8..i16,f32 keywords

      // special adaptive constants filled by bitdepth
      VARIABLE_RANGE_HALF,
      VARIABLE_RANGE_MAX,
      VARIABLE_RANGE_SIZE,
      VARIABLE_YMIN,
      VARIABLE_YMAX,
      VARIABLE_CMIN,
      VARIABLE_CMAX,

      FUNCTION_WITH_BITDEPTH_AS_AUTOPARAM,
      FUNCTION_CONFIG_SCRIPT_BITDEPTH,

      UNDEFINED

   } Type;

public:

   Type type;
   String value;
   String value2;
   int nParameter;
   double dValue;
   typedef double (*Process)(double x, double y, double z);
   Process process;

private:

private:
public:

   Symbol();
   Symbol(String value, Type type, int nParameter, Process process);
   Symbol(String value, String value2, Type type, int nParameter, Process process);
   Symbol(String value, double dValue, Type type, int nParameter, Process process);

   void setValue(double dValue);
   double getValue(double x, double y, double z) const;

   static Symbol Addition;
   static Symbol Multiplication;
   static Symbol Division;
   static Symbol Substraction;
   static Symbol Power;
   static Symbol Modulo;
   static Symbol Interrogation;
   static Symbol Equal;
   static Symbol Equal2;
   static Symbol NotEqual;
   static Symbol Inferior;
   static Symbol InferiorStrict;
   static Symbol Superior;
   static Symbol SuperiorStrict;
   static Symbol And;
   static Symbol Or;
   static Symbol AndNot;
   static Symbol Xor;
   static Symbol AndUB;
   static Symbol OrUB;
   static Symbol XorUB;
   static Symbol NegateUB;
   static Symbol PosShiftUB;
   static Symbol NegShiftUB;
   static Symbol AndSB;
   static Symbol OrSB;
   static Symbol XorSB;
   static Symbol NegateSB;
   static Symbol PosShiftSB;
   static Symbol NegShiftSB;
   static Symbol Pi;
   static Symbol X;
   static Symbol Y;
   static Symbol Z;
   // v2.2.1
   // 4th variable: a
   static Symbol A;
   // Auto bitdepth conv
   static Symbol BITDEPTH;
   static Symbol SCRIPT_BITDEPTH;
   // special adaptive constants filled by bitdepth
   static Symbol RANGE_HALF;
   static Symbol RANGE_MAX;
   static Symbol RANGE_SIZE;
   static Symbol YMIN;
   static Symbol YMAX;
   static Symbol CMIN;
   static Symbol CMAX;

   static Symbol Cos;
   static Symbol Sin;
   static Symbol Tan;
   static Symbol Log;
   static Symbol Abs;
   static Symbol Exp;
   static Symbol Acos;
   static Symbol Atan;
   static Symbol Asin;
   static Symbol Round;
   static Symbol Clip;
   static Symbol Min;
   static Symbol Max;
   static Symbol Ceil;
   static Symbol Floor;
   static Symbol Trunc;
   // auto bit depth conversions
   static Symbol ScaleByShift;
   static Symbol ScaleByStretch;
   // script bit depth setters
   static Symbol SetScriptBitDepthI8;
   static Symbol SetScriptBitDepthI10;
   static Symbol SetScriptBitDepthI12;
   static Symbol SetScriptBitDepthI14;
   static Symbol SetScriptBitDepthI16;
   static Symbol SetScriptBitDepthF32;
   static Symbol SetFloatToClampUseI8Range;
   static Symbol SetFloatToClampUseI10Range;
   static Symbol SetFloatToClampUseI12Range;
   static Symbol SetFloatToClampUseI14Range;
   static Symbol SetFloatToClampUseI16Range;
   static Symbol SetFloatToClampUseF32Range;
   static Symbol SetFloatToClampUseF32Range_2;

};

class Context {

   Symbol *pSymbols;
   int nSymbols;
   int nPos;

   double x, y, z, a;
   int bitdepth; // bit depth
   int sbitdepth; // source bit depth of values to scale
   
   // helpers for float input autoscales
   // 0: none
   // 8, 10, 12, 14, 16: scale input 0..1 float to this range
   // 32: no scaling needed but 0..1 clamping is still done before returning the result
   int float_autoscale_bitdepth;
   double float_input_scalefactor;
   double float_input_invscalefactor;
   
   double rec_compute();
   String rec_infix();

public:
   
   Context(const std::deque<Symbol> &expression);

   ~Context();

   bool check();
   // v2.2.1: variable a
   double compute(double x, double y = -1.0f, double z = -1.0f, double a = -1.0f, int bitdepth = 8);
   String infix();
   Byte compute_byte(double _x, double _y = -1.0f, double _z = -1.0f, double _a = -1.0f, int _bitdepth = 8) { return clip<Byte, double>( compute(_x, _y, _z, _a, _bitdepth) ); } // byte: default 8 bit
   
   template<int bits_per_pixel>
   Word compute_word_x(int _x) {
     if(bits_per_pixel == 16)
       return clip<Word, double>(compute(_x, -1.0, -1.0, -1.0, bits_per_pixel));
     else
       return min(clip<Word, double>(compute(_x, -1.0, -1.0, -1.0, bits_per_pixel)), (Word)((1 << bits_per_pixel) - 1));
   }

   template<int bits_per_pixel>
   Word compute_word_xy(int _x, int _y) {
     if (bits_per_pixel == 16)
       return clip<Word, double>(compute(_x, _y, -1.0, -1.0, bits_per_pixel));
     else
       return min(clip<Word, double>(compute(_x, _y, -1.0, -1.0, bits_per_pixel)), (Word)((1 << bits_per_pixel) - 1));
   }

   template<int bits_per_pixel>
   Word compute_word_xyz(int _x, int _y, int _z) {
     if (bits_per_pixel == 16)
       return clip<Word, double>(compute(_x, _y, _z, -1.0, bits_per_pixel));
     else
       return min(clip<Word, double>(compute(_x, _y, _z, -1.0, bits_per_pixel)), (Word)((1 << bits_per_pixel) - 1));
   }

   template<int bits_per_pixel>
   Word compute_word_xyza(int _x, int _y, int _z, int _a) {
     if (bits_per_pixel == 16)
       return clip<Word, double>(compute(_x, _y, _z, _a, bits_per_pixel));
     else
       return min(clip<Word, double>(compute(_x, _y, _z, _a, bits_per_pixel)), (Word)((1 << bits_per_pixel) - 1));
   }

   Word compute_word(double _x, double _y = -1.0f, double _z = -1.0f, double _a = -1.0f, int _bitdepth = 16) { return clip<Word, double>(compute(_x, _y, _z, _a, _bitdepth)); } // word: default 16 bit
   Word compute_word_safe(double _x, double _y = -1.0f, double _z = -1.0f, double _a = -1.0f, int _bitdepth = 16)
   { 
     return min(clip<Word, double>( compute(_x, _y, _z, _a, _bitdepth) ), (Word)((1 << _bitdepth) - 1));
   } // clamp valid range for 10-14 bits

   Float compute_float(double _x, double _y = -1.0f, double _z = -1.0f, double _a = -1.0f, int _bitdepth = 32) 
   { 
     // todo: make it a bit faster if less real parameter is provided
     float result;
     if(float_autoscale_bitdepth == 0 || _bitdepth != 32)
       return (float)(compute(_x, _y, _z, _a, _bitdepth));
     else {
       // input bitdepth = 32 and autoScale mode is on
       if (float_autoscale_bitdepth == 32) {
         // scaling target is 32 bit, no scaling needed, but clamping is still on
         return max(min((float)((compute(_x, _y, _z, _a, _bitdepth))), 1.0f), 0.0f);
       }

       // we pass the fake bitdepth (8..16) float_autoscale_bitdepth, scale up input, scale down result
       result = (float)(float_input_invscalefactor*(compute(float_input_scalefactor*_x, float_input_scalefactor*_y, float_input_scalefactor*_z, float_input_scalefactor*_a, float_autoscale_bitdepth)));
       result = max(min(result, 1.0f), 0.0f);
       return result;
     }
   }
};

} } // namespace Parser, Filtering

#endif
