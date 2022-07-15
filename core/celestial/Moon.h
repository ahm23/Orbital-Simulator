#pragma once
#include <chrono>
#include <math.h>
#include <Eigen/Dense>
#include "../mechanics/Orbit.h"
#include "CelestialBody.h"
#include <condition_variable>

using namespace Eigen;

struct MoonConfig {

};

struct MoonConfigPackage {
	ObjectConfig config_obj;
	CelestialConfig config_clst;
	MoonConfig config_moon;
};

class Moon: public CelestialBody {
public:
	Moon(MoonConfigPackage config);
	~Moon();
private:
	MoonConfig config_moon;
};