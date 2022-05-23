cmake_minimum_required (VERSION 3.1) 
project (GameEngineFromScrath)

set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(TargetArchDetect)

target_architecture(TARGET_ARCH)

if((TARGET_ARCH MATCHES "x86_64" OR TARGET_ARCH MATCHES "ia64") AND NOT OF_32BIT)
        set(ARCH_BIT 64)
else()
        set(ARCH_BIT 32)
endif()

include_directories("${PROJECT_SOURCE_DIR}/Framework/Common")
include_directories("${PROJECT_SOURCE_DIR}/Framework/Interface")
include_directories("${PROJECT_SOURCE_DIR}/Framework/GeomMath")
include_directories("${PROJECT_SOURCE_DIR}/RHI")
IF(${UNIX})
    include_directories("${PROJECT_SOURCE_DIR}/Platform/Linux")
ELSEIF(${WIN32})
    include_directories("${PROJECT_SOURCE_DIR}/Platform/Windows")
ENDIF(${UNIX})
include(CTest)
add_subdirectory(Framework)
add_subdirectory(Platform)
add_subdirectory(RHI)

