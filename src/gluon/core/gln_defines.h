#pragma once

#include <stdint.h>

#include <glm/glm.hpp>

#include <gluon/core/gln_macros.h>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

namespace gluon
{
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;

using vec2i = glm::vec<2, int>;
using vec3i = glm::vec<3, int>;
using vec4i = glm::vec<4, int>;
}
