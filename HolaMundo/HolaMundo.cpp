// HolaMundo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

#include <glm-0.9.9.5/glm/glm/glm.hpp>
#include <glm-0.9.9.5/glm/glm/gtc/matrix_transform.hpp>
#include <glm-0.9.9.5/glm/glm/gtc/type_ptr.hpp>

#include <glad.h>
#include <glfw3.h>

//Radius if bullet
#define RADIUS 0.14
//Z offset of gun
#define CENTERGUN 5.0
//Wall distance from gun barrel
#define WALL_SEPERATION 15.0

//Statics for gun
std::vector<float> vertexPoints;
std::vector<unsigned int> faceVertices;
//Statics for bullet
std::vector<float> bulletPoints;
std::vector<unsigned int> bulletFaces;
unsigned int bulletVertexArrayObject;
//statics to hold VAO and EBO count.
std::vector<unsigned int> vertexArrayObjects;
std::vector<unsigned int> vaoSizes;
//Statics for wall for ODE
static std::vector<float> verticeData;
static std::vector<unsigned int> faceVerticeData;
static std::vector<float> odeVertices;
//Statics for wall for opengl
static std::vector<float> wallData;
static std::vector<int> wallfaces;
static unsigned int vertexBufferObject2;
unsigned int vertexArrayObject2;

//ODE statics
static dGeomID world_mesh;
static dWorldID world;
static dJointGroupID contactgroup;
static dBodyID sphbody;
static dGeomID sphgeom;
static dMass bulletMass;
static dSpaceID collisionSpace;
//Position matrices.
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 bulletPosMat = glm::mat4(1.0f);
//GLFW statics
float vertDeg = 0;
float horDeg = 0;
double X;
double Y;
bool first = true;

//object file processing function.
void processFile(std::string src, std::vector<float>& verts, std::vector<unsigned int>& indices) {
	std::ifstream objectFile;
	std::string fileLine;
	std::string token;

	objectFile.open(src);
	//getline(objectFile, fileLine)
	while (objectFile >> token) {
		if (token == "v") {
			float v1, v2, v3;
			objectFile >> v1;
			objectFile >> v2;
			objectFile >> v3;
			verts.push_back(v1);
			verts.push_back(v2);
			verts.push_back(v3);

			getline(objectFile, fileLine);
		}
		if (token == "vn") {
			//TODO
			getline(objectFile, fileLine);
		}
		if (token == "f") {
			getline(objectFile, fileLine);
			// + x indicates a step over the found token to search for next token starting at prevToken + tokenLength.
			int firstSlashes = fileLine.find("//");
			int firstSpace = fileLine.find(' ', firstSlashes + 2);

			int secondSlashes = fileLine.find("//", firstSpace + 1);
			int secondSpace = fileLine.find(' ', secondSlashes + 2);

			int thirdSlashes = fileLine.find("//", secondSpace + 1);
			//third space should be end of string.

			indices.push_back(stoi(fileLine.substr(0, firstSlashes), NULL, 10));
			indices.push_back(stoi(fileLine.substr(firstSpace + 1, secondSlashes - (firstSpace + 1)), NULL, 10));
			indices.push_back(stoi(fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)), NULL, 10));
			//int n = fileLine.find("//");
			/*std::cout << fileLine.substr(0, firstSlashes) << " " << fileLine.substr(firstSlashes+2,firstSpace-(firstSlashes+2)) << " " <<
				fileLine.substr(firstSpace+1,secondSlashes-(firstSpace+1)) << " " << fileLine.substr(secondSlashes+2,secondSpace-(secondSlashes+2)) << " " <<
				fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)) << " " << fileLine.substr(thirdSlashes + 2, std::string::npos) << " " <<
				std::endl;*/
		}

		//std::cout << fileLine << std::endl;
	}
	std::vector<unsigned int> orderedIndices;
	for (int i = 0; i < indices.size(); i++) {
		orderedIndices.push_back(indices[i] - 1);
	}
	indices = orderedIndices;
}

//Called by ODE to determine what bodies/geoms are colliding.
static void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
	assert(o1);
	assert(o2);

	if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
	{
		fprintf(stderr, "testing space %p %p\n", (void*)o1, (void*)o2);
		// colliding a space with something
		dSpaceCollide2(o1, o2, data, &nearCallback);
		// Note we do not want to test intersections within a space,
		// only between spaces.
		return;
	}

	//  fprintf(stderr,"testing geoms %p %p\n", o1, o2);

	const int N = 32;
	dContact contact[N];
	int n = dCollide(o1, o2, N, &(contact[0].geom), sizeof(dContact));
	if (n > 0)
	{
		for (int i = 0; i < n; i++)
		{
			// Paranoia  <-- not working for some people, temporarily removed for 0.6
			//dIASSERT(dVALIDVEC3(contact[i].geom.pos));
			//dIASSERT(dVALIDVEC3(contact[i].geom.normal));
			//dIASSERT(!dIsNan(contact[i].geom.depth));
			contact[i].surface.slip1 = 0.7;
			contact[i].surface.slip2 = 0.7;
			contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
			contact[i].surface.mu = 50.0; // was: dInfinity
			contact[i].surface.soft_erp = 0.96;
			contact[i].surface.soft_cfm = 0.04;
			dJointID c = dJointCreateContact(world, contactgroup, &contact[i]);
			dJointAttach(c,
				dGeomGetBody(contact[i].geom.g1),
				dGeomGetBody(contact[i].geom.g2));
		}
	}
}

//GLFW callback function for mouse clicks.
void mouseClickCall(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		std::vector<float> bulletVerts;
		std::vector<unsigned int> bulletIndices;
		processFile("bullet.obj",bulletVerts,bulletIndices);


		unsigned int vertexBufferObject;
		unsigned int elementBufferObject;

		glGenVertexArrays(1, &bulletVertexArrayObject);
		glBindVertexArray(bulletVertexArrayObject);
		glGenBuffers(1, &vertexBufferObject);
		glGenBuffers(1, &elementBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, bulletVerts.size() * sizeof(float), &bulletVerts[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bulletIndices.size() * sizeof(unsigned int), &bulletIndices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		

	
		vertexArrayObjects.push_back(bulletVertexArrayObject);
		vaoSizes.push_back(bulletIndices.size());	

		float sx = glm::sin(glm::radians(-horDeg)) * 5;
		float sy = (glm::cos(glm::radians(-horDeg)) * 5) - 5.0f;
		float sz = glm::sin(glm::radians(vertDeg)) * 5;
	

		dQuaternion q;
		dQSetIdentity(q);
		dBodySetPosition(sphbody, sx, sy, sz);
		dBodySetQuaternion(sphbody, q);
		dBodySetLinearVel(sphbody, 0, -10.0f, 0.0f);
		dBodySetAngularVel(sphbody, 0, 0, 0);

	}
}
//GLFW keyboard callback for closing window/ending program.
void keyStrokeCall(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}
//GLFW mouse position callback. Used to determine the orientation of the gun.
static void mousePosCall(GLFWwindow* window, double xpos, double ypos) {
	model = glm::mat4(1.0f);
	//On mouse capture
	if (first) {
		X = xpos;
		Y = ypos;
		first = !first;
	}
	//After mouse is captured.
	else {
		//Center object.
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, CENTERGUN));
		//Rotate Up
		if (xpos < X) {
			if (horDeg <= 15.0f) {
				horDeg += 1.0f;
			}
			X = xpos;
		}
		//Rotate Down
		else if (xpos > X) {
			if (horDeg >= -15.0f) {
				horDeg -= 1.0f;
			}
			X = xpos;
		}
		//Rotate object on Y axis while maintaining global y.
		model = glm::rotate(model, glm::radians(horDeg), glm::vec3(0.0f, 1.0f, 0.0f));
		//Rotate left
		if (ypos < Y) {
			
			if (vertDeg < 15.0f) {
				vertDeg += 1.0f;
			}
			Y = ypos;
		}
		//Rotate right.
		else if (ypos > Y) {	
			if (vertDeg > -15.0f) {
				vertDeg -= 1.0f;
			}
			Y = ypos;
		}
		//Rotate object on x axis, losing global orientation
		model = glm::rotate(model, glm::radians(vertDeg), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -CENTERGUN));
		
	}
	
}

int main()
{
	//Identity rotation matrix to init wall in ODE
	dMatrix3 R;
	
	
	

	//Create physics world
	dInitODE2(0);
	world = dWorldCreate();
	collisionSpace = dHashSpaceCreate(0);
	contactgroup = dJointGroupCreate(0);
	dWorldSetGravity(world, 0, 0, -1.0);
	dWorldSetQuickStepNumIterations(world, 64);

	//Load wall into world
	processFile("G:\\Anthem\\C_files\\wall.obj", verticeData, faceVerticeData);

	//Translate wavefront obj vertices to ODE vertices. Switch Y, and Z.
	for (int i = 0; i < verticeData.size(); i += 3) {
		odeVertices.push_back(verticeData[i]);
		odeVertices.push_back(verticeData[i + 2]);
		odeVertices.push_back(verticeData[i + 1]);
	}
	//Re order the face winding by swapping x,z vertices for the face.
	for (int i = 0; i < faceVerticeData.size(); i += 3) {
		dTriIndex t = faceVerticeData[i];
		faceVerticeData[i] = faceVerticeData[i + 2];
		faceVerticeData[i + 2] = t;
	}

	dTriMeshDataID Data = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle(Data, &odeVertices[0], 3 * sizeof(float), odeVertices.size() / 3, &faceVerticeData[0], faceVerticeData.size(), 3 * sizeof(dTriIndex));
	world_mesh = dCreateTriMesh(collisionSpace, Data, 0, 0, 0);
	dGeomTriMeshEnableTC(world_mesh, dSphereClass, false);
	dGeomTriMeshEnableTC(world_mesh, dBoxClass, false);

	dGeomSetPosition(world_mesh, 0, -WALL_SEPERATION, 0.0);
	dRSetIdentity(R);
	//dIASSERT(dVALIDMAT3(R));

	dGeomSetRotation(world_mesh, R);

	sphbody = dBodyCreate(world);
	dMassSetSphere(&bulletMass, 1, RADIUS);
	dBodySetMass(sphbody, &bulletMass);
	sphgeom = dCreateSphere(0, RADIUS);
	dGeomSetBody(sphgeom, sphbody);
	dSpaceAdd(collisionSpace, sphgeom);
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1200, 900, "Hello Triangle", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed window creation" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Not glad didnt load" << std::endl;
		return -1;
	}
	glfwSetKeyCallback(window, keyStrokeCall);
	glfwSetMouseButtonCallback(window, mouseClickCall);
	glfwSetCursorPosCallback(window, mousePosCall);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	unsigned int fragmentShader;
	unsigned int vertexShader;
	unsigned int program;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Vertex shader processing.
	std::ifstream vFile;
	std::string line;
	std::stringstream vertSrcStream;
	//Open file and read into a stringstream. Append a newline for each line to be in correct format for opengl shader source.
	vFile.open("vShader.txt");
	while (getline(vFile, line)) {
		vertSrcStream << line << "\n";
	}
	//Convert stringstream to string and then to C style string.
	std::string  vertSrc = vertSrcStream.str();
	const char * vertString = vertSrc.c_str();

	glShaderSource(vertexShader, 1, &vertString,NULL);
	glCompileShader(vertexShader);
	
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "V err LOG: " << infoLog << std::endl;
	}
	//Fragment shader processing.
	std::ifstream fFile;
	std::stringstream fragSrcStream;
	//Open file and read into a stringstream. Append a newline for each line to be in correct format for opengl shader source.
	fFile.open("fShader.txt");
	while (getline(fFile, line)) {
		fragSrcStream << line << "\n";
	}
	//Convert stringstream to string and then to C style string.
	std::string  fragSrc = fragSrcStream.str();
	const char * fragString = fragSrc.c_str();
	
	glShaderSource(fragmentShader, 1, &fragString, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "F err LOG: " << infoLog << std::endl;
	}
	//Create and link program.
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
	}
	//Shaders can be deleted. TODO.

	//M4
	std::vector<float> vertsB;
	std::vector<unsigned int> indicesB;
	processFile("M4a1.obj",vertsB,indicesB);
	//Wall
	std::vector<float> wallVerts;
	std::vector<unsigned int> wallIndices;
	processFile("wall.obj", wallVerts, wallIndices);
	
	
	unsigned int elementBufferObject2;

	glGenVertexArrays(1, &vertexArrayObject2);
	glBindVertexArray(vertexArrayObject2);
	glGenBuffers(1, &vertexBufferObject2);
	glGenBuffers(1, &elementBufferObject2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject2);
	glBufferData(GL_ARRAY_BUFFER, wallVerts.size() * sizeof(float), &wallVerts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, wallIndices.size() * sizeof(unsigned int), &wallIndices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	vertexArrayObjects.push_back(vertexArrayObject2);
	vaoSizes.push_back(wallIndices.size());

	unsigned int vertexBufferObject;
	unsigned int vertexArrayObject;
	unsigned int elementBufferObject;

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	glGenBuffers(1, &vertexBufferObject);
	glGenBuffers(1, &elementBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertsB.size() * sizeof(float), &vertsB[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0] , GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesB.size()*sizeof(unsigned int), &indicesB[0], GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	vertexArrayObjects.push_back(vertexArrayObject);
	vaoSizes.push_back(indicesB.size());
	
	//model = glm::rotate(model, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	
	glViewport(0, 0, 1200, 900);
	glfwShowWindow(window);
	bool firstTick = true;
	double dnow;
	double dlast = 0.0;
	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(program);

		//Create camera matrix and projection matrix.
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(glm::vec3(10.0f, 5.0f, 10.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 1200.0f / 900.0f, 0.1f, 100.0f);
		//Uniform locations
		int modelLoc = glGetUniformLocation(program, "model");
		int viewLoc = glGetUniformLocation(program, "view");
		int projectionLoc = glGetUniformLocation(program, "projection");
		int colorLoc = glGetUniformLocation(program, "objColor");
		
		//ODE sim step time.
		double simstep = 0.0001; // 1ms simulation steps
		//Determine time since last simsteps.
		dnow = glfwGetTime();
		double elapsedTime = dnow - dlast;
		dlast = dnow;
		//Step count
		int nrofsteps = (int)ceilf(elapsedTime / simstep);
		for (int i = 0; i < nrofsteps; i++)
		{
			dSpaceCollide(collisionSpace, 0, &nearCallback);
			dWorldQuickStep(world, simstep);
			dJointGroupEmpty(contactgroup);
		}
		//Get information for bullet.
		const dReal *SPos = dBodyGetPosition(sphbody);
		const dReal *SRot = dBodyGetRotation(sphbody);
		float spos[3] = { SPos[0], SPos[1], SPos[2] };
		float srot[16] = { SRot[0], SRot[1], SRot[2], SRot[3], SRot[4], SRot[5], SRot[6], SRot[7], SRot[8], SRot[9], SRot[10], SRot[11], 0.0f, 0.0f, 0.0f, 0.0f};
		glm::mat4 test = glm::make_mat4(srot);

		//Set the universal matrices.
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//For every openGL VAO, bind it and draw it using model Matrix based on type of object.
		for (int i = 0; i < vertexArrayObjects.size(); i++) {
			if (vertexArrayObjects[i] == bulletVertexArrayObject){ 
				//If the object is the bullet, update its position based on ODE values grabbed above.
				bulletPosMat = glm::mat4(1.0f);
				bulletPosMat = glm::translate(bulletPosMat, glm::vec3(spos[0], spos[2], spos[1]));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bulletPosMat));
				glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
			}
			else if (vertexArrayObjects[i] == vertexArrayObject2) {
				//Wall object. Translate it back.
				glm::mat4 id = glm::mat4(1.0f);
				id = glm::translate(id, glm::vec3(0.0f, 0.0f, -WALL_SEPERATION));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(id));
				glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

			}
			else if (vertexArrayObjects[i] == vertexArrayObject) {
				//Gun object.
				glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f);
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			}
			glBindVertexArray(vertexArrayObjects[i]);

			glDrawElements(GL_TRIANGLES, vaoSizes[i], GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
