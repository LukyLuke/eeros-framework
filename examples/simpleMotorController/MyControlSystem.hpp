#ifndef MYCONTROLSYSTEM_HPP_
#define MYCONTROLSYSTEM_HPP_

#include <eeros/core/Runnable.hpp>
#include <eeros/control/Sum.hpp>
#include <eeros/control/D.hpp>
#include <eeros/control/Gain.hpp>
#include <eeros/control/Switch.hpp>
#include <eeros/control/Constant.hpp>
#include <eeros/control/RealPeripheralInput.hpp>
#include <eeros/control/RealPeripheralOutput.hpp>
#include <eeros/control/Mux.hpp>
#include <eeros/control/DeMux.hpp>
#include <eeros/core/Executor.hpp>

class MyControlSystem : public eeros::Runnable {

public:
	void run();
	
	static MyControlSystem& instance();
	
	void start();
	void stop();
	
	eeros::control::Constant setpoint;
	eeros::control::RealPeripheralInput enc;
	eeros::control::D diff1;
	eeros::control::Sum sum1;
	eeros::control::Gain posController;
	eeros::control::D diff2;
	eeros::control::Sum sum2;
	eeros::control::Gain speedController;
	eeros::control::Gain inertia;
	eeros::control::Gain invMotConst;
	eeros::control::RealPeripheralOutput dac;
	
private:
	MyControlSystem();
	MyControlSystem(const MyControlSystem&);
	MyControlSystem& operator=(const MyControlSystem&);
	
	eeros::Executor executor;
};

#endif // MYCONTROLSYSTEM_HPP_