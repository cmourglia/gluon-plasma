project(gluon)

add_library(${PROJECT_NAME} SHARED gln_renderer.cpp gln_application.cpp gln_text.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC gluon_render_backend PUBLIC gluon_core PRIVATE glfw)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/rapidjson/include)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLUON_API_MAKEDLL)
