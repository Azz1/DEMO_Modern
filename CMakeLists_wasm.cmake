cmake_minimum_required(VERSION 3.20)
project(demo1_wasm)
set(CMAKE_CXX_STANDARD 17)

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# All plugin + demo sources compiled as one executable (no .so in WASM)
add_executable(demo
    ${IMGUI_SOURCES}
    SHOW/Lorenz.cpp
    SHOW/Mandelbrot.cpp
    SHOW/Julia.cpp
    SHOW/Newton.cpp
    SHOW/Ecosys.cpp
    DEMO/main_wasm.cpp
)

target_include_directories(demo PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/SRCPOOL
    ${CMAKE_SOURCE_DIR}/SHOW
    ${CMAKE_SOURCE_DIR}/third_party
)

# Emscripten flags
set(EM_FLAGS
    -O2
    -s USE_SDL=2
    -s ALLOW_MEMORY_GROWTH=1
    -s EXPORTED_RUNTIME_METHODS=ccall
    --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html
)

target_compile_options(demo PRIVATE ${EM_FLAGS} -DIMGUI_IMPL_OPENGL_ES2)
target_link_options(demo PRIVATE
    ${EM_FLAGS}
    -s MIN_WEBGL_VERSION=2
    -s MAX_WEBGL_VERSION=2
    --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html
    -o ${CMAKE_BINARY_DIR}/demo.html
)
set_target_properties(demo PROPERTIES SUFFIX ".html")
