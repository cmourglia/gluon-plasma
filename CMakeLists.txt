cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(GluonPlasma)

add_subdirectory(external)

add_executable(gluon src/main.cpp)
target_link_libraries(gluon PUBLIC bgfx glfw loguru eastl)
target_include_directories(gluon PUBLIC external/rapidjson/include external/glm)
