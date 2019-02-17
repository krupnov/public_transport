cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME planner)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(Boost_USE_STATIC_LIBS   ON)
set(Protobuf_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS program_options filesystem date_time)

add_executable(${PROJECT_NAME} main.cpp parser.cpp parser.h structures.h)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})