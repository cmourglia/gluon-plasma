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

file(GLOB BGFX_SRC ${BGFX_DIR}/src/*.cpp ${BGFX_DIR}/src/*.mm)
list(REMOVE_ITEM BGFX_SRC ${BGFX_DIR}/src/amalgamated.cpp ${BGFX_DIR}/src/amalgamated.mm)

add_library(bgfx STATIC ${BGFX_SRC})
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
file(GLOB SPIRV_TOOLS_SRC
    ${BGFX_DIR}/3rdparty/spirv-tools/source/*.cpp
    ${BGFX_DIR}/3rdparty/spirv-tools/source/*.h
    ${BGFX_DIR}/3rdparty/spirv-tools/source/opt/*.cpp
    ${BGFX_DIR}/3rdparty/spirv-tools/source/opt/*.h
    ${BGFX_DIR}/3rdparty/spirv-tools/source/reduce/*.cpp
    ${BGFX_DIR}/3rdparty/spirv-tools/source/reduce/*.h
    ${BGFX_DIR}/3rdparty/spirv-tools/source/util/*.cpp
    ${BGFX_DIR}/3rdparty/spirv-tools/source/util/*.h
    ${BGFX_DIR}/3rdparty/spirv-tools/source/val/*.cpp
    ${BGFX_DIR}/3rdparty/spirv-tools/source/val/*.h
)

add_library(spirv-tools STATIC ${SPIRV_TOOLS_SRC})
target_include_directories(spirv-tools PUBLIC ${SPIRV_TOOLS} ${SPIRV_TOOLS}/include ${SPIRV_TOOLS}/include/generated ${SPIRV_TOOLS}/source ${SPIRV_HEADERS}/include)
target_compile_definitions(spirv-tools PRIVATE _CRT_SECURE_NO_WARNINGS)

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
target_compile_definitions(spirv-cross PRIVATE SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)

# glslang
file(GLOB_RECURSE GLSLANG_SRC ${GLSLANG}/glslang/*.cpp)
file(GLOB_RECURSE GLSLANG_HLSL ${GLSLANG}/hlsl/*.cpp)
file(GLOB_RECURSE GLSLANG_SPIRV ${GLSLANG}/SPIRV/*.cpp)
file(GLOB_RECURSE GLSLANG_OLGC ${GLSLANG}/OGLCompilersDLL/*.cpp)

file(GLOB_RECURSE GLSLANG_REMOVE ${GLSLANG}/glslang/*main.cpp ${GLSLANG}/glslang/OSDependent/Unix/*.cpp)
list(REMOVE_ITEM GLSLANG_SRC ${GLSLANG_REMOVE})

add_library(glslang STATIC ${GLSLANG_SRC} ${GLSLANG_HLSL} ${GLSLANG_SPIRV} ${GLSLANG_OLGC})
target_include_directories(glslang PUBLIC ${GLSLANG}/include ${SPIRV_TOOLS}/include ${SPIRV_TOOLS}/source)
target_compile_definitions(glslang PRIVATE ENABLE_OPT=1 ENABLE_HLSL=1)

# glsl-optimizer
set(OPTIMIZER_SRC
    "${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-lex.c"
    "${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-parse.c"
    "${GLSL_OPTIMIZER}/src/glsl/glcpp/pp.c"
    "${GLSL_OPTIMIZER}/src/glsl/ast_array_index.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ast_expr.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ast_function.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ast_to_hir.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ast_type.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/builtin_functions.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/builtin_types.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/builtin_variables.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/glsl_lexer.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/glsl_optimizer.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/glsl_parser.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/glsl_parser_extras.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/glsl_symbol_table.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/glsl_types.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/hir_field_selection.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_basic_block.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_builder.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_clone.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_constant_expression.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_equals.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_expression_flattening.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_function.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_function_can_inline.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_function_detect_recursion.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_hierarchical_visitor.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_hv_accept.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_import_prototypes.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_print_glsl_visitor.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_print_metal_visitor.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_print_visitor.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_rvalue_visitor.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_stats.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_unused_structs.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_validate.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/ir_variable_refcount.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_atomics.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_functions.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_interface_blocks.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_uniform_block_active_visitor.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_uniform_blocks.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_uniform_initializers.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_uniforms.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/link_varyings.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/linker.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/loop_analysis.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/loop_controls.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/loop_unroll.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_clip_distance.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_discard.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_discard_flow.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_if_to_cond_assign.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_instructions.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_jumps.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_mat_op_to_vec.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_named_interface_blocks.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_noise.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_offset_array.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_output_reads.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_packed_varyings.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_packing_builtins.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_ubo_reference.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_variable_index_to_cond_assign.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_vec_index_to_cond_assign.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_vec_index_to_swizzle.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_vector.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_vector_insert.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/lower_vertex_id.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_algebraic.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_array_splitting.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_constant_folding.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_constant_propagation.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_constant_variable.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_copy_propagation.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_copy_propagation_elements.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_cse.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_dead_builtin_variables.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_dead_builtin_varyings.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_dead_code.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_dead_code_local.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_dead_functions.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_flatten_nested_if_blocks.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_flip_matrices.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_function_inlining.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_if_simplification.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_minmax.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_noop_swizzle.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_rebalance_tree.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_redundant_jumps.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_structure_splitting.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_swizzle_swizzle.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_tree_grafting.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/opt_vectorize.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/s_expression.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/standalone_scaffolding.cpp"
    "${GLSL_OPTIMIZER}/src/glsl/strtod.c"
    "${GLSL_OPTIMIZER}/src/mesa/main/imports.c"
    "${GLSL_OPTIMIZER}/src/mesa/program/prog_hash_table.c"
    "${GLSL_OPTIMIZER}/src/mesa/program/symbol_table.c"
    "${GLSL_OPTIMIZER}/src/util/hash_table.c"
    "${GLSL_OPTIMIZER}/src/util/ralloc.c"
)

add_library(glsl-optimizer STATIC ${OPTIMIZER_SRC})
target_include_directories(glsl-optimizer PUBLIC ${GLSL_OPTIMIZER}/src ${GLSL_OPTIMIZER}/include ${GLSL_OPTIMIZER}/src/mesa ${GLSL_OPTIMIZER}/src/mapi ${GLSL_OPTIMIZER}/glsl)
if(MSVC)
    target_include_directories(glsl-optimizer PUBLIC ${GLSL_OPTIMIZER}/glsl/msvc)
    target_compile_definitions(glsl-optimizer PRIVATE __STDC__ __STDC_VERSION__=199901L strdup=_strdup alloca=_alloca isascii=__isascii _CRT_SECURE_NO_WARNINGS)
    target_compile_options(glsl-optimizer PUBLIC "/wd4291")
endif()

# fcpp
add_library(fcpp STATIC ${FCPP}/cpp1.c ${FCPP}/cpp2.c ${FCPP}/cpp3.c ${FCPP}/cpp4.c ${FCPP}/cpp5.c ${FCPP}/cpp6.c)
target_compile_definitions(fcpp PRIVATE NINCLUDE=64 NWORK=65536 NBUFF=65536 OLD_PREPROCESSOR=0)
if(MSVC)
    target_compile_definitions(fcpp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

set(SHADERC_DIR ${BGFX_DIR}/tools/shaderc)
file(GLOB SHADERC_SRC ${SHADERC_DIR}/shaderc.cpp ${SHADERC_DIR}/shaderc_glsl.cpp ${SHADERC_DIR}/shaderc_hlsl.cpp ${SHADERC_DIR}/shaderc_pssl.cpp ${SHADERC_DIR}/shaderc_spirv.cpp ${SHADERC_DIR}/shaderc_metal.cpp)
add_executable(shaderc ${SHADERC_SRC} ${BGFX_DIR}/src/vertexdecl.cpp ${BGFX_DIR}/src/shader_spirv.cpp)
target_link_libraries(shaderc PUBLIC bx fcpp glsl-optimizer glslang spirv-cross spirv-tools)
target_include_directories(
    shaderc PUBLIC
    ${BIMG_DIR}/include
    ${BGFX_DIR}/include
    ${BGFX_3RD}/dsdk/include
    ${FCPP}
    ${BGFX_3RD}/glslang/glslang/Public
    ${BGFX_3RD}/glslang/glslang/Include
    ${SPIRV_CROSS}
    ${GLSLANG}
    ${GLSL_OPTIMIZER}/include
    ${GLSL_OPTIMIZER}/src/glsl
)

if(WIN32)
    target_include_directories(shaderc PUBLIC ${GLSL_OPTIMIZER}/include/c99)
endif(WIN32)

# target_compile_definitions(shaderc PUBLIC _CRT_SECURE_NO_WARNINGS)
# target_compile_definitions(shaderc PUBLIC __STDC_LIMIT_MACROS __STDC_FORMAT_MACROS __STDC_CONSTANT_MACROS)
