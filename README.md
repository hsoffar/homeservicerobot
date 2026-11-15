# Home Service Robot

A TurtleBot 2 maps a two-room building, localizes itself in that map, and works as a
home service robot. It drives to a pickup zone, "loads" a virtual object, carries it
to a drop-off zone, and "delivers" it. The object is a cube marker in rviz.

Built and tested on **ROS Kinetic + Gazebo 7**.

## Packages used

### Mapping: `gmapping` (slam_gmapping)
`gmapping` runs grid-based FastSLAM: a Rao-Blackwellized particle filter where each
particle carries a robot trajectory and its own occupancy grid. It reads the laser scan
and odometry, scan-matches each new scan against every particle's map, resamples, and
publishes the map from the best particle on `/map`. On the TurtleBot, the scan comes
from the Kinect depth image via `depthimage_to_laserscan`.
`turtlebot_gazebo/gmapping_demo.launch` loads the Kinect-tuned settings: 80 particles,
5 cm cells, 6 m usable range.

`map/map.pgm` + `map/map.yaml` were made with gmapping. `scripts/auto_map.py` drove the
robot through both rooms with `move_base`, turning 360° at each waypoint so the Kinect
covered every wall. The map was then saved with `map_server map_saver`. `test_slam.sh`
runs the same SLAM pipeline under manual keyboard control.

### Localization: `amcl`
`amcl` (Adaptive Monte Carlo Localization) tracks the robot's pose in the known map
with a particle filter. Particles move according to the odometry motion model and are
weighted by how well the laser scan matches the map (likelihood-field model). KLD
sampling grows or shrinks the particle count with the pose uncertainty. AMCL publishes
the `map -> odom` transform, which makes `map -> base_footprint` available to the other
nodes. It is started by `turtlebot_gazebo/amcl_demo.launch` with `map_file:=map/map.yaml`.

### Navigation: `move_base` (ROS navigation stack)
`move_base` exposes the `move_base` action server that `pick_objects` sends goals to:
- **Global planner (`navfn`)**: Dijkstra's algorithm (a variant of uniform cost search)
  over the global costmap. The global costmap is the SLAM map with obstacles inflated by
  the robot radius.
- **Local planner (`dwa_local_planner`)**: the Dynamic Window Approach. It samples
  reachable velocities, simulates short trajectories, and scores them on how well they
  follow the path, how close they get to the goal, and how much clearance they keep. A
  rolling local costmap built from live sensor data lets it avoid obstacles that are not
  in the map and re-plan around them.
- Recovery behaviours clear the costmaps and rotate in place when the robot gets stuck.

### Simulation and tools
| Package | Role |
|---|---|
| `turtlebot_gazebo` | `turtlebot_world.launch` spawns the Kobuki/Kinect TurtleBot into `map/my_world.world` |
| `turtlebot_teleop` | `keyboard_teleop.launch` for manual driving during the SLAM test |
| `turtlebot_rviz_launchers` | `view_navigation.launch`: preconfigured rviz with robot model, map, costmaps, plans and particles |

### Project packages
| Package | What it does |
|---|---|
| `pick_objects` | Action client of `move_base`. Sends the pickup goal, logs *Reached the pickup zone*, waits 5 s, sends the drop-off goal, logs *Reached the drop-off zone*. Goals use the `map` frame. |
| `add_markers` | Publishes the virtual object (a cube) on `visualization_marker`, latched. `_mode:=demo`: shown at pickup for 5 s, hidden for 5 s, then shown at drop-off. `_mode:=home_service`: follows the robot through `tf` (`map -> base_footprint`); hides the cube when the robot is within 0.3 m of the pickup zone and shows it at the drop-off zone when the robot arrives there. |

Both nodes read the zones from `pick_objects/config/zones.yaml`:
pickup `(-3.0, -2.0)` in the left room, drop-off `(2.8, -2.2)` in the right room.

## Layout
```
src/
├── slam_gmapping/            official: gmapping (pinned to 1.4.2, see below)
├── turtlebot/                official: turtlebot_teleop
├── turtlebot_interactions/   official: turtlebot_rviz_launchers
├── turtlebot_simulator/      official: turtlebot_gazebo
├── map/                      my_world.world, its models, SLAM map (map.pgm/.yaml)
├── scripts/                  shell scripts (+ env.sh shared settings, auto_map.py)
├── rvizConfig/               home_service.rviz (navigation view + Virtual Object marker)
├── pick_objects/             src/pick_objects.cpp, config/zones.yaml
└── add_markers/              src/add_markers.cpp
```

## Running
Each script opens one xterm per node:

| Script | Launches |
|---|---|
| `scripts/launch.sh` | gazebo, roscore, rviz (course exercise) |
| `scripts/test_slam.sh` | world + gmapping + rviz + keyboard teleop |
| `scripts/test_navigation.sh` | world + AMCL/move_base on the map + rviz (use *2D Nav Goal*) |
| `scripts/pick_objects.sh` | world + AMCL/move_base + rviz + `pick_objects` |
| `scripts/add_marker.sh` | world + AMCL/move_base + rviz + `add_markers` (home_service mode) + `pick_objects`: marker at pickup when the robot starts, hidden when it reaches pickup, re-published at drop-off on delivery (rubric version) |
| `scripts/add_markers.sh` | world + AMCL/move_base + rviz + `add_markers` (demo mode): 5 s at pickup, 5 s hidden, then at drop-off (course exercise) |
| `scripts/home_service.sh` | world + AMCL/move_base + rviz + `add_markers` (home_service) + `pick_objects` |

```bash
cd ~/catkin_ws && catkin_make && source devel/setup.bash
./src/scripts/home_service.sh
```

The scripts set `TURTLEBOT_3D_SENSOR=kinect` and add `map/models` to
`GAZEBO_MODEL_PATH`, so nothing else needs configuring. They need `xterm`.

## Notes
- `slam_gmapping` is pinned to tag **1.4.2**. Later commits on `melodic-devel` use C++14
  generic lambdas, which Kinetic's GCC 5 (C++11) cannot compile.
