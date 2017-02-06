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
#include "Waiter.h"
#include "Customer.h"
#include "Chef.h"
#include "Caller.h"
#include "MessageBoard.h"


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

vector<CWaiter*> waiterList;
vector<Customer*> customerList;

// AI radius
const float AI_radius = 0.25f;
const float waypoint_radius = 0.35f;
const float proximity = 0.2f;

// Autorun related
bool autorun = false; // triggers autorun
bool cycle = false;
clock_t breakStart, breakEnd;
const float timer = 2.f;

// Message board
MessageBoard messageBoard;

#pragma region AI_STATES
// Chef related
Chef *chef = new Chef();


//List of orders
/*
	Pass in table numbers for the order
*/
vector<unsigned int> ListOfOrders;

MyVector cookingStation_1 = MyVector(-5.f, -2.5f);
// bool isCooking_1 = false;

// Waiter related
CWaiter *waiter_1 = new CWaiter();
CWaiter *waiter_2 = new CWaiter();
CWaiter *waiter_3 = new CWaiter();

unsigned int tableNumber;

// Customer related
Customer *customer_1 = new Customer();
Customer *customer_2 = new Customer();
Customer *customer_3 = new Customer();

// Caller related
Caller *caller = new Caller();

#pragma endregion

static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Triggers autorun
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		autorun = true;
		for (int i = 0; i < customerList.size(); i++)
		{
			customerList[i]->SetLine(true);
		}
	}

	// Triggers one-time run
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
	{
		for (int i = 0; i < customerList.size(); i++)
		{
			customerList[i]->SetLine(true);
		}
	}
}

void SimulationInit()
{
	waiterList.push_back(waiter_1);
	waiterList.push_back(waiter_2);
	waiterList.push_back(waiter_3);

	customerList.push_back(customer_1);
	customerList.push_back(customer_2);
	customerList.push_back(customer_3);
	float xpos = -9.f;
	for (int i = 0; i < waiterList.size(); i++)
	{
		waiterList[i]->SetSpawn(MyVector(xpos, -0.2f));
		xpos += 2.f;
	}
	chef->SetStation(MyVector(-5.f, -2.5f));
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
	for (int i = 0; i < waiterList.size(); i++)
	{
		waiterList[i]->waiterWayPoints.push_back(MyVector(waiterList[i]->GetSpawn().GetX(), waiterList[i]->GetSpawn().GetY()));
		waiterList[i]->waiterWayPoints.push_back(table_1->GetPos());
		waiterList[i]->waiterWayPoints.push_back(table_2->GetPos());
		waiterList[i]->waiterWayPoints.push_back(table_3->GetPos());
	}
	

	caller->callerWaypoints.push_back(MyVector(caller->GetSpawnLocation().GetX(), caller->GetSpawnLocation().GetY()));
	caller->callerWaypoints.push_back(table_1->GetPos());
	caller->callerWaypoints.push_back(table_2->GetPos());
	caller->callerWaypoints.push_back(table_3->GetPos());

	for (int i = 0; i < waiterList.size(); i++)
	{
		waiterList[i]->SetPos(waiterList[i]->waiterWayPoints[0]);
	}
	for (int i = 0; i < customerList.size(); i++)
	{
		customerList[i]->SetPos(customerList[i]->GetSpawnLocation());
	}
	caller->SetPos(caller->callerWaypoints[0]);
	
	//Set Chef Position
	chef->SetPos(MyVector(-6.5f, -2.5f));
	chef->SetSpawn(MyVector(-6.5f, -2.5f));

#pragma endregion


	//TEST ORDER

	// Set AI States
	for (int i = 0; i < waiterList.size(); i++)
	{
		waiterList[i]->SetState(CWaiter::WAITER_STATE::E_WAITER_IDLE);
	}
	for (int i = 0; i < customerList.size(); i++)
	{
		customerList[i]->SetState(Customer::CUS_STATE::E_CUSTOMER_IDLE);
	}
	chef->SetState(Chef::CHEF_STATE::E_CHEF_WAIT);
	caller->SetState(Caller::CALLER_STATE::E_CALLER_IDLE);

	//Set AI Conditions
	// Set Waiter 1 conditions
	for (int i = 0; i < waiterList.size(); i++)
	{
		waiterList[i]->SetFoodReady(false);
		waiterList[i]->SetAvailableCustomers(false);
		waiterList[i]->SetCustomerPickup(false);
		waiterList[i]->SetBusy(false);
	}

	// Customer
	for (int i = 0; i < customerList.size(); i++)
	{
		customerList[i]->SetSeated(false);
		customerList[i]->SetLine(false);
		customerList[i]->SetInLine(false);
		customerList[i]->SetBackSpawn(false);
	}

	// Chef
	chef->SetAtStation(false);
	chef->SetArrived(false);

	// Caller
	caller->SetClear(false);
	caller->SetArrive(false);
	caller->SetBackSpawn(false);

	arrived = false;
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
	RenderFillCircle(caller->GetPos().GetX(), caller->GetPos().GetY(), AI_radius, 0.f, 1.f, 0.f); // Caller object

	// CHEFS STUFF

	// Render the bigger rectangle
	RenderRectangle(-9.5f, -0.5f, -4.5f, -4.5f, 0.6f, 0.6f, 0.6f);
	// Render the smaller rectangle
	RenderRectangle(-9.25f, -0.75f, -5.5f, -4.25f, 0.2f, 0.2f, 0.2f);
	// Render the cooking station 
	RenderFillCircle(cookingStation_1.GetX(), cookingStation_1.GetY(), 0.4f, 1.f, 0.5f, 0.5f);

	// Waiter 1
	for (int i = 0; i < waiterList.size(); i++)
	{
		RenderFillCircle(waiterList[i]->GetPos().GetX(), waiterList[i]->GetPos().GetY(), AI_radius, 0.8f, 0.8f, 0.8f); // Waiter 1 object
	}

	// Customer (temp)
	for (int i = 0; i < customerList.size(); i++)
	{
		RenderFillCircle(customerList[i]->GetPos().x, customerList[i]->GetPos().y, AI_radius, 0.f, 0.9f, 1.f); // Customer 1 object
	}


	// Chef
	RenderFillCircle(chef->GetPos().GetX(), chef->GetPos().GetY(), AI_radius, 1.f, 0.f, 0.f);


	//// Waypoints
	//for (unsigned int i = 0; i < wayPoints.size(); i++ )
	//	RenderCircle(wayPoints[i].GetX(), wayPoints[i].GetY(), playerRadius + 0.1f, 1.0f, 0.0f, 0.0f); //  waypoints

	//Food Station
	//RenderCircle(chefStation.GetX(), chefStation.GetY(), waypoint_radius, 1.0f, 0.0f, 0.0f);

	glPopMatrix();
}

void RunFSM()
{
	for (int i = 0; i < waiterList.size(); i++)
	{
		if (waiterList[i]->GetBusy() == false)
		{
			if (waiterList[i]->GetFoodReady() && chef->GetArrived())
			{
				waiterList[i]->SetState(CWaiter::E_WAITER_SERVE);
				waiterList[i]->SetFoodReady(false);
				waiterList[i]->SetBusy(true);
				chef->SetArrived(false);
			}
			if (waiterList[i]->GetAvailableCustomers())
			{
				waiterList[i]->SetState(CWaiter::E_WAITER_PICKUPCUSTOMER);
				waiterList[i]->SetAssignedCustomer(customerList[i]);
				waiterList[i]->SetBusy(true);
			}

			if (waiterList[i]->GetCustomerPickup())
			{
				customerList[i]->SetState(Customer::E_CUSTOMER_MOVE);
				waiterList[i]->SetBusy(true);
			}

		}
	}

	for (int i = 0; i < customerList.size(); i++)
	{
		if (customerList[i]->GetLine())
		{
			customerList[i]->SetState(Customer::E_CUSTOMER_QUEUE);
		}
	}

	if (caller->GetClear())
	{
		caller->SetState(Caller::E_CALLER_MOVE);
	}


}

void Update()
{
#pragma region Waiter Updates
	for (int i = 0; i < waiterList.size(); i++)
	{
		if (waiterList[i]->GetState() == CWaiter::E_WAITER_IDLE)
		{
			MyVector direction = (waiterList[i]->GetPos() - waiterList[i]->GetSpawn()).Normalize();
			float distance = GetDistance(waiterList[i]->GetPos().GetX(), waiterList[i]->GetPos().GetY(), waiterList[i]->GetSpawn().GetX(), waiterList[i]->GetSpawn().GetY());

			if (distance < waiterList[i]->GetSpeed())
			{
				arrived = true;
			}
			else
			{
				waiterList[i]->SetPos(waiterList[i]->GetPos() + direction * waiterList[i]->GetSpeed());
			}

			if (arrived)
			{
				arrived = false;
			}
		}

		if (waiterList[i]->GetState() == CWaiter::E_WAITER_PICKUP)
		{
			MyVector direction = (waiterList[i]->GetPos() - chef->GetStation()).Normalize();
			float distance = GetDistance(waiterList[i]->GetPos().GetX(), waiterList[i]->GetPos().GetY(), chef->GetStation().GetX(), chef->GetStation().GetY());
			if (distance < waiterList[i]->GetSpeed())
			{
				arrived = true;
			}
			else
			{
				waiterList[i]->SetPos(waiterList[i]->GetPos() + direction * waiterList[i]->GetSpeed());
			}

			if (arrived)
			{
				waiterList[i]->SetState(CWaiter::E_WAITER_SERVE);
				arrived = false;
				waiterList[i]->SetFoodReady(false);
			}
		}
		if (waiterList[i]->GetState() == CWaiter::E_WAITER_SERVE)
		{
			// Find a table to serve
			if (ListOfOrders.size() > 0)
			{
				// take the tableNumber from the first order list
				tableNumber = ListOfOrders[0];
				// erase order from the list
				ListOfOrders.erase(ListOfOrders.begin());
			}

			MyVector direction = (waiterList[i]->GetPos() - waiterList[i]->waiterWayPoints[tableNumber]).Normalize();
			float distance = GetDistance(waiterList[i]->GetPos().GetX(), waiterList[i]->GetPos().GetY(), waiterList[i]->waiterWayPoints[tableNumber].GetX(), waiterList[i]->waiterWayPoints[tableNumber].GetY());
			if (distance < waiterList[i]->GetSpeed())
			{
				arrived = true;
			}
			else
			{
				waiterList[i]->SetPos(waiterList[i]->GetPos() + direction * waiterList[i]->GetSpeed());
			}
			if (arrived)
			{
				waiterList[i]->SetState(CWaiter::E_WAITER_IDLE);
				waiterList[i]->SetBusy(false);
				waiterList[i]->GetAssignedCustomer()->SetState(Customer::E_CUSTOMER_EAT);
				arrived = false;
				waiterList[i]->GetAssignedCustomer()->eatStart = clock();
			}

		}
		if (waiterList[i]->GetState() == CWaiter::E_WAITER_PICKUPCUSTOMER)
		{
			MyVector direction = (waiterList[i]->GetPos() - waiterList[i]->GetAssignedCustomer()->GetPos()).Normalize();
			float distance = GetDistance(waiterList[i]->GetPos().GetX(), waiterList[i]->GetPos().GetY(), waiterList[i]->GetAssignedCustomer()->GetPos().GetX(), waiterList[i]->GetAssignedCustomer()->GetPos().GetY());

			if (distance < waiterList[i]->GetSpeed())
			{
				waiterList[i]->SetCustomerPickup(true);
			}
			else
			{
				waiterList[i]->SetPos(waiterList[i]->GetPos() + direction * waiterList[i]->GetSpeed());
			}

			if (waiterList[i]->GetCustomerPickup())
			{
				waiterList[i]->SetState(CWaiter::E_WAITER_MOVE);
				waiterList[i]->GetAssignedCustomer()->SetState(Customer::E_CUSTOMER_MOVE);
				waiterList[i]->SetCustomerPickup(false);
				waiterList[i]->SetAvailableCustomers(false);

				messageBoard.Reset();
			}



			if (waiterList[i]->GetState() == CWaiter::E_WAITER_MOVE)
			{
				MyVector direction = (waiterList[i]->GetPos() - waiterList[i]->waiterWayPoints[tableNumber]).Normalize();
				float distance = GetDistance(waiterList[i]->GetPos().GetX(), waiterList[i]->GetPos().GetY(), waiterList[i]->waiterWayPoints[tableNumber].GetX(), waiterList[i]->waiterWayPoints[tableNumber].GetY());

				if (distance < waiterList[i]->GetSpeed())
				{
					waiterList[i]->GetAssignedCustomer()->SetSeated(true);
				}
				else
				{
					waiterList[i]->SetPos(waiterList[i]->GetPos() + direction * waiterList[i]->GetSpeed());
				}

				if (waiterList[i]->GetAssignedCustomer()->GetSeated())
				{
					waiterList[i]->GetAssignedCustomer()->SetSeated(false);
				}
			}
		}
	}

#pragma endregion

#pragma region Customer Updates
	for (int i = 0; i < customerList.size(); i++)
	{
		if (customerList[i]->GetState() == Customer::E_CUSTOMER_QUEUE)
		{
			MyVector direction = (customerList[i]->GetPos() - customerList[i]->GetLineLocation()).Normalize();
			float distance = GetDistance(customerList[i]->GetPos().GetX(), customerList[i]->GetPos().GetY(), customerList[i]->GetLineLocation().GetX(), customerList[i]->GetLineLocation().GetY());
			CWaiter* fw = new CWaiter();
			for (int i = 0; i < waiterList.size(); i++)
			{
				if (waiterList[i]->GetBusy() == false)
				{
					fw = waiterList[i];
				}
			}

			if (customerList[i]->GetAssignedWaiter() == NULL)
			{
				customerList[i]->SetAssignedWaiter(fw);
				fw->SetAvailableCustomers(true);
			}

			if (distance < customerList[i]->GetSpeed())
			{
				customerList[i]->SetInLine(true);
			}
			else
			{
				customerList[i]->SetPos(customerList[i]->GetPos() + direction * customerList[i]->GetSpeed());
			}

			if (customerList[i]->GetInLine())
			{
				messageBoard.setLabel_From("Caller");
				messageBoard.setLabel_To("Waiter");
				messageBoard.setMessage("CUSTOMERS HERE!");

				customerList[i]->SetState(Customer::E_CUSTOMER_IDLE);
				customerList[i]->SetLine(false);
				customerList[i]->SetInLine(false);


			}
		}

		if (customerList[i]->GetState() == Customer::E_CUSTOMER_MOVE)
		{
			MyVector direction = (customerList[i]->GetPos() - seats[0]).Normalize();
			float distance = GetDistance(customerList[i]->GetPos().GetX(), customerList[i]->GetPos().GetY(), seats[0].GetX(), seats[0].GetY());

			if (distance < customerList[i]->GetSpeed())
			{
				customerList[i]->SetSeated(true);
			}
			else
			{
				customerList[i]->SetPos(customerList[i]->GetPos() + direction * customerList[i]->GetSpeed());
			}

			if (customerList[i]->GetSeated())
			{
				customerList[i]->SetState(Customer::E_CUSTOMER_ORDER);
				CWaiter* assignedWaiter = customerList[i]->GetAssignedWaiter();
				assignedWaiter->SetState(CWaiter::E_WAITER_IDLE);
				customerList[i]->SetSeated(false);
				customerList[i]->orderStart = clock();
			}
		}

		if (customerList[i]->GetState() == Customer::E_CUSTOMER_ORDER)
		{
			//Add an order from table 1
			customerList[i]->orderEnd = clock();

			if (((float)(customerList[i]->orderEnd - customerList[i]->orderStart) / CLOCKS_PER_SEC) >= customerList[i]->GetOrderTime())
			{
				ListOfOrders.push_back(1);
				customerList[i]->SetState(Customer::E_CUSTOMER_IDLE);
			}
		}

		if (customerList[i]->GetState() == Customer::E_CUSTOMER_EAT)
		{
			customerList[i]->eatEnd = clock();

			if (((float)(customerList[i]->eatEnd - customerList[i]->eatStart) / CLOCKS_PER_SEC) >= customerList[i]->GetEatTime())
			{
				customerList[i]->SetState(Customer::E_CUSTOMER_LEAVE);

				messageBoard.setLabel_From("Waiter");
				messageBoard.setLabel_To("Caller");
				messageBoard.setMessage("CUSTOMERS LEAVING!");

				caller->SetState(Caller::E_CALLER_MOVE);
			}
		}

		if (customerList[i]->GetState() == Customer::E_CUSTOMER_LEAVE)
		{
			MyVector direction = (customerList[i]->GetPos() - customerList[i]->GetSpawnLocation()).Normalize();
			float distance = GetDistance(customerList[i]->GetPos().GetX(), customerList[i]->GetPos().GetY(), customerList[i]->GetSpawnLocation().x, customerList[i]->GetSpawnLocation().y);

			if (distance < customerList[i]->GetSpeed())
			{
				customerList[i]->SetBackSpawn(true);
			}
			else
			{
				customerList[i]->SetPos(customerList[i]->GetPos() + direction * customerList[i]->GetSpeed());
			}

			if (customerList[i]->GetBackSpawn())
			{
				customerList[i]->SetState(Customer::E_CUSTOMER_IDLE);
				customerList[i]->SetBackSpawn(false);
			}
		}
	}

#pragma endregion

#pragma region Chef Updates
	// When chef is idle with no cooking
	if (chef->GetState() == Chef::E_CHEF_WAIT)
	{
		//if there is an existing order
		if (ListOfOrders.size() > 0)
		{
			MyVector direction = (chef->GetPos() - cookingStation_1).Normalize();
			float distance = GetDistance(chef->GetPos().GetX(), chef->GetPos().GetY(), cookingStation_1.GetX(), cookingStation_1.GetY());

			if (distance < chef->GetSpeed())
			{
				chef->SetAtStation(true);
			}
			else
			{
				chef->SetPos(chef->GetPos() + direction * chef->GetSpeed());
			}

			if (chef->GetAtStation())
			{
				chef->SetState(Chef::E_CHEF_COOK);
				chef->SetAtStation(false);

				// Start the cook time
				// Assign current time value to t
				t1 = clock();

			}
		}
		//traverse back to spawn point?
		else if (ListOfOrders.size() == 0)
		{
			MyVector direction = (chef->GetPos() - chef->GetSpawn()).Normalize();
			float distance = GetDistance(chef->GetPos().GetX(), chef->GetPos().GetY(), chef->GetSpawn().GetX(), chef->GetSpawn().GetY());

			if (distance < chef->GetSpeed())
			{
				chef->SetAtStation(true);
			}
			else
			{
				chef->SetPos(chef->GetPos() + direction * chef->GetSpeed());
			}
		}

	}
	if (chef->GetState() == Chef::E_CHEF_COOK)
	{
		// Get Current Time 
		// Assign to t2
		t2 = clock();

		// If elasped time is more than 5 seconds
		if (((float)(t2 - t1) / CLOCKS_PER_SEC) >= 5.f)
		{
			for (int i = 0; i < waiterList.size(); i++)
			{
				if (waiterList[i]->GetBusy() == false)
				{
					waiterList[i]->SetFoodReady(true);
					break;
				}
			}
			chef->SetState(Chef::E_CHEF_SERVE);
		}
	}

	if (chef->GetState() == Chef::E_CHEF_SERVE)
	{
		/*if (waiter->GetBusy())
		{*/
			/*if (waiter2_isBusy)
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
			}*/
		//}
		//else if (!waiter->GetBusy())
		//{
		for (int i = 0; i < waiterList.size(); i++)
		{
			if (waiterList[i]->GetBusy() == false)
			{
				MyVector direction = (chef->GetPos() - waiterList[i]->GetSpawn()).Normalize();
				float distance = GetDistance(chef->GetPos().GetX(), chef->GetPos().GetY(), waiterList[i]->GetSpawn().GetX(), waiterList[i]->GetSpawn().GetY());

				if (distance < chef->GetSpeed())
				{
					chef->SetArrived(true);
				}
				else
				{
					chef->SetPos(chef->GetPos() + direction * chef->GetSpeed());
				}

				if (waiterList[i]->GetState() == CWaiter::E_WAITER_SERVE)
				{
					chef->SetState(Chef::E_CHEF_WAIT);
					chef->SetArrived(false);
				}
			}
		}



	}

#pragma endregion

#pragma region Caller Updates

	if (caller->GetState() == Caller::E_CALLER_MOVE)
	{
		MyVector direction = (caller->GetPos() - caller->callerWaypoints[tableNumber]).Normalize();
		float distance = GetDistance(caller->GetPos().GetX(), caller->GetPos().GetY(), caller->callerWaypoints[tableNumber].GetX(), caller->callerWaypoints[tableNumber].GetY());

		if (distance < caller->GetSpeed())
		{
			caller->SetArrive(true);
		}
		else
		{
			caller->SetPos(caller->GetPos() + direction * caller->GetSpeed());
		}

		if (caller->GetArrive())
		{
			caller->SetArrive(false);
			//caller->SetClear(true);
			caller->SetState(Caller::E_CALLER_CLEAR);
			caller->clearStart = clock();

			messageBoard.Reset();
		}
	}

	if (caller->GetState() == Caller::E_CALLER_CLEAR)
	{
		caller->clearEnd = clock();

		if (((float)(caller->clearEnd - caller->clearStart) / CLOCKS_PER_SEC) >= caller->GetClearTime())
		{
			caller->SetClear(false);
			caller->SetState(Caller::E_CALLER_SPAWN);
		}
	}

	if (caller->GetState() == Caller::E_CALLER_SPAWN)
	{
		MyVector direction = (caller->GetPos() - caller->GetSpawnLocation()).Normalize();
		float distance = GetDistance(caller->GetPos().GetX(), caller->GetPos().GetY(), caller->GetSpawnLocation().GetX(), caller->GetSpawnLocation().GetY());

		if (distance < caller->GetSpeed())
		{
			caller->SetBackSpawn(true);
		}
		else
		{
			caller->SetPos(caller->GetPos() + direction * caller->GetSpeed());
		}

		if (caller->GetBackSpawn())
		{
			caller->SetBackSpawn(false);
			cycle = true;
			caller->SetState(Caller::E_CALLER_IDLE);
			breakStart = clock();
		}
	}

#pragma endregion

	if (autorun && cycle)
	{
		breakEnd = clock();

		if (((float)(breakEnd - breakStart) / CLOCKS_PER_SEC) >= timer)
		{
			cycle = false;
			//customerList[i]->SetLine(true);
		}
	}

}


void RenderDebugText()
{
#pragma region Waiter 1 Debug text

	RenderText("WaiterState: ", face, -0.95f, 0.925f, 0.55f, 0.55f);
	float meme = 0.925f;

	for (int i = 0; i < waiterList.size(); i++)
	{
		switch (waiterList[i]->GetState())
		{
		case CWaiter::E_WAITER_IDLE:
		{
			RenderText("IDLE", face, -0.75f, meme, 0.55f, 0.55f);
			break;
		}
		case CWaiter::E_WAITER_MOVE:
		{
			RenderText("MOVE", face, -0.75f, meme, 0.55f, 0.55f);
			break;
		}
		case CWaiter::E_WAITER_PICKUP:
		{
			RenderText("PICKUP_FOOD", face, -0.75f, meme, 0.55f, 0.55f);
			break;
		}
		case CWaiter::E_WAITER_PICKUPCUSTOMER:
		{
			RenderText("PICKUP_CUSTOMER", face, -0.75f, meme, 0.55f, 0.55f);
			break;
		}
		case CWaiter::E_WAITER_SERVE:
		{
			RenderText("SERVE", face, -0.75f, meme, 0.55f, 0.55f);
			break;
		}
		}
		meme -= 0.2f;
	}

#pragma endregion

#pragma region Chef Debug Text

	RenderText("Chef State: ", face, -0.95f, 0.825f, 0.55f, 0.55f);

	switch (chef->GetState())
	{
	case Chef::E_CHEF_WAIT:
	{
		RenderText("Chef_Wait", face, -0.75f, 0.825f, 0.55f, 0.55f);
		break;
	}
	case Chef::E_CHEF_COOK:
	{
		RenderText("Chef_Cook", face, -0.75f, 0.825f, 0.55f, 0.55f);
		break;
	}
	case Chef::E_CHEF_SERVE:
	{
		RenderText("Chef_Serve", face, -0.75f, 0.825f, 0.55f, 0.55f);
		break;
	}
	}

#pragma endregion

#pragma region Customer debug text

	RenderText("Cus State: ", face, -0.5f, 0.925f, 0.55f, 0.55f);

	switch (customerList[0]->GetState())
	{
	case Customer::E_CUSTOMER_EAT:
	{
		RenderText("Eating", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Customer::E_CUSTOMER_IDLE:
	{
		RenderText("Idling", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Customer::E_CUSTOMER_LEAVE:
	{
		RenderText("Leaving", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Customer::E_CUSTOMER_MOVE:
	{
		RenderText("Moving to table", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Customer::E_CUSTOMER_ORDER:
	{
		RenderText("Ordering", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Customer::E_CUSTOMER_QUEUE:
	{
		RenderText("Lining up", face, -0.35f, 0.925f, 0.55f, 0.55f);
		break;
	}
	}

#pragma endregion

#pragma region Caller debug text

	RenderText("Caller State: ", face, -0.15f, 0.925f, 0.55f, 0.55f);

	switch (caller->GetState())
	{
	case Caller::E_CALLER_IDLE:
	{
		RenderText("Idling", face, 0.f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Caller::E_CALLER_CLEAR:
	{
		RenderText("Clearing", face, 0.f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Caller::E_CALLER_MOVE:
	{
		RenderText("Moving to table", face, 0.f, 0.925f, 0.55f, 0.55f);
		break;
	}
	case Caller::E_CALLER_SPAWN:
	{
		RenderText("Back to spawn", face, 0.f, 0.925f, 0.55f, 0.55f);
		break;
	}
	}

#pragma endregion

}

void Render(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		glUseProgram(0);
		glClear(GL_COLOR_BUFFER_BIT);

		Update();
		RunFSM();

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

		string stateString = "";
		stateString = "Message Board : " + messageBoard.getMessage();
		RenderText(stateString, face, 0.5f, -0.525f, 0.5f, 0.5f);

		stateString = "From : " + messageBoard.getLabel_From();
		RenderText(stateString, face, 0.5f, -0.625f, 0.5f, 0.5f);

		stateString = "To: " + messageBoard.getLabel_To();
		RenderText(stateString, face, 0.5f, -0.725f, 0.5f, 0.5f);

		/*RenderText("State : ", face, -0.95f, 0.925f, 0.55f, 0.55f);
		RenderText( stateString , face, -0.8f, 0.925f, 0.55f, 0.55f );
		RenderText("Player - Blue     Enemy - Green     Red - Patrol Point", face, -0.6f, 0.925f, 0.55f, 0.55f);*/

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}