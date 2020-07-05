project(gluon_core)

add_library(${PROJECT_NAME} STATIC
	gln_defines.h
	gln_math.h
	gln_timer.h
	gln_color.h
	gln_color.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC loguru eastl stb)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/glm)

if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS NODRAWTEXT)
endif()
