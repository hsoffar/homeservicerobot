// pick_objects: drives the robot to the pickup zone, waits 5 s to "load",
// then drives to the drop-off zone, using the move_base action server.
#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/transform_datatypes.h>
#include <string>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

struct Zone {
  double x;
  double y;
  double yaw;
};

// Zones come from /pickup/* and /dropoff/* (pick_objects/config/zones.yaml),
// falling back to the defaults below when the parameters are not loaded.
static Zone loadZone(const std::string& name, const Zone& fallback) {
  Zone zone;
  ros::param::param("/" + name + "/x", zone.x, fallback.x);
  ros::param::param("/" + name + "/y", zone.y, fallback.y);
  ros::param::param("/" + name + "/yaw", zone.yaw, fallback.yaw);
  return zone;
}

static bool goTo(MoveBaseClient& client, const Zone& zone, const std::string& label) {
  move_base_msgs::MoveBaseGoal goal;
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();
  goal.target_pose.pose.position.x = zone.x;
  goal.target_pose.pose.position.y = zone.y;
  goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(zone.yaw);

  ROS_INFO("Heading to the %s zone (%.2f, %.2f)", label.c_str(), zone.x, zone.y);
  client.sendGoal(goal);
  client.waitForResult();
  return client.getState() == actionlib::SimpleClientGoalState::SUCCEEDED;
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "pick_objects");
  ros::NodeHandle node;

  MoveBaseClient client("move_base", true);
  while (ros::ok() && !client.waitForServer(ros::Duration(5.0))) {
    ROS_INFO("Waiting for the move_base action server to come up");
  }
  if (!ros::ok()) {
    return 0;
  }

  const Zone pickup = loadZone("pickup", Zone{-3.0, -2.0, 0.0});
  const Zone dropoff = loadZone("dropoff", Zone{2.8, -2.2, 0.0});

  if (!goTo(client, pickup, "pickup")) {
    ROS_ERROR("Failed to reach the pickup zone");
    return 1;
  }
  ROS_INFO("Reached the pickup zone");

  ROS_INFO("Picking up the object (5 s)");
  ros::Duration(5.0).sleep();

  if (!goTo(client, dropoff, "drop-off")) {
    ROS_ERROR("Failed to reach the drop-off zone");
    return 1;
  }
  ROS_INFO("Reached the drop-off zone");

  return 0;
}
