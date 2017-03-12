#include "symbol.h"
#include <math.h>

using namespace Filtering;
using namespace Filtering::Parser;

// basic math
static double addition        (double x, double y, double z) { UNUSED(z); return x + y; }
static double multiplication  (double x, double y, double z) { UNUSED(z); return x * y; }
static double division        (double x, double y, double z) { UNUSED(z); return x / y; }
static double substraction    (double x, double y, double z) { UNUSED(z); return x - y; }
static double power           (double x, double y, double z) { UNUSED(z); return pow(x, y); }
static double modulo          (double x, double y, double z) { UNUSED(z); return double(convert<Int64, double>( x ) % convert<Int64, double>( y )); }
// Ternary operator helper
static double interrogation   (double x, double y, double z) { return x > 0 ? y : z; }
// comparison
static double equal           (double x, double y, double z) { UNUSED(z); return abs(x - y) < 0.000001 ? 1 : -1; }
static double notEqual        (double x, double y, double z) { UNUSED(z); return abs(x - y) >= 0.000001 ? 1 : -1; }
static double inferior        (double x, double y, double z) { UNUSED(z); return x <= y ? 1 : -1; }
static double inferiorStrict  (double x, double y, double z) { UNUSED(z); return x < y ? 1 : -1; }
static double superior        (double x, double y, double z) { UNUSED(z); return x >= y ? 1 : -1; }
static double superiorStrict  (double x, double y, double z) { UNUSED(z); return x > y ? 1 : -1; }
// bool
static double and             (double x, double y, double z) { UNUSED(z); return x > 0 && y > 0 ? 1 : -1; }
static double or              (double x, double y, double z) { UNUSED(z); return x > 0 || y > 0 ? 1 : -1; }
static double andNot          (double x, double y, double z) { UNUSED(z); return x > 0 && y <= 0 ? 1 : -1; }
static double xor             (double x, double y, double z) { UNUSED(z); return (x > 0 && y <= 0) || (x <= 0 && y > 0)? 1 : -1; }
// Unsigned Bit arithmetic
static double andUB           (double x, double y, double z) { UNUSED(z); return double(clip<Uint64, double>(x) & clip<Uint64, double>(y)); }
static double orUB            (double x, double y, double z) { UNUSED(z); return double(clip<Uint64, double>(x) | clip<Uint64, double>(y)); }
static double xorUB           (double x, double y, double z) { UNUSED(z); return double(clip<Uint64, double>(x) ^ clip<Uint64, double>(y)); }
static double negateUB        (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(~clip<Uint64, double>(x)); }
static double posshiftUB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Uint64, double>(x) << clip<Int64, double>(y)) : double(clip<Uint64, double>(x) >> clip<Int64, double>(-y)); }
static double negshiftUB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Uint64, double>(x) >> clip<Int64, double>(y)) : double(clip<Uint64, double>(x) << clip<Int64, double>(-y)); }
// Signed Bit arithmetic
static double andSB           (double x, double y, double z) { UNUSED(z); return double(clip<Int64, double>(x) & clip<Int64, double>(y)); }
static double orSB            (double x, double y, double z) { UNUSED(z); return double(clip<Int64, double>(x) | clip<Int64, double>(y)); }
static double xorSB           (double x, double y, double z) { UNUSED(z); return double(clip<Int64, double>(x) ^ clip<Int64, double>(y)); }
static double negateSB        (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(~clip<Int64, double>(x)); }
static double posshiftSB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Int64, double>(x) << clip<Int64, double>(y)) : double(clip<Int64, double>(x) >> clip<Int64, double>(-y)); }
static double negshiftSB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Int64, double>(x) >> clip<Int64, double>(y)) : double(clip<Int64, double>(x) << clip<Int64, double>(-y)); }
// Math
static double cos             (double x, double y, double z) { UNUSED(y); UNUSED(z); return cos(x); }
static double sin             (double x, double y, double z) { UNUSED(y); UNUSED(z); return sin(x); }
static double tan             (double x, double y, double z) { UNUSED(y); UNUSED(z); return tan(x); }
static double exp             (double x, double y, double z) { UNUSED(y); UNUSED(z); return exp(x); }
static double log             (double x, double y, double z) { UNUSED(y); UNUSED(z); return log(x); }
static double mabs             (double x, double y, double z) { UNUSED(y); UNUSED(z); return abs(x); }
static double acos            (double x, double y, double z) { UNUSED(y); UNUSED(z); return acos(x); }
static double asin            (double x, double y, double z) { UNUSED(y); UNUSED(z); return asin(x); }
static double atan            (double x, double y, double z) { UNUSED(y); UNUSED(z); return atan(x); }
static double round           (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(convert<Int64, double>( x )); }
static double clip            (double x, double y, double z) { return clip<double, double>( x, y, z ); }
static double mmin             (double x, double y, double z) { UNUSED(z); return min<double>( x, y ); }
static double mmax             (double x, double y, double z) { UNUSED(z); return max<double>( x, y ); }
static double floor           (double x, double y, double z) { UNUSED(y); UNUSED(z); return floor(x); }
static double ceil            (double x, double y, double z) { UNUSED(y); UNUSED(z); return ceil(x); }
static double trunc           (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(Int64(x)); }
// bit depth conversion helpers. x:value on 8 bit scale y: target bit depth 8-32 z:base bit depth
static double upscaleByShift(double x, double y, double z)
{
  const int target_bit_depth = (int)y;
  const int source_bit_depth = (int)z;
  if (target_bit_depth == source_bit_depth) return x; // same bit 
  if (target_bit_depth == 32) { // target float
    const int max_pixel_value_source = (1 << source_bit_depth) - 1;
    return x / max_pixel_value_source;
  }
  if (source_bit_depth == 32) { // treat original as float, target 8-16 bits
    const int max_pixel_value_target = (1 << target_bit_depth) - 1;
    return x * max_pixel_value_target;
  }
  // 8-16 <-> 8-16
  if (target_bit_depth > source_bit_depth) // e.g. 8-->10
    return double(x * (1 << (target_bit_depth - source_bit_depth))); // upscale, mul by 2^N bits
  else
    return double(x / (1 << (source_bit_depth - target_bit_depth))); // downscale: div by 2^N bits
}
static double upscaleByStretch(double x, double y, double z) // e.g. 8->10 bit rgb: x/255*1023
{
  const int target_bit_depth = (int)y;
  const int source_bit_depth = (int)z;
  if (target_bit_depth == source_bit_depth) return x; // same bit 
  
  if (target_bit_depth == 32) { // target float
    const int max_pixel_value_source = (1 << source_bit_depth) - 1;
    return x / max_pixel_value_source;
  }
  if (source_bit_depth == 32) { // treat original as float, target 8-16 bits
    const int max_pixel_value_target = (1 << target_bit_depth) - 1;
    return x * max_pixel_value_target;
  }
  // 8-16 <-> 8-16
  const int max_pixel_value_source = (1 << source_bit_depth) - 1;
  const int max_pixel_value_target = (1 << target_bit_depth) - 1;
  return double(x * max_pixel_value_target / max_pixel_value_source); // upscale or downscale
}

Symbol Symbol::Addition       ("+" , OPERATOR, 2, addition);
Symbol Symbol::Multiplication ("*" , OPERATOR, 2, multiplication);
Symbol Symbol::Division       ("/" , OPERATOR, 2, division);
Symbol Symbol::Substraction   ("-" , OPERATOR, 2, substraction);
Symbol Symbol::Power          ("^" , OPERATOR, 2, power);
Symbol Symbol::Modulo         ("%" , OPERATOR, 2, modulo);
Symbol Symbol::Interrogation  ("?" , TERNARY , 3, interrogation);
Symbol Symbol::Equal          ("==", OPERATOR, 2, equal);
Symbol Symbol::Equal2         ("=", OPERATOR, 2, equal);
Symbol Symbol::NotEqual       ("!=", OPERATOR, 2, notEqual);
Symbol Symbol::Inferior       ("<=", OPERATOR, 2, inferior);
Symbol Symbol::InferiorStrict ("<" , OPERATOR, 2, inferiorStrict);
Symbol Symbol::Superior       (">=", OPERATOR, 2, superior);
Symbol Symbol::SuperiorStrict (">" , OPERATOR, 2, superiorStrict);
Symbol Symbol::And            ("&" , OPERATOR, 2, and);
Symbol Symbol::Or             ("|" , OPERATOR, 2, or);
Symbol Symbol::AndNot         ("&!", OPERATOR, 2, andNot);
Symbol Symbol::Xor            ("°" , "@", OPERATOR, 2, xor);
Symbol Symbol::AndUB          ("&u" , OPERATOR, 2, andUB);
Symbol Symbol::OrUB           ("|u" , OPERATOR, 2, orUB);
Symbol Symbol::XorUB          ("°u" , "@u", OPERATOR, 2, xorUB);
Symbol Symbol::NegateUB       ("~u" , FUNCTION, 1, negateUB);
Symbol Symbol::PosShiftUB     ("<<", "<<u", OPERATOR, 2, posshiftUB);
Symbol Symbol::NegShiftUB     (">>", ">>u", OPERATOR, 2, negshiftUB);
Symbol Symbol::AndSB          ("&s" , OPERATOR, 2, andSB);
Symbol Symbol::OrSB           ("|s" , OPERATOR, 2, orSB);
Symbol Symbol::XorSB          ("°s" , "@s", OPERATOR, 2, xorSB);
Symbol Symbol::NegateSB       ("~s" , FUNCTION, 1, negateSB);
Symbol Symbol::PosShiftSB     ("<<s", OPERATOR, 2, posshiftSB);
Symbol Symbol::NegShiftSB     (">>s", OPERATOR, 2, negshiftSB);
Symbol Symbol::Pi             ("pi", 3.1415927, NUMBER  , 0, NULL);
// Lut variables
Symbol Symbol::X              ("x" , VARIABLE_X, 0, NULL);  
Symbol Symbol::Y              ("y" , VARIABLE_Y, 0, NULL);
Symbol Symbol::Z              ("z" , VARIABLE_Z, 0, NULL);
// preliminary lut variable: A(lpha)
Symbol Symbol::A              ("a" , VARIABLE_A, 0, NULL);
// global bitdepth parameter for autoscale, since v2.2.1
Symbol Symbol::BITDEPTH       ("bitdepth" , VARIABLE_BITDEPTH, 0, NULL); 
Symbol Symbol::SCRIPT_BITDEPTH("sbitdepth", VARIABLE_SCRIPT_BITDEPTH, 0, NULL);
// bit-depth adaptive constants, since v2.2.1
Symbol Symbol::RANGE_HALF     ("range_half", VARIABLE_RANGE_HALF, 0, NULL); // 128  scaled
Symbol Symbol::RANGE_MAX      ("range_max", VARIABLE_RANGE_MAX, 0, NULL);   // 255, 4095, .. 65535
Symbol Symbol::RANGE_SIZE     ("range_size", VARIABLE_RANGE_SIZE, 0, NULL); // 256, 1024, 4096, 16384, 65536
Symbol Symbol::YMIN           ("ymin", VARIABLE_YMIN, 0, NULL); // 16 scaled
Symbol Symbol::YMAX           ("ymax", VARIABLE_YMAX, 0, NULL); // 235 or scaled
Symbol Symbol::CMIN           ("cmin", VARIABLE_CMIN, 0, NULL); // 16 scaled = LIMITED_YMIN
Symbol Symbol::CMAX           ("cmax", VARIABLE_CMAX, 0, NULL); // 240 scaled
// Math
Symbol Symbol::Cos            ("cos", FUNCTION, 1, cos);
Symbol Symbol::Sin            ("sin", FUNCTION, 1, sin);
Symbol Symbol::Tan            ("tan", FUNCTION, 1, tan);
Symbol Symbol::Log            ("log", FUNCTION, 1, log);
Symbol Symbol::Exp            ("exp", FUNCTION, 1, exp);
Symbol Symbol::Abs            ("abs", FUNCTION, 1, mabs);
Symbol Symbol::Atan           ("atan", FUNCTION, 1, atan);
Symbol Symbol::Acos           ("acos", FUNCTION, 1, acos);
Symbol Symbol::Asin           ("asin", FUNCTION, 1, asin);
Symbol Symbol::Round          ("round", FUNCTION, 1, round);
Symbol Symbol::Clip           ("clip", FUNCTION, 3, clip);
Symbol Symbol::Min            ("min", FUNCTION, 2, mmin);
Symbol Symbol::Max            ("max", FUNCTION, 2, mmax);
Symbol Symbol::Ceil           ("ceil", FUNCTION, 1, ceil);
Symbol Symbol::Floor          ("floor", FUNCTION, 1, floor);
Symbol Symbol::Trunc          ("trunc", FUNCTION, 1, trunc);
// automatic bit-depth scaling helpers, since v2.2.1
Symbol Symbol::ScaleByShift  ("@B", FUNCTION_WITH_BITDEPTH_AS_AUTOPARAM, 1, upscaleByShift); // v 2.2.5: #B, #F -> @B, @F
Symbol Symbol::ScaleByStretch("@F", FUNCTION_WITH_BITDEPTH_AS_AUTOPARAM, 1, upscaleByStretch);
// admin config
Symbol Symbol::SetScriptBitDepthI8("i8", 8.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetScriptBitDepthI10("i10", 10.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetScriptBitDepthI12("i12", 12.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetScriptBitDepthI14("i14", 14.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetScriptBitDepthI16("i16", 16.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetScriptBitDepthF32("f32", 32.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseI8Range("clamp_f_i8", -8.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseI10Range("clamp_f_i10", -10.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseI12Range("clamp_f_i12", -12.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseI14Range("clamp_f_i14", -14.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseI16Range("clamp_f_i16", -16.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseF32Range("clamp_f_f32", -32.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);
Symbol Symbol::SetFloatToClampUseF32Range_2("clamp_f", -32.0, FUNCTION_CONFIG_SCRIPT_BITDEPTH, 0, NULL);

Symbol::Symbol() :
type(UNDEFINED), value(""), value2("")
{
}

Symbol::Symbol(String value, Type type, int nParameter, Process process) :
type(type), value(value), value2(""), nParameter(nParameter), process(process)
{
   if ( type == NUMBER )
      dValue = atof(value.c_str());
}

Symbol::Symbol(String value, String value2, Type type, int nParameter, Process process) :
type(type), value(value), value2(value2), nParameter(nParameter), process(process)
{
   if ( type == NUMBER )
      dValue = atof(value.c_str());
}

Symbol::Symbol(String value, double dValue, Type type, int nParameter, Process process) :
type(type), value(value), value2(""), nParameter(nParameter), dValue(dValue), process(process)
{
}

void Symbol::setValue(double _dValue)
{
   this->dValue = _dValue;
}

double Symbol::getValue(double x, double y, double z) const
{
   switch ( type )
   {
   case VARIABLE_X:
   case VARIABLE_Y:
   case VARIABLE_Z:
   // since v2.2.1:
   case VARIABLE_A: 
   
   case VARIABLE_BITDEPTH: // automatic silent parameter of the script 
   case VARIABLE_SCRIPT_BITDEPTH: // v2.2
     // embedded expr constants
   case VARIABLE_RANGE_HALF:
   case VARIABLE_RANGE_MAX:
   case VARIABLE_RANGE_SIZE:
   case VARIABLE_YMIN:
   case VARIABLE_YMAX:
   case VARIABLE_CMIN:
   case VARIABLE_CMAX:

   case NUMBER:
     return dValue;
   case FUNCTION_CONFIG_SCRIPT_BITDEPTH:
     return dValue;
   default:
      return process(x, y, z);
   }
}

Context::Context(const std::deque<Symbol> &expression)
{
   nPos = -1;
   nSymbols = expression.size();
   pSymbols = new Symbol[nSymbols];

   auto it = expression.begin();

   int default_sbitdepth = 8;
   int default_float_autoscale_bitdepth = 0; // no clamp and autoscale

   int symbolCount = 0;
   for (int i = 0; i < nSymbols; i++, it++) {
     // control mnemonics are filtered here, they are not put in the expression
     if (it->type == Symbol::FUNCTION_CONFIG_SCRIPT_BITDEPTH) {
       if (it->dValue > 0) {
         // i8, i10, i12, i14, i16, f32
         default_sbitdepth = (int)it->dValue;
       }
       else {
         // negative values show the bitdepth that inputs and output should autoscale.
         // clamp_f
         // clamp_f_i8..clamp_f_i16..clamp_f_f32
         default_float_autoscale_bitdepth = -(int)it->dValue;
       }
     }
     else {
       pSymbols[symbolCount++] = *it;
     }
   }
   nSymbols = symbolCount;
   sbitdepth = default_sbitdepth;

   float_autoscale_bitdepth = default_float_autoscale_bitdepth;

   // precalculate scale factor and its inverse
   float_input_scalefactor = 1.0; // for default and 32 bit
   if (float_autoscale_bitdepth >= 8 && float_autoscale_bitdepth <= 16)
     float_input_scalefactor = double((1 << float_autoscale_bitdepth) - 1);
   float_input_invscalefactor = 1.0 / float_input_scalefactor;

}

Context::~Context()
{
   delete[] pSymbols;
}

double Context::rec_compute()
{
   const Symbol &s = pSymbols[--nPos];

   switch ( s.type )
   {
   case Symbol::NUMBER: return s.dValue;
   case Symbol::VARIABLE_X: return x;
   case Symbol::VARIABLE_Y: return y;
   case Symbol::VARIABLE_Z: return z;
   case Symbol::VARIABLE_A: return a;
   case Symbol::VARIABLE_BITDEPTH: return bitdepth; // bit-depth for autoscale
   case Symbol::VARIABLE_SCRIPT_BITDEPTH: return sbitdepth; // source bit depth for autoscale

   case Symbol::VARIABLE_RANGE_HALF: return bitdepth == 32 ? 0.5 : (128 << (bitdepth - 8)); // or 0.0 for float in the future?
   case Symbol::VARIABLE_RANGE_MAX: return bitdepth == 32 ? 1.0 : ((1 << bitdepth) - 1); // max_pixel_value. 255, 1023, 4095, 16383, 65535 (1.0 for float)
   case Symbol::VARIABLE_RANGE_SIZE: return bitdepth == 32 ? 1.0 : (1 << bitdepth); // 256, 1024, 4096, 16384, 65536 (1.0 for float)
   case Symbol::VARIABLE_YMIN: return bitdepth == 32 ? 16.0 / 255 : (16 << (bitdepth - 8));    // 16 scaled
   case Symbol::VARIABLE_YMAX: return bitdepth == 32 ? 235.0 / 255 : (235 << (bitdepth - 8));  // 235 scaled
   case Symbol::VARIABLE_CMIN: return bitdepth == 32 ? 16.0 / 255 : (16 << (bitdepth - 8));    // 16 scaled
   case Symbol::VARIABLE_CMAX: return bitdepth == 32 ? 240.0 / 255 : (240 << (bitdepth - 8));  // 240 scaled

   case Symbol::FUNCTION_WITH_BITDEPTH_AS_AUTOPARAM: // silent bit-depth parameter for autoscale
     switch (s.nParameter)
     {
     case 1: return s.process(rec_compute(), bitdepth, sbitdepth); // automatic bit-depth parameters
     case 2: // two original parameters + bitdepth ; none of this
     {
       double yy = rec_compute();
       double xx = rec_compute();
       return s.process(xx, yy, bitdepth);
     }
     default:
       return s.process(-1.0f, -1.0f, -1.0f);
     }
   
   default:
      switch ( s.nParameter )
      {
      case 2 :
         {
            double yy = rec_compute();
            double xx = rec_compute();
            return s.process(xx, yy, -1.0f);
         }
      case 1: return s.process(rec_compute(), -1.0f, -1.0f);
      case 3:
         {
            double zz = rec_compute();
            double yy = rec_compute();
            double xx = rec_compute();
            return s.getValue(xx, yy, zz);
         }
      default: return s.process(-1.0f, -1.0f, -1.0f);
      }
   }
}

double Context::compute(double _x, double _y, double _z, double _a, int _bitdepth)
{
   nPos = nSymbols;
   this->x = _x;
   this->y = _y;
   this->z = _z;
   this->a = _a;

   this->bitdepth = _bitdepth;
   // all other expr constants are calculated from bitdepth

   return rec_compute();
}

String Context::rec_infix()
{
    const Symbol &s = pSymbols[--nPos];

    switch ( s.type )
    {
    case Symbol::VARIABLE_X: 
    case Symbol::VARIABLE_Y: 
    case Symbol::VARIABLE_Z: 
    case Symbol::VARIABLE_A:
    case Symbol::VARIABLE_BITDEPTH:
    case Symbol::VARIABLE_SCRIPT_BITDEPTH:
    case Symbol::VARIABLE_RANGE_HALF:
    case Symbol::VARIABLE_RANGE_MAX:
    case Symbol::VARIABLE_RANGE_SIZE:
    case Symbol::VARIABLE_YMIN:
    case Symbol::VARIABLE_YMAX:
    case Symbol::VARIABLE_CMIN:
    case Symbol::VARIABLE_CMAX:
    case Symbol::NUMBER: return s.value;
    case Symbol::FUNCTION:
        if (s.nParameter == 1) {
            return s.value + "(" + rec_infix() + ")";
        } else if (s.nParameter == 2) {
            auto op2 = rec_infix();
            return s.value + "(" + rec_infix() + "," + op2 + ")";
        } else {
            auto op3 = rec_infix();
            auto op2 = rec_infix();
            return s.value + "(" + rec_infix() + "," + op2 + "," + op3 + ")";
        }
    case Symbol::OPERATOR:
        {
            auto op2 = rec_infix();
            return "(" + rec_infix() + s.value + op2 + ")";
        }
    case Symbol::TERNARY:
        {
            auto op3 = rec_infix();
            auto op2 = rec_infix();
            return "((" + rec_infix() + ") ? " + op2 + " : " + op3 + ")";
        }
    case Symbol::FUNCTION_WITH_BITDEPTH_AS_AUTOPARAM:
    {
      if (s.nParameter == 1) {
        return s.value + "(" + rec_infix() + ")";
      }
    }

    default:
        assert(0);
        return "";
    }
}

String Context::infix()
{
   nPos = nSymbols;

   return rec_infix();
}

bool Context::check()
{
   return true;
}