cmake_minimum_required(VERSION 3.22.1)
project(ixo_engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build Lua 5.4 from source for the Android scripting runtime.
include(FetchContent)
set(LUA_BUILD_COMPILER OFF CACHE BOOL "" FORCE)
set(LUA_BUILD_INTERPRETER OFF CACHE BOOL "" FORCE)
FetchContent_Declare(lua
  URL https://www.lua.org/ftp/lua-5.4.8.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
  FetchContent_Populate(lua)
endif()
file(GLOB LUA_CORE CONFIGURE_DEPENDS "${lua_SOURCE_DIR}/src/*.c")
list(FILTER LUA_CORE EXCLUDE REGEX "/(lua|luac|onelua)\\.c$")
add_library(ixo_lua STATIC ${LUA_CORE})
target_include_directories(ixo_lua PUBLIC "${lua_SOURCE_DIR}/src")
target_compile_definitions(ixo_lua PRIVATE LUA_USE_POSIX)
set_target_properties(ixo_lua PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(ixo_engine SHARED src/main/cpp/ixo_engine.cpp)
target_compile_options(ixo_engine PRIVATE -Wall -Wextra)
target_link_libraries(ixo_engine PRIVATE android log vulkan ixo_lua)

option(IXO_FETCH_BOX2D "Fetch Box2D for the native physics module" OFF)
if(IXO_FETCH_BOX2D)
  FetchContent_Declare(box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.1
    GIT_SHALLOW TRUE)
  set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
  set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
  set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(box2d)
  target_link_libraries(ixo_engine PRIVATE box2d)
  target_compile_definitions(ixo_engine PRIVATE IXO_HAS_BOX2D=1)
endif()
