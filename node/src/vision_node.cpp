#include "pacman_vision/vision_node.h"

VisionNode::VisionNode()
{
  this->nh = ros::NodeHandle("pacman_vision");
  this->scene.reset(new PC);
  this->scene_processed.reset(new PC);
  //first call of dynamic reconfigure callback will only set gui to loaded parameters
  rqt_init = true;
  //service callback init
  srv_get_scene = nh.advertiseService("get_scene_processed", &VisionNode::cb_get_scene, this);
  //subscribe to depth_registered pointclouds topic
  std::string topic =nh.resolveName("/camera/depth_registered/points");
  sub_openni = nh.subscribe(topic, 3, &VisionNode::cb_openni, this);
  pub_scene = nh.advertise<PC> ("scene_processed", 3);

  //init filter params 
  nh.param<bool>("downsampling", downsample, false);
  nh.param<bool>("enable_estimator", en_estimator, false);
  nh.param<bool>("enable_tracker", en_tracker, false);
  nh.param<bool>("enable_broadcaster", en_broadcaster, false);
  nh.param<bool>("passthrough", filter, true);
  nh.param<bool>("plane_segmentation", plane, false);
  nh.param<bool>("keep_organized", keep_organized, false);
  nh.param<double>("leaf_size", leaf, 0.01);
  nh.param<double>("plane_tolerance", plane_tol, 0.004);
  nh.param<double>("pass_xmax", xmax, 0.5);
  nh.param<double>("pass_xmin", xmin, -0.5);
  nh.param<double>("pass_ymax", ymax, 0.5);
  nh.param<double>("pass_ymin", ymin, -0.5);
  nh.param<double>("pass_zmax", zmax, 1.0);
  nh.param<double>("pass_zmin", zmin, 0.3);
  //set callback for dynamic reconfigure
  this->dyn_srv.setCallback(boost::bind(&VisionNode::cb_reconfigure, this, _1, _2));
}

bool VisionNode::cb_get_scene(pacman_vision_comm::get_scene::Request& req, pacman_vision_comm::get_scene::Response& res)
{
  sensor_msgs::PointCloud2 msg;
  if (req.save.compare("false") != 0)
  {
    std::string home = std::getenv("HOME");
    mtx_scene.lock();
    pcl::io::savePCDFile( (home + "/" + req.save + ".pcd").c_str(), *scene_processed);
    mtx_scene.unlock();
    ROS_INFO("[PaCMaN Vision][%s] Processed scene saved to %s",__func__, (home + "/" + req.save + ".pcd").c_str() );
  }
  mtx_scene.lock();
  pcl::toROSMsg(*scene_processed, msg);
  mtx_scene.unlock();
  res.scene = msg; 
  ROS_INFO("[PaCMaN Vision][%s] Sent processed scene to service response.", __func__);
  return true;
}

void VisionNode::cb_openni(const sensor_msgs::PointCloud2::ConstPtr& message)
{
  mtx_scene.lock();
  pcl::fromROSMsg (*message, *(this->scene));
  PC::Ptr tmp (new PC);
  //filters
  //check if we need to filter scene
  if (filter)
  {
    PassThrough<PT> pass;
    PC::Ptr t (new PC);
    //check if we need to maintain scene organized
    if (keep_organized)
      pass.setKeepOrganized(true);
    else
      pass.setKeepOrganized(false);
    pass.setInputCloud (this->scene);
    pass.setFilterFieldName ("z");
    pass.setFilterLimits (zmin, zmax);
    pass.filter (*tmp);
    pass.setInputCloud (tmp);
    pass.setFilterFieldName ("y");
    pass.setFilterLimits (ymin, ymax);
    pass.filter (*t);
    pass.setInputCloud (t);
    pass.setFilterFieldName ("x");
    pass.setFilterLimits (xmin, xmax);
    pass.filter (*(this->scene_processed));
  }
  if (plane)
  {
    pcl::SACSegmentation<PT> seg;
    pcl::ExtractIndices<PT> extract;
    //coefficients
    pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
    pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
    //plane segmentation
    if (filter)
      seg.setInputCloud(this->scene_processed);
    else
      seg.setInputCloud(this->scene);
    seg.setOptimizeCoefficients (true);
    seg.setModelType (pcl::SACMODEL_PLANE);
    seg.setMethodType (pcl::SAC_RANSAC);
    seg.setMaxIterations (100);
    seg.setDistanceThreshold (this->plane_tol);
    seg.segment(*inliers, *coefficients);
    //extract what's on top of plane
    if (filter)
      extract.setInputCloud(this->scene_processed);
    else
      extract.setInputCloud(this->scene);
    extract.setIndices(inliers);
    extract.setNegative(true); 
    extract.filter(*tmp);
    pcl::copyPointCloud(*tmp, *(this->scene_processed));
  }
  //check if we need to downsample scene
  if (downsample) //cannot keep organized cloud after voxelgrid
  {
    VoxelGrid<PT> vg;
    vg.setLeafSize(leaf, leaf, leaf);
    vg.setDownsampleAllData(true);
    if (filter || plane)
      vg.setInputCloud (this->scene_processed);
    else
      vg.setInputCloud (this->scene);
    vg.filter (*tmp);
    pcl::copyPointCloud(*tmp, *(this->scene_processed));
  }
  //republish processed cloud
  if (filter || downsample || plane)
    pub_scene.publish(*scene_processed);
  else
    pub_scene.publish(*scene);
  mtx_scene.unlock();
}

void VisionNode::check_modules()
{
  //check if we want estimator module and it is not started, or if it is started but we want it disabled
  if (this->en_estimator && !this->estimator_module)
  {
    ROS_WARN("[PaCMaN Vision] Started Estimator module");
    this->estimator_module.reset( new Estimator(this->nh) );
    //spawn a thread to handle the module spinning
    estimator_driver = boost::thread(&VisionNode::spin_estimator, this);
  }
  else if (!this->en_estimator && this->estimator_module)
  {
    ROS_WARN("[PaCMaN Vision] Stopped Estimator module");
    //wait for the thread to stop, if not already, if already stopped this results in a no_op
    estimator_driver.join();
    //kill the module
    this->estimator_module.reset();
  }
  
  //check if we want broadcaster module and it is not started, or if it is started but we want it disabled
  if (this->en_broadcaster && !this->broadcaster_module)
  {
    ROS_WARN("[PaCMaN Vision] Started Broadcaster module");
    this->broadcaster_module.reset( new Broadcaster(this->nh) );
    //spawn a thread to handle the module spinning
    broadcaster_driver = boost::thread(&VisionNode::spin_broadcaster, this);
  }
  else if (!this->en_broadcaster && this->broadcaster_module)
  {
    ROS_WARN("[PaCMaN Vision] Stopped Broadcaster module");
    //wait for the thread to stop, if not already, if already stopped this results in a no_op
    broadcaster_driver.join();
    //kill the module
    this->broadcaster_module.reset();
  }
  
  //check if we want tracker module and it is not started, or if it is started but we want it disabled
  if (this->en_tracker && !this->tracker_module)
  {
    ROS_WARN("[PaCMaN Vision] Started Tracker module");
    this->tracker_module.reset( new Tracker(this->nh) );
    //spawn a thread to handle the module spinning
    tracker_driver = boost::thread(&VisionNode::spin_tracker, this);
  }
  else if (!this->en_tracker && this->tracker_module)
  {
    ROS_WARN("[PaCMaN Vision] Stopped Tracker module");
    //wait for the thread to stop, if not already, if already stopped this results in a no_op
    tracker_driver.join();
    //kill the module
    this->tracker_module.reset();
  }
}
///////Spinner threads//////////
////////////////////////////////
void VisionNode::spin_estimator()
{
  //spin until we disable it or it dies somehow
  while (this->en_estimator && this->estimator_module)
  {
    //push down filtered cloud to estimator
    //lock variables so no-one else can touch what is copied
    mtx_scene.lock();
    mtx_estimator.lock();
    if(this->filter || this->downsample || this->plane)
    {
      copyPointCloud(*(this->scene_processed), *(this->estimator_module->scene));
      if (this->downsample)
        this->estimator_module->downsampling = 0; //downsampling already done here
      if (this->plane)
        this->estimator_module->no_segment = true;
    }
    else
    {
      copyPointCloud(*scene, *(this->estimator_module->scene));
      this->estimator_module->downsampling = 1;
      this->estimator_module->no_segment = false;
    }
    //release lock
    mtx_estimator.unlock();
    mtx_scene.unlock();
    this->estimator_module->spin_once();
    boost::this_thread::sleep(boost::posix_time::milliseconds(50)); //estimator could try to go at 20Hz (no need to process those services faster)
  }
  //estimator got stopped
  return;
}

void VisionNode::spin_broadcaster()
{
  //spin until we disable it or it dies somehow
  while (this->en_broadcaster && this->broadcaster_module)
  {
    //check if we have an estimator and or a tracker 
    if (this->en_estimator && this->estimator_module)
    { //we have an estimator module running
      //check if it is not busy computing and it has some new estimations to upload
      if (!this->estimator_module->busy && this->estimator_module->up_broadcaster)
      {
        mtx_estimator.lock();
        mtx_broadcaster.lock();
        broadcaster_module->estimated.clear();
        broadcaster_module->names.clear();
        broadcaster_module->ids.clear();
        boost::copy(estimator_module->estimations, back_inserter(broadcaster_module->estimated));
        boost::copy(estimator_module->names, back_inserter(broadcaster_module->names));
        boost::copy(estimator_module->ids, back_inserter(broadcaster_module->ids));
        mtx_broadcaster.unlock();
        estimator_module->up_broadcaster = false;
        mtx_estimator.unlock();
        //process new data
        broadcaster_module->compute_transforms();
      }
    }
    //do the broadcasting
    this->broadcaster_module->broadcast_once();
    //spin
    this->broadcaster_module->spin_once();
    boost::this_thread::sleep(boost::posix_time::milliseconds(50)); //broadcaster could try to go at 20Hz
  }
  //broadcaster got stopped
  return;
}

void VisionNode::spin_tracker()
{
  //spin until we disable it or it dies somehow
  while (this->en_tracker && this->tracker_module)
  {
    //push down filtered cloud to tracker (never pass downsampled one, cause tracker has its own downsampling)
    //lock variables so no-one else can touch what is copied
    mtx_scene.lock();
    mtx_tracker.lock();
    if(filter || downsample || plane)
    {
     copyPointCloud(*(this->scene_processed), *(this->tracker_module->scene));
     if (downsample)
     {
       this->tracker_module->downsample = false;
       this->tracker_module->leaf = this->leaf;
     }
     else
       this->tracker_module->downsample = true;
    }
    else
      copyPointCloud(*(this->scene), *(this->tracker_module->scene));
    //release lock
    mtx_tracker.unlock();
    mtx_scene.unlock();
    //check if we have an estimator running
    if (this->en_estimator && this->estimator_module)
    { //we have an estimator module running
      //check if it is not busy computing and it has some new estimations to upload
      if (!this->estimator_module->busy && this->estimator_module->up_tracker)
      {
        if (!tracker_module->started)
        { //we copy only if tracker is not started already
          mtx_estimator.lock();
          mtx_tracker.lock();
          tracker_module->estimations.clear();
          tracker_module->names.clear();
          boost::copy(estimator_module->estimations, back_inserter(tracker_module->estimations));
          boost::copy(estimator_module->names, back_inserter(tracker_module->names));
          mtx_tracker.unlock();
          mtx_estimator.unlock();
        }
        estimator_module->up_tracker=false;
      }
    }
    //spin
    //TODO tmp timer
    if (this->tracker_module->started)
    {
      mtx_tracker.lock();
      boost::timer t;
      this->tracker_module->track();
      cout<<"Tracker Step: "<<t.elapsed()<<std::endl;
      this->tracker_module->broadcast_tracked_object();
      mtx_tracker.unlock();
    }
    this->tracker_module->spin_once();
    boost::this_thread::sleep(boost::posix_time::milliseconds(10)); //tracker could try to go at 100Hz
  }
  //tracker got stopped
  return;
}

//dynamic reconfigure callback
void VisionNode::cb_reconfigure(pacman_vision::pacman_visionConfig &config, uint32_t level)
{
  //initial gui init based on params (just do it once)
  if (rqt_init)
  {
    config.enable_estimator = en_estimator;
    config.enable_broadcaster = en_broadcaster;
    config.enable_tracker= en_tracker;
    config.downsampling = downsample;
    config.passthrough = filter;
    config.plane_segmentation = plane;
    config.plane_tolerance = plane_tol;
    config.keep_organized = keep_organized;
    config.leaf_size = leaf;
    config.pass_xmax = xmax;
    config.pass_xmin = xmin;
    config.pass_ymax = ymax;
    config.pass_ymin = ymin;
    config.pass_zmax = zmax;
    config.pass_zmin = zmin;
    nh.getParam("estimator_clus_tol", config.estimator_clus_tol);
    nh.getParam("estimator_iterations", config.estimator_iterations);
    nh.getParam("estimator_neighbors", config.estimator_neighbors);
    nh.getParam("estimator_object_calibration", config.estimator_object_calibration);
    nh.getParam("broadcaster_tf", config.broadcaster_tf);
    nh.getParam("broadcaster_rviz_markers", config.broadcaster_rviz_markers);
    nh.getParam("tracker_bounding_factor", config.tracker_bounding_factor);
    nh.getParam("tracker_leaf_size", config.tracker_leaf_size);
    nh.getParam("tracker_type", config.tracker_type);
    this->rqt_init = false;
    ROS_WARN("[PaCMaN Vision] Rqt-Reconfigure Default Values Initialized");
  }
  this->en_estimator = config.enable_estimator;
  this->en_tracker = config.enable_tracker;
  this->en_broadcaster = config.enable_broadcaster;
  this->filter = config.passthrough;
  this->plane = config.plane_segmentation;
  this->plane_tol = config.plane_tolerance;
  this->downsample = config.downsampling;
  this->keep_organized = config.keep_organized;
  this->xmin = config.pass_xmin;
  this->xmax = config.pass_xmax;
  this->ymin = config.pass_ymin;
  this->ymax = config.pass_ymax;
  this->zmin = config.pass_zmin;
  this->zmax = config.pass_zmax;
  this->leaf = config.leaf_size;
  if (this->estimator_module && this->en_estimator)
  {
    this->estimator_module->calibration = config.estimator_object_calibration;
    this->estimator_module->iterations = config.estimator_iterations;
    this->estimator_module->pe.setParam("progItera",estimator_module->iterations);
    this->estimator_module->neighbors = config.estimator_neighbors;
    this->estimator_module->pe.setParam("kNeighbors",estimator_module->neighbors);
    this->estimator_module->clus_tol = static_cast<double>(config.estimator_clus_tol);
  }
  if (this->broadcaster_module && this->en_broadcaster)
  {
    this->broadcaster_module->tf = config.broadcaster_tf;
    this->broadcaster_module->rviz_markers = config.broadcaster_rviz_markers;
  }
  if (this->tracker_module && this->en_tracker)
  {
    this->tracker_module->factor = config.tracker_bounding_factor;
    this->tracker_module->leaf = config.tracker_leaf_size;
    this->tracker_module->type = config.tracker_type;
  }
  ROS_INFO("[PaCMaN Vision] Reconfigure request accepted");
}

void VisionNode::spin_once()
{
    ros::spinOnce();
    this->check_modules();
}

int main (int argc, char *argv[])
{
  ros::init(argc, argv, "pacman_vision");
  VisionNode node;
  ros::Rate rate(30); //try to go at 30hz
  while (node.nh.ok())
  {
    node.spin_once(); 
    rate.sleep();
  }
  return 0;
}
