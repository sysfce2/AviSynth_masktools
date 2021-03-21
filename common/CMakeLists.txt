# Visual Studio 2019 is supported from CMake 3.14.1
# Tested generators:
# "MinGW Makefiles": MSYS2/Mingw32 GCC 8.3 build
# "Visual Studio 16 2019" optional platform generator Win32 and x64
# "Visual Studio 16 2019" + LLVM 8.0 (clang) optional platform generator Win32 and x64
CMAKE_MINIMUM_REQUIRED( VERSION 3.8.2 )

# not a plugin, common is a static library for the main masktools
set(PluginName "common")

if (NOT WIN32)
  string(TOLOWER "${PluginName}" PluginName)
endif()

set(ProjectName "${PluginName}")
project(${ProjectName} LANGUAGES CXX)

Include("Files.cmake")

# STATIC instead of shared. common is linked to masktools2
add_library(${PluginName} STATIC ${common_Sources})

set_target_properties(${PluginName} PROPERTIES "OUTPUT_NAME" "${PluginName}")
if (MINGW)
  set_target_properties(${PluginName} PROPERTIES PREFIX "")
  set_target_properties(${PluginName} PROPERTIES IMPORT_PREFIX "")
endif()

#require sse4.1, there are mixed parts in the source which is pain to separate to sse2/sse4.1
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINTEL_INTRINSICS -msse4.1")

# position independent code
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Automatically group source files according to directory structure
foreach(FILE ${common_Sources}) 
  get_filename_component(PARENT_DIR "${FILE}" PATH)

  string(REGEX REPLACE "(\\./)" "" GROUP "${PARENT_DIR}")
  string(REPLACE "/" "\\" GROUP "${GROUP}")

  # group into "Source Files" and "Header Files"
  if ("${FILE}" MATCHES ".*\\.cpp")
    set(GROUP "Source Files\\${GROUP}")
  elseif("${FILE}" MATCHES ".*\\.h")
    set(GROUP "Header Files\\${GROUP}")
  endif()

  source_group("${GROUP}" FILES "${FILE}")
endforeach()

if (MSVC_IDE)
  IF(CLANG_IN_VS STREQUAL "1")
     # special SSSE3 option for source files with *_ssse3.cpp pattern
      file(GLOB_RECURSE SRCS_SSSE3 "*_ssse3.cpp")
      set_source_files_properties(${SRCS_SSSE3} PROPERTIES COMPILE_FLAGS " -mssse3 ")

      # special SSE4.1 option for source files with *_sse41.cpp pattern
      file(GLOB_RECURSE SRCS_SSE41 "*_sse41.cpp")
      set_source_files_properties(${SRCS_SSE41} PROPERTIES COMPILE_FLAGS " -msse4.1 ")

      # special AVX option for source files with *_avx.cpp pattern
      file(GLOB_RECURSE SRCS_AVX "*_avx.cpp")
      set_source_files_properties(${SRCS_AVX} PROPERTIES COMPILE_FLAGS " -mavx ")

      # special AVX2 option for source files with *_avx2.cpp pattern
      file(GLOB_RECURSE SRCS_AVX2 "*_avx2.cpp")
      set_source_files_properties(${SRCS_AVX2} PROPERTIES COMPILE_FLAGS " -mavx2 -mfma ")

      # special AVX512 option for source files with *_avx512.cpp pattern
      file(GLOB_RECURSE SRCS_AVX512 "*_avx512.cpp")
      set_source_files_properties(${SRCS_AVX512} PROPERTIES COMPILE_FLAGS " -mavx512f -mavx512bw ")
  ELSE()
      # special AVX option for source files with *_avx.cpp pattern
      file(GLOB_RECURSE SRCS_AVX "*_avx.cpp")
      set_source_files_properties(${SRCS_AVX} PROPERTIES COMPILE_FLAGS " /arch:AVX ")

      # special AVX2 option for source files with *_avx2.cpp pattern
      file(GLOB_RECURSE SRCS_AVX2 "*_avx2.cpp")
      set_source_files_properties(${SRCS_AVX2} PROPERTIES COMPILE_FLAGS " /arch:AVX2 ")

      # special AVX512 option for source files with *_avx512.cpp pattern
      file(GLOB_RECURSE SRCS_AVX512 "*_avx512.cpp")
      set_source_files_properties(${SRCS_AVX512} PROPERTIES COMPILE_FLAGS " /arch:AVX512 ")
  ENDIF()
else()
  # special SSSE3 option for source files with *_ssse3.cpp pattern
  file(GLOB_RECURSE SRCS_SSSE3 "*_ssse3.cpp")
  set_source_files_properties(${SRCS_SSSE3} PROPERTIES COMPILE_FLAGS " -mssse3 ")

  # special SSE4.1 option for source files with *_sse41.cpp pattern
  file(GLOB_RECURSE SRCS_SSE41 "*_sse41.cpp")
  set_source_files_properties(${SRCS_SSE41} PROPERTIES COMPILE_FLAGS " -msse4.1 ")

  # special AVX option for source files with *_avx.cpp pattern
  file(GLOB_RECURSE SRCS_AVX "*_avx.cpp")
  set_source_files_properties(${SRCS_AVX} PROPERTIES COMPILE_FLAGS " -mavx ")

  # special AVX2 option for source files with *_avx2.cpp pattern
  file(GLOB_RECURSE SRCS_AVX2 "*_avx2.cpp")
  set_source_files_properties(${SRCS_AVX2} PROPERTIES COMPILE_FLAGS " -mavx2 -mfma ")

  # special AVX512 option for source files with *_avx512.cpp pattern
  file(GLOB_RECURSE SRCS_AVX512 "*_avx512.cpp")
  set_source_files_properties(${SRCS_AVX512} PROPERTIES COMPILE_FLAGS " -mavx512f -mavx512bw ")
endif()


# Specify include directories
target_include_directories(${ProjectName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#dedicated include dir for avisynth.h
target_include_directories(${ProjectName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Windows DLL dependencies 
#if (MSVC OR MINGW)
target_link_libraries(${ProjectName})
#else
target_link_libraries(${ProjectName})
# "dl" "pthread"
#endif()

include(GNUInstallDirs)

# we do not install it. this is not the main project
#INSTALL(TARGETS ${ProjectName}
#        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/avisynth")
