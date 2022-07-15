#pragma once
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iomanip>
#include "core/celestial/Planet.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"

#include <vector>
#include <Eigen/Dense>
#include <fstream>
#include "Renderer.h"
#include "Texture.h"

#include "gtc/matrix_transform.hpp"
#include "SphereRender.h"
#include "utils_debug.h"

#include "glm.hpp"
#include "gui.hpp"

#include "core/SolarSystem.h"

using namespace Eigen;


int main() {

    SolarSystem system;

    double position[3] = {0, -7161.8, 0};
    double velocity[3] = { -1.0749, 0, 7.3862 };
    //Object satellite(100.0, position, velocity);

    initGUI();

    glfwTerminate();

    return 0;
}