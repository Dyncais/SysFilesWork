cmake_minimum_required(VERSION 3.10)
project(vscode)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

include_directories(${LIBZIP_INCLUDE_DIRS})
link_directories(${LIBZIP_LIBRARY_DIRS})
add_definitions(${LIBZIP_CFLAGS_OTHER})

add_subdirectory(vendor/libzippp)
add_executable(WorkerFile main.cpp vendor/tinyxml2.cpp vendor/tinyxml2.h)
target_link_libraries(WorkerFile PRIVATE libzippp)
