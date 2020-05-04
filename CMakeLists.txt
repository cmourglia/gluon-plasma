cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gluon-plasma)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/ThirdParty)

add_subdirectory(ThirdParty)

add_subdirectory(Sources)
