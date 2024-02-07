cmake_minimum_required(VERSION 3.11)
project(spaceship VERSION 0.1.0)

add_subdirectory(libgame)


add_executable(spaceship src/spaceship.cpp)

target_link_libraries(spaceship engine)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


