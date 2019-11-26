/*

	CG Homework2 - Shadow Mapping & Dissolve Effects

	Objective - learning Shadow Implmentation and Dissolve Effects

	Overview:

		1. Render the model with Shadow using shadow mapping

		2. Implement dissolve effect

	!!!IMPORTANT!!! 

	1. Make sure to change the window name to your student ID!
	2. You are allow to use glmDraw this time.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include "../GL/glew.h"
#include "../GL/glut.h"
#include "../shader_lib/shader.h"
#include "glm/glm.h"
#include <vector>

extern "C"
{
	#include "glm_helper.h"
}

/*you may need to do something here
you may use the following struct type to perform your single VBO method,
or you can define/declare multiple VBOs for VAO method.
Please feel free to modify it*/
struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
};
typedef struct Vertex Vertex;
////////
GLuint vert;
GLuint frag;
GLuint program, debug;
GLuint VBO, VAO;
std::vector<Vertex> vertexs;
///////

//no need to modify the following function declarations and gloabal variables
void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
void draw_light_bulb(void);
void camera_light_ball_move();
GLuint loadTexture(char* name, GLfloat width, GLfloat height);

namespace
{
	char *obj_file_dir = "../Resources/Ball.obj";
	char *obj_file_dir2 = "../Resources/bunny.obj";
	char *main_tex_dir = "../Resources/Stone.ppm";
	char *floor_tex_dir = "../Resources/WoodFine.ppm";
	char *plane_file_dir = "../Resources/Plane.obj";
	char *noise_tex_dir = "../Resources/noise.ppm";
	
	GLfloat light_rad = 0.05; //radius of the light bulb
	float eyet = -5.59; //theta in 
	float eyep = 83.2; //phi in degree
	bool mleft = false;
	bool mright = false;
	bool mmiddle = false;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool lforward = false;
	bool lbackward = false;
	bool lleft = false;
	bool lright = false;
	bool lup = false;
	bool ldown = false;
	bool bforward = false;
	bool bbackward = false;
	bool bleft = false;
	bool bright = false;
	bool bup = false;
	bool bdown = false;
	bool bx = false;
	bool by = false;
	bool bz = false;
	bool brx = false;
	bool bry = false;
	bool brz = false;

	int mousex = 0;
	int mousey = 0;
}

// You can modify the moving/rotating speed if it's too fast/slow for you
const float speed = 0.03; // camera / light / ball moving speed
const float rotation_speed = 0.05; // ball rotating speed

//you may need to use some of the following variables in your program 

// No need for model texture, 'cause glmModel() has already loaded it for you.
// To use the texture, check glmModel documentation.
GLuint mainTextureID; // TA has already loaded this texture for you
GLuint floorTextureID;
GLuint noiseTextureID;

GLMmodel *model; // TA has already loaded the model for you(!but you still need to convert it to VBO(s)!)
GLMmodel *planeModel;
GLMmodel *subModel;

float eyex = -3.291;
float eyey = 1.57;
float eyez = 11.89;

GLfloat light_pos[] = { 1.1, 3.5, 1.3 };
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };
GLfloat plane_pos[] = { 0.0, -5.0, 0.0 };
GLfloat plane_rot[] = { 0.0, 0.0, 0.0 };
GLfloat subModel_pos[] = { -2.295, -5.0, -2.0 };
GLfloat subModel_rot[] = { 0.0, 0.0, 0.0 };

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	// remember to replace "YourStudentID" with your own student ID
	glutCreateWindow("CG_HW2_0866001");
	glutReshapeWindow(512, 512);

	glewInit();

	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutMainLoop();

	glmDelete(model);
	return 0;
}

void init(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);

	mainTextureID = loadTexture(main_tex_dir, 1024, 1024);
	floorTextureID = loadTexture(floor_tex_dir, 512, 512);
	noiseTextureID = loadTexture(noise_tex_dir, 320, 320);

	model = glmReadOBJ(obj_file_dir);
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	planeModel = glmReadOBJ(plane_file_dir);
	glmFacetNormals(planeModel);
	glmVertexNormals(planeModel, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(planeModel);	

	subModel = glmReadOBJ(obj_file_dir2);
	glmFacetNormals(subModel);
	glmVertexNormals(subModel, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(subModel);

	// you may need to do something here(create shaders/program(s) and create vbo(s)/vao from GLMmodel model)

	// APIs for creating shaders and creating shader programs have been done by TAs
	// following is an example for creating a shader program using given vertex shader and fragment shader
	
	vert = createShader("Shaders/shadow.vert", "vertex");
	frag = createShader("Shaders/shadow.frag", "fragment");
	program = createProgram(vert, frag);
	vert = createShader("Shaders/debug.vert", "vertex");
	frag = createShader("Shaders/debug.frag", "fragment");
	debug = createProgram(vert, frag);
	//get data from model
	GLMgroup *tgroup = model->groups;
	GLMtriangle work;
	for (int t = 0; t < tgroup->numtriangles; t++) {
		work = model->triangles[tgroup->triangles[t]];
		for (int v = 0; v < 3; v++) {
			Vertex workV;
			memcpy(workV.position, &model->vertices[work.vindices[v] * 3], sizeof(GLfloat) * 3);
			memcpy(workV.normal, &model->normals[work.nindices[v] * 3], sizeof(GLfloat) * 3);
			memcpy(workV.texcoord, &model->texcoords[work.tindices[v] * 2], sizeof(GLfloat) * 2);
			vertexs.push_back(workV);
		}
	}

	glGenVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexs.size(), vertexs.data(), GL_STATIC_DRAW);
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(0);
	//normal
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
	//glEnableVertexAttribArray(1);
	//texture-vector
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, texcoord));
	//glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//you may need to do something here(declare some local variables you need and maybe load Model matrix here...)
	GLfloat pro[16];
	GLfloat near_plane = 1.0f, far_plane = 7.5f;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	//glOrtho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glGetFloatv(GL_PROJECTION_MATRIX, pro);
	glPopMatrix();
	GLfloat mtx[16] = { eyex,eyey,eyez,eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180),eyey + sin(eyet*M_PI / 180),eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),0.0,1.0,0.0 };

	//please try not to modify the following block of code(you can but you are not supposed to)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		eyex, 
		eyey, 
		eyez,
		eyex+cos(eyet*M_PI/180)*cos(eyep*M_PI / 180), 
		eyey+sin(eyet*M_PI / 180), 
		eyez-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
		0.0,
		1.0,
		0.0);
	glPushMatrix();
		glColor3f(1, 1, 1);
		draw_light_bulb();
	glPopMatrix();
	
	glPushMatrix();
		glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
		glRotatef(ball_rot[0], 1, 0, 0);
		glRotatef(ball_rot[1], 0, 1, 0);
		glRotatef(ball_rot[2], 0, 0, 1);
		glColor3f(1, 1, 1);
		

		// you may need to do something here(pass uniform variable(s) to shader and render the model)
		//glmDraw(model, GLM_TEXTURE);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(plane_pos[0], plane_pos[1], plane_pos[2]);
		glRotatef(plane_rot[0], 1, 0, 0);
		glRotatef(plane_rot[1], 0, 1, 0);
		glRotatef(plane_rot[2], 0, 0, 1);
		glColor3f(1, 1, 1);
		// you may need to do something here(pass uniform variable(s) to shader and render the model)
		glmDraw(planeModel, GLM_TEXTURE);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(subModel_pos[0], subModel_pos[1], subModel_pos[2]);
	glRotatef(subModel_rot[0], 1, 0, 0);
	glRotatef(subModel_rot[1], 0, 1, 0);
	glRotatef(subModel_rot[2], 0, 0, 1);	
	glBindTexture(GL_TEXTURE0, mainTextureID);
	// you may need to do something here(pass uniform variable(s) to shader and render the model)
	glmDraw(subModel, GLM_TEXTURE);

	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	const GLuint SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;

	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//ConfigureShaderAndMatrices();
	glUseProgram(program);
	//GLfloat mtx[16] = { light_pos[0], light_pos[1], light_pos[2], 0, 0, 0, 0, 0, 0.0, 1.0, 0.0, 0 };
	glUniformMatrix4fv(glGetUniformLocation(program, "lightView"), 1, GL_FALSE, mtx);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
	glRotatef(ball_rot[0], 1, 0, 0);
	glRotatef(ball_rot[1], 0, 1, 0);
	glRotatef(ball_rot[2], 0, 0, 1);
	glGetFloatv(GL_MODELVIEW_MATRIX, mtx);
	glPopMatrix();
	glUniformMatrix4fv(glGetUniformLocation(program, "M"), 1, GL_FALSE, mtx);
	glGetFloatv(GL_PROJECTION_MATRIX, pro);
	glUniformMatrix4fv(glGetUniformLocation(program, "P"), 1, GL_FALSE, pro);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertexs.size());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	//////////
	//glUseProgram(debug);
	//glViewport(0, 0, 512, 512);
	////glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glUniform1f(glGetUniformLocation(debug, "near_plane"), near_plane);
	//glUniform1f(glGetUniformLocation(debug, "far_plane"), far_plane);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//
	//GLuint quadVAO = 0;
	//GLuint quadVBO;
	//if (quadVAO == 0)
	//{
	//	GLfloat quadVertices[] = {
	//		// Positions        // Texture Coords
	//		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
	//		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	//		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	//		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	//	};
	//	// Setup plane VAO
	//	glGenVertexArrays(1, &quadVAO);
	//	glGenBuffers(1, &quadVBO);
	//	glBindVertexArray(quadVAO);
	//	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	//	glEnableVertexAttribArray(0);
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	//}
	//glBindVertexArray(quadVAO);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//glBindVertexArray(0);
	//glUseProgram(0);
	glPopMatrix();

	glutSwapBuffers();
	camera_light_ball_move();
}

// please implement mode increase/decrease dissolve threshold in case '-' and case '=' (lowercase)
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
	{	// ESC
		break;
	}
	case '-': // increase dissolve threshold
	{
		// you may need to do somting here
		break;
	}
	case '=': // decrease dissolve threshold
	{
		// you may need to do somting here
		break;
	}
	case 'd':
	{
		right = true;
		break;
	}
	case 'a':
	{
		left = true;
		break;
	}
	case 'w':
	{
		forward = true;
		break;
	}
	case 's':
	{
		backward = true;
		break;
	}
	case 'q':
	{
		up = true;
		break;
	}
	case 'e':
	{
		down = true;
		break;
	}
	case 't':
	{
		lforward = true;
		break;
	}
	case 'g':
	{
		lbackward = true;
		break;
	}
	case 'h':
	{
		lright = true;
		break;
	}
	case 'f':
	{
		lleft = true;
		break;
	}
	case 'r':
	{
		lup = true;
		break;
	}
	case 'y':
	{
		ldown = true;
		break;
	}
	case 'i':
	{
		bforward = true;
		break;
	}
	case 'k':
	{
		bbackward = true;
		break;
	}
	case 'l':
	{
		bright = true;
		break;
	}
	case 'j':
	{
		bleft = true;
		break;
	}
	case 'u':
	{
		bup = true;
		break;
	}
	case 'o':
	{
		bdown = true;
		break;
	}
	case '7':
	{
		bx = true;
		break;
	}
	case '8':
	{
		by = true;
		break;
	}
	case '9':
	{
		bz = true;
		break;
	}
	case '4':
	{
		brx = true;
		break;
	}
	case '5':
	{
		bry = true;
		break;
	}
	case '6':
	{
		brz = true;
		break;
	}

	//special function key
	case 'z'://move light source to front of camera
	{
		light_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180);
		light_pos[1] = eyey + sin(eyet*M_PI / 180);
		light_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180);
		break;
	}
	case 'x'://move ball to front of camera
	{
		ball_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) * 3;
		ball_pos[1] = eyey + sin(eyet*M_PI / 180) * 5;
		ball_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180) * 3;
		break;
	}
	case 'c'://reset all pose
	{
		light_pos[0] = 1.1;
		light_pos[1] = 3.5;
		light_pos[2] = 1.3;
		ball_pos[0] = 0;
		ball_pos[1] = 0;
		ball_pos[2] = 0;
		ball_rot[0] = 0;
		ball_rot[1] = 0;
		ball_rot[2] = 0;
		eyex = -3.291;
		eyey = 1.57;
		eyez = 11.89;
		eyet = -5.59; //theta in degree
		eyep = 83.2; //phi in degree
		break;
	}
	default:
	{
		break;
	}
	}
}

//no need to modify the following functions
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	if (mleft)
	{
		eyep -= (x-mousex)*0.1;
		eyet -= (y - mousey)*0.12;
		if (eyet > 89.9)
			eyet = 89.9;
		else if (eyet < -89.9)
			eyet = -89.9;
		if (eyep > 360)
			eyep -= 360;
		else if (eyep < 0)
			eyep += 360;
	}
	mousex = x;
	mousey = y;
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN && !mright && !mmiddle)
		{
			mleft = true;
			mousex = x;
			mousey = y;
		}
		else
			mleft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mmiddle)
		{
			mright = true;
			mousex = x;
			mousey = y;
		}
		else
			mright = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mright)
		{
			mmiddle = true;
			mousex = x;
			mousey = y;
		}
		else
			mmiddle = false;
	}
}

void camera_light_ball_move()
{
	GLfloat dx = 0, dy = 0, dz=0;
	if(left|| right || forward || backward || up || down)
	{ 
		if (left)
			dx = -speed;
		else if (right)
			dx = speed;
		if (forward)
			dy = speed;
		else if (backward)
			dy = -speed;
		eyex += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		eyey += dy*sin(eyet*M_PI / 180);
		eyez += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (up)
			eyey += speed;
		else if (down)
			eyey -= speed;
	}
	if(lleft || lright || lforward || lbackward || lup || ldown)
	{
		dx = 0;
		dy = 0;
		if (lleft)
			dx = -speed;
		else if (lright)
			dx = speed;
		if (lforward)
			dy = speed;
		else if (lbackward)
			dy = -speed;
		light_pos[0] += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		light_pos[1] += dy*sin(eyet*M_PI / 180);
		light_pos[2] += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (lup)
			light_pos[1] += speed;
		else if(ldown)
			light_pos[1] -= speed;
	}
	if (bleft || bright || bforward || bbackward || bup || bdown)
	{
		dx = 0;
		dy = 0;
		if (bleft)
			dx = -speed;
		else if (bright)
			dx = speed;
		if (bforward)
			dy = speed;
		else if (bbackward)
			dy = -speed;
		ball_pos[0] += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		ball_pos[1] += dy*sin(eyet*M_PI / 180);
		ball_pos[2] += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (bup)
			ball_pos[1] += speed;
		else if (bdown)
			ball_pos[1] -= speed;
	}
	if(bx||by||bz || brx || bry || brz)
	{
		dx = 0;
		dy = 0;
		dz = 0;
		if (bx)
			dx = -rotation_speed;
		else if (brx)
			dx = rotation_speed;
		if (by)
			dy = rotation_speed;
		else if (bry)
			dy = -rotation_speed;
		if (bz)
			dz = rotation_speed;
		else if (brz)
			dz = -rotation_speed;
		ball_rot[0] += dx;
		ball_rot[1] += dy;
		ball_rot[2] += dz;
	}
}

void draw_light_bulb()
{
	GLUquadric *quad;
	quad = gluNewQuadric();
	glPushMatrix();
	glColor3f(0.4, 0.5, 0);
	glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
	gluSphere(quad, light_rad, 40, 20);
	glPopMatrix();
}

void keyboardup(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
	{
		right =false;
		break;
	}
	case 'a':
	{
		left = false;
		break;
	}
	case 'w':
	{
		forward = false;
		break;
	}
	case 's':
	{
		backward = false;
		break;
	}
	case 'q':
	{
		up = false;
		break;
	}
	case 'e':
	{
		down = false;
		break;
	}
	case 't':
	{
		lforward = false;
		break;
	}
	case 'g':
	{
		lbackward = false;
		break;
	}
	case 'h':
	{
		lright = false;
		break;
	}
	case 'f':
	{
		lleft = false;
		break;
	}
	case 'r':
	{
		lup = false;
		break;
	}
	case 'y':
	{
		ldown = false;
		break;
	}
	case 'i':
	{
		bforward = false;
		break;
	}
	case 'k':
	{
		bbackward = false;
		break;
	}
	case 'l':
	{
		bright = false;
		break;
	}
	case 'j':
	{
		bleft = false;
		break;
	}
	case 'u':
	{
		bup = false;
		break;
	}
	case 'o':
	{
		bdown = false;
		break;
	}
	case '7':
	{
		bx = false;
		break;
	}
	case '8':
	{
		by = false;
		break;
	}
	case '9':
	{
		bz = false;
		break;
	}
	case '4':
	{
		brx = false;
		break;
	}
	case '5':
	{
		bry = false;
		break;
	}
	case '6':
	{
		brz = false;
		break;
	}

	default:
	{
		break;
	}
	}
}

void idle(void)
{
	subModel_rot[1] += 1;
	glutPostRedisplay();
}

GLuint loadTexture(char* name, GLfloat width, GLfloat height)
{
	return glmLoadTexture(name, false, true, true, true, &width, &height);
}
