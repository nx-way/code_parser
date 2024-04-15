#
# Try to find libclang
#
# Once done this will define:
# - LIBCLANG_FOUND
#               System has libclang.
# - LIBCLANG_INCLUDE_DIRS
#               The libclang include directories.
# - LIBCLANG_LIBRARIES
#               The libraries needed to use libclang.
# - LIBCLANG_LIBRARY_DIR
#               The path to the directory containing libclang.
# - LIBCLANG_KNOWN_LLVM_VERSIONS
#               Known LLVM release numbers.

# most recent versions come first
set(LIBCLANG_KNOWN_LLVM_VERSIONS 16.0.1 16.0.2 16.0.3 16.0.4 16.0.5 16.0.6
  16.0.0 16.0 16
  15.0.0 15.0 15
  14.0.1 14.0.2 14.0.3 14.0.4 14.0.5 14.0.6
  14.0.0 14.0 14
  13.0.1
  13.0.0 13.0 13
  12.0.1
  12.0.0 12.0 12
  11.1.0 11.1
  11.0.1
  11.0.0 11.0 11
  10.0.1
  10.0.0 10.0 10
  9.0.1
  9.0.0_1 9.0.0 9.0 9
  8.0.1
  8.0.0_1 8.0.0 8.0 8
  7.1.0 7.1
  7.0.1 7.0.0 7.0 7
  6.0.1 6.0.0 6.0 6
  5.0.1 5.0.0 5.0 5
  4.0.1
  4.0.0_1 4.0.0 4.0 4
  3.9.1
  3.9.0 3.9
  3.8.1
  3.8.0 3.8
  3.7.1
  3.7
  3.6.2
  3.6.1
  3.6
  3.5.1
  3.5.0 3.5
  3.4.2 3.4.1 3.4 3.3 3.2 3.1)

set(libclang_llvm_header_search_paths
  # MacOS Homebrew
  /usr/local/opt/llvm/include
  /opt/homebrew/opt/llvm/include
  )
set(libclang_llvm_lib_search_paths
  # LLVM Fedora
  /usr/lib/llvm
  # MacOS Homebrew
  /usr/local/opt/llvm/lib
  /opt/homebrew/opt/llvm/lib
  )

foreach (version ${LIBCLANG_KNOWN_LLVM_VERSIONS})
  list(APPEND libclang_llvm_header_search_paths
    # LLVM Debian/Ubuntu nightly packages: http://llvm.org/apt/
    "/usr/lib/llvm-${version}/include"
    # LLVM Gentoo
    "/usr/lib/llvm/${version}/include"
    # LLVM MacPorts
    "/opt/local/libexec/llvm-${version}/include"
    # LLVM Homebrew
    "/usr/local/Cellar/llvm/${version}/include"
    # LLVM Homebrew/versions
    "/usr/local/lib/llvm-${version}/include"
    )

  list(APPEND libclang_llvm_lib_search_paths
    # LLVM Debian/Ubuntu nightly packages: http://llvm.org/apt/
    "/usr/lib/llvm-${version}/lib/"
    # LLVM Gentoo
    "/usr/lib/llvm/${version}/lib64/"
    "/usr/lib/llvm/${version}/lib32/"
    # LLVM MacPorts
    "/opt/local/libexec/llvm-${version}/lib"
    # LLVM Homebrew
    "/usr/local/Cellar/llvm/${version}/lib"
    # LLVM Homebrew/versions
    "/usr/local/lib/llvm-${version}/lib"
    )
endforeach()

find_path(LIBCLANG_INCLUDE_DIR clang-c/Index.h
  PATHS ${libclang_llvm_header_search_paths}
  PATH_SUFFIXES LLVM/include #Windows package from http://llvm.org/releases/
    llvm16/include llvm15/include llvm140/include llvm130/include llvm120/include llvm111/include llvm110/include llvm100/include llvm90/include llvm80/include llvm71/include llvm70/include llvm60/include llvm50/include llvm41/include llvm40/include llvm39/include llvm38/include llvm37/include llvm36/include # FreeBSD
  DOC "The path to the directory that contains clang-c/Index.h")

# On Windows with MSVC, the import library uses the ".imp" file extension
# instead of the comon ".lib"
if (MSVC)
  find_file(LIBCLANG_LIBRARY libclang.imp
    PATH_SUFFIXES LLVM/lib
    DOC "The file that corresponds to the libclang library.")
endif()

find_library(LIBCLANG_LIBRARY NAMES libclang.imp libclang clang
  PATHS ${libclang_llvm_lib_search_paths}
  PATH_SUFFIXES LLVM/lib #Windows package from http://llvm.org/releases/
    llvm16/lib llvm15/lib llvm140/lib llvm130/lib llvm120/lib llvm111/lib llvm110/lib llvm100/lib llvm90/lib llvm80/lib llvm71/lib llvm70/lib llvm60/lib llvm50/lib llvm41/lib llvm40/lib llvm39/lib llvm38/lib llvm37/lib llvm36/lib # FreeBSD
  DOC "The file that corresponds to the libclang library.")

get_filename_component(LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY} PATH)

set(LIBCLANG_LIBRARIES ${LIBCLANG_LIBRARY})
set(LIBCLANG_INCLUDE_DIRS ${LIBCLANG_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBCLANG_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LibClang DEFAULT_MSG
  LIBCLANG_LIBRARY LIBCLANG_INCLUDE_DIR)

mark_as_advanced(LIBCLANG_INCLUDE_DIR LIBCLANG_LIBRARY)
