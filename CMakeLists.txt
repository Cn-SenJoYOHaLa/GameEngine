cmake_minimum_required (VERSION 3.1) 
project (GameEngineFromScrath)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)
if(NOT MSVC)
set (CMAKE_CXX_FLAGS "-Wno-gnu-anonymous-struct -Wno-nested-anon-types")
endif(NOT MSVC)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(TargetArchDetect)

target_architecture(TARGET_ARCH)

if((TARGET_ARCH MATCHES "x86_64" OR TARGET_ARCH MATCHES "ia64") AND NOT OF_32BIT)
        set(ARCH_BIT 64)
else()
        set(ARCH_BIT 32)
endif()

include_directories("${PROJECT_SOURCE_DIR}/Framework/Common")
include_directories("${PROJECT_SOURCE_DIR}/Framework/GeomMath")
include_directories("${PROJECT_SOURCE_DIR}/Framework/Interface")
include_directories("${PROJECT_SOURCE_DIR}/Framework/Parser")
include_directories("${PROJECT_SOURCE_DIR}/RHI")
include_directories("${PROJECT_SOURCE_DIR}/")
IF(${UNIX})
    IF(${APPLE})
        set(MYGE_TARGET_PLATFORM "Darwin")
    ELSE(${APPLE})
        set(MYGE_TARGET_PLATFORM "Linux")
    ENDIF(${APPLE})
ELSEIF(${WIN32})
    set(MYGE_TARGET_PLATFORM "Windows")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Orbis)
    set(MYGE_TARGET_PLATFORM "Orbis")
    set(ORBIS 1)
ENDIF(${UNIX})
include_directories("${PROJECT_SOURCE_DIR}/Platform/${MYGE_TARGET_PLATFORM}")
set(MYGE_EXTERNAL_ROOT ${PROJECT_SOURCE_DIR}/External/${MYGE_TARGET_PLATFORM}/)
include_directories("${MYGE_EXTERNAL_ROOT}include")
set(MYGE_EXTERNAL_LIBRARY_PATH ${MYGE_EXTERNAL_ROOT}lib/)
find_library(XG_LIBRARY xg PATHS ${MYGE_EXTERNAL_LIBRARY_PATH} NO_CMAKE_FIND_ROOT_PATH)
find_library(OPENDDL_LIBRARY OpenDDL PATHS ${MYGE_EXTERNAL_LIBRARY_PATH} NO_CMAKE_FIND_ROOT_PATH)
find_library(OPENGEX_LIBRARY OpenGEX PATHS ${MYGE_EXTERNAL_LIBRARY_PATH} NO_CMAKE_FIND_ROOT_PATH)
include(CTest)
include(CheckCXXSourceCompiles)
include(PlatformDependencies)
CHECK_CXX_SOURCE_COMPILES(
"#include <memory>

int main(int argc, char** argv)
{
    std::unique_ptr<int> my_int = std::make_unique<int>();
    *my_int = 0;
    return *my_int;
}
"
 HAVE_MAKE_UNIQUE)
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/config.h)
add_subdirectory(Framework)
add_subdirectory(Platform)
add_subdirectory(RHI)
