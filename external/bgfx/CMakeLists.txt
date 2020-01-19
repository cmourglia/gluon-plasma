# bx
set(BX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bx)
add_library(bx STATIC ${BX_DIR}/src/amalgamated.cpp)
target_include_directories(bx PUBLIC ${BX_DIR}/include ${BX_DIR}/3rdparty)
if(MSVC)
    target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/msvc)
    target_compile_definitions(bx PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

target_compile_definitions(bx PUBLIC __STDC_LIMIT_MACROS __STDC_FORMAT_MACROS __STDC_CONSTANT_MACROS)

# bimg
set(BIMG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bimg)
set(BIMG_SRC ${BIMG_DIR}/src)
set(BIMG_3RD ${BIMG_DIR}/3rdparty)
file(GLOB ASTC_SRC ${BIMG_3RD}/astc-codec/src/decoder/*.cc)
add_library(bimg STATIC ${BIMG_SRC}/image.cpp ${BIMG_SRC}/image_gnf.cpp ${ASTC_SRC})
target_include_directories(bimg PUBLIC ${BIMG_DIR}/include ${BIMG_3RD}/astc-codec ${BIMG_3RD}/astc-codec/include ${BIMG_3RD})
target_link_libraries(bimg PUBLIC bx)

# bgfx
set(BGFX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bgfx)
set(BGFX_3RD ${BGFX_DIR}/3rdparty)
add_library(bgfx STATIC ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/src/amalgamated.cpp)
target_include_directories(bgfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/include ${BGFX_3RD} ${BGFX_3RD}/khronos ${BGFX_3RD}/dxsdk/include)
target_link_libraries(bgfx PUBLIC bx bimg)

# shaderc
set(GLSL_OPTIMIZER ${BGFX_3RD}/glsl-optimizer)
set(FCPP ${BGFX_3RD}/fcpp)
set(GLSLANG ${BGFX_3RD}/glslang)
set(SPIRV_CROSS ${BGFX_3RD}/spirv-cross)
set(SPIRV_HEADERS ${BGFX_3RD}/spirv-headers)
set(SPIRV_TOOLS ${BGFX_3RD}/spirv-tools)

# spirv-opt
file(GLOB_RECURSE SPIRV_TOOLS_OPT ${SPIRV_TOOLS}/source/opt/*.cpp)
file(GLOB_RECURSE SPIRV_TOOLS_REDUCE  ${SPIRV_TOOLS}/source/reduce/*.cpp)
set(SPIRV_TOOS_SRC
    "${SPIRV_TOOLS}/source/assembly_grammar.cpp"
    "${SPIRV_TOOLS}/source/binary.cpp"
    "${SPIRV_TOOLS}/source/diagnostic.cpp"
    "${SPIRV_TOOLS}/source/disassemble.cpp"
    "${SPIRV_TOOLS}/source/enum_string_mapping.cpp"
    "${SPIRV_TOOLS}/source/ext_inst.cpp"
    "${SPIRV_TOOLS}/source/extensions.cpp"
    "${SPIRV_TOOLS}/source/libspirv.cpp"
    "${SPIRV_TOOLS}/source/name_mapper.cpp"
    "${SPIRV_TOOLS}/source/opcode.cpp"
    "${SPIRV_TOOLS}/source/operand.cpp"
    "${SPIRV_TOOLS}/source/parsed_operand.cpp"
    "${SPIRV_TOOLS}/source/print.cpp"
    "${SPIRV_TOOLS}/source/software_version.cpp"
    "${SPIRV_TOOLS}/source/spirv_endian.cpp"
    "${SPIRV_TOOLS}/source/spirv_optimizer_options.cpp"
    "${SPIRV_TOOLS}/source/spirv_reducer_options.cpp"
    "${SPIRV_TOOLS}/source/spirv_target_env.cpp"
    "${SPIRV_TOOLS}/source/spirv_validator_options.cpp"
    "${SPIRV_TOOLS}/source/table.cpp"
    "${SPIRV_TOOLS}/source/text.cpp"
    "${SPIRV_TOOLS}/source/text_handler.cpp"
    "${SPIRV_TOOLS}/source/util/bit_vector.cpp"
    "${SPIRV_TOOLS}/source/util/parse_number.cpp"
    "${SPIRV_TOOLS}/source/util/string_utils.cpp"
    "${SPIRV_TOOLS}/source/val/basic_block.cpp"
    "${SPIRV_TOOLS}/source/val/construct.cpp"
    "${SPIRV_TOOLS}/source/val/function.cpp"
    "${SPIRV_TOOLS}/source/val/instruction.cpp"
    "${SPIRV_TOOLS}/source/val/validate.cpp"
    "${SPIRV_TOOLS}/source/val/validate_adjacency.cpp"
    "${SPIRV_TOOLS}/source/val/validate_annotation.cpp"
    "${SPIRV_TOOLS}/source/val/validate_arithmetics.cpp"
    "${SPIRV_TOOLS}/source/val/validate_atomics.cpp"
    "${SPIRV_TOOLS}/source/val/validate_barriers.cpp"
    "${SPIRV_TOOLS}/source/val/validate_bitwise.cpp"
    "${SPIRV_TOOLS}/source/val/validate_builtins.cpp"
    "${SPIRV_TOOLS}/source/val/validate_capability.cpp"
    "${SPIRV_TOOLS}/source/val/validate_cfg.cpp"
    "${SPIRV_TOOLS}/source/val/validate_composites.cpp"
    "${SPIRV_TOOLS}/source/val/validate_constants.cpp"
    "${SPIRV_TOOLS}/source/val/validate_conversion.cpp"
    "${SPIRV_TOOLS}/source/val/validate_debug.cpp"
    "${SPIRV_TOOLS}/source/val/validate_decorations.cpp"
    "${SPIRV_TOOLS}/source/val/validate_derivatives.cpp"
    "${SPIRV_TOOLS}/source/val/validate_execution_limitations.cpp"
    "${SPIRV_TOOLS}/source/val/validate_extensions.cpp"
    "${SPIRV_TOOLS}/source/val/validate_function.cpp"
    "${SPIRV_TOOLS}/source/val/validate_id.cpp"
    "${SPIRV_TOOLS}/source/val/validate_image.cpp"
    "${SPIRV_TOOLS}/source/val/validate_instruction.cpp"
    "${SPIRV_TOOLS}/source/val/validate_interfaces.cpp"
    "${SPIRV_TOOLS}/source/val/validate_layout.cpp"
    "${SPIRV_TOOLS}/source/val/validate_literals.cpp"
    "${SPIRV_TOOLS}/source/val/validate_logicals.cpp"
    "${SPIRV_TOOLS}/source/val/validate_memory.cpp"
    "${SPIRV_TOOLS}/source/val/validate_memory_semantics.cpp"
    "${SPIRV_TOOLS}/source/val/validate_misc.cpp"
    "${SPIRV_TOOLS}/source/val/validate_mode_setting.cpp"
    "${SPIRV_TOOLS}/source/val/validate_non_uniform.cpp"
    "${SPIRV_TOOLS}/source/val/validate_primitives.cpp"
    "${SPIRV_TOOLS}/source/val/validate_scopes.cpp"
    "${SPIRV_TOOLS}/source/val/validate_small_type_uses.cpp"
    "${SPIRV_TOOLS}/source/val/validate_type.cpp"
    "${SPIRV_TOOLS}/source/val/validation_state.cpp"
)

add_library(spirv-opt STATIC ${SPIRV_TOOLS_OPT} ${SPIRV_TOOLS_REDUCE} ${SPIRV_TOOLS_SRC})
target_include_directories(spirv-opt PUBLIC ${SPIRV_TOOLS} ${SPIRV_TOOLS}/include ${SPIRV_TOOLS}/include/generated ${SPIRV_TOOLS}/source ${SPIRV_HEADERS}/include)

# spirv-cross
set(SPIRV_CROSS_SRC
    "${SPIRV_CROSS}/spirv_cfg.cpp"
    "${SPIRV_CROSS}/spirv_cpp.cpp"
    "${SPIRV_CROSS}/spirv_cross.cpp"
    "${SPIRV_CROSS}/spirv_cross_parsed_ir.cpp"
    "${SPIRV_CROSS}/spirv_cross_util.cpp"
    "${SPIRV_CROSS}/spirv_glsl.cpp"
    "${SPIRV_CROSS}/spirv_hlsl.cpp"
    "${SPIRV_CROSS}/spirv_msl.cpp"
    "${SPIRV_CROSS}/spirv_parser.cpp"
    "${SPIRV_CROSS}/spirv_reflect.cpp"
)

add_library(spirv-cross STATIC ${SPIRV_CROSS_SRC})
target_include_directories(spirv-cross PUBLIC ${SPIRV_CROSS}/include)
target_compile_definitions(spirv-cross PUBLIC SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)

# glslang
file(GLOB_RECURSE GLSLANG_SRC ${GLSLANG}/glslang/*.cpp)
file(GLOB_RECURSE GLSLANG_HLSL ${GLSLANG}/hlsl/*.cpp)
file(GLOB_RECURSE GLSLANG_SPIRV ${GLSLANG}/SPIRV/*.cpp)
file(GLOB_RECURSE GLSLANG_OLGC ${GLSLANG}/OGLCompilersDLL/*.cpp)

file(GLOB_RECURSE GLSLANG_REMOVE ${GLSLANG}/glslang/*main.cpp ${GLSLANG}/glslang/OSDependent/Unix/*.cpp)
list(REMOVE_ITEM GLSLANG_SRC ${GLSLANG_REMOVE})

add_library(glslang STATIC ${GLSLANG_SRC} ${GLSLANG_HLSL} ${GLSLANG_SPIRV} ${GLSLANG_OLGC})
target_include_directories(glslang PUBLIC ${GLSLANG}/include ${SPIRV_TOOLS}/include ${SPIRV_TOOLS}/source)
target_compile_definitions(glslang PUBLIC ENABLE_OPT=1 ENABLE_HLSL=1)

# set(SHADERC_DIR ${BGFX_DIR}/tools/shaderc)
# file(GLOB SHADERC_SRC ${SHADERC_DIR}/*.cpp)
# add_executable(shaderc ${SHADERC_SRC})
# # target_link_libraries(shaderc PUBLIC bgfx)
# target_include_directories(
#     shaderc PUBLIC
#     ${BGFX_3RD}/fcpp
#     ${BGFX_3RD}/glslang
#     ${BGFX_3RD}/glslang/glslang/Public
#     ${BGFX_3RD}/glslang/glslang/Include
#     ${BGFX_3RD}/spirv-cross
#     ${BGFX_3RD}/spirv-tools/include
#     ${BGFX_3RD}/glsl-optimizer/src/glsl
#     ${BX_DIR}/include
#     ${BX_DIR}/include/compat/msvc
#     ${BGFX_DIR}/include
# )

# target_compile_definitions(shaderc PUBLIC _CRT_SECURE_NO_WARNINGS)
# target_compile_definitions(shaderc PUBLIC __STDC_LIMIT_MACROS __STDC_FORMAT_MACROS __STDC_CONSTANT_MACROS)
