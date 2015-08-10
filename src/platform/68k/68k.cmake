cmake_minimum_required(VERSION 2.6)

# Disable ranlib
set(CMAKE_RANLIB "echo")

# Force absolute paths
set(CMAKE_USE_RELATIVE_PATHS FALSE)

# Needed to make force the 'compiler' to tigcc-x3d
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

# Custom configuration for the archiver
set(CMAKE_AR $ENV{X3D}/tools/tigcc-x3d)
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -ar -o <TARGET> <OBJECTS> <LINK_LIBRARIES> -ar")

# Custom configuration for the linker
set(CMAKE_LINKER $ENV{X3D}/tools/tigcc-x3d)
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -n X3D")
set(CMAKE_EXECUTABLE_SUFFIX "")

# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)

if(X3D_SUBTARGET EQUAL "ti92plus")
  add_definitions(-DUSE_TI92PLUS)
elseif(X3D_SUBTARGET EQUAL "v200")
  add_definitions(-DUSE_V200)
elseif(X3D_SUBTARGET EQUAL "ti89")
  add_definitions(-DUSE_TI89)
endif()