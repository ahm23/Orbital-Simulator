#pragma once
#include "SolarSystem.h"

#include "../commons/error.h"

long SolarSystem::update_freq = 1000;


SolarSystem::SolarSystem() {

	loadStars();
    loadPlanets();
    loadMoons();

    engineStart();
    
    for (int i = 0; i < iBuffer_star.size(); i++) {
        initializeMechanics(i, iBuffer_star[i], STAR);
    }

    for (int i = 0; i < iBuffer_planet.size(); i++) {
        initializeMechanics(i, iBuffer_planet[i], PLANET);
    }

    for (int i = 0; i < iBuffer_moon.size(); i++) {
        initializeMechanics(i, iBuffer_moon[i], MOON);
    }

    mapSystem();


    //ke = new KinematicEngine(&elements, 1, &kinematic_m, &kinematic_cv);

    std::lock_guard<std::shared_mutex> lk(kinematic_m);
    toggle_kinematic = true;
    kinematic_cv.notify_all();
}

std::vector<Element*> SolarSystem::loadStars() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\stars.json");
    std::vector<Element*> starList;
    int start = (int)elements.size();
    int count = parser.parseObjects<Element, Star>(&elements);
    for (int i = start; i < count; i++) {
        iBuffer_star.push_back(i);
        starMap[elements[i]->obj->getName()] = elements[i]->obj->getID() - 1;
    }
    return starList;
}

std::vector<Element*> SolarSystem::loadPlanets() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\planets.json");
    std::vector<Element*> planetList;
    int start = (int)elements.size();
    int count = parser.parseObjects<Element, Planet>(&elements);
    for (int i = start; i < start + count; i++) {
        iBuffer_planet.push_back(i);
        planetMap[elements[i]->obj->getName()] = elements[i]->obj->getID() - 1;
    }
    return planetList;
}

std::vector<Element*> SolarSystem::loadMoons() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\moons.json");
    std::vector<Element*> moonList;
    int start = (int)elements.size();
    int count = parser.parseObjects<Element, Moon>(&elements);
    for (int i = start; i < start + count; i++) {
        iBuffer_moon.push_back(i);
        moonMap[elements[i]->obj->getName()] = elements[i]->obj->getID() - 1;
    }
    return moonList;
}

std::vector<Element*> SolarSystem::loadSattelites() {
    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\core\\Sattelites.dat");
    std::vector<Element*> satteliteList;
    parser.parseObjects<Element, Sattelite>(&elements);
    return satteliteList;// INCOMPLETE
}

void SolarSystem::initializeMechanics(int index, int num, ObjectTypes type) {
    Element* el;
    std::string ref_object;
    Eigen::Vector3d Position, Velocity;
    int ref_type;
    bool astronomical = false;

    FileParser parser("C:\\Users\\netagive\\Desktop\\Orbital\\" + (std::string)filenames[type - 1]);

    switch (type) {
    case STAR:
        el = elements[num];
        astronomical = true;
        break;
    case PLANET:
        el = elements[num];
        astronomical = true;
        break;
    case MOON:
        el = elements[num];
        astronomical = true;
        break;
    default:
        el = elements[num];
        break;
    }
    OrbitInit init = parser.parseOrbit(index, el->obj, &ref_type, &ref_object, index);
    if (ref_type == ObjectTypes::BARYCENTRE) {
        init.init_mu = 0;
        el->anchor = NULL;
    }
    else {
        el->anchor = getObjectFromName((ObjectTypes)ref_type, ref_object);
        init.init_mu = calculate_mu(el->obj->getMass(), el->anchor->getMass());
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

    el->obj->setPos(Position);
    el->obj->setVel(Velocity);

    P_INIT p_body = {
        astronomical, el->obj->getMass(), init.timestamp, Position, Velocity
    };

    P_PACKET p = {
        el->obj->getID(), REQ_TYPE::INIT, 0
    };
    memcpy(&p.p.body, p_body.buffer, sizeof(p_body.buffer));
    // NOTE: I might realloc() the body buffer

    engineRequest(p);
}


void SolarSystem::mapSystem() {
    for (int i = 0; i < elements.size(); i++) {
        Element* el = elements[i];
        Element* el_tmp = el;
        int iterator = 0;
        CelestialBody* next_anchor;
        while (iterator > -1) {
            el->depth_map[iterator] = (CelestialBody*)el_tmp->obj;
            el->depth_map_reverse[(CelestialBody*)el_tmp->obj] = iterator;
            next_anchor = (CelestialBody*)el_tmp->anchor;      /// might revert to Objects & cast to clst in utils_planetary
            if (!next_anchor || next_anchor == NULL)
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
        return elements[starMap[name]]->obj;
    case PLANET:
        return elements[planetMap[name]]->obj;
    case MOON:
        return elements[moonMap[name]]->obj;
    default:
        return nullptr;
    }
}

Element* SolarSystem::getElementFromName(ObjectTypes type, std::string name) {
    switch (type) {
    case STAR:
        return elements[starMap[name]];
    case PLANET:
        return elements[planetMap[name]];
    case MOON:
        return elements[moonMap[name]];
    default:
        return nullptr;
    }
}


void SolarSystem::engineStart() {
    HANDLE g_hInputFile = NULL;

    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;  // Set the bInheritHandle flag so pipe handles are inherited. 
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&StdOUT_R, &StdOUT_R, &saAttr, 0))
        ThrowError(ERR::FATAL, TEXT("Failed to Create Pipe for Kinetic Engine STDOUT"));

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(StdOUT_R, HANDLE_FLAG_INHERIT, 0))
        ThrowError(ERR::FATAL, TEXT("Failed to Set Read HANDLE Information for Kinetic Engine STDOUT"));

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&StdIN_R, &StdIN_W, &saAttr, 0))
        ThrowError(ERR::FATAL, TEXT("Failed to Create Pipe for Kinetic Engine STDIN"));

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(StdIN_W, HANDLE_FLAG_INHERIT, 0))
        ThrowError(ERR::FATAL, TEXT("Failed to Set Read HANDLE Information for Kinetic Engine STDIN"));

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = StdOUT_W;
    siStartInfo.hStdOutput = StdOUT_W;
    siStartInfo.hStdInput = StdIN_R;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(
        "C:\\Users\\netagive\\Desktop\\Orbital\\x64\\Debug\\KinematicEngine.exe",
        NULL,          // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    if (!bSuccess)
        ThrowError(ERR::FATAL, TEXT("Failed to Start Kinetic Engine Process"));
    else {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
}

void SolarSystem::engineRequest(P_PACKET packet) {
    DWORD dwWritten;
    BOOL bSuccess = FALSE;

    bSuccess = WriteFile(StdIN_W, packet.buffer, sizeof(PACKET), &dwWritten, NULL);
    if (!bSuccess) ThrowWarn(TEXT("Kinetic Engine IPC Packet Send Failiure"));
}