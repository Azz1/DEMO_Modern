add_library(show SHARED
    Lorenz.cpp Mandelbrot.cpp Julia.cpp Newton.cpp
    Ecosys.cpp Henon.cpp May.cpp Trammel.cpp
    Lsys.cpp Jout.cpp Pfrac.cpp
)
target_include_directories(show PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE /usr/include/eigen3)
target_link_libraries(show PUBLIC srcpool imgui)
set_target_properties(show PROPERTIES PREFIX "")
