cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(GluonPlasma)

add_subdirectory(external)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_SRC_DIR ${SHADER_DIR}/src)
set(SHADERS test.vert.sc)

function(GET_SHADER_TYPE IN_TYPE OUT_TYPE)
    if("${IN_TYPE}" STREQUAL "vert")
        set(${OUT_TYPE} "vertex" PARENT_SCOPE)
    elseif("${IN_TYPE}" STREQUAL "frag")
        set(${OUT_TYPE} "fragment" PARENT_SCOPE)
    elseif("${IN_TYPE}" STREQUAL "geom")
        set(${OUT_TYPE} "geometry" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Shader type ${IN_TYPE} to handle")
    endif()
endfunction()

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

set(IN_SHADERS
    test.vert.sc test.frag.sc
)

set(OUT_SHADERS )

macro(COMPILE_SHADER SHADER)
    get_shader_infos(${SHADER} SHADER_NAME SHADER_TYPE BIN_SHADER_NAME OK)
    if(OK)
        if(WIN32)
            set(PLATFORM "windows")
            if(${SHADER_TYPE} STREQUAL "vertex")
                set(PROFILE "vs_5_0")
            elseif(${SHADER_TYPE} STREQUAL "fragment")
                set(PROFILE "ps_5_0")
            elseif(${SHADER_TYPE} STREQUAL "compute")
                set(PROFILE "cs_5_0")
            endif()
            set(OPTIMIZATION -O 3)
        else()
            set(PLATFORM "linux") # TODO
            set(PROFILE "")
            set(OPTIMIZATION "")
        endif(WIN32)

        add_custom_command(
            OUTPUT ${SHADER_DIR}/bin/${BIN_SHADER_NAME}
            COMMAND
                shaderc
                    -f ${SHADER_SRC_DIR}/${SHADER}
                    -o ${SHADER_DIR}/bin/${BIN_SHADER_NAME}
                    --platform ${PLATFORM}
                    --type ${SHADER_TYPE}
                    --profile ${PROFILE}
                    -i ${CMAKE_SOURCE_DIR}/external/bgfx/bgfx/src
                    ${OPTIMIZATION}
            MAIN_DEPENDENCY ${SHADER_DIR}/src/${SHADER}
            COMMENT "Compiling ${SHADER_TYPE} shader ${SHADER_NAME}"
        )

        list(APPEND OUT_SHADERS ${SHADER_DIR}/bin/${BIN_SHADER_NAME})
    endif()
endmacro(COMPILE_SHADER)

# foreach(SHADER ${IN_SHADERS})
#     compile_shader(${SHADER})
# endforeach()

add_executable(gluon src/main.cpp src/renderer.cpp src/render_backend.cpp)
target_link_libraries(gluon PUBLIC glad glfw loguru eastl optick)
target_include_directories(gluon PUBLIC external/rapidjson/include external/glm)
target_compile_definitions(gluon PUBLIC USE_OPTICK=0)

if(MSVC)
target_compile_definitions(gluon PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
