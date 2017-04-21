#ifndef __Mt_Params_H__
#define __Mt_Params_H__

#include "../../../common/utils/utils.h"

namespace Filtering { namespace MaskTools {

typedef enum {
   NONE,
   MEMSET,
   COPY,
   PROCESS,
   COPY_SECOND,
   COPY_THIRD,
   COPY_FOURTH,

} Mode;

class Operator {

   Mode mode;
   int nValue;      /* only for mode == memset */
   float nValue_f;

public:

   Operator() : mode(NONE), nValue(-1), nValue_f(-1.0f)  {}
   Operator(Mode _mode, float _nValue_f = -1.0f) : mode(_mode), nValue(_mode == MEMSET ? (int)_nValue_f : -1), nValue_f(_mode == MEMSET ? _nValue_f : -1.0f) {}
   Operator(float _nValue_f)
   {
      this->nValue = -1;
      this->nValue_f = -1.0f;
      int _nValue = (int)_nValue_f;
      switch ( _nValue )
      {
      case 6 : mode = COPY_FOURTH; break;
      case 5 : mode = COPY_THIRD; break;
      case 4 : mode = COPY_SECOND; break;
      case 3 : mode = PROCESS; break;
      case 2 : mode = COPY; break;
      case 1 : mode = NONE; break;
      default: 
        mode = MEMSET; 
        this->nValue_f = -_nValue_f; // negate!
        this->nValue = int(this->nValue_f);
        // todo: problematic when float clip chroma is negative, shift by -0.5 later?
        // anyway, masktools has no float support yet
        break;
      }
   }

   bool operator==(const Operator &operation) const { return mode == operation.mode; }
   bool operator!=(const Operator &operation) const { return mode != operation.mode; }
   bool operator==(Mode _mode) const { return _mode == this->mode; }
   bool operator!=(Mode _mode) const { return _mode != this->mode; }
   Mode getMode() const { return mode; }
   int value() const { return nValue; }
   float value_f() const { return nValue_f; }
};

} } // namespace MaskTools, Filtering

#endif
