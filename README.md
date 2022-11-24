# gltf2cpp

**A lightweight and modern GLTF parser written in C++20**

## Usage

Data structures in `gltf2cpp` directly reflect the definitions in the GLTF spec, with the slight exception of `Accessor`s: instead of pointing to specific `BufferView`s, they pre-parse those raw bytes into a typed flat array of primitives. This is stored as a variant in `Accessor::data`, and aliases like `Accessor::UnsignedByte` have been provided for convenience (to use as arguments for visitor callbacks). In most cases you won't even need to bother further parsing this data, as a mesh primitive's geometry contains pre-parsed positions, normals, UVs, RGBs, tangents, and indices. Joints and weights may be added in future versions.

```cpp
// obtain root node
auto root = gltf2cpp::parse("path/to/asset.gltf");
if (!root) { /* handle error */ }
// store materials
for (auto const& material : root.materials) {
  // store textures
  if (material.pbr.base_color_texture) {
    auto const& texture = root.textures[material.pbr.base_color_texture->texture];
    auto const& image = root.images[texture.source];
    auto const* sampler = texture.sampler ? &root.samplers[*texture.sampler] : nullptr;
    add_texture(image.bytes.span(), sampler, texture.linear);
  }
  add_material(material);
}
// store meshes
for (auto const& mesh : root.meshes) {
  auto my_mesh = MyMesh{};
  for (auto const& primitive : mesh.primitives) {
    auto geometry = make_geometry(primitive.geometry.positions,
      primitive.geometry.tex_coords, primitive.geometry.normals, primitive.indices);
    my_mesh.add_primitive(std::move(geometry), get_material(primitive.material));
  }
  add_mesh(std::move(my_mesh));
}
```

## Requirements

- CMake 3.18+
- C++20 (GCC 10.x compatible)

## Setup

1. Obtain source via cloning, git submodules, CMake FetchContent, etc.
1. (Optional) Add custom `djson` to CMake build tree.
1. Add `gltf2cpp` to CMake build tree via `add_subdirectory`.
   1. `gltf2cpp` will fetch `djson` if the target is not already in the build tree.
1. Link to desired target via `target_link_libraries(foo PRIVATE gltf2cpp::gltf2cpp)`.
1. Use `#include <gltf2cpp/gltf2cpp.hpp>` to import the library.
