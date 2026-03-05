cmake_minimum_required(VERSION 3.20)
project(Demo1 VERSION 1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(SDL2 REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/imgui_impl_sdlrenderer2.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${SDL2_INCLUDE_DIRS})
target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})

add_subdirectory(SRCPOOL)
add_subdirectory(SHOW)
add_subdirectory(DEMO)
add_subdirectory(EYES)
