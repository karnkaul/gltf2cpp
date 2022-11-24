#pragma once
#include <cassert>
#include <memory>
#include <span>

namespace gltf2cpp {
template <typename T>
class DynArray {
  public:
	DynArray() = default;
	explicit DynArray(std::size_t size) : m_data(std::make_unique<T[]>(size)), m_size(size) {}

	explicit DynArray(std::unique_ptr<T[]>&& data, std::size_t size) : m_data(std::move(data)), m_size(size) {}

	T* data() const { return m_data.get(); }
	std::size_t size() const { return m_size; }
	std::span<T> span() const { return {m_data.get(), m_size}; }

	T& operator[](std::size_t index) const {
		assert(index < size());
		return m_data[index];
	}

	bool empty() const { return m_size == 0u || !*this; }
	explicit operator bool() const { return m_data != nullptr; }

	// TODO: ifdef wrap
	void debug_refresh() { debug_view = span(); }

  private:
	std::unique_ptr<T[]> m_data{};
	std::size_t m_size{};

	// TODO: ifdef wrap
	std::span<T const> debug_view{};
};

using ByteArray = DynArray<std::byte>;
} // namespace gltf2cpp
