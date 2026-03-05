cmake_minimum_required(VERSION 3.20)
project(demo1_wasm)
set(CMAKE_CXX_STANDARD 17)

set(BASE /home/pi/Workspace/DEMO1)
set(IMGUI_DIR ${BASE}/third_party/imgui)

add_executable(demo
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/imgui_impl_sdlrenderer2.cpp
    ${BASE}/SHOW/Lorenz.cpp
    ${BASE}/SHOW/Mandelbrot.cpp
    ${BASE}/SHOW/Julia.cpp
    ${BASE}/SHOW/Newton.cpp
    ${BASE}/SHOW/Ecosys.cpp
    ${BASE}/SHOW/Henon.cpp
    ${BASE}/SHOW/May.cpp
    ${BASE}/SHOW/Trammel.cpp
    ${BASE}/SHOW/Lsys.cpp
    ${BASE}/SHOW/Jout.cpp
    ${BASE}/SHOW/Pfrac.cpp
    ${BASE}/SHOW/Eyes.cpp
    ${BASE}/SHOW/GraphView.cpp
    ${BASE}/SHOW/Diagram.cpp
    ${BASE}/SHOW/Geograph.cpp
    ${BASE}/DEMO/main_wasm.cpp
)

target_include_directories(demo PRIVATE
    ${IMGUI_DIR}
    ${BASE}/SRCPOOL
    ${BASE}/SHOW
    ${BASE}/third_party
)

target_compile_options(demo PRIVATE -O2 -sUSE_SDL=2)
target_link_options(demo PRIVATE
    -O2
    -sUSE_SDL=2
    -sALLOW_MEMORY_GROWTH=1
    "-sEXPORTED_RUNTIME_METHODS=ccall,UTF8ToString"
    -sMIN_WEBGL_VERSION=2
    -sMAX_WEBGL_VERSION=2
    -sFORCE_FILESYSTEM=1
    --shell-file ${BASE}/web/shell.html
)
set_target_properties(demo PROPERTIES SUFFIX ".html")
