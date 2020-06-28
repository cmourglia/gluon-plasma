# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# BGFX
# add_subdirectory(bgfx)

# EASTL
add_subdirectory(eastl)

# Loguru
add_library(loguru STATIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru/loguru.cpp)
target_include_directories(loguru PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru)

# Optick
file(GLOB OPTICK_SRC optick/src/*.cpp)
add_library(optick STATIC ${OPTICK_SRC})
target_include_directories(optick PUBLIC optick/src)
target_compile_definitions(optick PUBLIC OPTICK_ENABLE_GPU_VULKAN=0)
target_compile_definitions(optick PRIVATE _CRT_SECURE_NO_WARNINGS)
if (MSVC)
	target_compile_definitions(optick PUBLIC OPTICK_ENABLE_GPU_D3D12=0)
endif()

# Glad
add_library(glad STATIC glad/glad.c)
target_include_directories(glad PUBLIC glad)

# # Temporary
# if(WIN32)
# 	set(FREETYPE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/msdfgen/freetype/include)
# 	set(FREETYPE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/msdfgen/freetype/win64/freetype.lib)
# else()
# 	message(FATAL_ERROR "Handle msdfgen for linux")
# endif()

# add_subdirectory(freetype2)

# # MSDF
# set(MSDFGEN_BUILD_MSDFGEN_STANDALONE ON CACHE BOOL "" FORCE)
# set(MSDFGEN_USE_OPENMP OFF CACHE BOOL "" FORCE)
# add_subdirectory(msdfgen)

add_library(stb STATIC stb.cpp)
target_include_directories(stb PUBLIC stb)
