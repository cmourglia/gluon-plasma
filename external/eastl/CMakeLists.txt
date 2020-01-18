set(EABASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/EABase)
set(EASTL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/EASTL)

file(GLOB EASTL_SRC ${EASTL_DIR}/source/*.cpp)
add_library(eastl STATIC ${EASTL_SRC} alloc.cpp)
target_include_directories(eastl PUBLIC ${EABASE_DIR}/include/Common ${EASTL_DIR}/include)
