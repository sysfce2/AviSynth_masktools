#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include "DeviceLocalData.h"
#include "../../../common/parser/parser.h"

namespace Filtering {

class LutData : DeviceLocalBase
{
   size_t size_per_plane; // num elements
   int element_size;   // in bytes
   int num_planes;

public:
   LutData(void* datq, size_t size_per_plane, int element_size, int num_planes, PNeoEnv env)
      : DeviceLocalBase(datq, size_per_plane * element_size * num_planes, env)
      , size_per_plane(size_per_plane)
      , element_size(element_size)
      , num_planes(num_planes)
   { }

   const void* GetTable(int plane, PNeoEnv env) {
      assert(plane >= 0 && plane < num_planes);
      return (uint8_t*)GetData_(env) + size_per_plane * element_size * plane;
   }
};

std::unique_ptr<LutData> make_lut_data(int bits_per_pixel, int num_input,
   const std::vector<std::unique_ptr<Parser::Context>>& exprs, PNeoEnv env);

} // namespace Filtering
