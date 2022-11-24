#pragma once
#include <stdexcept>

namespace gltf2cpp {
struct Error : std::runtime_error {
	using runtime_error::runtime_error;
};
} // namespace gltf2cpp
