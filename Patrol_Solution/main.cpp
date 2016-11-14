// Program : Patrol Solution

#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <math.h>
#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GLFw/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "MyVector.h"
using namespace std;

#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#pragma region DO NOT TOUCH

void Render( GLFWwindow* window );

int RandomInteger( int lowerLimit, int upperLimit )
{
	return rand() % ( upperLimit - lowerLimit + 1 ) + lowerLimit;
}

// long integer to string
string itos( const long value )
{ 
	ostringstream buffer; 
	buffer << value; 
	return buffer.str();
}
  
float GetDistance(float x1, float y1, float x2, float y2) { return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); } // OK

// Within range
bool Detect( MyVector pos1, MyVector pos2, float radius1, float radius2)
{
	bool detect = false;
	float totalRadius = radius1 + radius2;
	float distance = GetDistance(pos1.x, pos1.y, pos2.x, pos2.y);
	if (distance <= totalRadius) detect = true;
	return detect;
}

void RenderCircle( GLfloat x, GLfloat y, GLfloat radius, GLfloat r, GLfloat g, GLfloat b )
{
	int n = 360;
	glColor3f( r, g, b );
	glBegin( GL_POINTS );
	for ( int i = 0; i <= n; i++ )
	{
		float angle = (float) ( i * ( 2.0 * 3.14159 / n ) );
		glVertex2f( x + radius * cos( angle ), y + radius * sin( angle ) );
	}
	glEnd();
}

void RenderFillCircle( GLfloat x, GLfloat y, GLfloat radius, GLfloat r, GLfloat g, GLfloat b )
{
	int n = 360;
	glColor3f ( r, g, b );
	glBegin(GL_TRIANGLE_FAN );
	glVertex2f( x, y );
	for (int i = 0; i <= n; i++)
	{
		float angle = (float) ( i * ( 2.0 * 3.14159 / n ) );
		glVertex2f( x + radius * cos( angle ), y + radius * sin( angle ) );
	}
	glEnd();
}

#pragma region SHADER RELATED

// SHADERS //////////////////////////////////////////////////////////////////
const char *VERTEX_SHADER = ""
"#version 410 core\n"
"in vec4 in_Position;\n"
"out vec2 texCoords;\n"
"void main(void2 {\n"
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

#pragma endregion

#pragma region CALLBACKS

static void ErrorCallBack ( int error, const char*description )
{
	fputs( description, stderr );
}

float width, height;
GLFWwindow* window;
void Render(GLFWwindow* window);

static void ResizeCallBack(GLFWwindow *window, int w, int h)
{
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 60, (float) w / (float) h, 0, 100 );
	glMatrixMode( GL_MODELVIEW );
}

#pragma endregion

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

int state;            // Current state value
const int PATROL = 0; // Possible state definition
const int CHASE = 1;

// Vector of Table positions
vector<MyVector> tables;


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
const float chefSpeed = 0.01f;
const float cookTime = 5.0f;
MyVector chefPos;
MyVector chefStation;
vector<MyVector> chefWaypoints;

bool isDone; 

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
WAITER_STATE waiterState;
const float waiterSpeed = 0.02f;
MyVector waiterPos;
bool foodReady;
bool availableCustomers;
bool customerPickup;

// Customer related
enum CUS_STATE
{
	E_CUSTOMER_IDLE,
	E_CUSTOMER_MOVE,
	E_CUSTOMER_ORDER,
	E_CUSTOMER_EAT,
	E_CUSTOMER_LEAVE,
	E_CUSTOMER_MAX
};
CUS_STATE customerState;
const float customerSpeed = 0.015f;
float eatSpeed = 0.1f;
bool customerSeated;
MyVector customerPos;


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
MyVector callerPos;

#pragma endregion

const float playerSpeed = 0.0175f;
const float enemySpeed = 0.0200f;
const float playerRadius = 0.25f;
const float enemyRadius = 0.1f;
const float proximity = 0.4f;
int waypointIndex;
bool arrived;
MyVector playerPos, enemyPos;
vector <MyVector> wayPoints, intrusionPoints;
MyVector nextPoint;

static void KeyCallBack( GLFWwindow *window, int key, int scancode, int action, int mods )
{
	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
		glfwSetWindowShouldClose( window, GL_TRUE );
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		foodReady = true;
	}
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
	{
		availableCustomers = true;
	}
}

void SimulationInit()
{
	chefStation = MyVector(0.f, -3.f);
	srand( ( unsigned ) time( NULL ) );
	float offset = 2.0;
	wayPoints.push_back( MyVector( -offset, -offset ));
	wayPoints.push_back( MyVector( -offset,  offset));
	wayPoints.push_back( MyVector(  offset,  offset));
	wayPoints.push_back( MyVector(  offset, -offset));

	// Customer waypoint
	wayPoints.push_back(MyVector(3.5f, 2.5f));

	intrusionPoints.push_back( MyVector( -1.2f*offset, -0.3f*offset ) );
	intrusionPoints.push_back( MyVector( -1.2f*offset,  0.3f*offset ) );
	intrusionPoints.push_back( MyVector(  1.2f*offset,  0.3f*offset ) );
	intrusionPoints.push_back( MyVector(  1.2f*offset, -0.3f*offset ) );
	int randomIndex = RandomInteger(1, 3);
	enemyPos.SetPosition( intrusionPoints[ randomIndex ].GetX(), intrusionPoints[ randomIndex ].GetY() );

	// Diner AI usage
	waiterPos.SetPosition(wayPoints[0].GetX(), wayPoints[0].GetY());
	customerPos.SetPosition(wayPoints[4].GetX(), wayPoints[4].GetY());
	
	// Set AI States
	waiterState = E_WAITER_IDLE;
	customerState = E_CUSTOMER_IDLE;
	state = PATROL;

	//Set AI Conditions
	foodReady = false;
	waypointIndex = 1;
	arrived = false;
	availableCustomers = false;
	customerSeated = false;
	customerPickup = false;
}

void PrepareFood()
{
	//==========PSEUDO CODE============//
	//if order comes in, cook the food
	//when food is done change boolean foodReady to true;
	foodReady = true;
}

int main()
{
	// INIT ///////////////////////////////////////////////////////////////
	char *title = "Patrol";
	width = 1280;
	height = 720;
	
	glfwSetErrorCallback( ErrorCallBack );
	if ( !glfwInit() )
		exit( EXIT_FAILURE );
	glfwWindowHint(GLFW_SAMPLES,4);
	GLFWwindow* window = glfwCreateWindow( (int) width, (int) height, title, NULL , NULL );
	
	if (!window)
	{
		fprintf( stderr, "Failed to create GLFW windows.\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	
	glfwMakeContextCurrent( window );
	glfwSetKeyCallback( window, KeyCallBack );
	glfwSetWindowSizeCallback( window, ResizeCallBack );
	
	GLenum error =  glewInit();
	if ( error != GLEW_OK)
	{
		fprintf(stderr,"Error : %s\n", glewGetErrorString( error ) );
		exit( EXIT_FAILURE );
	}

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 60, width / height, 0, 100 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	///////////////////////////////////////////////////////////////////////////
	
	// Initialize and load our freetype face
	if ( FT_Init_FreeType( &ft_lib ) != 0 )
	{
		fprintf( stderr, "Couldn't initialize FreeType library\n" );
		Cleanup();
		exit( EXIT_FAILURE );
	}
	
	if ( FT_New_Face(ft_lib, "arial.ttf", 0, &face ) != 0 )
	{
		fprintf( stderr, "Unable to load arial.ttf\n" );
		Cleanup();
		exit( EXIT_FAILURE );
	}
	// Initialize our texture and VBOs
	glGenBuffers( 1, &vbo );
	glGenVertexArrays( 1, &vao );
	glGenTextures( 1, &texture );
	glGenSamplers(1, &sampler);
	glSamplerParameteri( sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glSamplerParameteri( sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glSamplerParameteri( sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glSamplerParameteri( sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
	// Initialize shader
	vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &VERTEX_SHADER, 0 );
	glCompileShader( vs );

	fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &FRAGMENT_SHADER, 0 );
	glCompileShader( fs );

	program = glCreateProgram();
	glAttachShader( program, vs );
	glAttachShader( program, fs );
	glLinkProgram( program );
	
	// Initialize GL state
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Get shader uniforms
	glUseProgram( program );
	glBindAttribLocation( program, 0, "in_Position" );
	texUniform = glGetUniformLocation( program, "tex" );
	colorUniform = glGetUniformLocation( program, "color" );

	SimulationInit();
	Render( window );
	DoExit( );
}

void RenderObjects()
{	
	glPushMatrix();
	glTranslatef( 0.0f, 0.0f, -10.0f );
	
	// Waiter
	RenderFillCircle(waiterPos.GetX(), waiterPos.GetY(), playerRadius, 0.0f, 0.0f, 1.0f); // player object
	RenderCircle(waiterPos.GetX(), waiterPos.GetY(), playerRadius + proximity, 0.1f, 0.1f, 0.1f); // player proximity

	// Customer (temp)
	RenderFillCircle(customerPos.GetX(), customerPos.GetY(), playerRadius, 1.f, 1.f, 0.f); // object
	RenderCircle(customerPos.GetX(), customerPos.GetY(), playerRadius + proximity, 0.1f, 0.1f, 0.1f); // proximity

	// Enemy
	RenderFillCircle( enemyPos.GetX(), enemyPos.GetY(), enemyRadius, 0.0f, 1.0f, 0.0f ); // enemy object
	// Waypoints
	for (unsigned int i = 0; i < wayPoints.size(); i++ )
		RenderCircle(wayPoints[i].GetX(), wayPoints[i].GetY(), playerRadius + 0.1f, 1.0f, 0.0f, 0.0f); //  waypoints

	//Food Station
	RenderCircle(chefStation.GetX(), chefStation.GetY(), playerRadius + 0.1f, 1.0f, 0.0f, 0.0f);
		
	glPopMatrix();
}

void RunFSM()
{
	bool enemyDetected = Detect( playerPos, enemyPos, playerRadius+proximity , enemyRadius );
	switch ( state )
	{
		case PATROL:	if ( enemyDetected ) 
							state = CHASE;
						break;

		case CHASE :	if ( !enemyDetected ) 
							state = PATROL;
						break;
	}

	if (foodReady)
	{
		waiterState = E_WAITER_PICKUP;
	}

	if (availableCustomers)
	{
		waiterState = E_WAITER_PICKUPCUSTOMER;
	}

	/*if (customerPickup)
	{
		customerState = E_CUSTOMER_MOVE;
	}*/
}

void Update()
{

	////Get User Input
	//if ( state != CHASE )
	//{
	//	nextPoint = wayPoints[waypointIndex];
	//	MyVector direction = ( playerPos - nextPoint ).Normalize();
	//	float distance = GetDistance(playerPos.GetX(), playerPos.GetY(), nextPoint.GetX(), nextPoint.GetY());
	//	if ( distance < playerSpeed )
	//	{
	//		playerPos = nextPoint;
	//		arrived = true;
	//	}
	//	else
	//		playerPos = playerPos + direction*playerSpeed;

	//	if ( arrived )
	//	{
	//		if ( waypointIndex == wayPoints.size() - 1)
	//			waypointIndex = 0;
	//		else
	//			waypointIndex++;
	//		arrived = false;
	//	}
	//}

#pragma region Waiter Updates

	if (waiterState == E_WAITER_PICKUP)
	{
		MyVector direction = (waiterPos - chefStation).Normalize();
		float distance = GetDistance(waiterPos.GetX(), waiterPos.GetY(), chefStation.GetX(), chefStation.GetY());
		if (distance < waiterSpeed)
		{
			arrived = true;
		}
		else
		{
			waiterPos = waiterPos + direction * waiterSpeed;
		}

		if (arrived)
		{
			waiterState = E_WAITER_SERVE;
			arrived = false;
			foodReady = false;
		}
	}
	if (waiterState == E_WAITER_SERVE)
	{
		//Find a table to serve
		MyVector direction = (waiterPos - wayPoints[1]).Normalize();
		float distance = GetDistance(waiterPos.GetX(), waiterPos.GetY(), wayPoints[0].GetX(), wayPoints[0].GetY());
		if (distance < waiterSpeed)
		{
			arrived = true;
		}
		else
		{
			waiterPos = waiterPos + direction * waiterSpeed;
		}

		if (arrived)
		{
			waiterState = E_WAITER_IDLE;
			arrived = false;
		}
	}
	if (waiterState == E_WAITER_PICKUPCUSTOMER)
	{
		MyVector direction = (waiterPos - customerPos).Normalize();
		float distance = GetDistance(waiterPos.GetX(), waiterPos.GetY(), customerPos.GetX(), customerPos.GetY());

		if (distance < waiterSpeed)
		{
			customerPickup = true;
		}
		else
		{
			waiterPos = waiterPos + direction * waiterSpeed;
		}

		if (customerPickup)
		{
			waiterState = E_WAITER_MOVE;
			customerState = E_CUSTOMER_MOVE;
			customerPickup = false;
			availableCustomers = false;
		}
	}

	if (waiterState == E_WAITER_MOVE)
	{
		MyVector direction = (waiterPos - wayPoints[1]).Normalize();
		float distance = GetDistance(waiterPos.GetX(), waiterPos.GetY(), wayPoints[1].GetX(), wayPoints[1].GetY());

		if (distance < waiterSpeed)
		{
			customerSeated = true;
		}
		else
		{
			waiterPos = waiterPos + direction * waiterSpeed;
		}

		if (customerSeated)
		{
			waiterState = E_WAITER_IDLE;
			customerSeated = false;
		}
	}

#pragma endregion

#pragma region Customer Updates

	if (customerState == E_CUSTOMER_MOVE)
	{
		MyVector direction = (customerPos - wayPoints[1]).Normalize();
		float distance = GetDistance(customerPos.GetX(), customerPos.GetY(), wayPoints[1].GetX(), wayPoints[1].GetY());

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
			customerState = E_CUSTOMER_IDLE;
			customerSeated = false;
		}
	}

#pragma endregion

}

void Render( GLFWwindow* window )
{
	while ( !glfwWindowShouldClose( window) )
	{
		glUseProgram( 0 );
		glClear( GL_COLOR_BUFFER_BIT );

		Update();
		RunFSM();
	
		string stateString = "";
		MyVector direction;
		switch ( state )
		{
			case PATROL  :	stateString = "PATROL";
							break;
			case CHASE :	stateString = "CHASE";
							direction = ( playerPos - enemyPos ).Normalize();
							enemyPos = enemyPos + direction * enemySpeed;
							playerPos = playerPos + direction  * playerSpeed;
							break;
		}
		
		RenderObjects();
	
		// Bind stuff
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, texture );
		glBindSampler( 0, sampler);
		glBindVertexArray( vao );
		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glUseProgram( program );
		glUniform4f( colorUniform, 1, 1, 1, 1 );
		glUniform1i( texUniform, 0 );

		FT_Set_Pixel_Sizes( face, 0, 50 );

		RenderText("State : ", face, -0.95f, 0.925f, 0.55f, 0.55f);
		RenderText( stateString , face, -0.8f, 0.925f, 0.55f, 0.55f );
		RenderText("Player - Blue     Enemy - Green     Red - Patrol Point", face, -0.6f, 0.925f, 0.55f, 0.55f);

		glfwSwapBuffers( window );
		glfwPollEvents();
	}
}