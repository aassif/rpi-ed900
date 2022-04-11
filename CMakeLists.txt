cmake_minimum_required (VERSION 3.18.4)

set (CMAKE_CXX_STANDARD 17)

project (PiED900)

find_package (Threads REQUIRED)

find_package (PkgConfig REQUIRED)
pkg_check_modules (SYSTEMD REQUIRED libsystemd)
pkg_check_modules (SDL2 REQUIRED sdl2)
pkg_check_modules (SDL2_IMAGE REQUIRED SDL2_image)

#find_package (SDL2 REQUIRED)

include_directories (${SYSTEMD_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

add_executable (PiED900 ed900.cpp app.cpp game.cpp cricket.cpp font.cpp main.cpp)

target_link_libraries (PiED900 ${SYSTEMD_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} Threads::Threads)

