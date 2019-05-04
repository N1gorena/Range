// HolaMundo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <glm-0.9.9.5/glm/glm/glm.hpp>
#include <glm-0.9.9.5/glm/glm/gtc/matrix_transform.hpp>
#include <glm-0.9.9.5/glm/glm/gtc/type_ptr.hpp>

#include <glad.h>
#include <glfw3.h>

std::vector<float> vertexPoints;
std::vector<unsigned int> faceVertices;
std::vector<float> verts(24);

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


void keyStrokeCall(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

std::vector<unsigned int> getObjIndices() {
	std::vector<unsigned int> orderedIndices;
	for (int i = 0; i < faceVertices.size(); i++) {
		orderedIndices.push_back(faceVertices[i]-1);
	}
	return orderedIndices;
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

int main()
{
	std::cout << "Here we go again!" << std::endl;
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

	
	glViewport(0, 0, 1200, 900);
	glfwShowWindow(window);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vertexArrayObject);
		glm::mat4 model = glm::mat4(1.0f);
		//TODO gun follow cursor
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f,1.0f,0.0f));
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(glm::vec3(2.0f, 0.0f, 10.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 1200.0f / 900.0f, 0.1f, 100.0f);

		int modelLoc = glGetUniformLocation(program, "model");
		int viewLoc = glGetUniformLocation(program, "view");
		int projectionLoc = glGetUniformLocation(program, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glDrawElements(GL_TRIANGLES, indicesB.size() , GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
