FILE(GLOB common_Sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "*.c"
  "*.cpp"
  "*.h"
  "../masktools/avsheaders/*.h"
  "../masktools/avsheaders/avs/*.h"
  "constraints/*.h"
  "constraints/*.cpp"
  "functions/*.cpp"
  "parser/*.cpp"
)

message("${common_Sources}")

IF( MSVC OR MINGW )
    # Export definitions in general are not needed on x64 and only cause warnings,
    # unfortunately we still must need a .def file for some COM functions.
    # NO C interface for this plugin
    # if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    #  LIST(APPEND common_Sources "common64.def")
    # else()
    #  LIST(APPEND common_Sources "common.def")
    # endif() 
ENDIF()

IF( MSVC_IDE )
    # Ninja, unfortunately, seems to have some issues with using rc.exe
# common lib of masktools is not a versionable entity
#    LIST(APPEND common_Sources "common.rc")
ENDIF()
