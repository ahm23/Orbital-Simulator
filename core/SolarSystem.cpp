#include "SolarSystem.h"

long SolarSystem::update_freq = 1000;

SolarSystem::SolarSystem()
{
	starList = loadStars();
    planetList = loadPlanets();


    for (int i = 0; i < starList.size(); i++) {
        //initializePlanetaryOrbit(i + 1);
        initializeMechanics(i + 1, STAR);
    }

    for (int i = 0; i < planetList.size(); i++) {
        //initializePlanetaryOrbit(i + 1);
        initializeMechanics(i + 1, PLANET);
    }

    mapSystem();

    std::lock_guard<shared_mutex> lk(kinematic_m);
    toggle_kinematic = true;
    kinematic_cv.notify_all();
}

std::vector<Element*> SolarSystem::loadStars() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\core\\Stars.dat");
    std::vector<Element*> starList;
    parser.parseObjects<Element, Star>(&starList);
    for (int i = 0; i < starList.size(); i++) {
        starList[i]->kinematic = new Kinematics(this, starList[i]->obj);
        starMap.push_back(starList[i]->obj->getName());
    }
    return starList;
}

std::vector<Element*> SolarSystem::loadPlanets() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\core\\Planets.dat");
    std::vector<Element*> planetList;
    parser.parseObjects<Element, Planet>(&planetList);
    for (int i = 0; i < planetList.size(); i++) {
        planetList[i]->kinematic = new Kinematics(this, planetList[i]->obj);
        planetMap.push_back(planetList[i]->obj->getName());
    }
    return planetList;
}

std::vector<Element*> SolarSystem::loadSattelites() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\core\\Sattelites.dat");
    std::vector<Element*> satteliteList;
    parser.parseObjects<Element, Sattelite>(&satteliteList);
    return satteliteList;
}

void SolarSystem::initializeMechanics(int num, ObjectTypes type) {
    Element* el;
    std::string ref_object;
    Eigen::Vector3d Position, Velocity;
    int ref_type;
    
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\core\\" + (std::string)filenames[type - 1]);

    switch (type) {
    case STAR:
        el = starList[num - 1];
        break;
    case PLANET:
        el = planetList[num - 1];
        break;
    default:
        return;
    }
    OrbitInit init = parser.parseOrbit(num, el->obj, &ref_type, &ref_object);
    if (ref_type == ObjectTypes::BARYCENTRE) {
        init.init_mu = 0;
        el->kinematic->setKinematicAnchor(NULL);
    }
    else {
        el->kinematic->setKinematicAnchor(getObjectFromName((ObjectTypes)ref_type, ref_object));
        init.init_mu = calculate_mu(el->obj->getMass(), el->kinematic->getKinematicAnchor()->getMass());        // Reason to put kinematic anchor in Object class. TODO!
    }

    el->obj->setMu(init.init_mu);
    switch (init.type) {
        case 0:
            el->obj->orbit.initOrbitCOE_TA(init, &Position, &Velocity);
            break;
        case 1:
            el->obj->orbit.initOrbitCOE_R(init, &Position, &Velocity);
            break;
        case 2:
            el->obj->orbit.initOrbitTLE(init, &Position, &Velocity); //TODO
            break;
        case 3:
            el->obj->orbit.initOrbitCOE_ML(init, &Position, &Velocity);
            break;
    }
    if (el->kinematic->initKinematicProcess(Position, Velocity)) {
        std::cout << "\033[0;32;49mSUCCESS: Initialized Kinematic Process for Object: " << el->obj->getName() << "\033[0m" << std::endl;
    }
    else {
        std::cout << "WARN: Attempt to Initialize Kinematic Process Failed for Object: " << el->obj->getName() << std::endl;
        std::cout << "Reason | Kinematic Process Already Initialized" << std::endl;
    }
  
}


void SolarSystem::mapSystem() {
    for (int i = 0; i < planetList.size(); i++) {
        Element* el = planetList[i];
        Element* el_tmp = el;
        int iterator = 0;
        Object* next_anchor;
        while (iterator > -1) {
            el->depth_map[iterator] = el_tmp->obj;
            el->depth_map_reverse[el_tmp->obj] = iterator;
            next_anchor = el_tmp->kinematic->getKinematicAnchor();
            if (!next_anchor || next_anchor == nullptr)
                break;
            el_tmp = getElementFromName(next_anchor->getType(), next_anchor->getName());
            iterator++;
        }
    }
}

void SolarSystem::start() {

}


Object* SolarSystem::getObjectFromName(ObjectTypes type, std::string name) {
    switch (type) {
    case STAR:
        return starList[std::distance(starMap.begin(), std::find(starMap.begin(), starMap.end(), name))]->obj;
    case PLANET:
        return planetList[(int)(std::distance(planetMap.begin(), std::find(planetMap.begin(), planetMap.end(), name)))]->obj;
    default:
        return nullptr;
    }
}

Element* SolarSystem::getElementFromName(ObjectTypes type, std::string name) {
    switch (type) {
    case STAR:
        return starList[std::distance(starMap.begin(), std::find(starMap.begin(), starMap.end(), name))];
    case PLANET:
        return planetList[(int)(std::distance(planetMap.begin(), std::find(planetMap.begin(), planetMap.end(), name)))];
    default:
        return nullptr;
    }
}


Eigen::Vector3d SolarSystem::getVectorBetweenObjects(Element* o1, Element* o2) {
   /* std::unordered_map<int, Object*>* map_long;
    std::unordered_map<int, Object*>* map_short;
    

    if (o2->depth_map.size() > o1->depth_map.size()) {
        map_long = &o2->depth_map;
        map_short = &o1->depth_map;
    }
    else {
        map_long = &o1->depth_map;
        map_short = &o2->depth_map;
    }
    */
    //std::unordered_map<Object*, Eigen::Vector3d> map1;
    int max_len = (std::max)(o1->depth_map.size(), o2->depth_map.size());

    Eigen::Vector3d pos1;
    Eigen::Vector3d pos2;
    pos1 << 0, 0, 0;
    pos2 << 0, 0, 0;

    for (int i = 0; i < max_len; i++) {
        if (o1->depth_map_reverse.find(o2->depth_map[i]) != o1->depth_map_reverse.end())
            for (int j = 0; j < o1->depth_map.size(); j++) {
                if (o1->depth_map[j] == o2->obj)
                    return pos1;
                pos1 += getElementFromName(o1->depth_map[j]->getType(), o1->depth_map[j]->getName())->kinematic->p;
            }
        if (o2->depth_map_reverse.find(o1->depth_map[i]) != o2->depth_map_reverse.end())
            for (int j = 0; j < o2->depth_map.size(); j++) {
                if (o2->depth_map[j] == o1->obj)
                    return pos1;
                pos1 += getElementFromName(o2->depth_map[j]->getType(), o2->depth_map[j]->getName())->kinematic->p;
            }
        //map1[o1->kinematic->getKinematicAnchor()] = o1->kinematic->p;
        //map2[o2->kinematic->getKinematicAnchor()] = o2->kinematic->p;
    }
    return pos1;
}