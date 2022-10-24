#include "RosExample.hpp"

#include <iostream>
#include <signal.h>

#include <eeros/hal/HAL.hpp>
#include <eeros/logger/Logger.hpp>
#include <eeros/logger/StreamLogWriter.hpp>

using namespace eeros;
using namespace eeros::logger;
using namespace eeros::hal;


void signalHandler(int signum) {
  Executor::stop();
}

// This callback function is only needed, if you want to sync the executor with a gazebo simulation
// void callback(const sensor_msgs::JointState::Type){
// 	std::cout << "callback" << std::endl;
// };

int main(int argc, char **argv) {	
  double dt = 0.2;
  
  Logger::setDefaultStreamLogger(std::cout);
  Logger log = Logger::getLogger('M');
  log.info() << "ROS example started";

  HAL& hal = HAL::instance();
  hal.readConfigFromFile(&argc, argv);

  char* dummy_args[] = {NULL};
  int dummy_argc = sizeof(dummy_args)/sizeof(dummy_args[0]) - 1;
  rclcpp::init(dummy_argc, dummy_args);
  auto node = rclcpp::Node::make_shared("rosExample");
  log.trace() << "ROS node initialized.";
  
  System::useRosTime();	// "builtin_interfaces::msg::Time::now()" is used to get system time
  
  // TODO: Rewrite this for ROS-2
  // This part only needed, if you want to sync the executor with a gazebo simulation
// 	ros::CallbackQueue syncCallbackQueue;
// 	syncNodeHandler.setCallbackQueue(&syncCallbackQueue);
// 	auto subscriberSync = syncNodeHandler.subscribe("motor_sim/joint_states", 1, &callback);
    
  MyControlSystem controlSystem(dt);
  MySafetyProperties safetyProperties(controlSystem);
  SafetySystem safetySystem(safetyProperties, dt);
  
  signal(SIGINT, signalHandler);	
  auto& executor = Executor::instance();
  executor.setMainTask(safetySystem);
// 	executor.syncWithRosTopic(&syncCallbackQueue);	// sync with gazebo simulation
  executor.run();

  log.info() << "ROS example end";	
  return 0;
}
