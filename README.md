### MaskTools 2 ###

**Masktools2 v2.2.15 (20180601)**

mod by pinterf

Differences to Masktools 2.0b1

- project moved to Visual Studio 2017, requires Visual Studio redistributables
- add back "none" and "ignore" for values to "chroma" parameter (2.2.9-)
- mt_merge at 8 bit clips: keep exact pixel values when mask is 0 or 255 (v2.2.7-)
- Fix: mt_merge (and probably other multi-clip filters) may result in corrupted results 
  under specific circumstances, due to using video frame pointers which were already released from memory
- no special function names for high bit depth filters
- filters are auto registering their mt mode as MT_NICE_FILTER for Avisynth+
- Avisynth+ high bit depth support (incl. planar RGB, color spaces with alpha plane are supported from v2.2.7)
  All filters are now supporting 10, 12, 14, 16 bits and float
  Note: From v2.2.15 the 32 bit float U and V chroma channels are 0 centered instead of 0.5, supporting the change in Avisynth+ in May 2018. This change affects lut functions and mt_diff.
  (The last Avisynth+ version that matches masktools 2.2.14 is Avs+ r2664)
  Threshold and sc_value parameters are scaled automatically to the current bit depth (v2.2.5-) from a default 8-bit value.
  Y,U,V,A (and parameters chroma/alpha) negative (memset) values are scaled automatically to the current bit depth (v2.2.7-, chroma/alpha v.2.2.8) from a default 8-bit value.
  Default range of such parameters can be overridden to 8-16 bits or float.
  Disable parameter scaling with paramscale="none"
- New plane mode: 6 (copy from fourth clip) for "Y", "U", "V" and "A"
  New "chroma" and "alpha" plane mode override: "copy fourth"
  Use for mt_lutxyza which has four clips
- YV411 (8 bit 4:1:1) support
- mt_merge accepts 4:2:2 clips when luma=true (8-16 bit)
- mt_merge accepts 4:1:1 clips when luma=true
- mt_merge to discard U and V automatically when input is greyscale
- some filters got AVX (float) and AVX2 (integer) support:
  mt_merge: 8-16 bit: AVX2, float:AVX
  mt_logic: 8-16 bit: AVX2, float:AVX
  mt_edge: 8-16 bit: AVX2, 32 bit float AVX
- mt_polish to recognize new constants and scaling operator, and some other operators introduced in earlier versions.
  For a complete list, see v2.2.4 change log
- new: mt_lutxyza. Accepts four clips. 4th variable name is 'a' (besides x, y and z)
- new: mt_luts: weight expressions as an addon for then main expression(s) (martin53's idea)
    - wexpr
    - ywExpr, uwExpr, vwExpr
  If the relevant parameter strings exist, the weighting expression is evaluated
  for each source/neighborhood pixel values (lut or realtime, depending on the bit depth and the "realtime" parameter). 
  Then the usual lut result is premultiplied by this weight factor before it gets accumulated. 
  
  Weights are float values. Weight luts are x,y (2D) luts, similarly to the base working mode,
  where x is the base pixel, y is the current pixel from the neighbourhood, defined in "pixels".
  
  When the weighting expression is "1", the result is the same as the basic weightless mode.
  
  For modes "average" and "std" the weights are summed up. Result is: sum(value_i*weight_i)/sum(weight_i).
  
  When all weights are equal to 1.0 then the expression will result in the average: sum(value_i)/n.
  
  Same logic works for min/max/median/etc., the "old" lut values are pre-multiplied with the weights before accumulation.

- expression syntax supporting bit depth independent expressions
  - bit-depth aware scale operators
  
      operator "scaleb" scales from 8 bit to current bit depth using bit-shifts. scaleb alternative: @B (see warning)
               Use this for YUV. "235 scaleb" -> always results in max luma
                  
      operator "scalef" scales from 8 bit to current bit depth using full range stretch. scalef alternative: @F (see warning)
                  "255 scalef" results in maximum pixel value of current bit depth.
                  Calculation: x/255*65535 for a 8->16 bit sample (rgb)

      Warning: please use scaleb or scalef instead of @B and @F, to match the syntax with avisynth's Expr filter
            
  - hints for non-8 bit based constants: 
      Added configuration keywords i8, i10, i12, i14, i16 and f32 in order to tell the expression evaluator 
      the bit depth of the values that are to scale by scaleb and scalef operators.
            
      By default scaleb and scalef scales from 8 bit to the bit depth of the clip.
      
      i8 .. i16 and f32 sets the default conversion base to 8..16 bits or float, respectively.
      
      These keywords can appear anywhere in the expression, but only the last occurence will be effective for the whole expression.
      
      Examples 
```
      8 bit video, no modifier: "x y - 256 scaleb *" evaluates as "x y - 256 *"
      10 bit video, no modifier: "x y - 256 scaleb *" evaluates as "x y - 1024 *"
      10 bit video: "i16 x y - 65536 scaleb *" evaluates as "x y - 1024 *"
      8 bit video: "i10 x y - 512 scaleb *" evaluates as "x y - 128 *"                  
```

  - new pre-defined, bit depth aware constants
  
    - bitdepth: automatic silent parameter of the lut expression (clip bit depth)
    - sbitdepth: automatic silent parameter of the lut expression (bit depth of values to scale)
    - range_half --> autoscaled 128 or 0.5 for float luma/rgb, 0.0 for float chroma
    - range_min --> 0 for 8-16 bits and non-UV 32bit, or -0.5 for float UV chroma (new from 2.2.15)
    - range_max --> 255/1023/4095/16383/65535 or 1.0 for float luma or 0.5 for float chroma
    - range_size --> 256/1024...65536
    - ymin, ymax, cmin, cmax --> 16/235 and 16/240 autoscaled. For zero based float: (16-128)/255.0 and (240-128)/255.0
      
Example #1 (bit depth dependent, all constants are treated as-is): 
```
      expr8_luma = "x 16 - 219 / 255 *"
      expr10_luma = "x 64 - 876 / 1023 *"
      expr16_luma = "x 4096 - 56064 / 65535 *"
```
      
Example #2 (new, with auto-scale operators )      
```
      expr_luma =  "x 16 scaleb - 219 scaleb / 255 scalef *"
      expr_chroma =  "x 16 scaleb - 224 scaleb / 255 scalef *"
```
      
Example #3 (new, with constants)
```
      expr_luma = "x ymin - ymax ymin - / range_max *"
      expr_chroma = "x cmin - cmax cmin - / range_max *"
      or works for float with: range_min: expr_chroma = "x cmin - cmax cmin - / range_max range_min - *"
```
  - new option for Lut expressions: 

    Parameter "clamp_float"

    True: clamps 32 bit float to valid ranges, which is 0..1 for Luma or for RGB color space and -0.5..0.5 for YUV chroma UV channels
    Default false, ignored (treated as true) when scale_inputs scales float
  - new option for Lut expressions: 

    parameter "scale_inputs" (default "none")

    Autoscale any input (x,y,z,a) bit depths to 8-16 bit for internal expression use, the conversion method
    is either full range or limited YUV range.
    Experimental, extends and replaces clamp_f_i8, clamp_f_i10, clamp_f_i12, clamp_f_i14 or clamp_f_i16, clamp_f_f32 or clamp_f
    
    Feature is available from v2.2.15

    The primary reason of this feature is the "easy" usage of formerly written expressions optimized for 8 bits.

    Use
    - "int" : scales limited range videos, only integer formats (8-16bits) to 8 (or bit depth specified by 'i8'..'i16')
    - "intf": scales full range videos, only integer formats (8-16bits) to 8 (or bit depth specified by 'i8'..'i16')
    - "float" or "floatf" : only scales 32 bit float format to 8 bit range (or bit depth specified by 'i8'..'i16')
    - "all": scales videos to 8 (or bit depth specified by 'i8'..'i16') - conversion uses limited_range logic (mul/div by two's power)
    - "allf": scales videos to 8 (or bit depth specified by 'i8'..'i16') - conversion uses full scale logic (stretch)
    - "none": no magic

    Usually limited range is for normal YUV videos, full scale is for RGB or known-to-be-fullscale YUV

    By default the internal conversion target is 8 bits, so old expressions written for 8 bit videos will probably work.
    This internal working bit-depth can be overwritten by the i8, i10, i12, i14, i16 specifiers.

    When using autoscale mode, scaleb and scalef keywords are meaningless, because there is nothing to scale.

    How it works:
    - This option scales (x,y,z,a) 8-32 bit inputs to a common bit depth value, which bit depth is 8 by default and can be 
      set to 10, 12, 14 and 16 bits by the 'i10'..'i16' keywords
      For example: scale_inputs="all" converts any inputs to 8 bit range. No truncation occurs however (no precision loss), 
      because even a 16 bit data is converted to 8 bit in floating point precision, using division by 256.0 (2^16/2^8). 
      So the conversion is _not_ a simple shift-right-8 in the integer domain, which would lose precision.
    - Calculates expression
    - Scales the result back to the output video bit depth.
      Clamping (clipping to valid range) and converting to integer occurs here.

    The predefined constants such as 'range_max', etc. will behave according to the internal working bit depth

    Warning#1 
    This feature was created for easy porting earlier 8-bit-video-only expressions.
    You have to understand how it works internally.

    Let's see a 16bit input in "all" and "allf" mode (target is the default 8 bits)

    Limited range 16->8 bits conversion has a factor of 1/256.0 (Instead of shift right 8 in integer domain, float-division is used or else it would lose presision)
    Full range 16->8 bits conversion has a factor of 255.0/65535

    Using bit shifts (really it's division and multiplication by 2^8=256.0): 
      result = calculate_lut_value(input / 256.0) * 256.0
    Full scale 16-8-16 bit mode ('intf', 'allf')
      result = calculate_lut_value(input / 65535.0 * 255.0 ) / 255.0 * 65535.0

    Use scale_inputs = "all" ("int", "float") for YUV videos with 'limited' range e.g. in 8 bits: Y=16..235, UV=16..240).
    Use scale_inputs = "allf" (intf, floatf) for RGB or YUV videos with 'full' range e.g. in 8 bits: channels 0..255.

    When input is 32bit float, the 0..1.0 (luma) and -0.5..0.5 (chroma) channel is scaled
    to 0..255 (8 bits), 0..1023 (i10 mode), 0..4095 (i12 mode), 0..16383(i14 mode), 0..65535(i16 mode) then back.

    Warning#2
    One cannot specify different conversion methods for converting before and after the expression.
    Neither can you specify different methods for different input clips (e.g. x is full, y is limited is not supported).
 
  - (obsolate) new expression syntax for lut-type filters: auto scale modifiers for float clips (test for real.finder):
    !!! Test only, will be removed in later editions, in v2.2.15 there are "scale_inputs" and "clamp_float" parameters! (also experimental)
    Keyword at the beginning of the expression:
    - clamp_f_i8, clamp_f_i10, clamp_f_i12, clamp_f_i14 or clamp_f_i16 for scaling and clamping
    - clamp_f_f32 or clamp_f: for clamping the result to 0..1 (floats are not clamped by default!)
  
    Input values 'x', 'y', 'z' and 'a' are autoscaled by 255.0, 1023.0, ... 65535.0 before the expression evaluation,
    so the working range is similar to native 8, 10, ... 16 bits. The predefined constants 'range_max', etc. will behave for 8, 10,..16 bits accordingly.
  
    The result is automatically scaled back to 0..1 _and_ is clamped to that range.
    When using clamp_f_f32 (or clamp_f) the scale factor is 1.0 (so there is no scaling), but the final clamping will be done anyway.
    No integer rounding occurs.

```
    # obsolate examples, from v2.2.15 use scale_inputs and clamp_float parameter instead of clamp_f_xx keywords
    expr = "x y - range_half +"  # good for 8..32 bits but float is not clamped
    expr = "clamp_f y - range_half +"  # good for 8..32 bits and float clamped to 0..1 (or +/-0.5 when chroma)
    expr = "x y - 128 + "  # good for 8 bits
    expr = "clamp_f_i8 x y - 128 +" # good for 8 bits and float, float will be clamped to 0..1 (or +/-0.5 when chroma)
    expr = "clamp_f_i8 x y - range_half +" # good for 8..32 bits, float will be clamped to 0..1 (or +/-0.5 when chroma)
```  

- parameter "stacked" (default false) for filters with stacked format support
  Stacked support is not intentional, but since tp7 did it, I did not remove the feature.
  Filters currently without stacked support will never have it. 
- parameter "realtime" for lut-type filters, slower but at least works on those bit depths
  where LUT tables would occupy too much memory.

  Also see: 'use_expr' which can pass realtime calculation to Avisynth+ Expr filter!

  For bit depth limits where realtime = true is set as the default working mode, see table below.
    
  realtime=true can be overridden, one can experiment and force realtime=false even for a 16 bit lutxy 
  (8GBytes lut table!, x64 only) or for 8 bit lutxzya (4GBytes lut table)

- parameter "use_expr" integer (default 0) for 'lut', 'lutxy', 'lutxyz', 'lutxyza' filters (from v2.2.15)
  Use it when realtime calculation (interpreted pixel-by-pixel expression calculation) is slow and an appropriate Avisynth+ version (>r2712) is available.

  By sending the expression to Avisynth+, lut filters can utilize a realtime JIT-compiled fast expression calculation.

  Possible values:
  0: uses lut and internal realtime calculation
  1: Expr, when bit depth>=10 or lutxyza
  2: When masktools would use realtime calc,  Passes the expressions, scale_inputs and clamp_float parameter to the "Expr" filter in Avisynth+

  For modes 1 and 2: passes the expression strings, scale_inputs and clamp_float parameters to the "Expr" filter in Avisynth+

  Note #1: Avisynth+ internal precision is 32bit float, masktools2 is double (usually no difference can be seen)
  Note #2: Some keywords (e.g. bit shift) are not available on Avisynth+
   
- parameter "paramscale" for filters working with threshold-like parameters (v2.2.5-)
  Filters: mt_binarize, mt_edge, mt_inpand, mt_expand, mt_inflate, mt_deflate, mt_motion, mt_logic, mt_clamp
  paramscale can be "i8" (default), "i10", "i10", "i12", "i14", "i16", "f32" or "none" or ""
  Using "paramscale" tells the filter that parameters are given at what bit depth range.
  By default paramscale is "i8", so existing scripts with parameters in the 0..255 range are working at any bit depths
```  
  mt_binarize(threshold=80*256, paramscale="i16") # threshold is assumed in 16 bit range 
  mt_binarize(threshold=80) # no param: threshold is assumed in 8 bit range
  
  thY1 = 0.1
  thC1 = 0.1
  thY2 = 0.1
  thC2 = 0.1
  paramscale="f32"
  mt_edge(mode="sobel",u=3,v=3,thY1=thY1,thY2=thY2,thC1=thC1,thC2=thC2,paramscale=paramscale) # f32: parameters assumed as float (0..1.0)
```  
- new: "swap" keyword in expressions (v2.2.5-)
  swaps the last two results during RPN evaluation. Not compatible with mt_infix()
```  
  expr="x 2 /"
  expr="2 x swap /"
```  
- new: "dup" keyword in expressions (v2.2.5-)
  duplicates the last result and put on the top of RPN evaluation stack. Not compatible with mt_infix()
```  
  expr="x 3 / x 3 / +"
  expr="x 3 / dup +"
```  
  
   
- Feature matrix   
```
                   8 bit | 10-16 bit | float | stacked | realtime
      mt_invert      X         X         X        -
      mt_binarize    X         X         X        X
      mt_inflate     X         X         X        X
      mt_deflate     X         X         X        X
      mt_inpand      X         X         X        X
      mt_expand      X         X         X        X
      mt_lut         X         X         X        X      when float
      mt_lutxy       X         X         X        -      when bits>=14
      mt_lutxyz      X         X         X        -      when bits>=10
      mt_lutxyza     X         X         X        -      always
      mt_luts        X         X         X        -      when bits>=14 
      mt_lutf        X         X         X        -      when bits>=14   
      mt_lutsx       X         X         X        -      when bits>=10
      mt_lutspa      X         X         X        -
      mt_merge       X         X         X        X
      mt_logic       X         X         X        X
      mt_convolution X         X         X        -
      mt_mappedblur  X         X         X        -
      mt_gradient    X         X         X        -
      mt_makediff    X         X         X        X
      mt_average     X         X         X        X
      mt_adddiff     X         X         X        X
      mt_clamp       X         X         X        X
      mt_motion      X         X         X        -
      mt_edge        X         X         X        -
      mt_hysteresis  X         X         X        -
      mt_infix/mt_polish: available only on non-XP builds
``` 
Masktools2 info:
http://avisynth.nl/index.php/MaskTools2

Forum:
https://forum.doom9.org/showthread.php?t=174333

Article by tp7
http://tp7.github.io/articles/masktools/

Project:
https://github.com/pinterf/masktools/tree/16bit/

Original version: tp7's MaskTools 2 repository.
https://github.com/tp7/masktools/

Changelog
**v2.2.15 (20180614)
- 32 bit float U and V chroma channels are now zero based (+/-0.5 for full scale). Was: 0..1, same as luma
  (Following the change in Avisynth+ over r2664: use this plugin with r2996 or newer)
  Affected predefined expression constants when plane is U or V: 
  cmin and cmax (limited range (16-128)/255 and (240-128)/255 instead of 16/255.0 and 240/255.0
  range_max: 0.5 instead of 1.0
  new: introduce range_min: -0.5 for float U/V chroma, 0 otherwise
  range_half (0.0 instead of 0.5)
  (range_size remained 1.0)
- New expression syntax for Lut expressions: autoscale any input (x,y,z,a) bit depths to 8-16 bits for internal 
  expression use. The primary reason of this feature is the "easy" usage of formerly written 8 bit optimized expressions.

  New parameters for lut functions: 
    String "scale_inputs": "all","allf","int","intf","float","floatf","none", default "none"
  and 
    Boolean "clamp_float": default false, but treated as always true (and thus ignored) when scale_inputs involves a float autoscale.
  and 
    Boolean "use_expr": default 0, calls fast JIT-compiled "Expr" in Avisynth+ for mt_lut, lutxy, lutxyz, lutxyza
    0: no Expr, use slow internal realtime calc if needed
    1: call Expr for bits>8 or lutxyza
    2: call Expr, when masktools would do its slow realtime calc

  Extends and replaces experimental clamp_xxxx keywords.

**v2.2.14 (20180225)
- Fix: mt_convolution invalid instruction on processors below SSE4.1

**v2.2.13 (20180201)
- Fix: rare crash in multithreading environment at the very first frames 
  (keeping XP compatibility with /Z:threadsafeinit- caused troubles!)
- mt_edge: AVX2 (1.4-1.9x speed) for 8 and 10-16 bits
- fix: "chroma" parameter with negative (memset) values were not working properly for 10-14 bits and 32bit float

**v2.2.12 (20180107)
- Fix: mt_merge 10-16 bits: right side artifacts when clip is non-mod 8 (non-AVX2) or mod16 (AVX2) widths

**v2.2.11 (20180105)
- Fix: mt_merge luma=true: broken output when: 8-16 bits AVX2, 32 bit float: SSE2, AVX
- move project to VS2017, vs141_xp toolset

**v2.2.10 (20170612)
- Fix: luts internal buffer overflow (crash)
- Speed: mt_inpand/mt_expand: 10-16 bits SSE4 (10-15x speed)
- Speed: mt_inflate/mt_deflate 10-16 bits SSE4 (4x speed)

**v2.2.9 (20170608)
- Add "none" and "ignore" to valid values for "chroma" and "alpha" parameters.
- Report error for invalid "chroma" or "alpha" parameter values instead of exception

**v2.2.8 (20170427)
- Fix: "chroma" and "alpha" parameter should be scaled like "Y","U","V" and "A" when providing negative (memset) values

**v2.2.7 (20170421)
- fix: mt_edge 10,12,14 bits: clamp mask value from 65535 to 1023 (10 bits), 4095 (12 bits) and 16383 (14 bits)
- fix: mt_merge 10-16 bits + non mod-16 width + luma=true + 4:2:2 colorspace, correct right side pixels
- fix: mt_merge 8 bit clips: keep original pixels from clip1/2 when mask is exactly 0 or 255
- YUVA, RGBAP support 8-32 bits
  - "A" parameter like "Y", "U" and "V". Default value for "A" is 1 (do nothing, same as for "U" and "V")
  - "alpha" parameter like "chroma" - overrides default plane mode
  - aExpr parameter for lut-type filters like uExpr, and vExpr
  - awExpr parameter for mt_luts like uwExpr, and vwExpr
  - dual signature filters (both integer and float) are provided in separate binaries
    In some cases specifying two different parameter lists with the same variables can cause troubles.
    (dual signature version can be used to override an earlierly loaded different masktools version
    (e.g. a 2.5 plugin) by defining the filters with both integer parameters _AND_ the new float parameter lists)
- Make "paramscale" to work consistent with all filters and parameters:
  parameters "Y","U","V" and "A" negative (memset) values are scaled automatically to the current bit depth from a default 8-bit value.
- New plane mode: 6 (copy from fourth clip) for "Y", "U", "V" and "A"
  New "chroma" and "alpha" plane mode override: "copy fourth"
  Use for mt_lutxyza which has four clips

**v2.2.6 (20170401)
- fix: >>u operator AV error

**v2.2.5 (20170330)
- Change #F and #B operators to @B and @F (# is reserved for Avisynth in-string comment character)
- Alias scaleb for @B
- Alias scalef for @F
- New: automatic scaling of parameters (threshold-like, sc_value) from the usual 8 bit range
  Scripts need no extra measures to work for all bit depths with the same "command line"
- New parameter "paramscale" for filters working with threshold-like parameters
  Filters: mt_binarize, mt_edge, mt_inpand, mt_expand, mt_inflate, mt_deflate, mt_motion, mt_logic, mt_clamp
  paramscale can be "i8" (default), "i10", "i10", "i12", "i14", "i16", "f32" or "none" or ""
  Using "paramscale" tells the filter that parameters are given at what bit depth range.
- dual function signatures (float and int), for backward compatibility with integer-type parameter list, prevent usage of earlier plugin-loaded masktools version
- keep old parameter ordering: parameters which are non-existant in 2.0b1 are inserted at the end of the parameter list, not before the common parameters Y, U, V
- new: "swap" keyword in expressions
- new: "dup" keyword in expressions
- a bit faster realtime lut calculation for 10+ bit depths
  
**v2.2.4 (20170304)
- mt_polish to recognize:
  - new v2.2.x constants and variables
    a, bitdepth, sbitdepth, range_half, range_max, range_size, ymin, ymax, cmin, cmax
  - v2.2.x scaling functions (written as #B(expression) and #F(expression) for mt_polish)
    #B() #F
  - single operand unsigned and signed negate introduced in 2.0.48
    ~u and ~s (written as ~u(expression)and ~s(expression) for mt_polish)
  - other operators introduced in 2.0.48:
    @, &u, |u, °u, @u, >>, >>u, <<, <<u, &s, |s, °s, @s, >>s, <<s
- mt_infix: don't put extra parameter after #F( and #B(
- new expression syntax: auto scale modifiers for float clips (test for real.finder):
  Keyword at the beginning of the expression:
  - clamp_f_i8, clamp_f_i10, clamp_f_i12, clamp_f_i14 or clamp_f_i16 for scaling and clamping
  - clamp_f_f32 or clamp_f: for clamping the result to 0..1
  
  Input values 'x', 'y', 'z' and 'a' are autoscaled by 255.0, 1023.0, ... 65535.0 before the expression evaluation,
  so the working range is similar to native 8, 10, ... 16 bits. The predefined constants 'range_max', etc. will behave for 8, 10,..16 bits accordingly.
  
  The result is automatically scaled back to 0..1 _and_ is clamped to that range.
  When using clamp_f_f32 (or clamp_f) the scale factor is 1.0 (so there is no scaling), but the final clamping will be done anyway.
  No integer rounding occurs.

**v2.2.3 (20170227)**
- mt_logic to 32 bit float (final filter lacking it)
- get CpuInfo from Avisynth (avx/avx2 preparation)
  Note: AVX/AVX2 prequisites
  - recent Avisynth+ which reports extra CPU flags
  - 64 bit OS (but Avisynth can be 32 bits)
  - Windows 7 SP1 or later
- mt_merge: 8-16 bit: AVX2, float:AVX
- mt_logic: 8-16 bit: AVX2, float:AVX
- mt_edge: 10-16 bit and 32 bit float: SSE2/SSE4 optimization
- mt_edge: 32 bit float AVX
- new: mt_luts: weight expressions as an addon for then main expression(s) (martin53's idea)
    - wexpr
    - ywExpr, uwExpr, vwExpr
  If the relevant parameter strings exist, the weighting expression is evaluated
  for each source/neighborhood pixel values (lut or realtime, depending on the bit depth and the "realtime" parameter). 
  Then the usual lut result is premultiplied by this weight factor before it gets accumulated. 

**v2.2.2 (20170223)** completed high bit depth support
- All filters work in 10,12,14,16 bits and float (except mt_logic which is 8-16 only)
- mt_lutxyza 4D lut available with realtime=false! (4 GBytes LUT table, slower initial lut table calculation)
  Allowed only on x64. When is it worth?
  Number of expression evaluations for 4G lut calculation equals to realtime calculation on 
  2100 frames (1920x1080 plane). Be warned, it would take be minutes.
- mt_gradient 10-16 bit / float
- mt_convolution 10-16 bit / float
- mt_motion 10-16 bit / float
- mt_xxpand and mt_xxflate to float
- mt_clamp to float
- mt_merge to float
- mt_binarize to float
- mt_invert to float
- mt_makediff and mt_adddiff to float
- mt_average to float
- Expression syntax supporting bit depth independent expressions: 
  Added configuration keywords i8, i10, i12, i14, i16 and f32 in order to inform the expression evaluator 
  about bit depth of the values that are to scale by #B and #F operators.
 
**v2.2.1 (20170218)** initial high bit depth release

- project moved to Visual Studio 2015 Update 3
  Requires VS2015 Update 3 redistributables
- Fix: mt_merge (and probably other multi-clip filters) may result in corrupted results 
  under specific circumstances, due to using video frame pointers which were already released from memory
- no special function names for high bit depth filters
- filters are auto registering their mt mode as MT_NICE_FILTER for Avisynth+
- Avisynth+ high bit depth support (incl. planar RGB, but color spaces with alpha plane are not yet supported)
  For accepted bit depth, see table below.
- YV411 (8 bit 4:1:1) support
- mt_merge accepts 4:2:2 clips when luma=true (8-16 bit)
- mt_merge accepts 4:1:1 clips when luma=true
- mt_merge to discard U and V automatically when input is greyscale
- new: mt_lutxyza. Accepts four clips. 4th variable name is 'a' (besides x, y and z)
- expression syntax supporting bit depth independent expressions
  - bit-depth aware scale operators #B and #F
  - pre-defined bit depth aware constants
    bitdepth (automatic silent parameter of the lut expression)
    range_half, range_max, range_size, ymin, ymax, cmin, cmax
  - parameter "stacked" (default false) for filters with stacked format support
  - parameter "realtime" for lut-type filters to override default behaviour
  
