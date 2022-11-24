#include <gltf2cpp/error.hpp>
#include <gltf2cpp/gltf2cpp.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace gltf2cpp {
#define EXPECT(expr) detail::expect(!!(expr), #expr)

namespace fs = std::filesystem;

namespace {
using Buffer = ByteArray;

struct Reader {
	fs::path prefix{};

	ByteArray operator()(std::string_view uri) const {
		auto file = std::ifstream{prefix / uri, std::ios::binary | std::ios::ate};
		if (!file) { return {}; }
		auto const size = file.tellg();
		auto ret = ByteArray{static_cast<std::size_t>(size)};
		file.read(reinterpret_cast<char*>(ret.data()), size);
		return ret;
	}
};

enum class BufferTarget : std::uint32_t {
	eArrayBuffer = 34962,
	eElementArrayBuffer = 34693,
};

template <typename T>
struct Limit {
	T data[16];
	std::size_t size{};

	constexpr void push_back(T t) {
		EXPECT(size < std::size(data));
		data[size++] = t;
	}

	constexpr std::span<T const> span() const { return {data, size}; }
};

struct BufferView {
	Index<Buffer> buffer{};
	std::size_t offset{};
	std::size_t length{};
	BufferTarget target{};

	std::span<std::byte const> to_span(Buffer const& buffer) const {
		if (offset + buffer.size() < length) { throw Error{"todo: gltf error"}; }
		if (length == 0) { return {}; }
		return {&buffer[offset], length};
	}
};

struct AccessorLayout {
	dj::Json const& min{};
	dj::Json const& max{};
	std::size_t count{};
	std::size_t component_coeff{};

	constexpr std::size_t container_size() const { return count * component_coeff; }
};

constexpr auto identity_matrix_v = Mat4x4{{
	Vec4{{1.0f, 0.0f, 0.0f, 0.0f}},
	Vec4{{0.0f, 1.0f, 0.0f, 0.0f}},
	Vec4{{0.0f, 0.0f, 1.0f, 0.0f}},
	Vec4{{0.0f, 0.0f, 0.0f, 1.0f}},
}};

constexpr std::size_t get_base64_start(std::string_view str) {
	constexpr auto match_v = std::string_view{";base64,"};
	auto const it = str.find(match_v);
	if (it == std::string_view::npos) { return it; }
	return it + match_v.size();
}

ByteArray to_byte_array(std::span<std::byte const> in) {
	auto ret = ByteArray{in.size()};
	std::memcpy(ret.data(), in.data(), ret.size());
	return ret;
}

ByteArray base64_decode(std::string_view const base64) {
	static constexpr std::uint8_t table[] = {
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,	1,	2,	3,	4,	5,	6,	7,	8,
		9,	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	};

	std::size_t const in_len = base64.size();
	EXPECT(in_len % 4 == 0);
	if (in_len % 4 != 0) { return {}; }

	auto out_len = in_len / 4 * 3;
	if (base64[in_len - 1] == '=') { out_len--; }
	if (base64[in_len - 2] == '=') { out_len--; }

	auto ret = ByteArray{out_len};

	for (std::uint32_t i = 0, j = 0; i < in_len;) {
		std::uint32_t const a = base64[i] == '=' ? 0 & i++ : table[static_cast<std::size_t>(base64[i++])];
		std::uint32_t const b = base64[i] == '=' ? 0 & i++ : table[static_cast<std::size_t>(base64[i++])];
		std::uint32_t const c = base64[i] == '=' ? 0 & i++ : table[static_cast<std::size_t>(base64[i++])];
		std::uint32_t const d = base64[i] == '=' ? 0 & i++ : table[static_cast<std::size_t>(base64[i++])];

		std::uint32_t const triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

		if (j < out_len) { ret[j++] = static_cast<std::byte>((triple >> 2 * 8) & 0xFF); }
		if (j < out_len) { ret[j++] = static_cast<std::byte>((triple >> 1 * 8) & 0xFF); }
		if (j < out_len) { ret[j++] = static_cast<std::byte>((triple >> 0 * 8) & 0xFF); }
	}

	return ret;
}

constexpr AlphaMode get_alpha_mode(std::string_view const mode) {
	if (mode == "MASK") {
		return AlphaMode::eMask;
	} else if (mode == "BLEND") {
		return AlphaMode::eBlend;
	}
	return AlphaMode::eOpaque;
}

AttributeMap make_attributes(dj::Json const& json) {
	auto ret = AttributeMap{};
	for (auto [key, value] : json.object_view()) { ret.insert_or_assign(std::string{key}, value.as<Index<Accessor>>()); }
	return ret;
}

enum class Bound { eFloor, eCeil };

template <Bound B, typename T>
constexpr void limit(std::span<T> out, std::span<T const> range) {
	if (range.empty()) { return; }
	EXPECT(out.size() % range.size() == 0);
	for (std::size_t i = 0; i < out.size(); ++i) {
		auto const j = i % range.size();
		if constexpr (B == Bound::eFloor) {
			out[i] = std::max(out[i], static_cast<T>(range[j]));
		} else {
			out[i] = std::min(out[i], static_cast<T>(range[j]));
		}
	}
}

template <std::size_t Dim>
Vec<Dim> get_vec(dj::Json const& value, Vec<Dim> const& fallback = {}) {
	auto ret = fallback;
	if (!value) { return ret; }
	EXPECT(value.array_view().size() >= Dim);
	for (std::size_t i = 0; i < Dim; ++i) { ret[i] = value[i].as<float>(); }
	return ret;
}

template <Bound B, typename T>
constexpr void apply_limit(std::span<T> out, dj::Json::ArrayProxy const& source, std::size_t width) {
	EXPECT(source.size() == width);
	auto range = Limit<T>{};
	for (auto const& element : source) { range.push_back(element.as<T>()); }
	limit<B>(out, range.span());
}

template <ComponentType C>
auto make_component_data(std::span<std::byte const> span, AccessorLayout ads) {
	using T = FromComponentType<C>;
	auto arr = DynArray<T>{ads.container_size()};
	if (!span.empty()) {
		auto const size_bytes = ads.container_size() * sizeof(T);
		EXPECT(span.size() >= size_bytes);
		std::memcpy(arr.data(), span.data(), size_bytes);
	}
	if (ads.min) { apply_limit<Bound::eFloor>(arr.span(), ads.min.array_view(), ads.component_coeff); }
	if (ads.max) { apply_limit<Bound::eCeil>(arr.span(), ads.max.array_view(), ads.component_coeff); }
	arr.debug_refresh();
	return arr;
}

Accessor::Data make_accessor_data(std::span<std::byte const> bytes, ComponentType ctype, AccessorLayout layout) {
	auto ret = Accessor::Data{};
	switch (ctype) {
	case ComponentType::eByte: ret = make_component_data<ComponentType::eByte>(bytes, layout); break;
	case ComponentType::eShort: ret = make_component_data<ComponentType::eShort>(bytes, layout); break;
	case ComponentType::eUnsignedShort: ret = make_component_data<ComponentType::eUnsignedShort>(bytes, layout); break;
	case ComponentType::eUnsignedInt: ret = make_component_data<ComponentType::eUnsignedInt>(bytes, layout); break;
	case ComponentType::eFloat: ret = make_component_data<ComponentType::eFloat>(bytes, layout); break;
	default:
	case ComponentType::eUnsignedByte: ret = make_component_data<ComponentType::eUnsignedByte>(bytes, layout); break;
	}
	return ret;
}

Transform get_transform(dj::Json const& node) {
	auto const& translation = node["translation"];
	auto const& rotation = node["rotation"];
	auto const& scale = node["scale"];
	if (translation || rotation || scale) {
		auto ret = Trs{};
		ret.translation = get_vec<3>(translation, ret.translation);
		ret.rotation = get_vec<4>(rotation, ret.rotation);
		ret.scale = get_vec<3>(scale, ret.scale);
		return ret;
	}
	auto ret = identity_matrix_v;
	if (auto const& matrix = node["matrix"]) {
		for (std::size_t i = 0; i < 4; ++i) {
			for (std::size_t j = 0; j < 4; ++j) { ret[i][j] = matrix[(i * 4) + j].as<float>(); }
		}
	}
	return ret;
}

std::vector<Vec3> to_rgbs(gltf2cpp::Accessor const& accessor) {
	EXPECT(accessor.component_type == gltf2cpp::ComponentType::eFloat); // facade restriction
	if (accessor.type == gltf2cpp::Accessor::Type::eVec3) { return accessor.to_vec<3>(); }
	auto const vec4 = accessor.to_vec<4>(); // spec
	auto ret = std::vector<Vec3>{};
	ret.reserve(vec4.size());
	for (auto& v : vec4) { ret.push_back({v[0], v[1], v[2]}); }
	return ret;
}

struct GltfParser {
	GetBytes const& get_bytes;
	Root& storage;
	std::vector<Buffer> buffers{};
	std::vector<BufferView> buffer_views{};

	void buffer(dj::Json const& json) {
		auto& b = buffers.emplace_back();
		auto const uri = json["uri"].as_string();
		EXPECT(!uri.empty());
		if (auto i = get_base64_start(uri); i != std::string_view::npos) {
			b = base64_decode(uri.substr(i));
		} else if (get_bytes) {
			b = get_bytes(uri);
		}
	}

	void buffer_view(dj::Json const& json) {
		auto& bv = buffer_views.emplace_back();
		EXPECT(json.contains("buffer") && json.contains("byteLength"));
		bv.buffer = json["buffer"].as<std::size_t>();
		bv.length = json["byteLength"].as<std::size_t>();
		bv.offset = json["byteOffset"].as<std::size_t>(0);
		bv.target = static_cast<BufferTarget>(json["target"].as<int>());
	}

	void accessor(dj::Json const& json) {
		auto& a = storage.accessors.emplace_back();
		EXPECT(json.contains("componentType") && json.contains("count") && json.contains("type"));
		a.component_type = static_cast<ComponentType>(json["componentType"].as<int>());
		a.type = Accessor::to_type(json["type"].as_string());
		a.name = json["name"].as_string(a.name);
		a.normalized = json["normalized"].as_bool(dj::Boolean{false}).value;
		auto bytes = std::span<std::byte const>{};
		if (auto const& bv = json["bufferView"]) {
			auto const& view = buffer_views[bv.as<std::size_t>()];
			bytes = view.to_span(buffers[view.buffer]).subspan(json["byteOffset"].as<std::size_t>(0));
		}
		a.extensions = json["extensions"];
		a.extras = json["extras"];

		auto const layout = AccessorLayout{
			.min = json["min"],
			.max = json["max"],
			.count = json["count"].as<std::size_t>(),
			.component_coeff = Accessor::type_coeff(a.type),
		};
		a.data = make_accessor_data(bytes, a.component_type, layout);
	}

	Camera::Orthographic orthographic(dj::Json const& json) const {
		EXPECT(json.contains("xmag") && json.contains("ymag") && json.contains("zfar") && json.contains("znear"));
		auto ret = Camera::Orthographic{};
		ret.xmag = json["xmag"].as<float>();
		ret.ymag = json["ymag"].as<float>();
		ret.zfar = json["zfar"].as<float>();
		ret.znear = json["znear"].as<float>();
		return ret;
	}

	Camera::Perspective perspective(dj::Json const& json) const {
		EXPECT(json.contains("yfov") && json.contains("znear"));
		auto ret = Camera::Perspective{};
		ret.yfov = json["yfov"].as<float>();
		ret.znear = json["znear"].as<float>();
		ret.aspect_ratio = json["aspectRatio"].as<float>(0.0f);
		if (auto zfar = json["zfar"]) { ret.zfar = zfar.as<float>(); }
		return ret;
	}

	void camera(dj::Json const& json) {
		auto& c = storage.cameras.emplace_back();
		EXPECT(json.contains("type"));
		c.name = json["name"].as_string(c.name);
		c.extensions = json["extensions"];
		c.extras = json["extras"];
		if (json["type"].as_string() == "orthographic") {
			EXPECT(json.contains("orthographic"));
			c.payload = orthographic(json["orthographic"]);
		} else {
			EXPECT(json.contains("perspective"));
			c.payload = perspective(json["perspective"]);
		}
	}

	void sampler(dj::Json const& json) {
		auto& s = storage.samplers.emplace_back();
		s.name = json["name"].as<std::string>(s.name);
		if (auto const& min = json["minFilter"]) { s.min_filter = static_cast<Filter>(min.as<int>()); }
		if (auto const& mag = json["magFilter"]) { s.mag_filter = static_cast<Filter>(mag.as<int>()); }
		s.wrap_s = static_cast<Wrap>(json["wrapS"].as<int>(static_cast<int>(s.wrap_s)));
		s.wrap_t = static_cast<Wrap>(json["wrapT"].as<int>(static_cast<int>(s.wrap_t)));
		s.extensions = json["extensions"];
		s.extras = json["extras"];
	}

	void populate_geometry(Geometry& out, std::optional<Index<Accessor>> indices) const {
		auto const& attributes = out.attributes;
		auto const it_pos = attributes.find("POSITION");
		if (it_pos == attributes.end()) { return; }
		out.positions = storage.accessors[it_pos->second].to_vec<3>();
		if (auto const it_tan = attributes.find("TANGENT"); it_tan != attributes.end()) { out.tangents = storage.accessors[it_tan->second].to_vec<4>(); }
		auto populate = [&](std::string_view prefix, auto& out_vec, auto to_vec) {
			for (std::size_t index = 0;; ++index) {
				auto const key = std::string{prefix} + std::to_string(index);
				auto it_col = attributes.find(key);
				if (it_col == attributes.end()) { break; }
				auto const& accessor = storage.accessors[it_col->second];
				if (accessor.component_type == ComponentType::eFloat) {
					out_vec.push_back(to_vec(accessor));
					EXPECT(out_vec.back().size() == out.positions.size());
				}
			}
		};
		populate("COLOR_", out.colors, [](auto const& a) { return to_rgbs(a); });
		populate("TEXCOORD_", out.tex_coords, [](auto const& a) { return a.template to_vec<2>(); });
		if (indices) { out.indices = storage.accessors[*indices].to_u32(); }
	}

	Mesh::Primitive primitive(dj::Json const& json) const {
		EXPECT(json.contains("attributes"));
		auto ret = Mesh::Primitive{};
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		ret.geometry.attributes = make_attributes(json["attributes"]);
		if (auto const& indices = json["indices"]) { ret.indices = indices.as<std::size_t>(); }
		if (auto const& material = json["material"]) { ret.material = material.as<std::size_t>(); }
		populate_geometry(ret.geometry, ret.indices);
		return ret;
	}

	void mesh(dj::Json const& json) {
		auto& m = storage.meshes.emplace_back();
		m.name = json["name"].as_string(m.name);
		m.extensions = json["extensions"];
		m.extras = json["extras"];
		for (auto const& j : json["primitives"].array_view()) { m.primitives.push_back(primitive(j)); }
	}

	void image(dj::Json const& json) {
		auto& i = storage.images.emplace_back();
		auto name = std::string{json["name"].as_string(unnamed_v)};
		i.extensions = json["extensions"];
		i.extras = json["extras"];
		EXPECT(json.contains("uri") || json.contains("bufferView"));
		if (auto const uri = json["uri"].as_string(); !uri.empty()) {
			if (auto const it = get_base64_start(uri); it != std::string_view::npos) {
				i = Image{base64_decode(uri.substr(it)), std::move(name)};
			} else if (get_bytes) {
				i = Image{get_bytes(uri), std::move(name)};
			}
		} else {
			auto const& bv = buffer_views[json["bufferView"].as<std::size_t>()];
			i = Image{to_byte_array(bv.to_span(buffers[bv.buffer])), std::move(name)};
		}
	}

	void texture(dj::Json const& json) {
		auto& t = storage.textures.emplace_back();
		t.name = json["name"].as<std::string>(t.name);
		if (auto const& sampler = json["sampler"]) { t.sampler = sampler.as<std::size_t>(); }
		EXPECT(json.contains("source"));
		t.source = json["source"].as<std::size_t>();
		t.extensions = json["extensions"];
		t.extras = json["extras"];
	}

	static TextureInfo get_texture_info(dj::Json const& json) {
		EXPECT(json.contains("index"));
		auto ret = TextureInfo{};
		ret.texture = json["index"].as<std::size_t>();
		ret.tex_coord = json["texCoord"].as<std::size_t>(ret.tex_coord);
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		return ret;
	}

	static NormalTextureInfo get_normal_texture_info(dj::Json const& json) {
		auto ret = NormalTextureInfo{};
		ret.info = get_texture_info(json);
		ret.scale = json["scale"].as<float>(ret.scale);
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		return ret;
	}

	static OcclusionTextureInfo get_occlusion_texture_info(dj::Json const& json) {
		auto ret = OcclusionTextureInfo{};
		ret.info = get_texture_info(json);
		ret.strength = json["strength"].as<float>(ret.strength);
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		return ret;
	}

	static PbrMetallicRoughness pbr_metallic_roughness(dj::Json const& json) {
		auto ret = PbrMetallicRoughness{};
		ret.base_color_factor = get_vec<4>(json["baseColorFactor"], ret.base_color_factor);
		if (auto const& bct = json["baseColorTexture"]) { ret.base_color_texture = get_texture_info(bct); }
		ret.metallic_factor = json["metallicFactor"].as<float>(ret.metallic_factor);
		ret.roughness_factor = json["roughnessFactor"].as<float>(ret.roughness_factor);
		if (auto const& mrt = json["metallicRoughnessTexture"]) { ret.metallic_roughness_texture = get_texture_info(mrt); }
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		return ret;
	}

	void material(dj::Json const& json) {
		auto& m = storage.materials.emplace_back();
		m.name = std::string{json["name"].as_string(m.name)};
		m.pbr = pbr_metallic_roughness(json["pbrMetallicRoughness"]);
		m.emissive_factor = get_vec<3>(json["emissiveFactor"]);
		if (auto const& nt = json["normalTexture"]) { m.normal_texture = get_normal_texture_info(nt); }
		if (auto const& ot = json["occlusionTexture"]) { m.occlusion_texture = get_occlusion_texture_info(ot); }
		if (auto const& et = json["emissiveTexture"]) { m.emissive_texture = get_texture_info(et); }
		m.alpha_mode = get_alpha_mode(json["alphaMode"].as_string());
		m.alpha_cutoff = json["alphaCutoff"].as<float>(m.alpha_cutoff);
		m.double_sided = json["doubleSided"].as_bool(dj::Boolean{m.double_sided}).value;
		m.extensions = json["extensions"];
		m.extras = json["extras"];
	}

	Animation::Sampler anim_sampler(dj::Json const& json) const {
		auto ret = Animation::Sampler{};
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		EXPECT(json.contains("input") && json.contains("output"));
		ret.input = json["input"].as<std::size_t>();
		ret.output = json["output"].as<std::size_t>();
		auto const& interpolation = json["interpolation"].as_string();
		if (interpolation == "STEP") {
			ret.interpolation = Interpolation::eStep;
		} else if (interpolation == "CUBICSPLINE") {
			ret.interpolation = Interpolation::eCubicSpline;
		}
		return ret;
	}

	static constexpr Animation::Path anim_path(std::string_view const path) {
		if (path == "translation") {
			return Animation::Path::eTranslation;
		} else if (path == "scale") {
			return Animation::Path::eScale;
		} else if (path == "weights") {
			return Animation::Path::eWeights;
		}
		return Animation::Path::eRotation;
	}

	Animation::Target anim_target(dj::Json const& json) const {
		auto ret = Animation::Target{};
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		EXPECT(json.contains("path"));
		if (auto const& node = json["node"]) {
			EXPECT(node.is_number());
			ret.node = node.as<std::size_t>();
		}
		ret.path = anim_path(json["path"].as_string());
		return ret;
	}

	Animation::Channel anim_channel(dj::Json const& json) const {
		auto ret = Animation::Channel{};
		ret.extensions = json["extensions"];
		ret.extras = json["extras"];
		EXPECT(json.contains("sampler") && json.contains("target"));
		ret.sampler = json["sampler"].as<std::size_t>();
		ret.target = anim_target(json["target"]);
		return ret;
	}

	void animation(dj::Json const& json) {
		auto& a = storage.animations.emplace_back();
		a.name = json["name"].as_string(a.name);
		a.extensions = json["extensions"];
		a.extras = json["extras"];
		for (auto const& sampler : json["samplers"].array_view()) { a.samplers.push_back(anim_sampler(sampler)); }
		for (auto const& channel : json["channels"].array_view()) { a.channels.push_back(anim_channel(channel)); }
	}

	void skin(dj::Json const& json) {
		auto& s = storage.skins.emplace_back();
		s.name = json["name"].as_string(s.name);
		s.extensions = json["extensions"];
		s.extras = json["extras"];
		EXPECT(json.contains("joints"));
		for (auto const& joint : json["joints"].array_view()) { s.joints.push_back(joint.as<std::size_t>()); }
		if (auto const& ibm = json["inverseBindMatrices"]) { s.inverseBindMatrices = ibm.as<std::size_t>(); }
		if (auto const& skeleton = json["skeleton"]) { s.skeleton = skeleton.as<std::size_t>(); }
	}

	void parse(dj::Json const& scene) {
		buffers.clear();
		buffer_views.clear();
		storage = {};

		for (auto const& b : scene["buffers"].array_view()) { buffer(b); }
		for (auto const& bv : scene["bufferViews"].array_view()) { buffer_view(bv); }

		for (auto const& s : scene["accessors"].array_view()) { accessor(s); }
		for (auto const& c : scene["cameras"].array_view()) { camera(c); }
		for (auto const& s : scene["samplers"].array_view()) { sampler(s); }
		for (auto const& i : scene["images"].array_view()) { image(i); }
		for (auto const& t : scene["textures"].array_view()) { texture(t); }
		for (auto const& m : scene["meshes"].array_view()) { mesh(m); }
		for (auto const& m : scene["materials"].array_view()) { material(m); }

		// Texture will use ColourSpace::sRGB by default; change non-colour textures to be linear
		auto set_linear = [this](std::size_t index) { storage.textures[index].linear = true; };

		// Determine whether a texture points to colour data by referring to the material(s) it is used in
		// In our case all material textures except pbr.base_colour_texture and emissive_texture are linear
		// The GLTF spec mandates image formats for corresponding material textures:
		// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material
		for (auto const& m : storage.materials) {
			if (m.pbr.metallic_roughness_texture) { set_linear(m.pbr.metallic_roughness_texture->texture); }
			if (m.occlusion_texture) { set_linear(m.occlusion_texture->info.texture); }
			if (m.normal_texture) { set_linear(m.normal_texture->info.texture); }
		}
	}
};

Asset make_asset(dj::Json const& json) {
	auto ret = Asset{};

	ret.copyright = json["copyright"].as_string();
	ret.generator = json["generator"].as_string();
	ret.version = Version::from(json["version"].as_string());
	ret.min_version = Version::from(json["minVersion"].as_string());

	ret.extensions = json["extensions"];
	ret.extras = json["extras"];

	return ret;
}

std::vector<std::string> make_extensions_list(dj::Json const& extensions) {
	auto ret = std::vector<std::string>{};
	ret.reserve(extensions.array_view().size());
	for (auto const& extension : extensions.array_view()) { ret.push_back(extension.as<std::string>()); }
	return ret;
}
} // namespace

std::string detail::print_error(char const* msg) {
	auto ret = std::string{"gltf2cpp assertion failed: "} + msg;
	std::fprintf(stderr, "%s\n", ret.c_str());
	return ret;
}

void detail::expect(bool pred, char const* expr) noexcept(false) {
	if (pred) { return; }
	throw Error{print_error(expr)};
}

auto Accessor::to_type(std::string_view const key) -> Type {
	static constexpr std::string_view type_key_v[] = {"SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"};
	static_assert(std::size(type_key_v) == static_cast<std::size_t>(Type::eCOUNT_));
	for (int i = 0; i < static_cast<int>(Type::eCOUNT_); ++i) {
		if (type_key_v[i] == key) { return static_cast<Type>(i); }
	}
	auto err = std::string{"Unknown attribute semantic ["};
	err += key;
	err += ']';
	throw Error{detail::print_error(err.c_str())};
}

std::vector<std::uint32_t> Accessor::to_u32() const {
	EXPECT(type == gltf2cpp::Accessor::Type::eScalar);
	auto ret = std::vector<std::uint32_t>{};
	if (auto const* d = std::get_if<UnsignedInt>(&data)) {
		ret.resize(d->size());
		std::memcpy(ret.data(), d->data(), d->span().size_bytes());
	} else {
		EXPECT(std::holds_alternative<UnsignedByte>(data) || std::holds_alternative<UnsignedShort>(data));
		auto write = [&ret](auto const& d) {
			ret.reserve(d.size());
			for (auto const i : d.span()) { ret.push_back(static_cast<std::uint32_t>(i)); }
		};
		std::visit(write, data);
	}
	return ret;
}

Metadata Parser::metadata() const {
	auto ret = Metadata{};
	ret.images = json["images"].array_view().size();
	ret.textures = json["textures"].array_view().size();
	for (auto const& mesh : json["meshes"].array_view()) { ret.primitives += mesh["primitives"].array_view().size(); }
	ret.animations = json["animations"].array_view().size();
	ret.extensions_used = make_extensions_list(json["extensionsUsed"]);
	ret.extensions_required = make_extensions_list(json["extensionsRequired"]);
	return ret;
}

Root Parser::parse(GetBytes const& get_bytes) const {
	auto ret = Root{};
	GltfParser{get_bytes, ret}.parse(json);

	auto const& nodes = json["nodes"].array_view();
	ret.nodes.reserve(nodes.size());
	for (auto const& jnode : nodes) {
		auto node = Node{.name = jnode["name"].as<std::string>()};
		node.index = ret.nodes.size();
		node.transform = get_transform(jnode);
		for (auto const& child : jnode["children"].array_view()) { node.children.push_back(child.as<std::size_t>()); }
		if (auto const& mesh = jnode["mesh"]) { node.mesh = mesh.as<std::size_t>(); }
		if (auto const& camera = jnode["camera"]) { node.camera = camera.as<std::size_t>(); }
		ret.nodes.push_back(std::move(node));
	}

	auto const& scenes = json["scenes"].array_view();
	ret.scenes.reserve(scenes.size());
	for (auto const& scene : scenes) {
		auto& s = ret.scenes.emplace_back();
		auto const& root_nodes = scene["nodes"].array_view();
		s.root_nodes.reserve(root_nodes.size());
		for (auto const& node : root_nodes) { s.root_nodes.push_back(node.as<std::size_t>()); }
	}

	ret.asset = make_asset(json["asset"]);

	ret.extensions = json["extensions"];
	ret.extras = json["extras"];

	ret.extensions_used = make_extensions_list(json["extensionsUsed"]);
	ret.extensions_required = make_extensions_list(json["extensionsRequired"]);

	return ret;
}

Root parse(char const* json_path) {
	if (!fs::is_regular_file(json_path)) { return {}; }
	auto json = dj::Json::from_file(json_path);
	if (!json) { return {}; }
	return Parser{json}.parse(Reader{fs::path{json_path}.parent_path()});
}
} // namespace gltf2cpp
