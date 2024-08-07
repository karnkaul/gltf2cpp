cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX "-d")

set(project_version 0.2.0)

project(gltf2cpp VERSION ${project_version})

set(is_root_project OFF)

if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
  set(is_root_project ON)
endif()

option(GLTF2CPP_DYNARRAY_DEBUG_VIEW "Enable debug views in DynArray instances" ${is_root_project})
option(GLTF2CPP_BUILD_TESTS "Build gltf2cpp tests" ${is_root_project})

include(FetchContent)

if(NOT TARGET djson)
  FetchContent_Declare(
    djson
    GIT_REPOSITORY https://github.com/karnkaul/djson
    GIT_TAG v2.1.1
  )
  FetchContent_MakeAvailable(djson)
endif()

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC
  include
  "${CMAKE_CURRENT_BINARY_DIR}/include"
)
target_link_libraries(${PROJECT_NAME} PUBLIC djson::djson)

target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${GLTF2CPP_DYNARRAY_DEBUG_VIEW}>:GLTF2CPP_DYNARRAY_DEBUG_VIEW>)

configure_file(src/build_version.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/build_version.hpp" @ONLY)

target_sources(${PROJECT_NAME} PRIVATE
  include/gltf2cpp/dyn_array.hpp
  include/gltf2cpp/gltf2cpp.hpp
  include/gltf2cpp/version.hpp

  src/gltf2cpp.cpp
  src/version.cpp
)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
  )
endif()

if(GLTF2CPP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
