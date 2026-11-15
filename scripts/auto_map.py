#!/usr/bin/env python
"""Drive the turtlebot through both rooms with move_base while gmapping builds the map.

Run alongside turtlebot_world.launch, gmapping_demo.launch and move_base, then save
with: rosrun map_server map_saver -f ~/catkin_ws/src/map/map
"""
import math
import rospy
import actionlib
from geometry_msgs.msg import Twist
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
from tf.transformations import quaternion_from_euler

# (x, y, spin): spin -> turn 360 deg on arrival so the kinect sweeps the room.
WAYPOINTS = [
    (-1.5, -1.2, True), (-3.2, -2.3, True), (-3.3, -0.9, False), (-3.3, 1.0, True),
    (-3.0, 2.4, True), (-0.8, 1.0, True), (0.3, 0.2, False),
    (1.5, -0.6, True), (3.2, -2.3, True), (3.3, -0.6, False), (2.8, 1.3, True),
    (1.0, 2.3, True), (1.0, -2.3, True), (0.0, 0.0, True),
]


def spin(pub):
    cmd = Twist()
    cmd.angular.z = 0.6
    end = rospy.Time.now() + rospy.Duration(2 * math.pi / cmd.angular.z + 1.0)
    rate = rospy.Rate(10)
    while not rospy.is_shutdown() and rospy.Time.now() < end:
        pub.publish(cmd)
        rate.sleep()
    pub.publish(Twist())


def main():
    rospy.init_node("auto_map")
    client = actionlib.SimpleActionClient("move_base", MoveBaseAction)
    client.wait_for_server()
    pub = rospy.Publisher("/cmd_vel_mux/input/navi", Twist, queue_size=1)
    for i, (x, y, do_spin) in enumerate(WAYPOINTS):
        goal = MoveBaseGoal()
        goal.target_pose.header.frame_id = "map"
        goal.target_pose.header.stamp = rospy.Time.now()
        goal.target_pose.pose.position.x = x
        goal.target_pose.pose.position.y = y
        q = quaternion_from_euler(0, 0, math.atan2(y, x) if (x or y) else 0.0)
        goal.target_pose.pose.orientation.z, goal.target_pose.pose.orientation.w = q[2], q[3]
        client.send_goal(goal)
        ok = client.wait_for_result(rospy.Duration(90))
        state = client.get_state()
        if not ok:
            client.cancel_goal()
        rospy.loginfo("waypoint %d (%.1f, %.1f): %s", i, x, y,
                      "reached" if state == actionlib.GoalStatus.SUCCEEDED else "skipped (state %d)" % state)
        if do_spin:
            spin(pub)
    rospy.loginfo("tour done")


if __name__ == "__main__":
    main()
