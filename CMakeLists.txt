cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(GluonPlasma)

add_subdirectory(external)

set(CMAKE_CXX_STANDARD 17)

function(GET_SHADER_INFOS FILENAME OUT_SHADER_NAME OUT_SHADER_TYPE OUT_BIN_SHADER_NAME OUT_OK)
    if(${FILENAME} MATCHES "^([a-zA-Z0-9_]+)\.(vert|frag|tesc|tesv|geom|comp)\.sc$")
        get_shader_type(${CMAKE_MATCH_2} SHADER_TYPE)
        set(${OUT_SHADER_NAME} ${CMAKE_MATCH_1} PARENT_SCOPE)
        set(${OUT_SHADER_TYPE} ${SHADER_TYPE} PARENT_SCOPE)
        set(${OUT_BIN_SHADER_NAME} ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.bin PARENT_SCOPE)
        set(${OUT_OK} True PARENT_SCOPE)
    else()
        message(WARNING "${FILENAME} is not a compatible name")
        set(${OUT_OK} False PARENT_SCOPE)
    endif()
endfunction(GET_SHADER_INFOS)

add_executable(gluon src/main.cpp src/renderer.cpp src/render_backend.cpp src/gln_color.cpp src/gln_text.cpp)
target_link_libraries(gluon PUBLIC glad glfw loguru eastl optick stb)
target_include_directories(gluon PUBLIC external/rapidjson/include external/glm)
target_compile_definitions(gluon PUBLIC USE_OPTICK=0)

if(MSVC)
    target_compile_definitions(gluon PUBLIC _CRT_SECURE_NO_WARNINGS NODRAWTEXT)
endif()
