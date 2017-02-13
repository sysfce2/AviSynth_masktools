#ifndef __Mt_Symbol_H__
#define __Mt_Symbol_H__

#include "../utils/utils.h"
#include <deque>

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

      VARIABLE_BITDEPTH,
      // special adaptive constants filled by bitdepth
      VARIABLE_RANGE_HALF,
      VARIABLE_RANGE_MAX,
      VARIABLE_RANGE_SIZE,
      VARIABLE_YMIN,
      VARIABLE_YMAX,
      VARIABLE_CMIN,
      VARIABLE_CMAX,

      FUNCTION_WITH_B_AS_PARAM,

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
   // Auto bitdepth conv
   static Symbol BITDEPTH;
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
   static Symbol UpscaleByShift;
   static Symbol UpscaleByStretch;

};

class Context {

   Symbol *pSymbols;
   int nSymbols;
   int nPos;

   double x, y, z;
   int bitdepth; // bit depth
   double rec_compute();
   String rec_infix();

public:
   
   Context(const std::deque<Symbol> &expression);

   ~Context();

   bool check();
   double compute(double x, double y = -1.0f, double z = -1.0f, double bitdepth = 8);
   String infix();
   Byte compute_byte(double _x, double _y = -1.0f, double _z = -1.0f, double _b = 8) { return clip<Byte, double>( compute(_x, _y, _z, _b) ); } // byte: default 8 bit
   Word compute_word(double _x, double _y = -1.0f, double _z = -1.0f, double _b = 16) { return clip<Word, double>( compute(_x, _y, _z, _b) ); } // word: default 16 bit
   Float compute_float(double _x, double _y = -1.0f, double _z = -1.0f, double _b = 0) { return (float)(compute(_x, _y, _z, _b)); } // float: 1<<0 = 1.0 default base
};

} } // namespace Parser, Filtering

#endif
