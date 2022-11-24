#include <gltf2cpp/version.hpp>
#include <iostream>
#include <sstream>

namespace gltf2cpp {
Version Version::from(std::string_view text) {
	auto str = std::stringstream{std::string{text}};
	if (str.peek() == 'v') { str.get(); }
	auto ret = Version{};
	char dot{};
	str >> ret.major >> dot >> ret.minor >> dot >> ret.patch;
	return ret;
}

std::string Version::to_string() const {
	auto ret = std::stringstream{};
	ret << 'v' << major << '.' << minor << '.' << patch;
	return ret.str();
}
} // namespace gltf2cpp
