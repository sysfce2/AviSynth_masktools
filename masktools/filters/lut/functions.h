#ifndef __Mt_Lut_Functions_H__
#define __Mt_Lut_Functions_H__

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut {

typedef enum {

   NONIZER = 0,
   AVERAGER = 1,
   MINIMIZER = 2,
   MAXIMIZER = 3,
   DEVIATER = 4,
   RANGIZER = 5,
   MEDIANIZER = 6,
   MEDIANIZER4 = 7,
   MEDIANIZER6 = 8,
   MEDIANIZER2 = 9,

   NUM_MODES,

} MPROCESSOR_MODE;

#define EXPRESSION_SINGLE( realtime, base, mode1, mode2 ) &base< realtime, mode2 >
#define EXPRESSION_DUAL( realtime, base, mode1, mode2 ) &base< mode1, mode2 >

#define MPROCESSOR(realtime, expr, param, mode) \
{ \
   expr( realtime, param, mode, Nonizer ), \
   expr( realtime, param, mode, Averager<int> ), \
   expr( realtime, param, mode, Minimizer ), \
   expr( realtime, param, mode, Maximizer ), \
   expr( realtime, param, mode, Deviater<int> ), \
   expr( realtime, param, mode, Rangizer ), \
   expr( realtime, param, mode, Medianizer ), \
   expr( realtime, param, mode, MedianizerBetter<4> ), \
   expr( realtime, param, mode, MedianizerBetter<6> ), \
   expr( realtime, param, mode, MedianizerBetter<2> ), \
}

#define MPROCESSOR_SINGLE( base, realtime )     MPROCESSOR( realtime, EXPRESSION_SINGLE, base, )
#define MPROCESSOR_DUAL( base, mode ) MPROCESSOR( false, EXPRESSION_DUAL, base, mode )
// for dual: parameter 'realtime' n/a

/* 16 bit */
#define EXPRESSION16_SINGLE( realtime, bits_per_pixel, base, mode1, mode2 ) &base< realtime, bits_per_pixel, mode2 >
//#define EXPRESSION_DUAL( realtime, base, mode1, mode2 ) &base< mode1, mode2 >

#define MPROCESSOR16(realtime, bits_per_pixel, expr, param, mode) \
{ \
   expr( realtime, bits_per_pixel, param, mode, Nonizer16<bits_per_pixel> ), \
   expr( realtime, bits_per_pixel, param, mode, Averager16 ), \
   expr( realtime, bits_per_pixel, param, mode, Minimizer16<bits_per_pixel> ), \
   expr( realtime, bits_per_pixel, param, mode, Maximizer16 ), \
   expr( realtime, bits_per_pixel, param, mode, Deviater16 ), \
   expr( realtime, bits_per_pixel, param, mode, Rangizer16<bits_per_pixel> ), \
   expr( realtime, bits_per_pixel, param, mode, Medianizer16 ), \
   expr( realtime, bits_per_pixel, param, mode, MedianizerBetter16<4> ), \
   expr( realtime, bits_per_pixel, param, mode, MedianizerBetter16<6> ), \
   expr( realtime, bits_per_pixel, param, mode, MedianizerBetter16<2> ), \
}

#define MPROCESSOR16_SINGLE( base, realtime, bits_per_pixel )     MPROCESSOR16( realtime, bits_per_pixel, EXPRESSION16_SINGLE, base, )

/* 32 bit float */
#define EXPRESSION32_SINGLE( base, mode1, mode2 ) &base< mode2 >
//#define EXPRESSION_DUAL( realtime, base, mode1, mode2 ) &base< mode1, mode2 >

#define MPROCESSOR32(expr, param, mode) \
{ \
   expr( param, mode, Nonizer32 ), \
   expr( param, mode, Averager32 ), \
   expr( param, mode, Minimizer32 ), \
   expr( param, mode, Maximizer32 ), \
   expr( param, mode, Deviater32 ), \
   expr( param, mode, Rangizer32 ), \
   expr( param, mode, Medianizer32 ), \
   expr( param, mode, MedianizerBetter32<4> ), \
   expr( param, mode, MedianizerBetter32<6> ), \
   expr( param, mode, MedianizerBetter32<2> ), \
}

#define MPROCESSOR32_SINGLE( base )     MPROCESSOR32( EXPRESSION32_SINGLE, base, )


static inline int ModeToInt(const String &mode)
{
   if ( mode == "average" || mode == "avg" )
      return AVERAGER;
   else if ( mode == "standard deviation" || mode == "std" )
      return DEVIATER;
   else if ( mode == "minimum" || mode == "min" )
      return MINIMIZER;
   else if ( mode == "maximum" || mode == "max" )
      return MAXIMIZER;
   else if ( mode == "range" )
      return RANGIZER;
   else if ( mode == "median" || mode == "med" )
      return MEDIANIZER4;
   else
      return NONIZER;
}

class Nonizer {

   Double *pdCoefficients;
   int nCoefficients;
   Double dTotalCoefficients;
   Double dSum;
   int nPosition;

public:
   Nonizer(const String &mode)
   {
      auto coefficients = Parser::getDefaultParser().parse(mode, " (),;").getExpression();
      nCoefficients = coefficients.size();
      pdCoefficients = new Double[nCoefficients];
      int i = 0;

      dTotalCoefficients = 0;

      while ( !coefficients.empty() )
      {
         dTotalCoefficients += (pdCoefficients[i++] = coefficients.front().getValue(0, 0, 0));
         coefficients.pop_front();
      }
   }
   ~Nonizer()
   {
      delete[] pdCoefficients;
   }
   void reset(int nValue = 255)
   {
      UNUSED(nValue);
      dSum = 0;
      nPosition = 0;
   }
   void add(int nValue)
   {
      if (nPosition < nCoefficients)
         dSum += pdCoefficients[nPosition++] * nValue;
   }
   Byte finalize() const
   {
      return clip<Byte, Int64>(convert<Int64, Double>(dSum / dTotalCoefficients));
   }
};

class Minimizer {

   int nMin;

public:

   Minimizer(const String &mode) { UNUSED(mode); }
   void reset() { nMin = 255; }
   void add(int nValue) { nMin = min<int>( nMin, nValue ); }
   Byte finalize() const { return static_cast<Byte>(nMin); }

};

class Medianizer {

   int nSize;
   int elements[256];

public:

   Medianizer(const String &mode) { UNUSED(mode); }
   void reset() { memset( elements, 0, sizeof( elements ) ); nSize = 0; }
   void reset(int nValue) { reset(); add( nValue ); }
   void add(int nValue) { elements[nValue]++; nSize++; }
   Byte finalize() const
   {
      int nCount = 0;
      int nIdx = -1;
      const int nLowHalf = (nSize + 1) >> 1;
      while ( nCount < nLowHalf )
         nCount += elements[++nIdx];

      if ( nSize & 1 ) /* nSize odd -> median belong to the middle class */
         return static_cast<Byte>(nIdx);
      else
      {
         if ( nCount >= nLowHalf + 1 ) /* nSize even, but middle class owns both median elements */
            return static_cast<Byte>(nIdx);
         else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
         {
            int nSndIdx = nIdx;
            while ( !elements[++nSndIdx] ) {}
            return static_cast<Byte>((nIdx + nSndIdx + 1) >> 1); /* and we return the average */
         }
      }
   }
};

template<int n>
class MedianizerBetter
{
   int nSize;
   int elements[256];
   int elements2[256 >> n];

public:
   MedianizerBetter(const String &mode) { UNUSED(mode); }
   void reset() { nSize = 0; memset( elements, 0, sizeof( elements ) ); memset( elements2, 0, sizeof( elements2 ) ); }
   void add(int nValue) { elements[nValue]++; elements2[nValue>>n]++; nSize++; }

   Byte finalize() const
   {
      int nCount = 0;
      int nIdx = -1;
      const int nLowHalf = (nSize + 1) >> 1;

      while ( nCount < nLowHalf )
         nCount += elements2[++nIdx]; /* low resolution search */

      nCount -= elements2[nIdx];
      nIdx <<= n;
      nIdx--;

      while ( nCount < nLowHalf )
         nCount += elements[++nIdx]; /* high resolution search */

      if ( nSize & 1 ) /* nSize odd -> median belong to the middle class */
         return static_cast<Byte>(nIdx);
      else
      {
         if ( nCount >= nLowHalf + 1 ) /* nSize even, but middle class owns both median elements */
            return static_cast<Byte>(nIdx);
         else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
         {
            int nSndIdx = nIdx;
            while ( !elements[++nSndIdx] ) {}
            return static_cast<Byte>((nIdx + nSndIdx + 1) >> 1); /* and we return the average */
         }
      }
   }
};

class Maximizer {

   int nMax;

public:

   Maximizer(const String &mode) { UNUSED(mode); }
   void reset() { nMax = 0; }
   void add(int nValue) { nMax = max<int>( nMax, nValue ); }
   Byte finalize() const { return static_cast<Byte>(nMax); }

};

class Rangizer {

   int nMin, nMax;

public:

   Rangizer(const String &mode) { UNUSED(mode); }
   void reset() { nMin = 255; nMax = 0; }
   void add(int nValue) { nMin = min<int>( nMin, nValue ); nMax = max<int>( nMax, nValue ); }
   Byte finalize() const { return static_cast<Byte>(nMax - nMin); }

};

// only <int> is used, for 4K+ videos <int64_t> needed
template<typename T>
class Averager
{

   T nSum, nCount;

public:

   Averager(const String &mode) { UNUSED(mode); }
   void reset() { nSum = 0; nCount = 0; }
   void add(int nValue) { nSum += nValue; nCount++; } 
   // 4K era warning!! Integer can hold the sum of 8 421 504 pixels (worst case 255)
   // 3840 x 2180 = 8371200, just below limit
   // 4096 x 2180 = 8929280 pixels, over limit!
   // unsigned int32 is OK, but is not enough for 8K videos
   Byte finalize() const { return static_cast<Byte>(rounded_division<T>( nSum, nCount )); }

};

template<typename T>
class Deviater
{
   T nSum, nCount;
   Int64 nSum2;

public:

   Deviater(const String &mode) { UNUSED(mode); }
   void reset() { nSum = 0; nSum2 = 0; nCount = 0; }
   void reset(int nValue) { nSum = nValue; nSum2 = nValue * nValue; nCount = 1; }
   void add(int nValue) { nSum += nValue; nSum2 += nValue * nValue; nCount++; }
   Byte finalize() const { return convert<Byte, Double>( sqrt( ( Double( nSum2 ) * nCount - Double( nSum ) * Double( nSum ) ) / ( Double( nCount ) * nCount ) ) ); }

};

/**********/
/* 16 bit */
/**********/
template<int bits_per_pixel>
class Nonizer16 {

  Double *pdCoefficients;
  int nCoefficients;
  Double dTotalCoefficients;
  Double dSum;
  int nPosition;

public:
  Nonizer16(const String &mode)
  {
    auto coefficients = Parser::getDefaultParser().parse(mode, " (),;").getExpression();
    nCoefficients = coefficients.size();
    pdCoefficients = new Double[nCoefficients];
    int i = 0;

    dTotalCoefficients = 0;

    while (!coefficients.empty())
    {
      dTotalCoefficients += (pdCoefficients[i++] = coefficients.front().getValue(0, 0, 0));
      coefficients.pop_front();
    }
  }
  ~Nonizer16()
  {
    delete[] pdCoefficients;
  }
  void reset(int nValue = (1 << bits_per_pixel) - 1)
  {
    UNUSED(nValue);
    dSum = 0;
    nPosition = 0;
  }
  void add(int nValue)
  {
    if (nPosition < nCoefficients)
      dSum += pdCoefficients[nPosition++] * nValue;
  }
  Word finalize() const
  {
    return clip<Word, Int64>(convert<Int64, Double>(dSum / dTotalCoefficients));
  }
};

template<int bits_per_pixel>
class Minimizer16 {

  int nMin;

public:

  Minimizer16(const String &mode) { UNUSED(mode); }
  void reset() { nMin = (1 << bits_per_pixel) - 1; }
  void add(int nValue) { nMin = min<int>(nMin, nValue); }
  Word finalize() const { return static_cast<Word>(nMin); }

};

// no bits_per_pixel template
class Medianizer16 {

  int nSize;
  int elements[65536]; // really 1 << bits_per_pixel, but avoid overflow

public:

  Medianizer16(const String &mode) { UNUSED(mode); }
  void reset() { memset(elements, 0, sizeof(elements)); nSize = 0; }
  void reset(int nValue) { reset(); add(nValue); }
  void add(int nValue) { elements[nValue]++; nSize++; }
  Word finalize() const
  {
    int nCount = 0;
    int nIdx = -1;
    const int nLowHalf = (nSize + 1) >> 1;
    while (nCount < nLowHalf)
      nCount += elements[++nIdx];

    if (nSize & 1) /* nSize odd -> median belong to the middle class */
      return static_cast<Word>(nIdx);
    else
    {
      if (nCount >= nLowHalf + 1) /* nSize even, but middle class owns both median elements */
        return static_cast<Word>(nIdx);
      else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
      {
        int nSndIdx = nIdx;
        while (!elements[++nSndIdx]) {}
        return static_cast<Word>((nIdx + nSndIdx + 1) >> 1); /* and we return the average */
      }
    }
  }
};

// no bits_per_pixel template
template<int n>
class MedianizerBetter16
{
  int nSize;
  int elements[65536];
  int elements2[65536 >> n];

public:
  MedianizerBetter16(const String &mode) { UNUSED(mode); }
  void reset() { nSize = 0; memset(elements, 0, sizeof(elements)); memset(elements2, 0, sizeof(elements2)); }
  void add(int nValue) { elements[nValue]++; elements2[nValue >> n]++; nSize++; }

  Word finalize() const
  {
    int nCount = 0;
    int nIdx = -1;
    const int nLowHalf = (nSize + 1) >> 1;

    while (nCount < nLowHalf)
      nCount += elements2[++nIdx]; /* low resolution search */

    nCount -= elements2[nIdx];
    nIdx <<= n;
    nIdx--;

    while (nCount < nLowHalf)
      nCount += elements[++nIdx]; /* high resolution search */

    if (nSize & 1) /* nSize odd -> median belong to the middle class */
      return static_cast<Word>(nIdx);
    else
    {
      if (nCount >= nLowHalf + 1) /* nSize even, but middle class owns both median elements */
        return static_cast<Word>(nIdx);
      else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
      {
        int nSndIdx = nIdx;
        while (!elements[++nSndIdx]) {}
        return static_cast<Word>((nIdx + nSndIdx + 1) >> 1); /* and we return the average */
      }
    }
  }
};

// no bits_per_pixel template
class Maximizer16 {

  int nMax;

public:

  Maximizer16(const String &mode) { UNUSED(mode); }
  void reset() { nMax = 0; }
  void add(int nValue) { nMax = max<int>(nMax, nValue); }
  Word finalize() const { return static_cast<Word>(nMax); }

};

template<int bits_per_pixel>
class Rangizer16 {

  int nMin, nMax;

public:

  Rangizer16(const String &mode) { UNUSED(mode); }
  void reset() { nMin = (1 << bits_per_pixel) - 1; nMax = 0; }
  void add(int nValue) { nMin = min<int>(nMin, nValue); nMax = max<int>(nMax, nValue); }
  Word finalize() const { return static_cast<Word>(nMax - nMin); }

};

// for 4K+ videos or high bit depth <int64_t> needed
// no bits_per_pixel template
class Averager16
{

  int64_t nSum;
  int nCount;

public:

  Averager16(const String &mode) { UNUSED(mode); }
  void reset() { nSum = 0; nCount = 0; }
  void add(int nValue) { nSum += nValue; nCount++; }
  // 4K era warning!! Integer can hold the sum of 8 421 504 pixels (worst case 255)
  // 3840 x 2180 = 8371200, just below limit
  // 4096 x 2180 = 8929280 pixels, over limit!
  // unsigned int32 is OK, but is not enough for 8K videos
  Word finalize() const { return static_cast<Word>(rounded_division<int64_t>(nSum, nCount)); }

};

class Deviater16
{
  Int64 nSum;
  int nCount;
  Int64 nSum2;

public:

  Deviater16(const String &mode) { UNUSED(mode); }
  void reset() { nSum = 0; nSum2 = 0; nCount = 0; }
  void reset(int nValue) { nSum = nValue; nSum2 = nValue * nValue; nCount = 1; }
  void add(int nValue) { nSum += nValue; nSum2 += nValue * nValue; nCount++; }
  Word finalize() const { return convert<Word, Double>(sqrt((Double(nSum2) * nCount - Double(nSum) * Double(nSum)) / (Double(nCount) * nCount))); }

};

/**********/
/* 32 bit */
/**********/
class Nonizer32 {

  Double *pdCoefficients;
  int nCoefficients;
  Double dTotalCoefficients;
  Double dSum;
  int nPosition;

public:
  Nonizer32(const String &mode)
  {
    auto coefficients = Parser::getDefaultParser().parse(mode, " (),;").getExpression();
    nCoefficients = coefficients.size();
    pdCoefficients = new Double[nCoefficients];
    int i = 0;

    dTotalCoefficients = 0;

    while (!coefficients.empty())
    {
      dTotalCoefficients += (pdCoefficients[i++] = coefficients.front().getValue(0, 0, 0));
      coefficients.pop_front();
    }
  }
  ~Nonizer32()
  {
    delete[] pdCoefficients;
  }
  void reset(Float nValue = 1.0)
  {
    UNUSED(nValue);
    dSum = 0;
    nPosition = 0;
  }
  void add(Float nValue)
  {
    if (nPosition < nCoefficients)
      dSum += pdCoefficients[nPosition++] * nValue;
  }
  Float finalize() const
  {
    return (Float)(dSum / dTotalCoefficients);
  }
};

class Minimizer32 {

  Float nMin;

public:

  Minimizer32(const String &mode) { UNUSED(mode); }
  void reset() { nMin = 1.0f; }
  void add(Float nValue) { nMin = min<Float>(nMin, nValue); }
  Float finalize() const { return nMin; }

};

class Medianizer32 {

  int nSize;
  // fake median, we simulate median as float quantized to 16 bit, then scale back the result
  int elements[65536]; 

public:

  Medianizer32(const String &mode) { UNUSED(mode); }
  void reset() { memset(elements, 0, sizeof(elements)); nSize = 0; }
  void reset(Float nValue) { reset(); add(nValue); }
  void add(Float nValue) { elements[nValue <= 0.0f ? 0 : (nValue>=1.0f ? 65535 : Word(nValue * 65535))]++; nSize++; }
  Float finalize() const
  {
    int nCount = 0;
    int nIdx = -1;
    const int nLowHalf = (nSize + 1) >> 1;
    while (nCount < nLowHalf)
      nCount += elements[++nIdx];

    if (nSize & 1) /* nSize odd -> median belong to the middle class */
      return nIdx / 65535.0f;
    else
    {
      if (nCount >= nLowHalf + 1) /* nSize even, but middle class owns both median elements */
        return nIdx / 65535.0f;
      else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
      {
        int nSndIdx = nIdx;
        while (!elements[++nSndIdx]) {}
        return (nIdx + nSndIdx)/2.0f / 65535.0f; /* and we return the average, and scale back to float */
      }
    }
  }
};

template<int n>
class MedianizerBetter32
{
  int nSize;
  // fake median, we simulate median as float quantized to 16 bit, then scale back the result
  int elements[65536];
  int elements2[65536 >> n];

public:
  MedianizerBetter32(const String &mode) { UNUSED(mode); }
  void reset() { nSize = 0; memset(elements, 0, sizeof(elements)); memset(elements2, 0, sizeof(elements2)); }
  void add(Float nValue) { int nValueNew = nValue <= 0.0f ? 0 : (nValue >= 1.0f ? 65535 : Word(nValue * 65535));  elements[nValueNew]++; elements2[nValueNew >> n]++; nSize++; }

  Float finalize() const
  {
    int nCount = 0;
    int nIdx = -1;
    const int nLowHalf = (nSize + 1) >> 1;

    while (nCount < nLowHalf)
      nCount += elements2[++nIdx]; /* low resolution search */

    nCount -= elements2[nIdx];
    nIdx <<= n;
    nIdx--;

    while (nCount < nLowHalf)
      nCount += elements[++nIdx]; /* high resolution search */

    if (nSize & 1) /* nSize odd -> median belong to the middle class */
      return nIdx / 65535.0f; // scale back
    else
    {
      if (nCount >= nLowHalf + 1) /* nSize even, but middle class owns both median elements */
        return nIdx / 65535.0f;
      else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
      {
        int nSndIdx = nIdx;
        while (!elements[++nSndIdx]) {}
        return (nIdx + nSndIdx) / 2.0f / 65535.0f; /* and we return the average */
      }
    }
  }
};

class Maximizer32 {

  Float nMax;

public:

  Maximizer32(const String &mode) { UNUSED(mode); }
  void reset() { nMax = 0; }
  void add(Float nValue) { nMax = max<Float>(nMax, nValue); }
  Float finalize() const { return nMax; }

};

class Rangizer32 {

  Float nMin, nMax;

public:

  Rangizer32(const String &mode) { UNUSED(mode); }
  void reset() { nMin = 1.0f; nMax = 0.0f; }
  void add(Float nValue) { nMin = min<Float>(nMin, nValue); nMax = max<Float>(nMax, nValue); }
  Float finalize() const { return nMax - nMin; }

};

// for 4K+ videos or high bit depth <int64_t> needed
// no bits_per_pixel template
class Averager32
{

  double nSum;
  int nCount;

public:

  Averager32(const String &mode) { UNUSED(mode); }
  void reset() { nSum = 0.0f; nCount = 0; }
  void add(Float nValue) { nSum = nSum + nValue; nCount++; }
  Float finalize() const { return (Float)(nSum / nCount); }

};

class Deviater32
{
  Float nSum;
  int nCount;
  Float nSum2;

public:

  Deviater32(const String &mode) { UNUSED(mode); }
  void reset() { nSum = 0; nSum2 = 0; nCount = 0; }
  void reset(Float nValue) { nSum = nValue; nSum2 = nValue * nValue; nCount = 1; }
  void add(Float nValue) { nSum += nValue; nSum2 += nValue * nValue; nCount++; }
  Float finalize() const { return (Float)(sqrt((Double(nSum2) * nCount - Double(nSum) * Double(nSum)) / (Double(nCount) * nCount))); }

};


} } } }

#endif
