// add_markers: models the virtual object as a cube marker in rviz.
//
// ~mode:=demo          time based: show at pickup 5 s, hide 5 s, show at drop-off
// ~mode:=home_service  follows the robot: shown at pickup until the robot gets there,
//                      hidden while carried, shown at drop-off once the robot arrives
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <tf/transform_listener.h>
#include <cmath>
#include <string>

struct Zone {
  double x;
  double y;
};

// Same parameters as pick_objects (pick_objects/config/zones.yaml).
static Zone loadZone(const std::string& name, const Zone& fallback) {
  Zone zone;
  ros::param::param("/" + name + "/x", zone.x, fallback.x);
  ros::param::param("/" + name + "/y", zone.y, fallback.y);
  return zone;
}

class VirtualObject {
 public:
  explicit VirtualObject(ros::NodeHandle& node)
      // Latched, so an rviz that starts later still receives the current state.
      : pub_(node.advertise<visualization_msgs::Marker>("visualization_marker", 1, true)) {
    marker_.header.frame_id = "map";
    marker_.ns = "add_markers";
    marker_.id = 0;
    marker_.type = visualization_msgs::Marker::CUBE;
    marker_.pose.orientation.w = 1.0;
    marker_.scale.x = 0.3;
    marker_.scale.y = 0.3;
    marker_.scale.z = 0.3;
    marker_.color.r = 0.0f;
    marker_.color.g = 0.4f;
    marker_.color.b = 1.0f;
    marker_.color.a = 1.0f;
    marker_.lifetime = ros::Duration();  // never expire
  }

  void showAt(const Zone& zone) {
    marker_.action = visualization_msgs::Marker::ADD;
    marker_.pose.position.x = zone.x;
    marker_.pose.position.y = zone.y;
    marker_.pose.position.z = marker_.scale.z / 2.0;
    publish();
  }

  void hide() {
    marker_.action = visualization_msgs::Marker::DELETE;
    publish();
  }

  void publish() {
    marker_.header.stamp = ros::Time::now();
    pub_.publish(marker_);
  }

  // Give rviz a chance to subscribe so the first state is not missed.
  void waitForSubscriber(double timeout_s) {
    const ros::WallTime deadline = ros::WallTime::now() + ros::WallDuration(timeout_s);
    while (ros::ok() && pub_.getNumSubscribers() == 0 && ros::WallTime::now() < deadline) {
      ros::WallDuration(0.2).sleep();
    }
  }

 private:
  ros::Publisher pub_;
  visualization_msgs::Marker marker_;
};

static void runDemo(VirtualObject& object, const Zone& pickup, const Zone& dropoff) {
  object.waitForSubscriber(30.0);

  ROS_INFO("Marker at the pickup zone");
  object.showAt(pickup);
  ros::Duration(5.0).sleep();

  ROS_INFO("Marker hidden");
  object.hide();
  ros::Duration(5.0).sleep();

  ROS_INFO("Marker at the drop-off zone");
  object.showAt(dropoff);
  ros::spin();
}

static void runHomeService(VirtualObject& object, const Zone& pickup, const Zone& dropoff,
                           const std::string& robot_frame, double tolerance) {
  enum State { WAITING_FOR_PICKUP, CARRYING, DELIVERED };
  State state = WAITING_FOR_PICKUP;

  // No waitForSubscriber here: tracking must start at once so an early arrival is
  // not missed; the latched topic still hands the current state to a late rviz.
  tf::TransformListener listener;
  object.showAt(pickup);
  ROS_INFO("Object waiting at the pickup zone (%.2f, %.2f)", pickup.x, pickup.y);

  ros::Rate rate(10.0);
  while (ros::ok() && state != DELIVERED) {
    rate.sleep();

    tf::StampedTransform robot;
    try {
      listener.lookupTransform("map", robot_frame, ros::Time(0), robot);
    } catch (const tf::TransformException& ex) {
      ROS_WARN_THROTTLE(5.0, "Waiting for map -> %s: %s", robot_frame.c_str(), ex.what());
      continue;
    }
    const double x = robot.getOrigin().x();
    const double y = robot.getOrigin().y();

    if (state == WAITING_FOR_PICKUP && std::hypot(x - pickup.x, y - pickup.y) < tolerance) {
      object.hide();
      state = CARRYING;
      ROS_INFO("Robot reached the pickup zone: object picked up");
    } else if (state == CARRYING && std::hypot(x - dropoff.x, y - dropoff.y) < tolerance) {
      object.showAt(dropoff);
      state = DELIVERED;
      ROS_INFO("Robot reached the drop-off zone: object delivered");
    }
  }
  ros::spin();
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "add_markers");
  ros::NodeHandle node;
  ros::NodeHandle private_node("~");

  std::string mode;
  std::string robot_frame;
  double tolerance;
  private_node.param<std::string>("mode", mode, "demo");
  private_node.param<std::string>("robot_frame", robot_frame, "base_footprint");
  private_node.param("tolerance", tolerance, 0.3);

  const Zone pickup = loadZone("pickup", Zone{-3.0, -2.0});
  const Zone dropoff = loadZone("dropoff", Zone{2.8, -2.2});

  VirtualObject object(node);
  if (mode == "home_service") {
    runHomeService(object, pickup, dropoff, robot_frame, tolerance);
  } else if (mode == "demo") {
    runDemo(object, pickup, dropoff);
  } else {
    ROS_FATAL("Unknown ~mode '%s' (expected 'demo' or 'home_service')", mode.c_str());
    return 1;
  }
  return 0;
}
