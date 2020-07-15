# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# EASTL
add_subdirectory(eastl)

# Loguru
add_library(loguru STATIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru/loguru.cpp)
target_include_directories(loguru PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru)

# Glad
add_library(glad STATIC glad/glad.c)
target_include_directories(glad PUBLIC glad)

add_library(stb STATIC stb.cpp)
target_include_directories(stb PUBLIC stb)
