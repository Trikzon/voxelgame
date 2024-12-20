#pragma once

#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace mellohi
{
	using isize = std::ptrdiff_t;
	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using usize = std::size_t;
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;

	using f32 = float;
	using f64 = double;
	
	using glm::ivec2;
	using glm::ivec3;
	using glm::ivec4;
	
	using glm::uvec2;
	using glm::uvec3;
	using glm::uvec4;
	
	using glm::fvec2;
	using glm::fvec3;
	using glm::fvec4;
	
	using glm::fmat2x2;
	using glm::fmat2x3;
	using glm::fmat2x4;
	using glm::fmat3x2;
	using glm::fmat3x3;
	using glm::fmat3x4;
	using glm::fmat4x2;
	using glm::fmat4x3;
	using glm::fmat4x4;
	
	using glm::quat;
}

#define MH_CONTAINS(vector, value) std::find(vector.begin(), vector.end(), value) != vector.end()
