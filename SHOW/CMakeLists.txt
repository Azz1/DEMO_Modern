add_library(show SHARED
    Lorenz.cpp Mandelbrot.cpp Julia.cpp Newton.cpp
    Ecosys.cpp Henon.cpp May.cpp Trammel.cpp
    Lsys.cpp Jout.cpp Pfrac.cpp
    Eyes.cpp
    GraphView.cpp
    Diagram.cpp
    Geograph.cpp
    GWB/gwb_mat.c
    GWB/gwb_3d.c
    GWB/gwb_core.c
    GWB/gwb_euler.c
    GWB/gwb_adv.c
    GWB/gwb_geo.c
    GWB/gwb_bool.c
    GWB/gwb_split.c
    GWB/gwb_trans.c
    GWB/gwb_undo.c
    GWB/gwb_shgwb.c
    Gwb.cpp
)
target_include_directories(show PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE /usr/include/eigen3)
target_link_libraries(show PUBLIC srcpool imgui)
set_target_properties(show PROPERTIES PREFIX "")
