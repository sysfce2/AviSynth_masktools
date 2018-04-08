#pragma once

#include "lut_data.h"

namespace Filtering {

template <int bits_per_pixel>
std::unique_ptr<LutData> make_lut_data(int num_input,
   const std::vector<std::unique_ptr<Parser::Context>>& exprs, PNeoEnv env)
{
   int num_planes = (int)exprs.size();
   int depth = (1 << bits_per_pixel);
   size_t size_per_plane = (size_t(1) << (bits_per_pixel * num_input));
   std::vector<uint16_t> data(size_per_plane * num_planes);
   for (int i = 0; i < num_planes; ++i) {
      if (exprs[i]) {
         auto ptr = data.begin() + size_per_plane * i;
         switch (num_input) {
         case 1:
            for (int x = 0; x < depth; ++x) {
               ptr[x] = exprs[i]->compute_word_x<bits_per_pixel>(x);
            }
            break;
         case 2:
            for (int x = 0; x < depth; ++x) {
               for (int y = 0; y < depth; ++y) {
                  int idx = (x << bits_per_pixel) + y;
                  ptr[idx] = exprs[i]->compute_word_xy<bits_per_pixel>(x, y);
               }
            }
            break;
         //case 3:
         //   for (int x = 0; x < depth; ++x) {
         //      for (int y = 0; y < depth; ++y) {
         //         for (int z = 0; z < depth; ++z) {
         //            int idx = (x << (bits_per_pixel * 2)) + (y << bits_per_pixel) + z;
         //            ptr[idx] = exprs[i]->compute_word_xyz<bits_per_pixel>(x, y, z);
         //         }
         //      }
         //   }
         //   break;
         }
      }
   }
   return std::unique_ptr<LutData>(new LutData(data.data(), size_per_plane, sizeof(uint16_t), num_planes, env));
}

template <>
std::unique_ptr<LutData> make_lut_data<8>(int num_input,
   const std::vector<std::unique_ptr<Parser::Context>>& exprs, PNeoEnv env)
{
   int num_planes = (int)exprs.size();
   int depth = (1 << 8);
   size_t size_per_plane = (size_t(1) << (8 * num_input));
   std::vector<uint8_t> data(size_per_plane * num_planes);
   for (int i = 0; i < num_planes; ++i) {
      auto ptr = data.begin() + size_per_plane * i;
      switch (num_input) {
      case 1:
         for (int x = 0; x < depth; ++x) {
            ptr[x] = exprs[i]->compute_byte_x(x);
         }
         break;
      case 2:
         for (int x = 0; x < depth; ++x) {
            for (int y = 0; y < depth; ++y) {
               int idx = (x << 8) + y;
               ptr[idx] = exprs[i]->compute_byte_xy(x, y);
            }
         }
         break;
      case 3:
         for (int x = 0; x < depth; ++x) {
            for (int y = 0; y < depth; ++y) {
               for (int z = 0; z < depth; ++z) {
                  int idx = (x << (8 * 2)) + (y << 8) + z;
                  ptr[idx] = exprs[i]->compute_byte_xyz(x, y, z);
               }
            }
         }
         break;
      }
   }
   return std::unique_ptr<LutData>(new LutData(data.data(), size_per_plane, sizeof(uint8_t), num_planes, env));
}

std::unique_ptr<LutData> make_lut_data(int bits_per_pixel, int num_input,
   const std::vector<std::unique_ptr<Parser::Context>>& exprs, PNeoEnv env)
{
   if (bits_per_pixel * num_input > 24) {
      env->ThrowError("[kmt_lut] %d bit %d input is not supported");
   }

   switch (bits_per_pixel) {
   case 8: return make_lut_data<8>(num_input, exprs, env);
   case 10: return make_lut_data<10>(num_input, exprs, env);
   case 12: return make_lut_data<12>(num_input, exprs, env);
   case 14: return make_lut_data<14>(num_input, exprs, env);
   case 16: return make_lut_data<16>(num_input, exprs, env);
   }

   // never come here
   return std::unique_ptr<LutData>();
}

} // namespace Filtering
