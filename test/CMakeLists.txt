project(gltf2cpp-test)

add_executable(gltf2cpp-data-uri)
target_sources(gltf2cpp-data-uri PRIVATE common.hpp test.cpp)
target_include_directories(gltf2cpp-data-uri PRIVATE .)
target_link_libraries(gltf2cpp-data-uri PRIVATE gltf2cpp::gltf2cpp)
add_test(data-uri gltf2cpp-data-uri)
