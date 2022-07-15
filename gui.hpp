#pragma once
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "SphereRender.h"
#include "IndexBuffer.h"
#include "stb_image.h"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

float cameraRadius = 3.0f;

// Initial position : on +Z
glm::vec3 position = glm::vec3(cameraRadius, 0, 0);
glm::vec3 positionModifier = glm::vec3(3, 0, 0);
// Initial horizontal angle : toward -Z
float horizontalAngle = 0;
// Initial vertical angle : none
float verticalAngle = 3.14f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 1.0f; // 3 units / second
float mouseSpeed = 0.005f;


void computeMatrices(GLFWwindow* window) {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);
	
	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);
	
	position = cameraRadius * glm::vec3(cos(verticalAngle) * cos(horizontalAngle), cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle));

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	/*glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);*/

	glm::vec3 direction = position * -1.0f;

	//glm::vec3 direction(
	//	-3, 0, 0
	//);

	// Right vector
	glm::vec3 right = glm::vec3(
		cos(horizontalAngle - 3.14f / 2.0f),
		sin(horizontalAngle - 3.14f / 2.0f),
		0
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);
	
	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		cameraRadius -= deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		cameraRadius += deltaTime * speed;
	}
	//// Strafe right
	//if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
	//	position += right * deltaTime * speed;
	//}
	//// Strafe left
	//if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
	//	position -= right * deltaTime * speed;
	//}
	
	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

int initGUI() {

	GLFWwindow* window;
	GLFWimage icons[1];
	icons[0].pixels = stbi_load("./earth.png", & icons[0].width, & icons[0].height, 0, 4);

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
	glfwWindowHint(GLFW_REFRESH_RATE, 60);


	window = glfwCreateWindow(1024, 768, "A M OGU S", NULL, NULL);
	if (!window) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwSetWindowIcon(window, 1, icons);
	glfwMakeContextCurrent(window);


	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cout << "ERROR!";
		getchar();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	glfwSwapInterval(1);

	glShadeModel(GL_SMOOTH);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	GL(glEnable(GL_DEPTH_TEST));
	//GL(glEnable(GL_BLEND));
	//GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL(glDepthFunc(GL_LESS));
	GL(glEnable(GL_CULL_FACE));
	VertexArray va;
	Shader shader("gui/shaders/vertex.shader", "gui/shaders/fragment.shader");
	shader.bind();


	Texture texture("highres.jpg");

	SphereRender sphere(1, 100, 100);

	IndexBuffer ib(sphere.getIndices(), sphere.getIndexCount());

	VertexBuffer vb(sphere.getBufferData(), sphere.getBufferDataSize());

	BufferLayout layout;
	layout.push<float>(3);
	layout.push<float>(3);
	layout.push<float>(2);
	va.AddBuffer(vb, layout);

	// debug
	//std::cout << sphere.getVerticiesSize() << endl;
	//std::cout << sphere.getIndexCount() << endl;

	glm::vec3 lightPos = glm::vec3(-4, -6, 0);
	shader.SetUniform3f("LightPosition_worldspace", lightPos.x, lightPos.y, lightPos.z);
	shader.SetUniform1i("myTextureSampler", 0);

	texture.Bind();
	ib.bind();
	va.bind();

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.bind();

		computeMatrices(window);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		shader.SetUniformMat4f("MVP", MVP);
		shader.SetUniformMat4f("V", ViewMatrix);
		shader.SetUniformMat4f("M", ModelMatrix);

		GL(glDrawElements(
			GL_TRIANGLES,      // mode
			ib.GetCount(),    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		));

		shader.unbind();

		GL(glfwSwapBuffers(window));
		GL(glfwPollEvents());

	} while (!glfwWindowShouldClose(window));

	return 0;
}

