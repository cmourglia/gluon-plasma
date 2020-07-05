project(gluon_render_backend)

add_library(${PROJECT_NAME} STATIC gln_renderbackend.h gln_renderbackend.cpp)

# TODO: public glad is temp
target_link_libraries(${PROJECT_NAME} PUBLIC glad PUBLIC gluon_core)

target_compile_definitions(${PROJECT_NAME} PUBLIC GLUON_RENDERBACKEND_MAKELIB)
# target_compile_definitions(${PROJECT_NAME} PRIVATE GLUON_RENDERBACKEND_MAKEDLL)
