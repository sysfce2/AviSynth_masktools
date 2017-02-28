### MaskTools 2 ###

**Masktools2 v2.2.4 (20170228 ??)
**Masktools2 v2.2.3 (20170227)**

mod by pinterf

Differences to Masktools 2.0b1

- project moved to Visual Studio 2015 Update 3
  Requires VS2015 Update 3 redistributables
- Fix: mt_merge (and probably other multi-clip filters) may result in corrupted results 
  under specific circumstances, due to using video frame pointers which were already released from memory
- no special function names for high bit depth filters
- filters are auto registering their mt mode as MT_NICE_FILTER for Avisynth+
- Avisynth+ high bit depth support (incl. planar RGB, but color spaces with alpha plane are not yet supported)
  All filters are now supporting 10, 12, 14, 16 bits and float
  Note that parameters like threshold are not scaled automatically to the current bit depth
- YV411 (8 bit 4:1:1) support
- mt_merge accepts 4:2:2 clips when luma=true (8-16 bit)
- mt_merge accepts 4:1:1 clips when luma=true
- mt_merge to discard U and V automatically when input is greyscale
- some filters got AVX (float) and AVX2 (integer) support:
  mt_merge: 8-16 bit: AVX2, float:AVX
  mt_logic: 8-16 bit: AVX2, float:AVX
  mt_edge: 10-16 bit and 32 bit float: SSE2/SSE4 optimization
  mt_edge: 32 bit float AVX
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
  
      operator #B scales from 8 bit to current bit depth using bit-shifts
                  Use this for YUV. "235 #B" -> always results in max luma
                  
      operator #F scales from 8 bit to current bit depth using full range stretch
                  "255 #F" results in maximum pixel value of current bit depth.
                  Calculation: x/255*65535 for a 8->16 bit sample (rgb)
                  
  - hints for non-8 bit based constants: 
      Added configuration keywords i8, i10, i12, i14, i16 and f32 in order to tell the expression evaluator 
      the bit depth of the values that are to scale by #B and #F operators.
            
      By default #B and #F scales from 8 bit to the bit depth of the clip.
      
      i8 .. i16 and f32 sets the default conversion base to 8..16 bits or float, respectively.
      
      These keywords can appear anywhere in the expression, but only the last occurence will be effective for the whole expression.
      
      Examples 
```
      8 bit video, no modifier: "x y - 256 #B *" evaluates as "x y - 256 *"
      10 bit video, no modifier: "x y - 256 #B *" evaluates as "x y - 1024 *"
      10 bit video: "i16 x y - 65536 #B *" evaluates as "x y - 1024 *"
      8 bit video: "i10 x y - 512 #B *" evaluates as "x y - 128 *"                  
```

  - new pre-defined, bit depth aware constants
  
    - bitdepth: automatic silent parameter of the lut expression      
    - range_half --> autoscaled 128 or 0.5 for float
    - range_max --> 255/1023/4095/16383/65535 or 1.0 for float
    - range_size --> 256/1024...65536
    - ymin, ymax, cmin, cmax --> 16/235 and 16/240 autoscaled.
      
Example #1 (bit depth dependent, all constants are treated as-is): 
```
      expr8_luma = "x 16 - 219 / 255 *"
      expr10_luma = "x 64 - 876 / 1023 *"
      expr16_luma = "x 4096 - 56064 / 65535 *"
```
      
Example #2 (new, with auto-scale operators )      
```
      expr_luma =  "x 16 #B - 219 #B / 255 #F *"
      expr_chroma =  "x 16 #B - 224 #B / 255 #F *"
```
      
Example #3 (new, with constants)
```
      expr_luma = "x ymin - ymax ymin - / range_max *"
      expr_chroma = "x cmin - cmax cmin - / range_max *"
```
- parameter "stacked" (default false) for filters with stacked format support
  Stacked support is not intentional, but since tp7 did it, I did not remove the feature.
  Filters currently without stacked support will never have it. 
- parameter "realtime" for lut-type filters, slower but at least works on those bit depths
  where LUT tables would occupy too much memory.
  For bit depth limits where realtime = true is set as the default working mode, see table below.
    
  realtime=true can be overridden, one can experiment and force realtime=false even for a 16 bit lutxy 
  (8GBytes lut table!, x64 only) or for 8 bit lutxzya (4GBytes lut table)
   
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
**v2.2.4 (20170228?)
- mt_polish to recognize:
  - new v2.2.x constants and variables
    a, bitdepth, sbitdepth, range_half, range_max, range_size, ymin, ymax, cmin, cmax
  - v2.2.x scaling functions (written as #B(expression) and #F(expression) for mt_polish)
    #B() #F
  - one opearand unsigned and signed negate introduced in older versions
    ~u and ~s (written as ~u(expression)and ~s(expression) for mt_polish)
  - other operators introduced in older versions:
    @, &u, |u, °u, @u, >>, >>u, <<, <<u, &s, |s, °s, @s, >>s, <<s,

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
  
