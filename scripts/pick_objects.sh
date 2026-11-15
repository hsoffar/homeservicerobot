#!/bin/sh
# Autonomous goals: drive to the pickup zone, wait 5 s, drive to the drop-off zone.
. "$(dirname "$0")/env.sh"

run_xterm "turtlebot_world" "roslaunch turtlebot_gazebo turtlebot_world.launch world_file:=$WORLD_FILE"
sleep 10
run_xterm "amcl" "roslaunch turtlebot_gazebo amcl_demo.launch map_file:=$MAP_FILE"
sleep 5
run_xterm "rviz" "roslaunch turtlebot_rviz_launchers view_navigation.launch"
sleep 5
run_xterm "pick_objects" "rosparam load $ZONES_FILE; rosrun pick_objects pick_objects; exec bash"
wait
