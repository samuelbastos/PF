#include <stdio.h>
#include <string>

#include "Volume.h"
#include "Camera.h"
#include "Renderer.h"
#include "Preprocessor.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

GLFWwindow *window;
Renderer* renderer;
ooc::Preprocessor* preprocessor;
Camera cam(60.0f);
GLint WIDTH = 1024, HEIGHT = 768;
int screenWidth, screenHeight;

/* Callback do cursor */
void cursor_callback(GLFWwindow* window, double x, double y)
{
	cam.mouseMotion(x, y);
}

/* Callback de click do mouse */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//std::cout << "button: " << button << " " << "action: " << action << " " << "mods: " << mods << std::endl;
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	cam.mouseButton(button, action, x, y);
}

/* Callback de scroll do mouse */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//std::cout << "x: " << xoffset << " " << "y: " << yoffset << std::endl;
}

/* Initializes GLEW and GLFW window */
int initGLContext()
{
	// Init GLFW
	glfwInit();

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WIDTH, HEIGHT, "KOOOCVR", nullptr, nullptr);
	
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwMakeContextCurrent(window);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}
	auto glversion = glewIsSupported("GL_VERSION_4_0");
	if (glversion) std::cout << "supported" << std::endl;
	return true;
}

void initialize()
{
	initGLContext(); // Inicializa GlEW e GLFW ;

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	cam.setInitialConfig();

	renderer = new Renderer();
	//preprocessor = new ooc::Preprocessor("Base.gst");
	//preprocessor->preprocessData();

	renderer->init(WIDTH, HEIGHT);
}

void render() 
{        
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	renderer->render(&cam);
	//auto error = glGetError();
	//std::cout << glewGetErrorString(error) << std::endl;
	/* Swap front and back buffers */
	glfwSwapBuffers(window);
	/* Poll for and process events */
	glfwPollEvents();
}

int main()
{
	initialize();
	while (!glfwWindowShouldClose(window)) render();
	glfwTerminate();
	return EXIT_SUCCESS;
}