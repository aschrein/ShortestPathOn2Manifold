#include <float.h>          // FLT_MAX
#include <stdarg.h>         // va_list
#include <stddef.h>         // ptrdiff_t, NULL
#include <string.h>         // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp
#include <stdlib.h>
#include <stdio.h>
#define GLEW_STATIC
#include "GL\glew.h"
#include "tiny_obj_loader.h"
#include "GLFW/glfw3.h"
#include "math\vec.hpp"
#include "Camera.hpp"
#include <iostream>
#include <memory>
using namespace Math;
static void error_callback( int error , const char* description )
{
	fprintf( stderr , "Error: %s\n" , description );
}
static const char* vertex_shader_text =
"attribute vec3 vPos;\n"
"uniform mat4 viewProj;\n"
"void main()\n"
"{\n"
"    gl_Position = viewProj * vec4(vPos, 1.0);\n"
"}\n";
static const char* fragment_shader_text =
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color);//texture( tex , uv );\n"
"}\n";

struct Vertex
{
	typedef std::shared_ptr< Vertex > PTR;
	float3 pos;
};
struct Edge
{
	typedef std::shared_ptr< Edge > PTR;
	Vertex::PTR origin;
	Vertex::PTR end;
};
struct HalfEdge
{
	typedef std::shared_ptr< HalfEdge > PTR;
	Edge::PTR edge;
};
struct DrawList
{
	std::vector< float > positions;
	std::vector< int > indices;
	void pushTriangle( float3 p1 , float3 p2 , float3 p3 )
	{
		int topIndex = positions.size() / 3;
		positions.push_back( p1.x );
		positions.push_back( p1.y );
		positions.push_back( p1.z );
		positions.push_back( p2.x );
		positions.push_back( p2.y );
		positions.push_back( p2.z );
		positions.push_back( p3.x );
		positions.push_back( p3.y );
		positions.push_back( p3.z );
		indices.push_back( topIndex );
		indices.push_back( topIndex + 1 );
		indices.push_back( topIndex + 2 );
	}
};
struct Face
{
	std::vector< HalfEdge::PTR > loop;
	void draw( DrawList &drawList)
	{
		drawList.pushTriangle( loop[ 0 ]->edge->end->pos , loop[ 1 ]->edge->end->pos , loop[ 2 ]->edge->end->pos );
	}
	typedef std::shared_ptr< Face > PTR;
};
int main()
{
	GLFWwindow* window;
	GLuint vertex_buffer , index_buffer , vertex_shader , fragment_shader , program;
	glfwSetErrorCallback( error_callback );
	if( !glfwInit() )
		exit( EXIT_FAILURE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR , 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR , 0 );
	window = glfwCreateWindow( 640 , 480 , "Simple example" , NULL , NULL );
	if( !window )
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	glfwMakeContextCurrent( window );
	if( glewInit() )
		exit( EXIT_FAILURE );
	glfwSwapInterval( 1 );
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj( &attrib , &shapes , &materials , &err , "test.obj" , "" , true );

	if( !err.empty() )
	{
		std::cerr << err << std::endl;
	}

	if( !ret )
	{
		printf( "Failed to load/parse .obj.\n" );
		return false;
	}
	std::vector<int > indices;
	for( auto t : shapes[ 0 ].mesh.indices )
	{
		indices.push_back( t.vertex_index );
	}
	glGenBuffers( 1 , &index_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER , indices.size() * 4 , &indices[ 0 ] , GL_STATIC_DRAW );
	
	glGenBuffers( 1 , &vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER , vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER , attrib.vertices.size() * 4 , &attrib.vertices[ 0 ] , GL_STATIC_DRAW );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0 , 3 , GL_FLOAT , GL_FALSE , 12 , 0 );

	vertex_shader = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vertex_shader , 1 , &vertex_shader_text , NULL );
	glCompileShader( vertex_shader );
	fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fragment_shader , 1 , &fragment_shader_text , NULL );
	glCompileShader( fragment_shader );
	program = glCreateProgram();
	glAttachShader( program , vertex_shader );
	glAttachShader( program , fragment_shader );
	glLinkProgram( program );
	glEnable( GL_DEPTH_TEST );
	//glDisable( GL_BLEND );

	while( !glfwWindowShouldClose( window ) )
	{
		int width , height;
		glfwGetFramebufferSize( window , &width , &height );
		glViewport( 0 , 0 , width , height );
		glClearColor( 0.5f , 0.5f , 0.5f , 1.0f );
		glClearDepth( 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


		glUseProgram( program );
		auto viewProj = Camera::perspectiveLookAt( float3{ -10.0f ,-10.0f,10.0f } * 0.5f , { 0.0f,0.0f,0.0f } , { 0.0f,0.0f,1.0f } , 0.01f , 1000.0f , 1.4f , 1.4f );
		glUniformMatrix4fv( 1 , 1 , GL_TRUE , viewProj._data );
		glUniform4f( 0 , 1.0f , 1.0f , 1.0f , 1.0f );
		glDepthFunc( GL_LESS );
		glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
		glDrawElements( GL_TRIANGLES , indices.size() , GL_UNSIGNED_INT , 0 );
		glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
		glDepthFunc( GL_LEQUAL );
		glUniform4f( 0 , 0.0f , 0.0f , 0.0f , 1.0f );
		glDrawElements( GL_TRIANGLES , indices.size() , GL_UNSIGNED_INT , 0 );
		glfwSwapBuffers( window );
		glfwPollEvents();
	}
	glfwDestroyWindow( window );
	glfwTerminate();
	exit( EXIT_SUCCESS );
	return 0;
}