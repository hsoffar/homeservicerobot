#!/bin/sh
# Manual navigation test: turtlebot + AMCL on the saved map + rviz.
# Use "2D Nav Goal" in rviz to send goals.
. "$(dirname "$0")/env.sh"

run_xterm "turtlebot_world" "roslaunch turtlebot_gazebo turtlebot_world.launch world_file:=$WORLD_FILE"
sleep 10
run_xterm "amcl" "roslaunch turtlebot_gazebo amcl_demo.launch map_file:=$MAP_FILE"
sleep 5
run_xterm "rviz" "roslaunch turtlebot_rviz_launchers view_navigation.launch"
wait
