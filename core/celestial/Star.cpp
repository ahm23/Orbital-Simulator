#include "Star.h"

Star::Star(StarConfigPackage config) : CelestialBody{ config.config_obj, config.config_clst } {
	this->config = config.config_star;
	luminosity = 4*PI*pow(config.config_clst.avg_radius,2)*stefan_boltzmann*pow(config.config_star.temp_surface,4);
}
