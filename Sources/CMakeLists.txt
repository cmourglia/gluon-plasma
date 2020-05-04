project(gluon)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME}
	main.cpp gvulkan.cpp gswapchain.cpp gbuffer.cpp gshader.cpp
	gvulkan.h gswapchain.h gbuffer.h gshader.h
	${THIRD_PARTY_DIR}/volk/volk.c
	${THIRD_PARTY_DIR}/loguru/loguru.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm glfw::glfw)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/ThirdParty/loguru ${CMAKE_SOURCE_DIR}/ThirdParty/volk)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
