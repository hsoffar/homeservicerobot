#!/bin/sh
# Manual SLAM test: turtlebot in my world + gmapping + rviz + keyboard teleop.
# Drive with the "keyboard_teleop" xterm; save the map with:
#   rosrun map_server map_saver -f ~/catkin_ws/src/map/map
. "$(dirname "$0")/env.sh"

run_xterm "turtlebot_world" "roslaunch turtlebot_gazebo turtlebot_world.launch world_file:=$WORLD_FILE"
sleep 10
run_xterm "gmapping" "roslaunch turtlebot_gazebo gmapping_demo.launch"
sleep 5
run_xterm "rviz" "roslaunch turtlebot_rviz_launchers view_navigation.launch"
sleep 5
run_xterm "keyboard_teleop" "roslaunch turtlebot_teleop keyboard_teleop.launch"
wait
