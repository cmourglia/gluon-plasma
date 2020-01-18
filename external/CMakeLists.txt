# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

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

add_library(loguru STATIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru/loguru.cpp)
target_include_directories(loguru PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru)
