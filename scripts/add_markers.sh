#!/bin/sh
# Course exercise (time based): marker at pickup for 5 s, hidden for 5 s, then at drop-off.
# See add_marker.sh for the rubric version that follows the robot.
. "$(dirname "$0")/env.sh"

run_xterm "turtlebot_world" "roslaunch turtlebot_gazebo turtlebot_world.launch world_file:=$WORLD_FILE"
sleep 10
run_xterm "amcl" "roslaunch turtlebot_gazebo amcl_demo.launch map_file:=$MAP_FILE"
sleep 5
run_xterm "rviz" "rosrun rviz rviz -d $RVIZ_CONFIG"
sleep 5
run_xterm "add_markers" "rosparam load $ZONES_FILE; rosrun add_markers add_markers _mode:=demo"
wait
