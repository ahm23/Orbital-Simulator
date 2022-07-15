#pragma once

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "../celestial/Star.h"
#include "../celestial/Planet.h"
#include "../celestial/Moon.h"
#include "../Sattelite.h"
#include "../../commons/constants.h"
#include <limits>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

template<typename T>
void dp_destruct(T** dp, int count) {
    for (int i = 0; i < count; i++)
        free(dp[i]);
    free(dp);
}

class FileParser {
public:
    FileParser(std::string filepath) {
        file.open(filepath);
        if (!file)
            std::cerr << "File can't be opened! " << std::endl;
        file >> j;
    };
    
    int getCount() { return count; }

    void parseObjectConfig(ObjectConfig* cfg, ObjectTypes type, int i) {
        if (j[i]["name"].is_string())
            if (std::find(v_name.begin(), v_name.end(), j[i]["name"]) == v_name.end()) cfg->name = j[i]["name"];            // Verify that the name is unused
            else throw (202);
        else throw (201);
        if (j[i]["displayName"].is_string()) cfg->name = j[i]["displayName"];                                               // Assign display name if defined
        if (j[i]["mass"].is_number() && j[i]["mass"] > 0) cfg->mass = j[i]["mass"];                                         // Verify and assign mass
        else throw (204);
        if (j[i]["grav_param"].is_number())                                                                                 // Verify and assign graviational parameter
            if (j[i]["grav_param"] > 0) cfg->mu = j[i]["grav_param"];
            else throw(205);                                                                                                
        else
            cfg->mu = GRAV * j[i]["mass"];
        cfg->type = type;                           // Assign object type
    }

    bool validateObjectConfig(ObjectConfig* cfg) {
        if (!cfg->id || !cfg->type || cfg->mass <= 0 || cfg->mu <= 0) {
            throw (200);
            return false;
        }
        if (!cfg->name.size()) {
            throw (203);
            return false;
        }
    }

    void parseCelestialConfig(CelestialConfig* cfg, int i) {
        // Verify that the name is unused
        if (j[i]["avg_radius"].is_number()) cfg->avg_radius = j[i]["avg_radius"];
        else throw (206);
        if (j[i]["incl_ecliptic"].is_number()) cfg->ecliptic_i = j[i]["incl_ecliptic"];
        // TODO: Add Initialization Warnings
    }

    template<typename T, typename X>
    int parseObjects(std::vector<T*>* var) { std::cout << "Unknown Object Type!"; return 0;}
    template<>
    int parseObjects<Element, Star>(std::vector <Element*>* list) {
        int i = 0;
        for (;i < j.size(); i++) {
            try {
                StarConfigPackage star;
                star.config_obj.id = (int)list->size() + 1;                 // Assign internal object ID
                parseObjectConfig(&star.config_obj, ObjectTypes::STAR, i);
                parseCelestialConfig(&star.config_clst, i);
                star.config_star.temp_surface = j[i]["temp_surface"];
                Element* el = new Element;
                el->obj = new Star(star);
                list->push_back(el);
            } catch (int code) {
                std::cout << code;
                // TODO: Proper Error Handle
            }
        }
        return i;
    }
    template<>
    int parseObjects<Element, Planet>(std::vector<Element*>* list) {
        int i = 0;
        for (; i < j.size(); i++) {
            try {
                PlanetConfigPackage planet;
                planet.config_obj.id = (int)list->size() + 1;                 // Assign internal object ID
                parseObjectConfig(&planet.config_obj, ObjectTypes::PLANET, i);
                parseCelestialConfig(&planet.config_clst, i);
                Element* el = new Element;
                el->obj = new Planet(planet);
                list->push_back(el);
            }
            catch (int code) {
                std::cout << code;
                // TODO: Proper Error Handle
            }
        }
        return i;
    }
    template<>
    int parseObjects<Element, Moon>(std::vector<Element*>* list) {
        int i = 0;
        for (; i < j.size(); i++) {
            try {
                MoonConfigPackage moon;
                moon.config_obj.id = (int)list->size() + 1;                 // Assign internal object ID
                parseObjectConfig(&moon.config_obj, ObjectTypes::MOON, i);
                parseCelestialConfig(&moon.config_clst, i);
                Element* el = new Element;
                el->obj = new Moon(moon);
                list->push_back(el);
            }
            catch (int code) {
                std::cout << code;
                // TODO: Proper Error Handle
            }
        }
        return i;
    }
    template<>
    int parseObjects<Element, Sattelite>(std::vector<Element*>* list) {
        int i = 0;
        for (; i < j.size(); i++) {
            SatteliteConfigPackage sattelite;
            sattelite.config_obj.id = (int)list->size();
            sattelite.config_obj.type = ObjectTypes::SATTELITE;
            file >> sattelite.config_obj.name >> sattelite.config_obj.mass >> sattelite.config_obj.mu;
            std::string line; std::getline(file, line, '\n');
            std::getline(file, line, '\n');
            Element* el = new Element;
            el->obj = new Sattelite(sattelite);
            list->push_back(el);
        }
        return i;
    }


    OrbitInit parseOrbit(int num, Object* planet, int* ref_t, std::string* ref_o, int i) {
        OrbitInit init;
        std::cout << i;
        init.type = j[i]["init_orbit"]["type"];
        std::string type_angle = j[i]["init_orbit"]["type_angle"];
        *ref_t = j[i]["init_orbit"]["type_anchor"];
        *ref_o = j[i]["init_orbit"]["anchor"];
        // TODO: Fix other preliminary orbit determinations
        init.timestamp = j[i]["init_orbit"]["timestamp"];
        switch (init.type) {
            case 0:
                file >> init.coe.TA >> init.coe.a >> init.coe.e >> init.coe.i >> init.coe.raan >> init.coe.omega;
                return init;
            case 1:
                file >> init.init_r >> init.coe.a >> init.coe.e >> init.coe.i >> init.coe.raan >> init.coe.omega;
                return init;
            case 2:
                file >> init.init_ma >> init.coe.e >> init.coe.i >> init.coe.raan >> init.coe.omega;
                return init;
            case 3:
                init.coe.a = j[i]["init_orbit"]["semimajor"];
                init.coe.e = j[i]["init_orbit"]["eccentricity"];
                init.coe.i = j[i]["init_orbit"]["inclination"] * (type_angle == "deg") ? d2r : 1;
                init.coe.raan = j[i]["init_orbit"]["raan"] * (type_angle == "deg") ? d2r : 1;
                init.coe.omega = j[i]["init_orbit"]["argumentOfPeriapsis"] * (type_angle == "deg") ? d2r : 1;
                init.init_ml = j[i]["init_orbit"]["meanLongitude"] * (type_angle == "deg") ? d2r : 1;
                return init;
            default:
                return init;
        }
    }

private:
    std::ifstream file;
    json j;
    int count;
    std::vector<std::string> v_name {};
};

enum OrbitPrelimMethod {
    COE_R = 0,
    COE_T = 1
};