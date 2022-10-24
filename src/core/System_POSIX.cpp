#include <eeros/core/System.hpp>
#include <eeros/core/Fault.hpp>
#include <time.h>

#define NS_PER_SEC 1000000000
#define CLOCK CLOCK_MONOTONIC_RAW

using namespace eeros;

uint64_t timespec2nsec(struct timespec ts) {
	return static_cast<uint64_t>(ts.tv_sec) * NS_PER_SEC + static_cast<uint64_t>(ts.tv_nsec);
}

double timespec2sec(struct timespec ts) {
	return static_cast<double>(timespec2nsec(ts)) / NS_PER_SEC;
}

double System::getClockResolution() {
	struct timespec ts;
	if(clock_getres(CLOCK, &ts) != 0) {
		throw Fault("Failed to get clock resolution!");
	}
	return timespec2sec(ts);
}

double System::getTime() {
	return static_cast<double>(System::getTimeNs()) / NS_PER_SEC;
}

#ifdef USE_ROS
void System::useRosTime() {
	rosTimeIsUsed = true;
}
#endif

uint64_t System::getTimeNs() {
#ifdef USE_ROS
	if (rosTimeIsUsed) {
		return eeros::System::getTimeNs();
	}
#endif

	struct timespec ts;
	if(clock_gettime(CLOCK, &ts) != 0) {
		throw Fault("Failed to get time!");
	}
	return timespec2nsec(ts);
}


