cmake_minimum_required(VERSION 3.13)
project(WorkOnLabels)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
if(CMAKE_VERSION VERSION_LESS "2.8.11")
    # Add OpenCV headers location to your include paths
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

find_package(Boost COMPONENTS system filesystem REQUIRED)

include_directories(./include)

file(GLOB HEADERS "src/*/*.cpp" "src/*.cpp")
file(GLOB SOURCES "src/*/*.h" "src/*.h")

add_executable(WorkOnLabels ${SOURCES} ${HEADERS} main.cpp)

target_link_libraries(WorkOnLabels ${OpenCV_LIBS})
target_link_libraries(WorkOnLabels ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})