#ifndef _INCL_TRACKER
#define _INCL_TRACKER
// ROS headers
#include <ros/ros.h>
#include <ros/console.h>
#include <ros/package.h>
#include <ros/callback_queue.h>
#include <ros/callback_queue_interface.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf/transform_broadcaster.h>
#include <visualization_msgs/Marker.h>
//PCL
#include <pcl/common/centroid.h>
#include <pcl/common/eigen.h>
#include <pcl/common/common.h>
#include <pcl/registration/correspondence_rejection_distance.h>
#include <pcl/registration/correspondence_estimation.h>
#include <pcl/registration/correspondence_rejection_one_to_one.h>
#include <pcl/registration/correspondence_rejection_trimmed.h>
#include <pcl/registration/transformation_estimation_dual_quaternion.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/search/kdtree.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/sample_consensus/ransac.h>
#include <pcl/sample_consensus/sac_model_plane.h>
#include <pcl/segmentation/sac_segmentation.h>
// ROS generated headers
#include "pacman_vision_comm/track_object.h"
#include "pacman_vision_comm/stop_track.h"
#include "pacman_vision_comm/grasp_verification.h"
//general utilities
#include <cmath>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <Eigen/Dense>
#include <string>
#include <stdlib.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "pacman_vision/storage.h"

class VisionNode;

class Tracker
{
  friend class VisionNode;

  public:
    Tracker(ros::NodeHandle &n, boost::shared_ptr<Storage> &stor);
    ~Tracker();
  private:
    ros::NodeHandle nh;
    boost::shared_ptr<ros::CallbackQueue> queue_ptr;
    boost::shared_ptr<Storage> storage;
    //Service Server
    ros::ServiceServer srv_track_object;
    //service server to comm with dual manip
    ros::ServiceServer srv_grasp;
    //service server to stop tracker
    ros::ServiceServer srv_stop;

    //tracker transforms
    boost::shared_ptr<Eigen::Matrix4f> transform;
    Eigen::Matrix4f T_rotx, T_rotz, T_roty;

    //name and id of object to be tracked
    std::string name;
    std::string id;
    //index of tracked object (referred to storage estimations)
    int index;

    //actual scene
    PXC::Ptr scene;
    //actual model
    PXC::Ptr model;
    //loaded model
    PXC::Ptr orig_model;
    //model centroid
    PX model_centroid;

    //config//
    bool started, lost_it, manual_disturbance ;
    //factor to bounding box dimensions
    float factor;
    //leaf size to downsample model cloud
    float leaf, old_leaf;
    //counters for errors
    int error_count, disturbance_counter, centroid_counter, disturbance_done;
    //tracker transform estimation type
    // unused so far (TODO possible improvements)
    int type;
    //boundingbox of object computed from model
    float x1,x2,y1,y2,z1,z2;
    //distance threshold for rejector
    float rej_distance;
    // ICP fitness
    double fitness;

    //icp object
    pcl::IterativeClosestPoint<PX, PX,float> icp;
    //correspondences
    pcl::registration::CorrespondenceEstimation<PX, PX, float>::Ptr ce;
    pcl::registration::CorrespondenceRejectorDistance::Ptr crd;
    pcl::registration::CorrespondenceRejectorTrimmed::Ptr crt;
    pcl::registration::CorrespondenceRejectorOneToOne::Ptr cro2o;
    pcl::registration::TransformationEstimationDualQuaternion<PX,PX,float>::Ptr teDQ;
//  pcl::registration::CorrespondenceRejectorSampleConsensus< PX >::Ptr crsc;

    //filters
    pcl::PassThrough<PX> pass;
    pcl::VoxelGrid<PX> vg;

    //rviz marker
    visualization_msgs::Marker marker;
    //tf and marker broadcaster
    tf::TransformBroadcaster tf_broadcaster;
    ros::Publisher rviz_marker_pub;

    //track_object service callback
    bool cb_track_object(pacman_vision_comm::track_object::Request& req, pacman_vision_comm::track_object::Response& res);

    //stop tracker service callback
    bool cb_stop_tracker(pacman_vision_comm::stop_track::Request& req, pacman_vision_comm::stop_track::Response& res);

    //grasp_verification service callback
    bool cb_grasp(pacman_vision_comm::grasp_verification::Request& req, pacman_vision_comm::grasp_verification::Response& res);

    //method to publish transform and marker of tracked object
    void broadcast_tracked_object();

    //tracker methods
    void track();
    void find_object_in_scene();

    //custom spin method
    void spin_once();
};
#endif
