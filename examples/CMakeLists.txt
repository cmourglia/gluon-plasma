project(examples)

add_executable(rectangles rectangles/rectangles.cpp)
target_link_libraries(rectangles PRIVATE gluon glfw)

add_executable(text text/text.cpp)
target_link_libraries(text PRIVATE gluon glfw)
