find_package(X11 REQUIRED)
add_executable(eyes eyes.cpp)
target_link_libraries(eyes PRIVATE ${SDL2_LIBRARIES} ${X11_LIBRARIES})
set_target_properties(eyes PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
