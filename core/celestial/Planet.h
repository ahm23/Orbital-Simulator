#pragma once
#include <chrono>
#include <math.h>
#include <Eigen/Dense>
#include "../mechanics/Orbit.h"
#include "CelestialBody.h"
#include <condition_variable>

using namespace Eigen;

struct PlanetConfig {
	
};

struct PlanetConfigPackage {
	ObjectConfig config_obj;
	CelestialConfig config_clst;
	PlanetConfig config_planet;
};

class Planet: public CelestialBody {
public:
	Planet(PlanetConfigPackage config);
	~Planet();

	//Planet& operator=(const Planet& rhs) {};

private:
	PlanetConfig config_planet;
};