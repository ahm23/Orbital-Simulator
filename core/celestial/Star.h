#pragma once
#include "../../commons/constants.h"
#include "CelestialBody.h"

struct StarConfig {
	double temp_surface;
};

struct StarConfigPackage {
	ObjectConfig config_obj;
	CelestialConfig config_clst;
	StarConfig config_star;
};

class Star : public CelestialBody {
public:
	Star(StarConfigPackage config);
private:
	StarConfig config;
	double luminosity;
};