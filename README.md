### MaskTools 2 ###

Masktools2 v2.2.0.0 (20170212)

Test version

Changed:
- mt_merge accepts 4:2:2 clips when luma=true
- mt_merge to discard U and V automatically when input is greyscale
- Some filters got native 10-16 bits support with recent Avisynth+ versions for YUV and planar RGB data 
  (no alpha plane at the moment)
- 16 bit filters accept stacked clips. Specify stacked=true for them.
  Stacked support is not intentional, but since tp7 did it, I did not remove the feature.
  Filters that will be ported later, probably will not support stacked format.
- Compiled with Visual Studio 2015 Update 3, with XP support
  Require VS2015 Update 3 redistributables.

Fix
- mt_merge (and probably other multi-clip filters) may result in corrupted results 
  under specific circumstances, due to using video frame pointers which were already released from memory
  
New filters that work for 10-16 bit data on Avisynth+ or stacked data on previous Avisynth versions
- mt_binarize16
- mt_lut16
- mt_logic16
- mt_merge16
- mt_average16
- mt_makediff16
- mt_adddiff16
- mt_clamp16
- mt_inflate16, mt_deflate16
- mt_inpand16, mt_expand16

Notes: 
- There is no automatic range scaling in expressions.
- In future versions the base filters (e.g. mt_merge) will accept any-format clip automatically.
  No separate mt_merge16 will be necessary for non-8 bit clips.

Masktools2 info:
http://avisynth.nl/index.php/MaskTools2

Project:
https://github.com/pinterf/masktools/tree/16bit/masktools

Original version: tp7's MaskTools 2 repository.
https://github.com/tp7/masktools/


