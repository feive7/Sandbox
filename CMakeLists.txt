cmake_minimum_required(VERSION 3.10)

project(raybox)

if(WIN32)
set(raylib_DIR "../Libraries/raylib")
endif()

find_package(raylib REQUIRED)
add_executable(${PROJECT_NAME} main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} raylib)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
