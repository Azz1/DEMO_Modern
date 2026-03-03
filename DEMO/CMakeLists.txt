add_executable(demo main.cpp)
target_include_directories(demo PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
target_link_libraries(demo PRIVATE srcpool show imgui ${SDL2_LIBRARIES})
