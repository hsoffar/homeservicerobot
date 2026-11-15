#!/bin/sh
# Full home service: the robot picks the virtual object up and delivers it.
#   add_markers (home_service mode) shows/hides the object as the robot moves,
#   pick_objects sends the pickup and drop-off goals to move_base.
. "$(dirname "$0")/env.sh"

run_xterm "turtlebot_world" "roslaunch turtlebot_gazebo turtlebot_world.launch world_file:=$WORLD_FILE"
sleep 10
run_xterm "amcl" "roslaunch turtlebot_gazebo amcl_demo.launch map_file:=$MAP_FILE"
sleep 5
run_xterm "rviz" "rosrun rviz rviz -d $RVIZ_CONFIG"
sleep 5
run_xterm "add_markers" "rosparam load $ZONES_FILE; rosrun add_markers add_markers _mode:=home_service"
sleep 5
run_xterm "pick_objects" "rosparam load $ZONES_FILE; rosrun pick_objects pick_objects; exec bash"
wait
