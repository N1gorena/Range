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

#define RADIUS 0.14

std::vector<float> vertexPoints;
std::vector<float> bulletPoints;
std::vector<unsigned int> faceVertices;
std::vector<unsigned int> bulletFaces;
std::vector<float> verts(24);
std::vector<unsigned int> vertexArrayObjects;
std::vector<unsigned int> vaoSizes;

static std::vector<float> verticeData;
static std::vector<int> faceVerticeData;

static std::vector<float> wallData;
static std::vector<int> wallfaces;

static std::vector<float> odeVertices;
static dGeomID world_mesh;
static dWorldID world;
static dJointGroupID contactgroup;
static dBodyID sphbody;
static dGeomID sphgeom;
static dMass bulletMass;
static dSpaceID collisionSpace;

unsigned int bulletVertexArrayObject;
static unsigned int vertexBufferObject2;
unsigned int vertexArrayObject2;




glm::mat4 model = glm::mat4(1.0f);
glm::mat4 bulletPosMat = glm::mat4(1.0f);
float vertDeg = 0;
float horDeg = 0;
double X;
double Y;
bool first = true;

float vertices[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {
0, 1, 2,  // second triangle
3,4,5,
5,0,2,
4,6,0,
6,3,1,
2,3,5,
0,6,1,
3,7,4,
5,4,0,
4,7,6,
6,7,3,
2,1,3

};

std::vector<unsigned int> getBulletIndices();
std::vector<float> getBulletVerts();


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
		std::cout << "COLLISION DETECTED" << std::endl;
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


void mouseClickCall(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		std::vector<float> bulletVerts = getBulletVerts();
		std::vector<unsigned int> bulletIndices = getBulletIndices();


		unsigned int vertexBufferObject;
		
		unsigned int elementBufferObject;

		glGenVertexArrays(1, &bulletVertexArrayObject);
		glBindVertexArray(bulletVertexArrayObject);
		glGenBuffers(1, &vertexBufferObject);
		glGenBuffers(1, &elementBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, bulletVerts.size() * sizeof(float), &bulletVerts[0], GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0] , GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bulletIndices.size() * sizeof(unsigned int), &bulletIndices[0], GL_STATIC_DRAW);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		std::cout << vertDeg << " <Vert Hor> " << horDeg << std::endl;
		bulletPosMat = glm::translate(bulletPosMat, glm::vec3(0.0f, glm::sin(glm::radians(vertDeg)) * 5, 0.0f));
		bulletPosMat = glm::translate(bulletPosMat, glm::vec3(glm::sin(glm::radians(-horDeg)) * 5,0.0f, glm::cos(glm::radians(-horDeg)) * 5));
		bulletPosMat = glm::translate(bulletPosMat, glm::vec3(0.0f, 0.0f, -5.0f));

		//bulletPosMat = glm::translate(bulletPosMat, glm::vec3(-3.0f,-3.0f,0.0f));
		vertexArrayObjects.push_back(bulletVertexArrayObject);
		vaoSizes.push_back(bulletIndices.size());	

		//TODELETE
		/*sphbody = dBodyCreate(world);
		dMassSetSphere(&bulletMass, 1, RADIUS);
		dBodySetMass(sphbody, &bulletMass);
		sphgeom = dCreateSphere(0, RADIUS);
		dGeomSetBody(sphgeom, sphbody);
		*/
		float sx = 0.0f, sy = 0.0f, sz = 1.15;

		dQuaternion q;
		dQSetIdentity(q);
		dBodySetPosition(sphbody, sx, sy, sz);
		dBodySetQuaternion(sphbody, q);
		dBodySetLinearVel(sphbody, 0, -10.0f, 0.0f);
		dBodySetAngularVel(sphbody, 0, 0, 0);


		//dSpaceAdd(collisionSpace, sphgeom);


		//DELETEABOVE
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		//std::cout << glm::sin(glm::radians(vertDeg)) * 5 << std::endl;

		
		bulletPosMat = glm::mat4(1.0f);
		bulletPosMat = glm::translate(bulletPosMat, glm::vec3(0.0f, glm::sin(glm::radians(vertDeg)) * 5, 0.0f));
		bulletPosMat = glm::translate(bulletPosMat, glm::vec3(glm::sin(glm::radians(-horDeg)) * 5, 0.0f, glm::cos(glm::radians(-horDeg)) * 5));
		bulletPosMat = glm::translate(bulletPosMat, glm::vec3(0.0f, 0.0f, -5.0f));
		
	}
}

void keyStrokeCall(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

static void mousePosCall(GLFWwindow* window, double xpos, double ypos) {
	model = glm::mat4(1.0f);
	if (first) {
		X = xpos;
		Y = ypos;
		first = !first;
	}
	else {
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0f));
		if (xpos < X) {
			if (horDeg <= 15.0f) {
				horDeg += 1.0f;
			}
			X = xpos;
		}
		else if (xpos > X) {
			if (horDeg >= -15.0f) {
				horDeg -= 1.0f;
			}
			X = xpos;
		}
		model = glm::rotate(model, glm::radians(horDeg), glm::vec3(0.0f, 1.0f, 0.0f));
		if (ypos < Y) {
			if (vertDeg < 15.0f) {
				vertDeg += 1.0f;
			}
			Y = ypos;
		}
		else if (ypos > Y) {	
			if (vertDeg > -15.0f) {
				vertDeg -= 1.0f;
			}
			Y = ypos;
		}
		model = glm::rotate(model, glm::radians(vertDeg), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
		
	}
	//std::cout << vertDeg << ":V  H:" << horDeg << std::endl;
	
}
std::vector<unsigned int> getBulletIndices() {
	std::vector<unsigned int> orderedIndices;

	for (int i = 0; i < bulletFaces.size(); i++) {
		orderedIndices.push_back(bulletFaces[i] - 1);
	}

	return orderedIndices;
}

std::vector<unsigned int> getObjIndices() {
	std::vector<unsigned int> orderedIndices;
	for (int i = 0; i < faceVertices.size(); i++) {
		orderedIndices.push_back(faceVertices[i]-1);
	}
	return orderedIndices;
}
std::vector<float> getBulletVerts() {
	std::ifstream bulletFile;
	std::string fileLine;
	std::string token;

	bulletFile.open("bullet.obj");

	while (bulletFile >> token) {
		if (token == "v") {
			float v1, v2, v3;
			bulletFile >> v1;
			bulletFile >> v2;
			bulletFile >> v3;
			bulletPoints.push_back(v1);
			bulletPoints.push_back(v2);
			bulletPoints.push_back(v3);

			getline(bulletFile, fileLine);
		}
		if (token == "vn") {
			//TODO
			getline(bulletFile, fileLine);
		}
		if (token == "f") {
			getline(bulletFile, fileLine);
			// + x indicates a step over the found token to search for next token starting at prevToken + tokenLength.
			int firstSlashes = fileLine.find("//");
			int firstSpace = fileLine.find(' ', firstSlashes + 2);

			int secondSlashes = fileLine.find("//", firstSpace + 1);
			int secondSpace = fileLine.find(' ', secondSlashes + 2);

			int thirdSlashes = fileLine.find("//", secondSpace + 1);
			//third space should be end of string.

			bulletFaces.push_back(stoi(fileLine.substr(0, firstSlashes), NULL, 10));
			bulletFaces.push_back(stoi(fileLine.substr(firstSpace + 1, secondSlashes - (firstSpace + 1)), NULL, 10));
			bulletFaces.push_back(stoi(fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)), NULL, 10));
			//int n = fileLine.find("//");
			/*std::cout << fileLine.substr(0, firstSlashes) << " " << fileLine.substr(firstSlashes+2,firstSpace-(firstSlashes+2)) << " " <<
				fileLine.substr(firstSpace+1,secondSlashes-(firstSpace+1)) << " " << fileLine.substr(secondSlashes+2,secondSpace-(secondSlashes+2)) << " " <<
				fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)) << " " << fileLine.substr(thirdSlashes + 2, std::string::npos) << " " <<
				std::endl;*/
		}

		//std::cout << fileLine << std::endl;
	}
	return bulletPoints;
}


std::vector<float> getObjVerts() {
	std::ifstream objectFile;
	std::string fileLine;
	std::string token;

	objectFile.open("M4a1.obj");
	//getline(objectFile, fileLine)
	while(objectFile >> token) {
		if (token == "v") {
			float v1, v2, v3;
			objectFile >> v1;
			objectFile >> v2;
			objectFile >> v3;
			vertexPoints.push_back(v1);
			vertexPoints.push_back(v2);
			vertexPoints.push_back(v3);

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
			int firstSpace = fileLine.find(' ', firstSlashes+2);
			
			int secondSlashes = fileLine.find("//", firstSpace + 1);
			int secondSpace = fileLine.find(' ', secondSlashes + 2);

			int thirdSlashes = fileLine.find("//", secondSpace + 1);
			//third space should be end of string.

			faceVertices.push_back( stoi( fileLine.substr( 0, firstSlashes ), NULL , 10 ) );
			faceVertices.push_back(stoi ( fileLine.substr(firstSpace + 1, secondSlashes - (firstSpace + 1)), NULL , 10 ) );
			faceVertices.push_back(stoi (fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)), NULL , 10 ) );
			//int n = fileLine.find("//");
			/*std::cout << fileLine.substr(0, firstSlashes) << " " << fileLine.substr(firstSlashes+2,firstSpace-(firstSlashes+2)) << " " <<
				fileLine.substr(firstSpace+1,secondSlashes-(firstSpace+1)) << " " << fileLine.substr(secondSlashes+2,secondSpace-(secondSlashes+2)) << " " <<
				fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)) << " " << fileLine.substr(thirdSlashes + 2, std::string::npos) << " " <<
				std::endl;*/
		}
		
		//std::cout << fileLine << std::endl;
	}


	/*
	//MAGIC NUMBER 3: 3 components per face vertex x,y,z
	std::vector<float> orderedVertices(faceVertices.size() *3);
	//MAGIC NUMBER 3: 3 vertices per face so iterating over faces should be a third of vertex count/size.
	for (int i = 0; i < faceVertices.size()/3; i++) {
		//Each face is 3 vertices so the ith face starts at i*3 in the face array.
		//Total vertice data is 3x faceVertices size because each vertex in a face has an x,y,z component.
		
		orderedVertices[ i * 9 ]     = vertexPoints[ (faceVertices[i * 3] - 1) * 3];
		orderedVertices[(i * 9) + 1] = vertexPoints[((faceVertices[i * 3] - 1) * 3)+1];
		orderedVertices[(i * 9) + 2] = vertexPoints[((faceVertices[i * 3] - 1) * 3)+2];
		orderedVertices[(i * 9) + 3] = vertexPoints[((faceVertices[(i * 3) + 1] - 1) * 3)];
		orderedVertices[(i * 9) + 4] = vertexPoints[((faceVertices[(i * 3) + 1] - 1) * 3) + 1];
		orderedVertices[(i * 9) + 5] = vertexPoints[((faceVertices[(i * 3) + 1] - 1) * 3) + 2];
		orderedVertices[(i * 9) + 6] = vertexPoints[((faceVertices[(i * 3) + 2] - 1) * 3)];
		orderedVertices[(i * 9) + 7] = vertexPoints[((faceVertices[(i * 3) + 2] - 1) * 3) + 1];
		orderedVertices[(i * 9) + 8] = vertexPoints[((faceVertices[(i * 3) + 2] - 1) * 3) + 2];

		
	}
	*/
	
	return vertexPoints;
}


std::vector<float> getWallVerts() {
	std::ifstream objectFile;
	std::string fileLine;
	std::string token;

	objectFile.open("wall.obj");
	//getline(objectFile, fileLine)
	while (objectFile >> token) {
		if (token == "v") {
			float v1, v2, v3;
			objectFile >> v1;
			objectFile >> v2;
			objectFile >> v3;
			wallData.push_back(v1);
			wallData.push_back(v2);
			wallData.push_back(v3);

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

			wallfaces.push_back(stoi(fileLine.substr(0, firstSlashes), NULL, 10));
			wallfaces.push_back(stoi(fileLine.substr(firstSpace + 1, secondSlashes - (firstSpace + 1)), NULL, 10));
			wallfaces.push_back(stoi(fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)), NULL, 10));
			//int n = fileLine.find("//");
			/*std::cout << fileLine.substr(0, firstSlashes) << " " << fileLine.substr(firstSlashes+2,firstSpace-(firstSlashes+2)) << " " <<
				fileLine.substr(firstSpace+1,secondSlashes-(firstSpace+1)) << " " << fileLine.substr(secondSlashes+2,secondSpace-(secondSlashes+2)) << " " <<
				fileLine.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)) << " " << fileLine.substr(thirdSlashes + 2, std::string::npos) << " " <<
				std::endl;*/
		}

		//std::cout << fileLine << std::endl;
	}

	return wallData;
}

std::vector<unsigned int> getWallIndices() {
	std::vector<unsigned int> orderedIndices;
	for (int i = 0; i < wallfaces.size(); i++) {
		orderedIndices.push_back(wallfaces[i] - 1);
	}
	return orderedIndices;
}

int main()
{
	std::cout << "Here we go again!" << std::endl;
	
	dMatrix3 R;
	
	
	

	//Create physics world
	dInitODE2(0);
	world = dWorldCreate();
	collisionSpace = dHashSpaceCreate(0);
	contactgroup = dJointGroupCreate(0);
	dWorldSetGravity(world, 0, 0, -1.0);
	dWorldSetQuickStepNumIterations(world, 64);

	//Load wall into world


	std::ifstream blenderObj;
	std::string fline;
	std::string token;
	blenderObj.open("G:\\Anthem\\C_files\\wall.obj");

	while (blenderObj >> token) {
		if (token == "v") {
			float v1, v2, v3;
			blenderObj >> v1;
			blenderObj >> v2;
			blenderObj >> v3;
			verticeData.push_back(v1);
			verticeData.push_back(v2);
			verticeData.push_back(v3);

			getline(blenderObj, fline);
		}
		if (token == "vn") {
			//TODO
			getline(blenderObj, fline);
		}
		if (token == "f") {
			getline(blenderObj, fline);
			// + x indicates a step over the found token to search for next token starting at prevToken + tokenLength.
			int firstSlashes = fline.find("//");
			int firstSpace = fline.find(' ', firstSlashes + 2);

			int secondSlashes = fline.find("//", firstSpace + 1);
			int secondSpace = fline.find(' ', secondSlashes + 2);

			int thirdSlashes = fline.find("//", secondSpace + 1);
			//third space should be end of string.
			faceVerticeData.push_back(stoi(fline.substr(0, firstSlashes), NULL, 10));
			faceVerticeData.push_back(stoi(fline.substr(firstSpace + 1, secondSlashes - (firstSpace + 1)), NULL, 10));
			faceVerticeData.push_back(stoi(fline.substr(secondSpace + 1, thirdSlashes - (secondSpace + 1)), NULL, 10));
		}


	}
	blenderObj.close();

	for (int i = 0; i < verticeData.size(); i += 3) {
		odeVertices.push_back(verticeData[i]);
		odeVertices.push_back(verticeData[i + 2]);
		odeVertices.push_back(verticeData[i + 1]);
	}
	for (int i = 0; i < faceVerticeData.size(); i++) {
		faceVerticeData[i] -= 1;
	}
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

	dGeomSetPosition(world_mesh, 0, -25, 0.5);
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
	glCullFace(GL_BACK);

	unsigned int fragmentShader;
	unsigned int vertexShader;
	unsigned int program;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::ifstream vFile;
	std::string line;
	std::stringstream vertSrcStream;
	
	vFile.open("vShader.txt");
	while (getline(vFile, line)) {
		vertSrcStream << line << "\n";
	}
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

	std::ifstream fFile;
	std::stringstream fragSrcStream;

	fFile.open("fShader.txt");
	while (getline(fFile, line)) {
		fragSrcStream << line << "\n";
	}
	std::string  fragSrc = fragSrcStream.str();
	const char * fragString = fragSrc.c_str();
	
	glShaderSource(fragmentShader, 1, &fragString, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "F err LOG: " << infoLog << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
	}


	verts[0] = 1.0f;
	verts[1] = -1.0f;
	verts[2] = 1.0f;

	verts[3] = -1.0f;
	verts[4] = -1.0f;
	verts[5] = -1.0f;

	verts[6] =  1.0f;
	verts[7] = -1.0f;
	verts[8] = -1.0f;

	verts[9] = -1.0f;
	verts[10] = 1.0f;
	verts[11] = -1.0f;

	verts[12] = 1.0f;
	verts[13] = 1.0f;
	verts[14] = 1.0f;
	
	verts[15] = 1.0f;
	verts[16] = 1.0f;
	verts[17] = -1.0f;
	
	verts[18] = -1.0f;
	verts[19] = -1.0f;
	verts[20] =  1.0f;
	
	verts[21] = -1.0f;
	verts[22] =  1.0f;
	verts[23] =  1.0f;

	
	std::vector<float> vertsB = getObjVerts();
	
	/*for (int i = 0; i < vertsB.size(); i++) {
		std::cout << vertsB[i] << std::endl;
	}*/

	std::vector<unsigned int> indicesB = getObjIndices();
	/*for (int i = 0; i < indicesB.size(); i++) {
		std::cout << "IND:" << indicesB[i] << std::endl;
	}*/

	std::vector<float> wallVerts = getWallVerts();
	std::vector<unsigned int> wallIndices = getWallIndices();

	
	
	unsigned int elementBufferObject2;

	glGenVertexArrays(1, &vertexArrayObject2);
	glBindVertexArray(vertexArrayObject2);
	glGenBuffers(1, &vertexBufferObject2);
	glGenBuffers(1, &elementBufferObject2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject2);
	glBufferData(GL_ARRAY_BUFFER, wallVerts.size() * sizeof(float), &wallVerts[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0] , GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, wallIndices.size() * sizeof(unsigned int), &wallIndices[0], GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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
		
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(glm::vec3(10.0f, 5.0f, 10.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 1200.0f / 900.0f, 0.1f, 100.0f);

		int modelLoc = glGetUniformLocation(program, "model");
		int viewLoc = glGetUniformLocation(program, "view");
		int projectionLoc = glGetUniformLocation(program, "projection");

		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));	
		//glBindVertexArray(vertexArrayObject);
		//glDrawElements(GL_TRIANGLES, indicesB.size() , GL_UNSIGNED_INT, 0);
		
		double simstep = 0.0001; // 1ms simulation steps

		dnow = glfwGetTime();
		double elapsedTime = dnow - dlast;
		dlast = dnow;
		
		int nrofsteps = (int)ceilf(elapsedTime / simstep);
		//  fprintf(stderr, "dt=%f, nr of steps = %d\n", dt, nrofsteps);

		for (int i = 0; i < nrofsteps; i++)
		{
			dSpaceCollide(collisionSpace, 0, &nearCallback);
			dWorldQuickStep(world, simstep);
			dJointGroupEmpty(contactgroup);
		}

		const dReal *SPos = dBodyGetPosition(sphbody);
		const dReal *SRot = dBodyGetRotation(sphbody);
		float spos[3] = { SPos[0], SPos[1], SPos[2] };
		float srot[16] = { SRot[0], SRot[1], SRot[2], SRot[3], SRot[4], SRot[5], SRot[6], SRot[7], SRot[8], SRot[9], SRot[10], SRot[11], 0.0f, 0.0f, 0.0f, 0.0f};
		glm::mat4 test = glm::make_mat4(srot);


		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


		for (int i = 0; i < vertexArrayObjects.size(); i++) {
			//std::cout << vertexArrayObjects.size();
			if (vertexArrayObjects[i] == bulletVertexArrayObject) {
				//std::cout << spos[0] << " " << spos[1] << " " << spos[2] << std::endl;
				bulletPosMat = glm::mat4(1.0f);
				bulletPosMat = glm::translate(bulletPosMat, glm::vec3(spos[0], spos[2], spos[1]));
				//bulletPosMat = glm::rotate(bulletPosMat,)
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bulletPosMat));
			}
			else if (vertexArrayObjects[i] == vertexArrayObject2) {
				glm::mat4 id = glm::mat4(1.0f);
				id = glm::translate(id, glm::vec3(0.0f, 0.0f, -25.0f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(id));

			}
			else if (vertexArrayObjects[i] == vertexArrayObject) {
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			}
			glBindVertexArray(vertexArrayObjects[i]);

			glDrawElements(GL_TRIANGLES, vaoSizes[i], GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
