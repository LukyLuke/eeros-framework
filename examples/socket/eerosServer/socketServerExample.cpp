#include <eeros/control/TimeDomain.hpp> 
#include <eeros/control/Constant.hpp>
#include <eeros/control/SocketData.hpp>
#include <eeros/math/Matrix.hpp>
#include <eeros/core/Executor.hpp>
#include <eeros/task/Lambda.hpp>
#include <eeros/safety/SafetySystem.hpp>
#include <eeros/safety/SafetyProperties.hpp>
#include <eeros/logger/Logger.hpp>
#include <eeros/logger/StreamLogWriter.hpp>
#include <eeros/task/Periodic.hpp>

using namespace eeros::control;
using namespace eeros::sockets;
using namespace eeros::logger;
using namespace eeros::math;
using namespace eeros::task;
using namespace eeros;

class TestAppCS {
public:
	TestAppCS(double dt) : 
		dt(dt),
		log('C'),
		socketA(9876, 0.01),
		c1({1.5, 2.2, 3.3, 4.6}),
		c2(56.5),
		c3(-28),
		timedomain("Main time domain", dt, true) {
		
		socketA.getOut().getSignal().setName("socketRead");
		socketA.getIn().connect(c1.getOut());
// 		socketA.getIn().connect(c2.getOut());
// 		socketA.getIn().connect(c3.getOut());
		timedomain.addBlock(c1);
		timedomain.addBlock(c2);
		timedomain.addBlock(c3);
		timedomain.addBlock(socketA);
		
		Executor::instance().add(timedomain);
		
	}
		
	// Define blocks
	Constant<Vector4> c1;
	Constant<double> c2;
	Constant<int> c3;
	SocketData<Vector4, Matrix<6,1,double>> socketA;	// send Vector4, receive Matrix<6,1,double>
// 	SocketData<Vector4, double> socketA;			// send Vector4, receive double
// 	SocketData<Vector4, int> socketA;			// send Vector4, receive int
// 	SocketData<Vector4, std::nullptr_t> socketA;		// send Vector4, receive nothing
// 	SocketData<double, Vector4> socketA;			// send double, receive Vector4
// 	SocketData<int, Vector4> socketA;			// send int, receive Vector4
// 	SocketData<std::nullptr_t, Vector4> socketA;		// send nothing, receive Vector4
	Logger log;
		
protected:
	double dt;
	bool realtime;
	TimeDomain timedomain;
};
 
class TestAppSafetyProperties : public eeros::safety::SafetyProperties {
public: 
	TestAppSafetyProperties() : off("System off") {
		addLevel(off);
		setEntryLevel(off);
	}
	~TestAppSafetyProperties(){};
	eeros::safety::SafetyLevel off;
}; 

int main(int argc, char **argv) {
	double dt = 0.01;
	
	StreamLogWriter w(std::cout);
	w.show(LogLevel::TRACE);
	Logger::setDefaultWriter(&w);
	Logger log;
 
	log.info() << "EEROS started";
	
	// Control System
	TestAppCS controlSystem (dt);
	
	// Safety System
	TestAppSafetyProperties safetyProperties;
	SafetySystem safetySystem(safetyProperties, dt);

	Lambda l1;
	Periodic periodic("per1", 1.0, l1);
	periodic.monitors.push_back([&](PeriodicCounter &pc, Logger &log){
		log.info() << controlSystem.socketA.getOut().getSignal();
	});
		
	// Create and run executor
	auto& executor = Executor::instance();
	executor.setMainTask(safetySystem);
	executor.add(periodic);
	
// 	// Sequencer
// 	eeros::sequencer::Sequencer S;
// 	MainSequence mainSequence(S, &controlSystem, "mainSequence");
// 	S.addMainSequence(&mainSequence);
	
	log.info() << "executor.run()";
	executor.run();
	
	return 0;
}
