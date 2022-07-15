#pragma once
#include <vector>
#include "Object.h"
#include "celestial/Star.h"
#include "celestial/Planet.h"
#include "Sattelite.h"
#include "Element.h"
#include "utils/utils_parse.h"
#include <shared_mutex>
#include "KinematicEngine.h"
#include <Windows.h>

#include "../commons/ipc.h"




class SolarSystem {
public:
	std::unordered_map<std::string, int> starMap;
	std::unordered_map<std::string, int> planetMap;
	std::unordered_map<std::string, int> moonMap;

	std::vector<int> iBuffer_star;
	std::vector<int> iBuffer_planet;
	std::vector<int> iBuffer_moon;
	std::vector<int> fBuffer_star;
	std::vector<int> fBuffer_planet;


	std::vector<Element*> elements;

	SolarSystem();

	Object* getObjectFromName(ObjectTypes type, std::string name);
	Element* getElementFromName(ObjectTypes type, std::string name);
	void engineStart();
	Eigen::Vector3d getVectorBetweenObjects(Element* o1, Element* o2);
	void mapSystem();

	static long update_freq;
	bool toggle_kinematic = false;
	std::condition_variable_any kinematic_cv;
	std::shared_mutex kinematic_m;

private:
	std::vector<Element*> loadStars();
	std::vector<Element*> loadPlanets();
	std::vector<Element*> loadMoons();
	std::vector<Element*> loadSattelites();
	
	// Kinematic Engine Process Vars
	KinematicEngine* ke;
	HANDLE StdIN_R;
	HANDLE StdIN_W;
	HANDLE StdOUT_R;
	HANDLE StdOUT_W;

	void engineRequest(P_PACKET);

	//void initializePlanetaryOrbit(int num);
	void initializeMechanics(int, int num, ObjectTypes type);
	void start();
	//void initializeOrbit(OrbitInit init, Orbit* orbit, Eigen::Vector3d* Position, Eigen::Vector3d* Velocity);
};

