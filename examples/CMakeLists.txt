project(examples)

add_executable(helloworld helloworld/helloworld.cpp)
target_link_libraries(helloworld PRIVATE gluon)

add_executable(rectangles rectangles/rectangles.cpp)
target_link_libraries(rectangles PRIVATE gluon)

add_executable(text text/text.cpp)
target_link_libraries(text PRIVATE gluon)

add_executable(button button/button.cpp)
target_link_libraries(button PRIVATE gluon)
