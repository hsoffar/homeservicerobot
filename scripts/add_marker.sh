#!/bin/sh
# Rubric "add_marker.sh": the virtual object reacts to the robot itself.
#   - marker published at the pickup zone when the robot starts
#   - marker hidden when the robot reaches the pickup zone (pickup)
#   - marker re-published at the drop-off zone when the robot delivers it
# pick_objects drives the robot so the sequence runs without manual input.
# (add_markers.sh is the course's time-based version: 5 s shown, 5 s hidden, drop-off.)
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
