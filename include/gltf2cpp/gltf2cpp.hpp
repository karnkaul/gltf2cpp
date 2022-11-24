#pragma once
#include <djson/json.hpp>
#include <gltf2cpp/build_version.hpp>
#include <gltf2cpp/dyn_array.hpp>
#include <gltf2cpp/version.hpp>
#include <array>
#include <cstring>
#include <functional>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace gltf2cpp {
///
/// \brief Alias for an index for a particular type.
///
template <typename T>
using Index = std::size_t;

///
/// \brief Geometric vector modelled as an array of floats.
///
template <std::size_t Dim>
using Vec = std::array<float, Dim>;

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;

///
/// \brief 4x4 float matrix.
///
using Mat4x4 = std::array<Vec4, 4>;

///
/// \brief Alias for callable that returns bytes given a URI.
///
using GetBytes = std::function<ByteArray(std::string_view)>;

///
/// \brief GLTF Material Alpha Mode.
///
enum class AlphaMode : std::uint32_t {
	eOpaque = 0,
	eBlend,
	eMask,
};

///
/// \brief GLTF Animation Interpolation.
///
enum class Interpolation : std::uint32_t {
	eLinear,
	eStep,
	eCubicSpline,
};

///
/// \brief GLTF Accessor Component Type.
///
enum class ComponentType : std::uint32_t {
	eByte = 5120,
	eUnsignedByte = 5121,
	eShort = 5122,
	eUnsignedShort = 5123,
	eUnsignedInt = 5125,
	eFloat = 5126,
};

///
/// \brief GLTF Sampler Filter.
///
enum class Filter : std::uint32_t {
	eNearest = 9728,
	eLinear = 9729,
	eNearestMipmapNearest = 9984,
	eLinearMipmapNearest = 9985,
	eNearestMipmapLinear = 9986,
	eLinearMipmapLinear = 9987,
};

///
/// \brief GLTF Sampler Wrap.
///
enum class Wrap : std::uint32_t {
	eClampEdge = 33071,
	eMirrorRepeat = 33648,
	eRepeat = 10497,
};

///
/// \brief Convert ComponentType to its corresponding C++ type.
///
template <ComponentType C>
constexpr auto from_component_type() {
	if constexpr (C == ComponentType::eByte) {
		return std::int8_t{};
	} else if constexpr (C == ComponentType::eUnsignedByte) {
		return std::uint8_t{};
	} else if constexpr (C == ComponentType::eShort) {
		return std::int16_t{};
	} else if constexpr (C == ComponentType::eUnsignedShort) {
		return std::uint16_t{};
	} else if constexpr (C == ComponentType::eUnsignedInt) {
		return std::uint32_t{};
	} else {
		return float{};
	}
}

///
/// \brief Alias to convert ComponentType to its corresponding C++ type.
///
template <ComponentType C>
using FromComponentType = decltype(from_component_type<C>());

///
/// \brief GLTF Transform encoded as Translation, Rotation, Scale.
///
struct Trs {
	Vec3 translation{};
	Vec4 rotation{{1.0f, 0.0f, 0.0f, 0.0f}};
	Vec3 scale{Vec3{{1.0f, 1.0f, 1.0f}}};
};

///
/// \brief GLTF Transform encoded as Trs or a 4x4 matrix.
///
using Transform = std::variant<Trs, Mat4x4>;

///
/// \brief Default name.
///
inline constexpr char const* unnamed_v{"(Unnamed)"};

///
/// \brief GLTF Accessor.
///
/// gltf2cpp does not expose raw GLTF Buffers or BufferViews; instead each Accessor's data
/// is pre-interpreted and stored directly within it as a flat array of ComponentType.
/// Eg, Data for Type::eVec4 / ComponentType::Float will contain 4x float components
/// for each element (and not a Vec4 for each element).
///
struct Accessor {
	template <ComponentType C>
	using ComponentArray = DynArray<FromComponentType<C>>;

	using UnsignedByte = ComponentArray<ComponentType::eUnsignedByte>;
	using Byte = ComponentArray<ComponentType::eByte>;
	using Short = ComponentArray<ComponentType::eShort>;
	using UnsignedShort = ComponentArray<ComponentType::eUnsignedShort>;
	using UnsignedInt = ComponentArray<ComponentType::eUnsignedInt>;
	using Float = ComponentArray<ComponentType::eFloat>;

	using Data = std::variant<UnsignedByte, Byte, Short, UnsignedShort, UnsignedInt, Float>;

	enum class Type { eScalar, eVec2, eVec3, eVec4, eMat2, eMat3, eMat4, eCOUNT_ };

	std::string name{unnamed_v};
	Data data{};
	ComponentType component_type{};
	Type type{};
	bool normalized{};
	dj::Json extensions{};
	dj::Json extras{};

	///
	/// \brief Obtain the width multiplier for type.
	/// \param type Type to get width multiplier for
	/// \returns Width multiplier for type
	///
	/// Eg: Type::eVec2 = 2, Type::eMat3 = 9
	///
	static constexpr std::size_t type_coeff(Accessor::Type type);
	///
	/// \brief Convert a GLTF type semantic to its corresponding Type.
	/// \param key Semantic key to convert to Type
	/// \returns Corresponding Type for key
	///
	static Type to_type(std::string_view key);

	///
	/// \brief Obtain data as a vector of u32.
	/// \returns Data as std::vector of u32
	///
	/// type must be Type::eScalar, and component_type must be unsigned.
	///
	std::vector<std::uint32_t> to_u32() const;

	///
	/// \brief Obtain data as a vector of Vec<Dim>.
	/// \returns Data as std::vector of Vec<Dim>
	///
	/// type must be Type::eVecI (I == Dim), and component_type must be ComponentType::eFloat.
	///
	template <std::size_t Dim>
	std::vector<Vec<Dim>> to_vec() const;
};

struct Node;

///
/// \brief GLTF Animation.
///
struct Animation {
	///
	/// \brief GLTF Animation Path.
	///
	enum class Path { eTranslation, eRotation, eScale, eWeights };

	///
	/// \brief GLTF Animation Sampler.
	///
	struct Sampler {
		Index<Accessor> input{};
		Interpolation interpolation{};
		Index<Accessor> output{};
		dj::Json extensions{};
		dj::Json extras{};
	};

	///
	/// \brief GLTF Animation Target.
	///
	struct Target {
		std::optional<Index<Node>> node{};
		Path path{};
		dj::Json extensions{};
		dj::Json extras{};
	};

	///
	/// \brief GLTF Animation Channel.
	///
	struct Channel {
		Index<Sampler> sampler{};
		Target target{};
		dj::Json extensions{};
		dj::Json extras{};
	};

	std::string name{unnamed_v};
	std::vector<Channel> channels{};
	std::vector<Sampler> samplers{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Asset.
///
struct Asset {
	std::string copyright{};
	std::string generator{};
	Version version{};
	Version min_version{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Camera.
///
struct Camera {
	///
	/// \brief GLTF Perspective Camera.
	///
	struct Perspective {
		float yfov{};
		float znear{};

		float aspect_ratio{};
		std::optional<float> zfar{};
	};

	///
	/// \brief GLTF Orthographic Camera.
	///
	struct Orthographic {
		float xmag{};
		float ymag{};
		float zfar{};
		float znear{};
	};

	std::string name{unnamed_v};
	std::variant<Perspective, Orthographic> payload{Perspective{}};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Image.
///
struct Image {
	ByteArray bytes{};
	std::string name{unnamed_v};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Mesh Primitive Attributes.
///
using AttributeMap = std::unordered_map<std::string, Index<Accessor>>;

///
/// \brief GLTF Mesh Primitive Geometry.
///
/// Geometry represents all the Attributes in a Mesh Primitive.
/// Positions, normals, tangents, tex_coords, colors, and indices are pre-parsed for convenience.
///
/// tex_coords and colors are nested vectors, where the Ith element corresponds to SEMANTIC_I,
/// eg. tex_coords[2] is populated from the TEXCOORD_2 Attribute's Accessor.
///
/// These vectors will only be populated if the corresponding Accessor's ComponentType is eFloat.
/// For other component types, obtain and use the Accessor directly.
/// Since the POSITION Attribute is required to be in Accessor::Type::eVec3 / ComponentType::eFloat
/// format by the GLTF spec, .positions is always expected to be populated.
/// All vectors will either be empty or the same size as positions.
///
struct Geometry {
	AttributeMap attributes{};
	std::vector<Vec3> positions{};
	std::vector<Vec3> normals{};
	std::vector<Vec4> tangents{};
	std::vector<std::vector<Vec2>> tex_coords{};
	std::vector<std::vector<Vec3>> colors{};
	std::vector<std::uint32_t> indices{};
};

struct Texture;

///
/// \brief GLTF Material Texture Info.
///
struct TextureInfo {
	Index<Texture> texture{};
	Index<Accessor> tex_coord{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Material Normal Texture Info.
///
struct NormalTextureInfo {
	TextureInfo info{};
	float scale{1.0f};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Material Occlusion Texture Info.
///
struct OcclusionTextureInfo {
	TextureInfo info{};
	float strength{1.0f};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Material PBR Metallic Roughness.
///
struct PbrMetallicRoughness {
	std::optional<TextureInfo> base_color_texture{};
	std::optional<TextureInfo> metallic_roughness_texture{};
	Vec4 base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
	float metallic_factor{1.0f};
	float roughness_factor{1.0f};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Material.
///
struct Material {
	std::string name{};
	PbrMetallicRoughness pbr{};
	std::optional<NormalTextureInfo> normal_texture{};
	std::optional<OcclusionTextureInfo> occlusion_texture{};
	std::optional<TextureInfo> emissive_texture{};
	Vec3 emissive_factor{};
	AlphaMode alpha_mode{AlphaMode::eOpaque};
	float alpha_cutoff{0.5f};
	bool double_sided{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Mesh and its primitives.
///
struct Mesh {
	///
	/// \brief GLTF Mesh Primitive.
	///
	struct Primitive {
		Geometry geometry{};
		std::optional<Index<Accessor>> indices{};
		std::optional<Index<Material>> material{};
		dj::Json extensions{};
		dj::Json extras{};
	};

	std::string name{unnamed_v};
	std::vector<Primitive> primitives{};
	dj::Json extensions{};
	dj::Json extras{};
};

struct Skin;

///
/// \brief GLTF Scene Node.
///
struct Node {
	///
	/// \brief Name of the node.
	///
	std::string name{unnamed_v};
	///
	/// \brief Transform for the node.
	///
	Transform transform{};
	///
	/// \brief Indices of child nodes.
	///
	std::vector<Index<Node>> children{};
	///
	/// \brief Index of the node.
	///
	Index<Node> index{};

	std::optional<Index<Camera>> camera{};
	std::optional<Index<Mesh>> mesh{};
	std::optional<Index<Skin>> skin{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Texture Sampler.
///
struct Sampler {
	std::string name{unnamed_v};
	std::optional<Filter> min_filter{};
	std::optional<Filter> mag_filter{};
	Wrap wrap_s{Wrap::eRepeat};
	Wrap wrap_t{Wrap::eRepeat};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Skin.
///
struct Skin {
	std::string name{unnamed_v};
	std::optional<Index<Accessor>> inverseBindMatrices{};
	std::optional<Index<Node>> skeleton{};
	std::vector<Index<Node>> joints{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Texture.
///
struct Texture {
	std::string name{unnamed_v};
	std::optional<Index<Sampler>> sampler{};
	Index<Image> source{};
	bool linear{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief GLTF Scene.
///
struct Scene {
	std::vector<Index<Node>> root_nodes{};
	dj::Json extensions{};
	dj::Json extras{};
};

///
/// \brief Metadata for a GLTF asset.
///
struct Metadata {
	std::size_t images{};
	std::size_t textures{};
	std::size_t primitives{};
	std::size_t animations{};

	std::vector<std::string> extensions_used{};
	std::vector<std::string> extensions_required{};
};

///
/// \brief GLTF root.
///
/// Contains all the data parsed from a GLTF file (and resources it points to).
/// Buffers and BufferViews are not exposed, the raw bytes in those data are refined
/// into typed vectors in each Accessor.
///
struct Root {
	std::vector<Accessor> accessors{};
	std::vector<Animation> animations{};
	std::vector<Camera> cameras{};
	std::vector<Image> images{};
	std::vector<Material> materials{};
	std::vector<Mesh> meshes{};
	std::vector<Node> nodes{};
	std::vector<Sampler> samplers{};
	std::vector<Skin> skins{};
	std::vector<Texture> textures{};

	std::vector<Scene> scenes{};
	std::optional<Index<Scene>> start_scene{};

	std::vector<std::string> extensions_used{};
	std::vector<std::string> extensions_required{};
	dj::Json extensions{};
	dj::Json extras{};
	Asset asset{};

	///
	/// \brief Check if this instance represents a parsed GLTF asset.
	/// \returns true if asset.version has been set
	///
	/// This is based on the fact that gltf.asset and gltf.asset.version are required fields.
	///
	explicit operator bool() const { return asset.version > Version{}; }
};

///
/// \brief Parser to obtain Metadata / Root given a Json and GetBytes.
///
struct Parser {
	///
	/// \brief Json to parse.
	///
	dj::Json const& json;

	///
	/// \brief Obtain Metadata.
	/// \returns Metadata for json
	///
	Metadata metadata() const;
	///
	/// \brief Parse GLTF data.
	/// \param get_bytes Callable to load bytes given a URI (relative to the input JSON)
	/// \returns Parsed GLTF Root
	///
	Root parse(GetBytes const& get_bytes) const;
};

///
/// \brief Parse json as GLTF.
/// \param json_path path to .gltf JSON
/// \returns Parsed GLTF Root
///
Root parse(char const* json_path);

// impl

namespace detail {
std::string print_error(char const* msg);
void expect(bool pred, char const* expr) noexcept(false);
} // namespace detail

#define GLTF2CPP_EXPECT(expr) detail::expect(!!(expr), #expr)

constexpr std::size_t Accessor::type_coeff(Accessor::Type type) {
	switch (type) {
	default:
	case Accessor::Type::eScalar: return 1u;
	case Accessor::Type::eVec2: return 2u;
	case Accessor::Type::eVec3: return 3u;
	case Accessor::Type::eVec4: return 4u;
	case Accessor::Type::eMat2: return 2u * 2u;
	case Accessor::Type::eMat3: return 3u * 3u;
	case Accessor::Type::eMat4: return 4u * 4u;
	}
}

template <std::size_t Dim>
std::vector<Vec<Dim>> Accessor::to_vec() const {
	GLTF2CPP_EXPECT(type_coeff(type) == Dim);
	GLTF2CPP_EXPECT(std::holds_alternative<Float>(data));
	auto ret = std::vector<Vec<Dim>>{};
	auto const& d = std::get<Float>(data);
	GLTF2CPP_EXPECT(d.size() % Dim == 0);
	ret.resize(d.size() / Dim);
	std::memcpy(ret.data(), d.data(), d.span().size_bytes());
	return ret;
}

#undef GLTF2CPP_EXPECT
} // namespace gltf2cpp
