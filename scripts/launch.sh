#!/bin/sh
# Course exercise: gazebo, roscore and rviz, each in its own xterm.
xterm -e " gazebo " &
sleep 5
xterm -e bash -c " source /opt/ros/kinetic/setup.bash; roscore" &
sleep 5
xterm -e bash -c " source /opt/ros/kinetic/setup.bash; rosrun rviz rviz"
