//#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <GL\glew.h>
#include <GL\freeglut.h>

#include <vector>



typedef struct s_Vertex
{
	float x;
	float y;
	float z;

	s_Vertex() {}

	s_Vertex(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	float operator[](std::size_t idx) {
		switch (idx)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			_ASSERT(false && "wrong index with MyVertex");
			return -1;
		}
	}

	operator const float*() const
	{
		return &(x);
	}
} MyVertex;

std::ostream& operator<<(std::ostream& os, const MyVertex& v)
{
	os << "(" << v.x << "," << v.y << "," << v.z << ")";
	return os;
}

GLuint VBO;	  //a handle for vertex buffer
GLuint IBO;	  //a handle for array buffer
MyVertex* Vertices = nullptr;	//vertex data
int* Indices = nullptr;		    //index data
int n_indices = 0;				//number of indices
//used to store values of mouse coordinates on click
int OldX, OldY;
//stores arbitrary values for the rotation
double AngleX, AngleY;
// zoom value defines the xyz coordinate of the camera to zoom
float ZoomVal = 2;

void MenuValue(int option) {
	if (option == 1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (option == 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glutPostRedisplay();
}

void Zoom(unsigned char key, int x, int y) {
	if (key == 61||key=='s') {					   //I don't really know which key is 61, so I put in my own control
		ZoomVal -= .05;
	}
	else if (key == 93 || key == 'w') {
		ZoomVal += .05;
	}
	glutPostRedisplay();
}

void MouseClick(int button, int state, int x, int y) {
	//if the left button is clicked the coordinates are saved for use in rotation
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		OldX = x;
		OldY = y;
	}
}

void init(void) {
	//Set background color 
	glClearColor(1.0, 1.0, 1.0, 0.0);

	//Initialize lighting 
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	//Initialize camera 
	glMatrixMode(GL_PROJECTION);
	gluPerspective(50, 1, 0.1, 10);
	glMatrixMode(GL_MODELVIEW);

	//Initialize Menu and options
	glutCreateMenu(MenuValue);
	glutAddMenuEntry("Wireframe", 1);
	glutAddMenuEntry("Solid", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		//default to display wireframe
	//Zooming in and out
	glutKeyboardFunc(Zoom);
}

void display(void) {
	//Print OpenGL errors 
	GLenum err_code;
	do {
		err_code = glGetError();
		if (err_code != GL_NO_ERROR)
			printf("Error: %s\n", gluErrorString(err_code));
	} while (err_code != GL_NO_ERROR);

	//Clear buffer data 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set camera 
	glLoadIdentity();
	gluLookAt(ZoomVal, ZoomVal, ZoomVal, 0, 0, 0, 0, 1, 0);
	glRotated(AngleX, 0, 1, 0);
	glRotated(AngleY, 1, 0, 0);

	//Set light position 
	GLfloat light_pos[] = { 2, 1, 0, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	//Display face - add your code here
	glEnableVertexAttribArray(0);										
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);	  //draw wireframe with index buffer

	glDisableVertexAttribArray(0);

	glutSwapBuffers();


	//Flush data 
	glFlush();
}

void RotateObject(int x, int y) {
	double diffX, diffY;
	//calculate the difference of old x value to current
	diffX = x - OldX;
	diffY = y - OldY;
	//arbitrarily scales the difference
	diffX = diffX / 2;
	diffY = diffY / 2;
	//creates and alters the angle to rotate by
	AngleX += diffX;
	AngleY += diffY;
	//reassigns x and y value to be continuously used over and over
	OldX = x;
	OldY = y;
	glutPostRedisplay();
}

MyVertex ParseVertex(std::string& line)				  //get vertex data from line
{
	float coord[3];
	int length = line.length();
	int begin = 0;
	int end = line.find("f");
	for (int i = 0; i < 3; i++)
	{
		coord[i] = atof(line.substr(begin, end).c_str());
		length = length - end - 1;					//length of the other part
		line = line.substr(end + 1, length);
		begin = line.find(",")+1;
		end = line.find("f");
	}
	return MyVertex(coord[0], coord[1], coord[2]);
}

void CreateVertexBuffer()
{
	int n_vertices = 0;
	
	//std::vector<MyVertex> vertices;
	std::string line;
	std::ifstream myfile("face-vertices.txt");
	if (myfile.is_open())
	{
		while (getline(myfile, line))	 //just count the number of vertices
		{
			n_vertices++;
		}
		Vertices=new MyVertex[n_vertices];
		myfile.clear();
		myfile.seekg(0);
		int i = 0;
		while (getline(myfile, line))
		{
			Vertices[i] = ParseVertex(line);
			i++;
		}
	}
	myfile.close();

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices)*n_vertices, Vertices, GL_STATIC_DRAW);
}

void CreateIndexBuffer()
{
	std::string line;
	std::ifstream myfile("face-index.txt");
	if (myfile.is_open())
	{
		while (getline(myfile, line))		   //count number of indices
		{
			n_indices+=3;
		}
		Indices = new int[n_indices];
		myfile.clear();
		myfile.seekg(0);
		int i = 0;
		while (getline(myfile, line))
		{
			int length = line.length();
			int begin = 0;
			int end = line.find(",");
			for (int j = 0; j < 3; j++)
			{
				Indices[i] = atoi(line.substr(begin, end).c_str());
				i++;
				length = length - end - 1;					//length of the other part
				line = line.substr(end + 1, length);
				end = line.find(",");
				if (end == std::string::npos)
					end = -1;
			}
		}
	}
	myfile.close();

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*n_indices, Indices, GL_STATIC_DRAW);
}

int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Assignment 2");

	init();
	glutDisplayFunc(display);

	//Rotational
	glutMotionFunc(RotateObject);
	glutMouseFunc(MouseClick);

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	glutMainLoop();

	if (Vertices)
		delete[] Vertices;

	if (Indices)
		delete[] Indices;
	return 0;
}