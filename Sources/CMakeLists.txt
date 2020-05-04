project(gluon)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

include(${THIRD_PARTY_DIR}/VulkanUtils/CMake/ShaderUtils.cmake)

set(SHADER_SRC ${CMAKE_SOURCE_DIR}/Assets/Shaders/Sources)
set(SHADER_DST ${CMAKE_SOURCE_DIR}/Assets/Shaders/Bin)

set(IN_SHADERS triangle.vert.glsl triangle.frag.glsl)
set(OUT_SHADERS )

foreach(SHADER ${IN_SHADERS})
	compile_shader(${SHADER} ${SHADER_SRC} ${SHADER_DST} OUT_SHADER)
	list(APPEND OUT_SHADERS ${OUT_SHADER})
endforeach()

add_executable(${PROJECT_NAME}
	main.cpp gvulkan.cpp gswapchain.cpp gbuffer.cpp gshader.cpp
	gvulkan.h gswapchain.h gbuffer.h gshader.h
	${THIRD_PARTY_DIR}/volk/volk.c
	${THIRD_PARTY_DIR}/loguru/loguru.cpp
	tinygltf.cpp
	${OUT_SHADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm glfw::glfw)
target_include_directories(${PROJECT_NAME} PRIVATE
	${Vulkan_INCLUDE_DIRS}
	${THIRD_PARTY_DIR}/loguru
	${THIRD_PARTY_DIR}/volk
	${THIRD_PARTY_DIR}/tinygltf)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
