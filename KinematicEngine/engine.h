#include <vector>
#include <condition_variable>
#include <shared_mutex>
#include <Eigen/Dense>
#include <chrono>

#include "../commons/ipc.h"
#include "../commons/constants.h"

struct object {
	int id;
	bool astronomical;
	double mass;
	uint64_t time;
	Eigen::Vector3d p;
	Eigen::Vector3d v;
};

struct object_sm {
	unsigned int id;
	double mass;
	uint64_t time;
	Eigen::Vector3d p;
	Eigen::Vector3d v;
};

class ENGINE {
public:
	ENGINE(int, std::mutex*, std::condition_variable_any*, int*);
	~ENGINE();

	int addObject(int, B_INIT);

private:
	int maxThreads = 0;
	std::vector<std::thread> threads;
	
	const int time_increment = 1000;

	void ComputeWorker(int, std::mutex* m, std::condition_variable_any* cv, int*, bool);
	
	void addToBuffer(std::mutex*, object_sm*, bool);

	std::atomic<int> working = 0;
	bool catchup = true;
	unsigned int buffer_capacity = 86400;

	std::vector<object> sys_objects;
	std::vector<object> objects;
	std::vector<object> queue;
	std::vector<std::vector<object>> buffer_sys_objects;
	std::vector<std::vector<object>> buffer_objects;

	int b_count = -1;


	// Engine toggle for pause/resume.
	bool toggle = false;
	bool sig_terminate = false;
	int maxTh;
};

static double calculate_mu(double mass, double ref_mass) {
	return GRAV_KM * (mass + ref_mass);
}