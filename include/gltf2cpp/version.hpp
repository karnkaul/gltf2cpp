#pragma once
#include <compare>
#include <string>

namespace gltf2cpp {
///
/// \brief Semantic version.
///
struct Version {
	std::uint32_t major{};
	std::uint32_t minor{};
	std::uint32_t patch{};

	static Version from(std::string_view text);

	auto operator<=>(Version const&) const = default;

	std::string to_string() const;
};
} // namespace gltf2cpp
