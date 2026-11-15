cmake_minimum_required(VERSION 2.8.3)
project(pick_objects)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS roscpp actionlib move_base_msgs tf)
catkin_package()

include_directories(${catkin_INCLUDE_DIRS})

add_executable(pick_objects src/pick_objects.cpp)
target_link_libraries(pick_objects ${catkin_LIBRARIES})
