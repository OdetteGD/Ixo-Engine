cmake_minimum_required(VERSION 3.22.1)
project(ixo_engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ixo_engine SHARED src/main/cpp/ixo_engine.cpp)
target_compile_options(ixo_engine PRIVATE -Wall -Wextra)
target_link_libraries(ixo_engine PRIVATE android log vulkan)

# Physics dependency is opt-in until the Android package includes the Box2D runtime.
option(IXO_FETCH_BOX2D "Fetch Box2D for the native physics module" OFF)
if(IXO_FETCH_BOX2D)
  include(FetchContent)
  set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
  set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
  set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.1
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(box2d)
  target_link_libraries(ixo_engine PRIVATE box2d)
  target_compile_definitions(ixo_engine PRIVATE IXO_HAS_BOX2D=1)
endif()
