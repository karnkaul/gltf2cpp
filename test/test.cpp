#include <common.hpp>
#include <gltf2cpp/gltf2cpp.hpp>

namespace {
constexpr std::string_view json_v = R"({
  "scene": 0,
  "scenes" : [
    {
      "nodes" : [ 0 ]
    }
  ],
  
  "nodes" : [
    {
      "mesh" : 0
    }
  ],
  
  "meshes" : [
    {
      "primitives" : [ {
        "attributes" : {
          "POSITION" : 1
        },
        "indices" : 0
      } ]
    }
  ],

  "buffers" : [
    {
      "uri" : "data:application/octet-stream;base64,AAABAAIAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAA=",
      "byteLength" : 44
    }
  ],
  "bufferViews" : [
    {
      "buffer" : 0,
      "byteOffset" : 0,
      "byteLength" : 6,
      "target" : 34963
    },
    {
      "buffer" : 0,
      "byteOffset" : 8,
      "byteLength" : 36,
      "target" : 34962
    }
  ],
  "accessors" : [
    {
      "bufferView" : 0,
      "byteOffset" : 0,
      "componentType" : 5123,
      "count" : 3,
      "type" : "SCALAR",
      "max" : [ 2 ],
      "min" : [ 0 ]
    },
    {
      "bufferView" : 1,
      "byteOffset" : 0,
      "componentType" : 5126,
      "count" : 3,
      "type" : "VEC3",
      "max" : [ 1.0, 1.0, 0.0 ],
      "min" : [ 0.0, 0.0, 0.0 ]
    }
  ],
  
  "asset" : {
    "version" : "2.0"
  }
}
)";
} // namespace

int main() {
	try {
		auto json = dj::Json::parse(json_v);
		ASSERT(!json.is_null());
		auto root = gltf2cpp::Parser{json}.parse({});
		EXPECT(root.nodes.size() == 1);
		ASSERT(root.meshes.size() == 1);
		ASSERT(root.meshes[0].primitives.size() == 1);
		auto const& primitive = root.meshes[0].primitives[0];
		ASSERT(primitive.geometry.positions.size() == 3);
		ASSERT(primitive.geometry.indices.size() == 3);
		EXPECT(primitive.geometry.indices[0] == 0);
		EXPECT(primitive.geometry.indices[1] == 1);
		EXPECT(primitive.geometry.indices[2] == 2);
		gltf2cpp::Vec3 const positions[] = {
			{0.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
		};
		EXPECT(primitive.geometry.positions[0] == positions[0]);
		EXPECT(primitive.geometry.positions[1] == positions[1]);
		EXPECT(primitive.geometry.positions[2] == positions[2]);
	} catch (...) {}
	return test::result();
}
