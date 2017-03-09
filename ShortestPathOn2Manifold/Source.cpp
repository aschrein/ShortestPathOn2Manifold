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
#include <unordered_set>
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
struct Edge;
struct HalfEdge;
struct Face;

struct Vertex
{
	float3 pos;
	std::vector< Edge* > aEdges;
	int useCounter;
	Vertex( float3 pos ) :
		pos( pos )
	{}
};
struct Edge
{
	Vertex *pOrigin;
	Vertex *pEnd;
	std::vector< HalfEdge* > aHalfEdges;
	int useCounter = 0;
	Edge( Vertex *pOrigin , Vertex *pEnd ) :
		pOrigin( pOrigin ) ,
		pEnd( pEnd )
	{
		pEnd->useCounter++;
		pOrigin->useCounter++;
	}
	~Edge()
	{
		if( !--pEnd->useCounter )
		{
			delete pEnd;
		}
		if( !--pOrigin->useCounter )
		{
			delete pEnd;
		}
	}
};
struct HalfEdge
{
	Edge *pEdge;
	HalfEdge *pNext , *pPrev;
	bool orient;
	Face *pFace;
	HalfEdge( Edge *pEdge , Face *pFace , bool orient ) :
		pEdge( pEdge ) ,
		pFace( pFace ) ,
		orient( orient )
	{
		pEdge->useCounter++;
	}
	float3 getOrigin() const
	{
		if( orient )
		{
			return pEdge->pOrigin->pos;
		} else
		{
			return pEdge->pEnd->pos;
		}
	}
	~HalfEdge()
	{
		if( !--pEdge->useCounter )
		{
			delete pEdge;
		}
	}
};
struct DrawList
{
	std::vector< float > aPositions;
	std::vector< int > aIndices;
	void pushTriangle( float3 p1 , float3 p2 , float3 p3 )
	{
		int topIndex = aPositions.size() / 3;
		aPositions.push_back( p1.x );
		aPositions.push_back( p1.y );
		aPositions.push_back( p1.z );
		aPositions.push_back( p2.x );
		aPositions.push_back( p2.y );
		aPositions.push_back( p2.z );
		aPositions.push_back( p3.x );
		aPositions.push_back( p3.y );
		aPositions.push_back( p3.z );
		aIndices.push_back( topIndex );
		aIndices.push_back( topIndex + 1 );
		aIndices.push_back( topIndex + 2 );
	}
};
struct Face
{
	std::vector< HalfEdge* > loop;
	void draw( DrawList &drawList)
	{
		drawList.pushTriangle( loop[ 0 ]->getOrigin() , loop[ 1 ]->getOrigin() , loop[ 2 ]->getOrigin() );
	}
	~Face()
	{
		for( auto &hedge : loop )
		{
			delete hedge;
		}
	}
};
typedef std::shared_ptr< Face > FacePTR;
void createHalfEdge( Face *pFace , Vertex *pOrigin , Vertex *pEnd )
{
	Edge *pEdge = nullptr;
	bool order = true;
	for( auto const &pIterEdge : pOrigin->aEdges )
	{
		if( pIterEdge->pOrigin == pOrigin && pIterEdge->pEnd == pEnd )
		{
			pEdge = pIterEdge;
			break;
		}
		if( pIterEdge->pEnd == pOrigin && pIterEdge->pOrigin == pEnd )
		{
			order = false;
			pEdge = pIterEdge;
			break;
		}
	}
	if( !pEdge )
	{
		pEdge = new Edge( pOrigin , pEnd );
		pOrigin->aEdges.push_back( pEdge );
		pEnd->aEdges.push_back( pEdge );
	}

	auto pHalfEdge = new HalfEdge( pEdge , pFace , order );
	pFace->loop.push_back( pHalfEdge );
	pHalfEdge->pNext = pFace->loop[ 0 ];
	pHalfEdge->pPrev = pFace->loop[ pFace->loop.size() - 1 ];
	pFace->loop[ 0 ]->pPrev = pHalfEdge;
}
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
	std::vector< Vertex* > vertices;
	std::vector< Face* > faces;
	{
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
		for( int i = 0; i < attrib.vertices.size() / 3; i++ )
		{
			vertices.push_back(
				new Vertex{
					{attrib.vertices[ i * 3 ] ,attrib.vertices[ i * 3 + 1 ] ,attrib.vertices[ i * 3 + 2 ] }
			} );
		}
		for( int i = 0; i < shapes[ 0 ].mesh.indices.size() / 3; i++ )
		{
			auto vertex0 = vertices[ shapes[ 0 ].mesh.indices[ i * 3 ].vertex_index ];
			auto vertex1 = vertices[ shapes[ 0 ].mesh.indices[ i * 3 + 1 ].vertex_index ];
			auto vertex2 = vertices[ shapes[ 0 ].mesh.indices[ i * 3 + 2 ].vertex_index ];
			auto pFace = new Face();
			createHalfEdge( pFace , vertex0 , vertex1 );
			createHalfEdge( pFace , vertex1 , vertex2 );
			createHalfEdge( pFace , vertex2 , vertex0 );
			faces.push_back( pFace );
		}
	}
	DrawList drawList;
	for( auto pFace : faces )
	{
		pFace->draw( drawList );
	}
	glGenBuffers( 1 , &index_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER , drawList.aIndices.size() * 4 , &drawList.aIndices[ 0 ] , GL_STATIC_DRAW );
	
	glGenBuffers( 1 , &vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER , vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER , drawList.aPositions.size() * 4 , &drawList.aPositions[ 0 ] , GL_STATIC_DRAW );
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
		glDrawElements( GL_TRIANGLES , drawList.aIndices.size() , GL_UNSIGNED_INT , 0 );
		glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
		glDepthFunc( GL_LEQUAL );
		glUniform4f( 0 , 0.0f , 0.0f , 0.0f , 1.0f );
		glDrawElements( GL_TRIANGLES , drawList.aIndices.size() , GL_UNSIGNED_INT , 0 );
		glfwSwapBuffers( window );
		glfwPollEvents();
	}
	glfwDestroyWindow( window );
	glfwTerminate();
	exit( EXIT_SUCCESS );
	return 0;
}