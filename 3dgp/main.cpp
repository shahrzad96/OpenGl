#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;

// 3D Models
C3dglTerrain terrain, road, river;
C3dglBitmap bm;
C3dglBitmap bm1;
C3dglBitmap bm2;
C3dglBitmap bm3;
GLuint idTexGrass;
GLuint idTexStreet;
GLuint idTexWater;
GLuint idTexWood;
GLuint idTexNone;
GLuint idTexNone1;
GLuint idTexNone2;
C3dglSkyBox skyBox;
C3dglModel cabin;

// GLSL Objects (Shader Program)
C3dglProgram Program;

// camera position (for first person type camera navigation)
float matrixView[16];		// The View Matrix
float angleTilt = 0;		// Tilt Angle
float deltaX = 0, deltaY = 0, deltaZ = 0;	// Camera movement values

//boolean variable for flickering fire
bool pointOn = false;
int time = 0;

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// load your 3D models here!
	if (!terrain.loadHeightmap("models\\island1.png", 10)) return false;
	if (!road.loadHeightmap("models\\islandRoad.png", 10)) return false;
	if (!river.loadHeightmap("models\\river.png", 10)) return false;
	if (!cabin.load("models\\WoodenCabinObj\\WoodenCabinObj.obj")) return false;

	//load skyBox
	if (!skyBox.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;

	//setting up fog
	Program.SendUniform("fogColour", 0.6, 0.6, 0.6);
	Program.SendUniform("fogDensity", 0.1);

	//load bitmap for terrain texture
	//terrain texture
	bm.Load("models/grass.png", GL_RGBA);
	if (!bm.GetBits()) return false;

	//setup texture buffer
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	//street texture
	bm1.Load("models/street.png", GL_RGBA);
	if (!bm1.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(2, &idTexStreet);
	glBindTexture(GL_TEXTURE_2D, idTexStreet);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm1.GetWidth(), bm1.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm1.GetBits());

	//river's texture
	bm2.Load("models/water.jpg", GL_RGBA);
	if (!bm2.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0 + 2);
	glGenTextures(3, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm2.GetWidth(), bm2.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm2.GetBits());

	//cabin's texture
	bm3.Load("models/WoodenCabinObj/WoodCabinDif.jpg", GL_RGBA);
	if (!bm3.GetBits()) return false;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm3.GetWidth(), bm3.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm3.GetBits());


	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);
	Program.SendUniform("texture1", 1);
	Program.SendUniform("texture2", 2);

	// setup ambient light
	Program.SendUniform("lightAmbient.on", 1);
	Program.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	
	// setup directional light
	Program.SendUniform("lightDir.on", 1);
	Program.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	Program.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);

	//setup point light
	Program.SendUniform("lightPoint1.on", 1);
	Program.SendUniform("lightPoint1.position", 25.17, 3.7, 30.6);
	Program.SendUniform("lightPoint1.diffuse", 0.3, 0.12, 0.00);
	Program.SendUniform("lightPoint1.specular", 0.3, 0.12, 0.00);
	pointOn = true;

	// Initialise the View Matrix (initial position for the first-person camera)
	glMatrixMode(GL_MODELVIEW);
	angleTilt = 15;
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	gluLookAt(26.0, 1.0, 34.5, 
	          4.0, 1.5, 0.0,
	          0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void render()
{
	float matrix[16];

	// clear screen and buffers
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);					// switch tilt off
	glTranslatef(deltaX, deltaY, deltaZ);			// animate camera motion (controlled by WASD keys)
	glRotatef(-angleTilt, 1, 0, 0);					// switch tilt on
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	// set the camera height above the ground
	gluInvertMatrix(matrixView, matrix);
	glTranslatef(0, -terrain.getInterpolatedHeight(matrix[12], matrix[14]), 0);

	// setup View Matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	Program.SendUniform("matrixView", matrix);

	//adjusting point light's flicker (imitating fire)
	if (pointOn == true && time == 28) {
		Program.SendUniform("lightPoint1.on", 1);
		Program.SendUniform("lightPoint1.position", 25.17, 3.7, 30.6);
		Program.SendUniform("lightPoint1.diffuse", 0.3, 0.1647, 0.000);
		Program.SendUniform("lightPoint1.specular", 0.3, 0.1647, 0.000);
		pointOn = false;
		time = 0;
	}
	else if(pointOn == false && time == 28){
		Program.SendUniform("lightPoint1.on", 1);
		Program.SendUniform("lightPoint1.position", 25.17, 3.7, 30.6);
		Program.SendUniform("lightPoint1.diffuse", 0.3, 0.12, 0.00);
		Program.SendUniform("lightPoint1.specular", 0.3, 0.12, 0.00);
		pointOn = true;
		time = 0;
	}
	//incrementing time
	time++;

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes1[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes1);

	//render skyBox
	Program.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
	skyBox.render();

	// setup materials for the terrain
	Program.SendUniform("materialDiffuse", 0.2f, 0.8f, 0.2f);	// grassy green
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);		// full power (note: ambient light is extremely dim)

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	// render the terrain
	glPushMatrix();
	float modelviewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
	Program.SendUniform("matrixModelView", modelviewMatrix);
	terrain.render();
	glPopMatrix();	

	// none (simple-white) texture
	glGenTextures(1, &idTexNone1);
	glBindTexture(GL_TEXTURE_2D, idTexNone1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	// setup material for the road
	Program.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, idTexStreet);

	// render the road
	glPushMatrix();
	glTranslatef(27.86f, 0.05f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
	Program.SendUniform("matrixModelView", modelviewMatrix);
	road.render();
	glPopMatrix();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	//Cabin
	glPushMatrix();
	glTranslatef(24.6, 3.74, 30.0); 
	glScalef(0.04f, 0.04f, 0.04f);
	cabin.render();
	glPopMatrix();

	// none (simple-white) texture
	glGenTextures(1, &idTexNone2);
	glBindTexture(GL_TEXTURE_2D, idTexNone2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes2[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes2);

	//animation uniform vars
	Program.SendUniform("time", (float)GetTickCount() / 1000.0f);
	Program.SendUniform("speedX", 0.1f);
	Program.SendUniform("speedY", 0.1f);

	// setup material for the river
	Program.SendUniform("materialDiffuse", 0.0f, 0.1f, 1.0f);

	//river's water texture
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, idTexWater);

	// render the river
	glPushMatrix();
	glTranslatef(19.05f, 0.05f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
	Program.SendUniform("matrixModelView", modelviewMatrix);
	river.render();
	glPopMatrix();

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	// find screen aspect ratio
	float ratio = w * 1.0f / h;      // we hope that h is not zero

	// setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(60.0f, ratio, 0.02f, 1000.0f);

	float matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, matrix);
	Program.SendUniform("matrixProjection", matrix);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': deltaZ = max(deltaZ * 1.05f, 0.01f); break;
	case 's': deltaZ = min(deltaZ * 1.05f, -0.01f); break;
	case 'a': deltaX = max(deltaX * 1.05f, 0.01f); break;
	case 'd': deltaX = min(deltaX * 1.05f, -0.01f); break;
	case 'e': deltaY = max(deltaY * 1.05f, 0.01f); break;
	case 'q': deltaY = min(deltaY * 1.05f, -0.01f); break;
	}
	// speed limit
	deltaX = max(-0.15f, min(0.15f, deltaX));
	deltaY = max(-0.15f, min(0.15f, deltaY));
	deltaZ = max(-0.15f, min(0.15f, deltaZ));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': deltaZ = 0; break;
	case 'a':
	case 'd': deltaX = 0; break;
	case 'q':
	case 'e': deltaY = 0; break;
	case ' ': deltaY = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (state == GLUT_DOWN)
	{
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		glutWarpPointer(cx, cy);
	}
	else
		glutSetCursor(GLUT_CURSOR_INHERIT);
}

// handle mouse move
void onMotion(int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	if (x == cx && y == cy)
		return;	// caused by glutWarpPointer

	float amp = 0.25;
	float deltaTilt = amp * (y - cy);
	float deltaPan = amp * (x - cx);

	glutWarpPointer(cx, cy);

	// handle camera tilt (mouse move up & down)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(deltaTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	angleTilt += deltaTilt;

	// handle camera pan (mouse move left & right)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	glRotatef(deltaPan, 0, 1, 0);
	glRotatef(-angleTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}
