cmake_minimum_required (VERSION 3.12)
project (code_parser)
enable_testing()

include_directories(${CMAKE_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(LLVM_CONFIG true)
	set(CLANG_INCLUDEDIR "C:/Program Files (x86)/LLVM/include")
	set(CLANG_LIBDIR "C:/Program Files (x86)/LLVM/lib")
endif()
# ---------------------------------------------------------------------------------------------------------
# Handle C++ standard version.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++... instead of -std=gnu++...


# ---------------------------------------------------------------------------------------------------------

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------------------------------------


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")
find_package(LibClang REQUIRED)
include_directories(${CLANG_INCLUDEDIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(libreflang STATIC lib/reflang.hpp lib/reflang.cpp)
target_link_libraries(libreflang PUBLIC ${LIBCLANG_LIBRARIES})
target_include_directories(libreflang PUBLIC ${LIBCLANG_INCLUDE_DIRS}  )

add_library(tinyalloc STATIC ./include/tinyalloc/tinyalloc.c)


add_subdirectory(parser)
add_subdirectory(serialize_gen)

