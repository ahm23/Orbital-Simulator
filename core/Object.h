#pragma once
#include <iostream>

#include <math.h>
#include <Eigen/Dense>

#include <thread>
#include <chrono>

#include "mechanics/Orbit.h"

enum ObjectTypes {
	BARYCENTRE = 0,
	STAR = 1,
	PLANET = 2,
	MOON = 3,
	ASTEROID = 4,
	COMET = 5,
	SATTELITE = 6
};

static const char* filenames[] = { "stars.json", "planets.json", "moons.json" };

struct ObjectConfig {
	int id;
	std::string name;
	std::string name_display = "N/A";
	ObjectTypes type;
	double mass;
	double mu;
};

class Object {

public:
	std::condition_variable kinematic_cv;
	std::mutex kinematic_m;

	Orbit orbit;
	//Kinematic kinematics;
	
	Object(ObjectConfig config);
	~Object();

	std::string getName() { return config_obj.name; }
	double getMass() { return config_obj.mass; }
	ObjectTypes getType() { return config_obj.type;  }
	int getID() { return config_obj.id; }

	void setMu(double mu) {
		//tmp function
		config_obj.mu = mu;
	}
	//void setKinematicAnchor(Object* anchor) { kinematics.anchor = anchor; }

	Eigen::Vector3d getPos() { return p; }
	Eigen::Vector3d getVel() { return v; }
	Eigen::Vector3d getAcl() { return a; }
	void setPos(Eigen::Vector3d p) { this->p = p; }
	void setVel(Eigen::Vector3d v) { this->v = v; }
	void setAcl(Eigen::Vector3d a) { this->a = a; }

	//void initKinematicProcess(Eigen::Vector3d position, Eigen::Vector3d velocity);
	long time = (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

protected:
	Eigen::Vector3d p;
	Eigen::Vector3d v;
	Eigen::Vector3d a;
	
	Eigen::Matrix3d ROTATION;

	ObjectConfig config_obj;
	std::unordered_map<int, std::string> depthMap = { {0, "Barycentre"} };

	//void kinematicProcess();
};

