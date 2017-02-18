### MaskTools 2 ###

Masktools2 v2.2.1 (20170218)
mod by pinterf

Differences to Masktools 2.0b1

  - project moved to Visual Studio 2015 Update 3
    Requires VS2015 Update 3 redistributables
  - Fix: mt_merge (and probably other multi-clip filters) may result in corrupted results 
    under specific circumstances, due to using video frame pointers which were already released from memory
  - no special function names for high bit depth filters
  - Avisynth+ high bit depth support (incl. planar RGB, but color spaces with alpha plane are not yet supported)
    For accepted bit depth, see table below.
  - YV411 (8 bit 4:1:1) support
  - mt_merge accepts 4:2:2 clips when luma=true (8-16 bit)
  - mt_merge accepts 4:1:1 clips when luma=true
  - mt_merge to discard U and V automatically when input is greyscale
  - new: mt_lutxyza. Accepts four clips. 4th variable name is 'a' (besides x, y and z)
  - expression syntax
    - new: bit-depth aware scale operators
      operator #B scales from 8 bit to current bit depth using bit-shifts
                  Use this for YUV. "235 #B" -> always results in max luma 
      operator #F scales from 8 bit to current bit depth using full range stretch
                  "255 #F" results in maximum pixel value of current bit depth.
                  Calculation: x/255*65535 for a 8->16 bit sample (rgb)
    - new pre-defined, bit depth aware constants
      bitdepth: automatic silent parameter of the lut expression
      range_half --> autoscaled 128 or 0.5 for float
      range_max --> 255/1023/4095/16383/65535 or 1.0 for float
      range_size --> 256/1024...65536
      ymin, ymax, cmin, cmax --> 16/235 and 16/240 autoscaled.
      
      Example #1 (old): 
        expr8_luma = "x 16 - 219 / 255 *"
        expr10_luma = "x 64 - 876 / 1023 *"
        expr16_luma = "x 4096 - 56064 / 65535 *"
      
      Example #2 (new, with auto-scale operators )
        expr_luma =  "x 16 #B - 219 #B / 255 #F *"
        expr_chroma =  "x 16 #B - 224 #B / 255 #F *"
        
      Example #3 (new, with constants)
        expr_luma = "x ymin - ymax ymin - / range_max *"
        expr_chroma = "x cmin - cmax cmin - / range_max *"

  - parameter "stacked" (default false) for filters with stacked format support
    Stacked support is not intentional, but since tp7 did it, I did not remove the feature.
    Filters currently without stacked support will never have it. 
  - parameter "realtime" for lut-type filters, slower but at least works on those bit depths
    where LUT tables would occupy too much memory.
    For bit depth limits where realtime = true will be the default working mode, see table below.
    But you can force realtime=false even for a 16 bit lutxy (8GBytes lut table, x64 only)
   
  - Feature matrix   
  
                   8 bit | 10-16 bit | float | stacked | realtime
      mt_invert      X         X         -        -
      mt_binarize    X         X         -        X
      mt_inflate     X         X         -        X
      mt_deflate     X         X         -        X
      mt_inpand      X         X         -        X
      mt_expand      X         X         -        X
      mt_lut         X         X         X        X      when float
      mt_lutxy       X         X         X        -      when bits>=14
      mt_lutxyz      X         X         X        -      when bits>=10
      mt_lutxyza     X         X         X        -      always
      mt_luts        X         -         -        -
      mt_lutf        X         X         X        -      when bits>=14   
      mt_lutsx       X         -         -        -
      mt_lutspa      X         X         X        -      
      mt_merge       X         X         -        X      
      mt_logic       X         X         -        X      
      mt_convolution X         -         -        -     
      mt_mappedblur  X         X         X        -                
      mt_gradient    X         -         -        -              
      mt_makediff    X         X         -        X      
      mt_average     X         X         -        X      
      mt_adddiff     X         X         -        X      
      mt_clamp       X         X         -        X      
      mt_motionmask  X         -         -        -    
      mt_edge        X         X         X        -
      mt_hysteresis  X         X         X        -
      mt_infix/mt_polish: available only on non-XP builds
 
Masktools2 info:
http://avisynth.nl/index.php/MaskTools2

Article by tp7
http://tp7.github.io/articles/masktools/

Project:
https://github.com/pinterf/masktools/tree/16bit/masktools

Original version: tp7's MaskTools 2 repository.
https://github.com/tp7/masktools/


