// Program : Patrol Solution

#include <stdlib.h>
#include <windows.h>
#include <time.h>
//#include <math.h>
#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GLFw/glfw3.h>
#include <ft2build.h>
#include "DiningTable.h"

#include FT_FREETYPE_H
using namespace std;

#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")



#pragma region BORING SHIT
void Render(GLFWwindow* window);

int RandomInteger(int lowerLimit, int upperLimit)
{
	return rand() % (upperLimit - lowerLimit + 1) + lowerLimit;
}

// long integer to string
string itos(const long value)
{
	ostringstream buffer;
	buffer << value;
	return buffer.str();
}



float GetDistance(float x1, float y1, float x2, float y2) { return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); } // OK

// Within range
bool Detect(MyVector pos1, MyVector pos2, float radius1, float radius2)
{
	bool detect = false;
	float totalRadius = radius1 + radius2;
	float distance = GetDistance(pos1.x, pos1.y, pos2.x, pos2.y);
	if (distance <= totalRadius) detect = true;
	return detect;
}

void RenderCircle(GLfloat x, GLfloat y, GLfloat radius, GLfloat r, GLfloat g, GLfloat b)
{
	int n = 360;
	glColor3f(r, g, b);
	glBegin(GL_POINTS);
	for (int i = 0; i <= n; i++)
	{
		float angle = (float)(i * (2.0 * 3.14159 / n));
		glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
	}
	glEnd();
}

void RenderFillCircle(GLfloat x, GLfloat y, GLfloat radius, GLfloat r, GLfloat g, GLfloat b)
{
	int n = 360;
	glColor3f(r, g, b);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	for (int i = 0; i <= n; i++)
	{
		float angle = (float)(i * (2.0 * 3.14159 / n));
		glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
	}
	glEnd();
}

void RenderRectangle(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat r, GLfloat g, GLfloat b)
{
	glColor3f(r, g, b);
	glBegin(GL_POLYGON);

	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);

	glEnd();
}

// SHADERS //////////////////////////////////////////////////////////////////
const char *VERTEX_SHADER = ""
"#version 410 core\n"
"in vec4 in_Position;\n"
"out vec2 texCoords;\n"
"void main(void) {\n"
"    gl_Position = vec4(in_Position.xy, 0, 1);\n"
"    texCoords = in_Position.zw;\n"
"}\n";

const char *FRAGMENT_SHADER = ""
"#version 410 core\n"
"precision highp float;\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"in vec2 texCoords;\n"
"out vec4 fragColor;\n"
"void main(void) {\n"
"    fragColor = vec4(1, 1, 1, texture(tex, texCoords).r) * color;\n"
"}\n";

GLuint texUniform, colorUniform;
GLuint texture{ 0 }, sampler{ 0 };
GLuint vbo{ 0 }, vao{ 0 };
GLuint vs{ 0 }, fs{ 0 }, program{ 0 };

///////////////////////////////////////////////////////////////////

static void ErrorCallBack(int error, const char*description)
{
	fputs(description, stderr);
}

float width, height;
GLFWwindow* window;
void Render(GLFWwindow* window);

static void ResizeCallBack(GLFWwindow *window, int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)w / (float)h, 0, 100);
	glMatrixMode(GL_MODELVIEW);
}

// Free Type //////////////////////////////////////////////////////
FT_Library ft_lib{ nullptr };
FT_Face face{ nullptr };

void RenderText(const string str, FT_Face face, float x, float y, float _sx, float _sy)
{
	float sx = _sx / width;
	float sy = _sy / height;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	const FT_GlyphSlot glyph = face->glyph;
	for (auto c : str) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
			continue;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
			glyph->bitmap.width, glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

		const float vx = x + glyph->bitmap_left * sx;
		const float vy = y + glyph->bitmap_top * sy;
		const float w = glyph->bitmap.width * sx;
		const float h = glyph->bitmap.rows * sy;

		struct {
			float x, y, s, t;
		} data[6] = {
			{ vx, vy, 0, 0 },
			{ vx, vy - h, 0, 1 },
			{ vx + w, vy, 1, 0 },
			{ vx + w, vy, 1, 0 },
			{ vx, vy - h, 0, 1 },
			{ vx + w, vy - h, 1, 1 }
		};
		glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (glyph->advance.x >> 6) * sx;
		y += (glyph->advance.y >> 6) * sy;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
///////////////////////////////////////////////////////////////////

void Cleanup()
{
	FT_Done_Face(face);
	FT_Done_FreeType(ft_lib);
	glDeleteTextures(1, &texture);
	glDeleteSamplers(1, &sampler);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(program);
}

void DoExit()
{
	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

#pragma endregion

bool arrived;

// Time ??

clock_t t1, t2;

// Vector of Table positions
vector<CDiningTable*>tables;
vector<MyVector> seats;
MyVector bigtable;
MyVector tableCaller;

// AI radius
const float AI_radius = 0.25f;
const float waypoint_radius = 0.35f;
const float proximity = 0.2f;

bool autorun = false;

#pragma region AI_STATES
// Chef related
enum CHEF_STATE
{
	E_CHEF_COOK,
	E_CHEF_SERVE,
	E_CHEF_WAIT,
	E_CHEF_MAX
};
CHEF_STATE chefState;
const float chefSpeed = 0.02f;
const float cookTime = 5.0f;
MyVector chefPos;
MyVector chefSpawn;
MyVector chefStation;
vector<MyVector> chefWaypoints;
bool isAtStation;
bool chefArrived;

//List of orders
/*
	Pass in table numbers for the order
*/
vector<unsigned int> ListOfOrders;

MyVector cookingStation_1 = MyVector(-5.f, -2.5f);
// bool isCooking_1 = false;

// Waiter related
enum WAITER_STATE
{
	E_WAITER_SERVE,
	E_WAITER_IDLE,
	E_WAITER_PICKUP,
	E_WAITER_PICKUPCUSTOMER,
	E_WAITER_MOVE,
	E_WAITER_MAX
};
WAITER_STATE waiterOneState;
WAITER_STATE waiterTwoState;
WAITER_STATE waiterThreeState;
const float waiterSpeed = 0.02f;
MyVector waiterOnePos;
MyVector waiterTwoPos;
MyVector waiterThreePos;
bool foodReady;
bool availableCustomers;
bool customerPickup;
MyVector waiterOneSpawn = MyVector(-9.f, 0.5f);
MyVector waiterTwoSpawn = MyVector(-7.5f, 0.5f);
MyVector waiterThreeSpawn = MyVector(-6.f, 0.5f);
bool waiter1_isBusy;
bool waiter2_isBusy;
bool waiter3_isBusy;
vector <MyVector> waiterOneWaypoints;
vector <MyVector> waiterTwoWaypoints;
vector <MyVector> waiterThreeWaypoints;

unsigned int tableNumber = 0;

// Customer related
enum CUS_STATE
{
	E_CUSTOMER_IDLE,
	E_CUSTOMER_QUEUE,
	E_CUSTOMER_MOVE,
	E_CUSTOMER_ORDER,
	E_CUSTOMER_EAT,
	E_CUSTOMER_LEAVE,
	E_CUSTOMER_MAX
};
CUS_STATE customerState;
const float customerSpeed = 0.02f;
const float orderTime = 3.f;
const float eatTime = 10.f;
bool customerSeated;
bool customerLine;
bool customerInLine;
bool backToSpawn;
MyVector customerPos;
const MyVector defaultSpawn = MyVector(12.f, 0.f);
const MyVector defaultLine = MyVector(8.5f, 3.5f);
clock_t orderStart, orderEnd, eatStart, eatEnd;
float prob = 30.f;

// Caller related
enum CALLER_STATE
{
	E_CALLER_IDLE,
	E_CALLER_CHECK,
	E_CALLER_WAIT,
	E_CALLER_CALL,
	E_CALLER_MAX
};
CALLER_STATE callerState;
const MyVector callerPos = MyVector(6.25f, 3.5f);

#pragma endregion

static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	//if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	//{
	//	foodReady = true;
	//}
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
	{
		customerLine = true;
	}
}

void SimulationInit()
{
	chefStation = MyVector(-5.f, -2.5f);
	srand((unsigned)time(NULL));
	float seat_offset = 1.25f;
	float big_seat_offset_diagonal = 2.f;
	float big_seat_offset_straight = 2.75f;

#pragma region Furniture positions

	// Table positions
	CDiningTable *table_1 = new CDiningTable();
	table_1->SetPos(MyVector(-7.5f, 3.5f));
	table_1->SetStatus(false);

	CDiningTable *table_2 = new CDiningTable();
	table_2->SetPos(MyVector(-3.f, 3.5f));
	table_2->SetStatus(false);

	CDiningTable *table_3 = new CDiningTable();
	table_3->SetPos(MyVector(1.5f, 3.5f));
	table_3->SetStatus(false);

	tables.push_back(table_1);
	tables.push_back(table_2);
	tables.push_back(table_3);
	bigtable.SetPosition(1.5f, -2.f);

	// Table 1 seat positions
	seats.push_back(MyVector(tables[0]->GetPos().GetX() - seat_offset, tables[0]->GetPos().GetY() - seat_offset));
	seats.push_back(MyVector(tables[0]->GetPos().GetX() + seat_offset, tables[0]->GetPos().GetY() - seat_offset));
	seats.push_back(MyVector(tables[0]->GetPos().GetX() + seat_offset, tables[0]->GetPos().GetY() + seat_offset));
	seats.push_back(MyVector(tables[0]->GetPos().GetX() - seat_offset, tables[0]->GetPos().GetY() + seat_offset));

	// Table 2 seat positions
	seats.push_back(MyVector(tables[1]->GetPos().GetX() - seat_offset, tables[1]->GetPos().GetY() - seat_offset));
	seats.push_back(MyVector(tables[1]->GetPos().GetX() + seat_offset, tables[1]->GetPos().GetY() - seat_offset));
	seats.push_back(MyVector(tables[1]->GetPos().GetX() + seat_offset, tables[1]->GetPos().GetY() + seat_offset));
	seats.push_back(MyVector(tables[1]->GetPos().GetX() - seat_offset, tables[1]->GetPos().GetY() + seat_offset));

	// Table 3 seat positions
	seats.push_back(MyVector(tables[2]->GetPos().GetX() - seat_offset, tables[2]->GetPos().GetY() - seat_offset));
	seats.push_back(MyVector(tables[2]->GetPos().GetX() + seat_offset, tables[2]->GetPos().GetY() - seat_offset));
	seats.push_back(MyVector(tables[2]->GetPos().GetX() + seat_offset, tables[2]->GetPos().GetY() + seat_offset));
	seats.push_back(MyVector(tables[2]->GetPos().GetX() - seat_offset, tables[2]->GetPos().GetY() + seat_offset));

	// Big table seat position
	seats.push_back(MyVector(bigtable.GetX() - big_seat_offset_diagonal, bigtable.GetY() - big_seat_offset_diagonal));
	seats.push_back(MyVector(bigtable.GetX() - big_seat_offset_straight, bigtable.GetY()));
	seats.push_back(MyVector(bigtable.GetX(), bigtable.GetY() - big_seat_offset_straight));

	seats.push_back(MyVector(bigtable.GetX() + big_seat_offset_diagonal, bigtable.GetY() - big_seat_offset_diagonal));
	seats.push_back(MyVector(bigtable.GetX() + big_seat_offset_straight, bigtable.GetY()));
	seats.push_back(MyVector(bigtable.GetX(), bigtable.GetY() - big_seat_offset_straight));

	seats.push_back(MyVector(bigtable.GetX() + big_seat_offset_diagonal, bigtable.GetY() + big_seat_offset_diagonal));
	seats.push_back(MyVector(bigtable.GetX() + big_seat_offset_straight, bigtable.GetY()));
	seats.push_back(MyVector(bigtable.GetX(), bigtable.GetY() + big_seat_offset_straight));

	seats.push_back(MyVector(bigtable.GetX() - big_seat_offset_diagonal, bigtable.GetY() + big_seat_offset_diagonal));
	seats.push_back(MyVector(bigtable.GetX() - big_seat_offset_straight, bigtable.GetY()));
	seats.push_back(MyVector(bigtable.GetX(), bigtable.GetY() + big_seat_offset_straight));

#pragma endregion

	// Diner AI usage
	//customerPos.SetPosition(wayPoints[4].GetX(), wayPoints[4].GetY());

#pragma region AI Waypoints

	// Waiter 1 Waypoints
	waiterOneWaypoints.push_back(MyVector(waiterOneSpawn.GetX(), waiterOneSpawn.GetY()));
	waiterOneWaypoints.push_back(table_1->GetPos());
	waiterOneWaypoints.push_back(table_2->GetPos());
	waiterOneWaypoints.push_back(table_3->GetPos());

	// Waiter 2 Waypoints
	waiterTwoWaypoints.push_back(MyVector(waiterTwoSpawn.GetX(), waiterTwoSpawn.GetY()));

	// Waiter 3 Waypoints
	waiterThreeWaypoints.push_back(MyVector(waiterThreeSpawn.GetX(), waiterThreeSpawn.GetY()));

	waiterOnePos.SetPosition(waiterOneWaypoints[0].GetX(), waiterOneWaypoints[0].GetY());
	waiterTwoPos.SetPosition(waiterTwoWaypoints[0].GetX(), waiterTwoWaypoints[0].GetY());
	waiterThreePos.SetPosition(waiterThreeWaypoints[0].GetX(), waiterThreeWaypoints[0].GetY());

	customerPos.SetPosition(defaultSpawn.x, defaultSpawn.y);

	//Set Chef Position
	chefPos = MyVector(-6.5f, -2.5f);
	chefSpawn = MyVector(-6.5f, -2.5f);

#pragma endregion


	//TEST ORDER

	// Set AI States
	waiterOneState = E_WAITER_IDLE;
	customerState = E_CUSTOMER_IDLE;
	chefState = E_CHEF_WAIT;


	//Set AI Conditions
	foodReady = false;
	arrived = false;
	availableCustomers = false;
	customerSeated = false;
	customerLine = false;
	customerInLine = false;
	customerPickup = false;
	backToSpawn = false;
	isAtStation = false;

	waiter1_isBusy = false;
	waiter2_isBusy = false;
	waiter3_isBusy = false;
	chefArrived = false;
}


int main()
{
	// INIT ///////////////////////////////////////////////////////////////
	char *title = "The Diner";
	width = 1280;
	height = 720;

	glfwSetErrorCallback(ErrorCallBack);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);

	if (!window)
	{
		fprintf(stderr, "Failed to create GLFW windows.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, KeyCallBack);
	glfwSetWindowSizeCallback(window, ResizeCallBack);

	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		fprintf(stderr, "Error : %s\n", glewGetErrorString(error));
		exit(EXIT_FAILURE);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, width / height, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	///////////////////////////////////////////////////////////////////////////

	// Initialize and load our freetype face
	if (FT_Init_FreeType(&ft_lib) != 0)
	{
		fprintf(stderr, "Couldn't initialize FreeType library\n");
		Cleanup();
		exit(EXIT_FAILURE);
	}

	if (FT_New_Face(ft_lib, "arial.ttf", 0, &face) != 0)
	{
		fprintf(stderr, "Unable to load arial.ttf\n");
		Cleanup();
		exit(EXIT_FAILURE);
	}
	// Initialize our texture and VBOs
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glGenTextures(1, &texture);
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Initialize shader
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &VERTEX_SHADER, 0);
	glCompileShader(vs);

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &FRAGMENT_SHADER, 0);
	glCompileShader(fs);

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	// Initialize GL state
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Get shader uniforms
	glUseProgram(program);
	glBindAttribLocation(program, 0, "in_Position");
	texUniform = glGetUniformLocation(program, "tex");
	colorUniform = glGetUniformLocation(program, "color");

	SimulationInit();
	Render(window);
	DoExit();
}

void RenderObjects()
{
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -10.0f);

	/*
	 *	Color codes:
	 *
	 *	- White = waiter
	 *	- Green = caller
	 *	- Light blue = customer
	 *	- Red = chef
	 */

	 // Background
	RenderRectangle(-15.f, -15.f, 15.f, 15.f, 0.15f, 0.f, 0.15f);

	// Tables
	for (unsigned int i = 0; i < tables.size(); i++)
	{
		RenderFillCircle(tables[i]->GetPos().GetX(), tables[i]->GetPos().GetY(), 1.f, 0.6f, 0.3f, 0.f);
	}

	RenderFillCircle(bigtable.GetX(), bigtable.GetY(), 2.f, 0.6f, 0.3f, 0.f);

	// Seats
	for (unsigned int i = 0; i < seats.size(); i++)
	{
		RenderCircle(seats[i].GetX(), seats[i].GetY(), waypoint_radius, 0.6f, 0.3f, 0.f);
	}

	// Caller's table
	RenderRectangle(7.f, 4.25f, 7.25f, 2.75f, 0.5f, 0.5f, 0.5f);

	// Caller
	RenderFillCircle(callerPos.x, callerPos.y, AI_radius, 0.f, 1.f, 0.f); // Caller object

	// CHEFS STUFF

	// Render the bigger rectangle
	RenderRectangle(-9.5f, -0.5f, -4.5f, -4.5f, 0.6f, 0.6f, 0.6f);
	// Render the smaller rectangle
	RenderRectangle(-9.25f, -0.75f, -5.5f, -4.25f, 0.2f, 0.2f, 0.2f);
	// Render the cooking station 
	RenderFillCircle(cookingStation_1.GetX(), cookingStation_1.GetY(), 0.4f, 1.f, 0.5f, 0.5f);

	// Waiter 1
	RenderFillCircle(waiterOnePos.GetX(), waiterOnePos.GetY(), AI_radius, 0.8f, 0.8f, 0.8f); // Waiter 1 object

	// Waiter 2
	RenderFillCircle(waiterTwoPos.GetX(), waiterTwoPos.GetY(), AI_radius, 0.8f, 0.8f, 0.8f); // Waiter 2 object

	// Waiter 3
	RenderFillCircle(waiterThreePos.GetX(), waiterThreePos.GetY(), AI_radius, 0.8f, 0.8f, 0.8f); // Waiter 3 object

	// Customer (temp)
	RenderFillCircle(customerPos.GetX(), customerPos.GetY(), AI_radius, 0.f, 0.9f, 1.f); // Customer 1 object


	// Chef
	RenderFillCircle(chefPos.GetX(), chefPos.GetY(), AI_radius, 1.f, 0.f, 0.f);


	//// Waypoints
	//for (unsigned int i = 0; i < wayPoints.size(); i++ )
	//	RenderCircle(wayPoints[i].GetX(), wayPoints[i].GetY(), playerRadius + 0.1f, 1.0f, 0.0f, 0.0f); //  waypoints

	//Food Station
	//RenderCircle(chefStation.GetX(), chefStation.GetY(), waypoint_radius, 1.0f, 0.0f, 0.0f);

	glPopMatrix();
}

void RunFSM()
{
	if (foodReady && chefArrived == true)
	{
		waiterOneState = E_WAITER_SERVE;
		foodReady = false;
		chefArrived = false;
	}

	if (availableCustomers)
	{
		waiterOneState = E_WAITER_PICKUPCUSTOMER;
	}

	if (customerPickup)
	{
		customerState = E_CUSTOMER_MOVE;
	}

	if (customerLine)
	{
		customerState = E_CUSTOMER_QUEUE;
	}
}

void Update()
{
#pragma region Waiter Updates

	if (waiterOneState == E_WAITER_IDLE)
	{
		MyVector direction = (waiterOnePos - waiterOneSpawn).Normalize();
		float distance = GetDistance(waiterOnePos.GetX(), waiterOnePos.GetY(), waiterOneSpawn.GetX(), waiterOneSpawn.GetY());

		if (distance < waiterSpeed)
		{
			arrived = true;
		}
		else
		{
			waiterOnePos = waiterOnePos + direction * waiterSpeed;
		}

		if (arrived)
		{
			arrived = false;
		}
	}

	if (waiterOneState == E_WAITER_PICKUP)
	{
		MyVector direction = (waiterOnePos - chefStation).Normalize();
		float distance = GetDistance(waiterOnePos.GetX(), waiterOnePos.GetY(), chefStation.GetX(), chefStation.GetY());
		if (distance < waiterSpeed)
		{
			arrived = true;
		}
		else
		{
			waiterOnePos = waiterOnePos + direction * waiterSpeed;
		}

		if (arrived)
		{
			waiterOneState = E_WAITER_SERVE;
			arrived = false;
			foodReady = false;
		}
	}
	if (waiterOneState == E_WAITER_SERVE)
	{
		// Find a table to serve
		if (ListOfOrders.size() > 0)
		{
			// take the tableNumber from the first order list
			tableNumber = ListOfOrders[0];
			// erase order from the list
			ListOfOrders.erase(ListOfOrders.begin());
		}

		MyVector direction = (waiterOnePos - waiterOneWaypoints[tableNumber]).Normalize();
		float distance = GetDistance(waiterOnePos.GetX(), waiterOnePos.GetY(), waiterOneWaypoints[tableNumber].GetX(), waiterOneWaypoints[tableNumber].GetY());
		if (distance < waiterSpeed)
		{
			arrived = true;
		}
		else
		{
			waiterOnePos = waiterOnePos + direction * waiterSpeed;
		}
		if (arrived)
		{
			waiterOneState = E_WAITER_IDLE;
			customerState = E_CUSTOMER_EAT;
			arrived = false;
			eatStart = clock();
		}

	}
	if (waiterOneState == E_WAITER_PICKUPCUSTOMER)
	{
		MyVector direction = (waiterOnePos - customerPos).Normalize();
		float distance = GetDistance(waiterOnePos.GetX(), waiterOnePos.GetY(), customerPos.GetX(), customerPos.GetY());

		if (distance < waiterSpeed)
		{
			customerPickup = true;
		}
		else
		{
			waiterOnePos = waiterOnePos + direction * waiterSpeed;
		}

		if (customerPickup)
		{
			waiterOneState = E_WAITER_MOVE;
			customerState = E_CUSTOMER_MOVE;
			customerPickup = false;
			availableCustomers = false;
		}
	}

	if (waiterOneState == E_WAITER_MOVE)
	{
		MyVector direction = (waiterOnePos - waiterOneWaypoints[1]).Normalize();
		float distance = GetDistance(waiterOnePos.GetX(), waiterOnePos.GetY(), waiterOneWaypoints[1].GetX(), waiterOneWaypoints[1].GetY());

		if (distance < waiterSpeed)
		{
			customerSeated = true;
		}
		else
		{
			waiterOnePos = waiterOnePos + direction * waiterSpeed;
		}

		if (customerSeated)
		{
			customerSeated = false;
		}
	}

#pragma endregion

#pragma region Customer Updates

	if (customerState == E_CUSTOMER_QUEUE)
	{
		MyVector direction = (customerPos - defaultLine).Normalize();
		float distance = GetDistance(customerPos.GetX(), customerPos.GetY(), defaultLine.x, defaultLine.y);

		if (distance < customerSpeed)
		{
			customerInLine = true;
		}
		else
		{
			customerPos = customerPos + direction * customerSpeed;
		}

		if (customerInLine)
		{
			customerState = E_CUSTOMER_IDLE;
			customerLine = false;
			customerInLine = false;
			availableCustomers = true;
		}
	}

	if (customerState == E_CUSTOMER_MOVE)
	{
		MyVector direction = (customerPos - seats[0]).Normalize();
		float distance = GetDistance(customerPos.GetX(), customerPos.GetY(), seats[0].GetX(), seats[0].GetY());

		if (distance < customerSpeed)
		{
			customerSeated = true;
		}
		else
		{
			customerPos = customerPos + direction * customerSpeed;
		}

		if (customerSeated)
		{
			customerState = E_CUSTOMER_ORDER;
			waiterOneState = E_WAITER_IDLE;
			customerSeated = false;
			orderStart = clock();
		}
	}

	if (customerState == E_CUSTOMER_ORDER)
	{
		//Add an order from table 1
		orderEnd = clock();

		if (((float)(orderEnd - orderStart) / CLOCKS_PER_SEC) >= orderTime)
		{
			ListOfOrders.push_back(1);
			customerState = E_CUSTOMER_IDLE;
		}
	}

	if (customerState == E_CUSTOMER_EAT)
	{
		eatEnd = clock();

		if (((float)(eatEnd - eatStart) / CLOCKS_PER_SEC) >= eatTime)
		{
			customerState = E_CUSTOMER_LEAVE;
		}
	}

	if (customerState == E_CUSTOMER_LEAVE)
	{
		MyVector direction = (customerPos - defaultSpawn).Normalize();
		float distance = GetDistance(customerPos.GetX(), customerPos.GetY(), defaultSpawn.x, defaultSpawn.y);

		if (distance < customerSpeed)
		{
			backToSpawn = true;
		}
		else
		{
			customerPos = customerPos + direction * customerSpeed;
		}

		if (backToSpawn)
		{
			customerState = E_CUSTOMER_IDLE;
			backToSpawn = false;
		}
	}

#pragma endregion

#pragma region Chef Updates
	// When chef is idle with no cooking
	if (chefState == E_CHEF_WAIT)
	{
		//if there is an existing order
		if (ListOfOrders.size() > 0)
		{
			MyVector direction = (chefPos - cookingStation_1).Normalize();
			float distance = GetDistance(chefPos.GetX(), chefPos.GetY(), cookingStation_1.GetX(), cookingStation_1.GetY());

			if (distance < chefSpeed)
			{
				isAtStation = true;
			}
			else
			{
				chefPos = chefPos + direction * chefSpeed;
			}

			if (isAtStation)
			{
				chefState = E_CHEF_COOK;
				isAtStation = false;

				// Start the cook time
				// Assign current time value to t
				t1 = clock();

			}
		}
		//traverse back to spawn point?
		else if (ListOfOrders.size() == 0)
		{
			MyVector direction = (chefPos - chefSpawn).Normalize();
			float distance = GetDistance(chefPos.GetX(), chefPos.GetY(), chefSpawn.GetX(), chefSpawn.GetY());

			if (distance < chefSpeed)
			{
				isAtStation = true;
			}
			else
			{
				chefPos = chefPos + direction * chefSpeed;
			}
		}

	}
	if (chefState == E_CHEF_COOK)
	{
		// Get Current Time 
		// Assign to t2
		t2 = clock();

		// If elasped time is more than 5 seconds
		if (((float)(t2 - t1) / CLOCKS_PER_SEC) >= 5.f)
		{
			foodReady = true;
			chefState = E_CHEF_SERVE;
		}
	}

	if (chefState == E_CHEF_SERVE)
	{
		if (waiter1_isBusy)
		{
			if (waiter2_isBusy)
			{
				if (!waiter3_isBusy)
				{
					MyVector direction = (chefPos - waiterThreeSpawn).Normalize();
					float distance = GetDistance(chefPos.GetX(), chefPos.GetY(), waiterThreeSpawn.GetX(), waiterThreeSpawn.GetY());

					if (distance < chefSpeed)
					{
						chefArrived = true;
					}
					else
					{
						chefPos = chefPos + direction * chefSpeed;
					}

					if (waiterOneState == E_WAITER_SERVE)
					{
						chefState = E_CHEF_WAIT;
						chefArrived = false;
					}
				}
			}
			else if (!waiter2_isBusy)
			{
				MyVector direction = (chefPos - waiterTwoSpawn).Normalize();
				float distance = GetDistance(chefPos.GetX(), chefPos.GetY(), waiterTwoSpawn.GetX(), waiterTwoSpawn.GetY());

				if (distance < chefSpeed)
				{
					chefArrived = true;
				}
				else
				{
					chefPos = chefPos + direction * chefSpeed;
				}

				if (waiterOneState == E_WAITER_SERVE)
				{
					chefState = E_CHEF_WAIT;
					chefArrived = false;
				}
			}
		}
		else if (!waiter1_isBusy)
		{
			MyVector direction = (chefPos - waiterOneSpawn).Normalize();
			float distance = GetDistance(chefPos.GetX(), chefPos.GetY(), waiterOneSpawn.GetX(), waiterOneSpawn.GetY());

			if (distance < chefSpeed)
			{
				chefArrived = true;
			}
			else
			{
				chefPos = chefPos + direction * chefSpeed;
			}

			if (waiterOneState == E_WAITER_SERVE)
			{
				chefState = E_CHEF_WAIT;
				chefArrived = false;
			}
		}
	}

#pragma endregion
}

void RenderDebugText()
{
	// Waiter 1 Debug text

	RenderText("WaiterState: ", face, -0.95f, 0.925f, 0.55f, 0.55f);

	switch (waiterOneState)
	{
	case E_WAITER_IDLE:
	{
		RenderText("IDLE", face, -0.75f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_WAITER_MOVE:
	{
		RenderText("MOVE", face, -0.75f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_WAITER_PICKUP:
	{
		RenderText("PICKUP_FOOD", face, -0.75f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_WAITER_PICKUPCUSTOMER:
	{
		RenderText("PICKUP_CUSTOMER", face, -0.75f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_WAITER_SERVE:
	{
		RenderText("SERVE", face, -0.75f, 0.925f, 0.55f, 0.55f);
		break;
	}
	}

	//Chef Debug Text
	RenderText("Chef State: ", face, -0.95f, 0.825f, 0.55f, 0.55f);

	switch (chefState)
	{
	case E_CHEF_WAIT:
	{
		RenderText("Chef_Wait", face, -0.75f, 0.825f, 0.55f, 0.55f);
		break;
	}
	case E_CHEF_COOK:
	{
		RenderText("Chef_Cook", face, -0.75f, 0.825f, 0.55f, 0.55f);
		break;
	}
	case E_CHEF_SERVE:
	{
		RenderText("Chef_Serve", face, -0.75f, 0.825f, 0.55f, 0.55f);
		break;
	}
	}

	// Customer debug text
	RenderText("Cus State: ", face, -0.5f, 0.925f, 0.55f, 0.55f);

	switch (customerState)
	{
	case E_CUSTOMER_EAT:
	{
		RenderText("Eating", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_CUSTOMER_IDLE:
	{
		RenderText("Idling", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_CUSTOMER_LEAVE:
	{
		RenderText("Leaving", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_CUSTOMER_MOVE:
	{
		RenderText("Moving to table", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_CUSTOMER_ORDER:
	{
		RenderText("Ordering", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case E_CUSTOMER_QUEUE:
	{
		RenderText("Lining up", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	}

}

void Render(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		glUseProgram(0);
		glClear(GL_COLOR_BUFFER_BIT);

		Update();
		RunFSM();

		string stateString = "";
		MyVector direction;

		/*switch ( state )
		{
			case PATROL  :	stateString = "PATROL";
							break;
			case CHASE :	stateString = "CHASE";
							direction = ( playerPos - enemyPos ).Normalize();
							enemyPos = enemyPos + direction * enemySpeed;
							playerPos = playerPos + direction  * playerSpeed;
							break;
		}*/

		RenderObjects();

		// Bind stuff
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindSampler(0, sampler);
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glUseProgram(program);
		glUniform4f(colorUniform, 1, 1, 1, 1);
		glUniform1i(texUniform, 0);

		FT_Set_Pixel_Sizes(face, 0, 50);

		RenderDebugText();

		/*RenderText("State : ", face, -0.95f, 0.925f, 0.55f, 0.55f);
		RenderText( stateString , face, -0.8f, 0.925f, 0.55f, 0.55f );
		RenderText("Player - Blue     Enemy - Green     Red - Patrol Point", face, -0.6f, 0.925f, 0.55f, 0.55f);*/

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}