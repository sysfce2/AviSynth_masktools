#include "spirit.h"
#include "../../../common/parser/parser.h"

/* conditional compilation */
#ifdef MT_HAVE_BOOST_SPIRIT
// Hint:
// http://www.boost.org/doc/libs/1_48_0/libs/spirit/doc/html/spirit/what_s_new/spirit_1_x.html
// 1.) download full boost 1.62 or newer: boost_1_62_0.7z (537MBytes)
//     http://www.boost.org/users/download/
//     Extract to e.g. c:\soft\boost, or to the place what is set in VC++ Directories|Include project properties
// 2.) Binaries: vc14 for VS2015 both 64 and 32 bit
//     https://sourceforge.net/projects/boost/files/boost-binaries/1.62.0/
//     Set library paths for x64 and x32 in VC++ Directories | Library directories project properties
//     Needed libs:
//      libboost_date_time-vc140-mt-s-1_62.lib
//      libboost_thread-vc140-mt-s-1_62.lib
//      libboost_chrono-vc140-mt-s-1_62.lib
//      libboost_system-vc140-mt-s-1_62.lib
#include <boost/spirit/home/classic.hpp>

using namespace boost;
using namespace boost::spirit::classic;

namespace Filtering { namespace MaskTools { namespace Helpers { namespace PolishConverter {

struct AddNamedSymbol {
public:
   AddNamedSymbol(const String &name, String &rpn) : name(name), rpn(rpn) {}
   template<typename IteratorT>
   void operator()( IteratorT a ) const
   {
      UNUSED(a);
      rpn.append(name).append(" ");
   }
   template<typename IteratorT>
   void operator()( IteratorT a , IteratorT b ) const
   {
      UNUSED(a); UNUSED(b);
      rpn.append(name).append(" ");
   }
   template<typename IteratorT>
   void operator()( IteratorT a , IteratorT b, IteratorT c ) const
   {
      UNUSED(a); UNUSED(b); UNUSED(c);
      rpn.append(name).append(" ");
   }
private:
   String name;
   String &rpn;
};

struct AddDouble {
public:
   AddDouble(String &rpn) : rpn(rpn) {}
   template<typename IteratorT>
   void operator()( IteratorT a ) const
   {
      rpn.append(FToS(a)).append(" ");
   }
private:
   String FToS(double v) const
   {
      char buffer[20];
      sprintf(buffer, "%7f", v);
      buffer[19] = 0; /* safety */
      return buffer;
   }
   String &rpn;
};

struct AddInteger {
public:
   AddInteger(String &rpn) : rpn(rpn) {}
   template<typename IteratorT>
   void operator()( IteratorT a ) const
   {
      rpn.append(std::to_string(a)).append(" ");
   }
private:
   String &rpn;
};

struct Syntax :
   public grammar<Syntax>
{
public:
   Syntax(String &rpn) : rpn(rpn) {}
   template <typename ScannerT>
   struct definition
   {
   public:
      definition( Syntax const &self )
      {
         /* variables and numbers are processed in the same way */
         factor = strict_real_p[AddDouble(self.rpn)] | int_p[AddInteger(self.rpn)] | ('(' >> termter >> ')') | ( ch_p('x')[AddNamedSymbol("x", self.rpn)] | ch_p('y')[AddNamedSymbol("y", self.rpn)] | ch_p('z')[AddNamedSymbol("z", self.rpn)] | str_p("pi")[AddNamedSymbol("pi", self.rpn)] ) | term0;

         /* functions */
         term0 = (str_p("abs") >> '(' >> (termter) >> ')')[AddNamedSymbol("abs", self.rpn)] |
                 (str_p("sin") >> '(' >> (termter) >> ')')[AddNamedSymbol("sin", self.rpn)] |
                 (str_p("cos") >> '(' >> (termter) >> ')')[AddNamedSymbol("cos", self.rpn)] |
                 (str_p("tan") >> '(' >> (termter) >> ')')[AddNamedSymbol("tan", self.rpn)] |
                 (str_p("exp") >> '(' >> (termter) >> ')')[AddNamedSymbol("exp", self.rpn)] |
                 (str_p("log") >> '(' >> (termter) >> ')')[AddNamedSymbol("log", self.rpn)] |
                 (str_p("atan") >> '(' >> (termter) >> ')')[AddNamedSymbol("atan", self.rpn)] |
                 (str_p("acos") >> '(' >> (termter) >> ')')[AddNamedSymbol("acos", self.rpn)] |
                 (str_p("asin") >> '(' >> (termter) >> ')')[AddNamedSymbol("asin", self.rpn)] |
                 (str_p("round") >> '(' >> (termter) >> ')')[AddNamedSymbol("round", self.rpn)] |
                 (str_p("min") >> '(' >> (termter) >> ',' >> (termter) >> ')')[AddNamedSymbol("min", self.rpn)] |
                 (str_p("max") >> '(' >> (termter) >> ',' >> (termter) >> ')')[AddNamedSymbol("max", self.rpn)] |
                 (str_p("clip") >> '(' >> (termter) >> ',' >> (termter) >> ',' >> (termter) >> ')')[AddNamedSymbol("clip", self.rpn)];

         /* operators : precendence is manages with order of declaration */
         /* powers */
         term1 = factor >> *( (ch_p('^') >> factor)[AddNamedSymbol("^", self.rpn)] );
         /* multiplications / divisions */
         term2 = term1 >> *( (ch_p('*') >> term1)[AddNamedSymbol("*", self.rpn)] | (ch_p('/') >> term1)[AddNamedSymbol("/", self.rpn)] );
         /* modulos */
         term3 = term2 >> *( (ch_p('%') >> term2)[AddNamedSymbol("%", self.rpn)] );
         /* additions / substractions */
         term4 = term3 >> *( (ch_p('+') >> term3)[AddNamedSymbol("+", self.rpn)] | (ch_p('-') >> term3)[AddNamedSymbol("-", self.rpn)] );
         /* comparisons */
         term5 = term4 >> *( ("==" >> term4)[AddNamedSymbol("==", self.rpn)] |
            ("!=" >> term4)[AddNamedSymbol("!=", self.rpn)] |
            ("<=" >> term4)[AddNamedSymbol("<=", self.rpn)] |
            (">=" >> term4)[AddNamedSymbol(">=", self.rpn)] |
            ("<" >> term4)[AddNamedSymbol("<", self.rpn)] |
            (">" >> term4)[AddNamedSymbol(">", self.rpn)]);
         /* logic */
         term6 = term5 >> *( ("!&" >> term5)[AddNamedSymbol("!&", self.rpn)] |
            ('|' >> term5)[AddNamedSymbol("|", self.rpn)] |
            ("&" >> term5)[AddNamedSymbol("&", self.rpn)] |
            ('°' >> term5)[AddNamedSymbol("°", self.rpn)]);
         /* ternary ops */
         termter = term6 >> *((ch_p('?') >> term6 >> ch_p(':') >> term6)[AddNamedSymbol("?", self.rpn)]);
        }
        rule<ScannerT> factor, term0, term1, term2, term3, term4, term5, term6, termter;

        const rule<ScannerT> &start() const { return termter; }
    };
    //friend struct definition;
private:
   String &rpn;
};

String Converter(const String &str)
{
   String result;
   Syntax parser(result);
   parse_info<> info;

   try {
      info = parse(str.c_str(), parser, space_p);
   }
   catch (...)
   {
      result = "unvalid expression";
   }

   return result;
}

String Infix(const String &str)
{
   Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y).addSymbol(Parser::Symbol::Z);
   parser.parse(str, " ");
   Parser::Context ctx(parser.getExpression());
   return ctx.infix();
}

} } } }

#else

Filtering::String Filtering::MaskTools::Helpers::PolishConverter::Converter(const Filtering::String&)
{
   return "spirit parser not available";
}

Filtering::String Filtering::MaskTools::Helpers::PolishConverter::Infix(const Filtering::String&)
{
    return "spirit parser not available";
}

#endif