#!/bin/sh
# Shared settings for the Home Service Robot scripts (sourced, not run).
# Run the scripts inside the ROS Kinetic container:
#   ./rosdocker -k -w catkin_ws_home_service
#   ~/catkin_ws/src/scripts/home_service.sh
SRC_DIR=$(cd "$(dirname "$0")/.." && pwd)
WS_DIR=$(cd "$SRC_DIR/.." && pwd)
WORLD_FILE="$SRC_DIR/map/my_world.world"
MAP_FILE="$SRC_DIR/map/map.yaml"
ZONES_FILE="$SRC_DIR/pick_objects/config/zones.yaml"
RVIZ_CONFIG="$SRC_DIR/rvizConfig/home_service.rviz"

# Prefix for every xterm: ROS + workspace overlay, kinect sensor for the
# turtlebot launch files, and the custom building models for Gazebo.
ROS_ENV="source /opt/ros/kinetic/setup.bash; \
source $WS_DIR/devel/setup.bash; \
export TURTLEBOT_3D_SENSOR=kinect; \
export GAZEBO_MODEL_PATH=$SRC_DIR/map/models:/usr/share/gazebo-7/models:\$GAZEBO_MODEL_PATH"

# run_xterm "<title>" "<command>": run a command in its own xterm.
run_xterm() {
  xterm -T "$1" -e bash -c "$ROS_ENV; $2" &
}
