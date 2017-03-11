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
#include <deque>
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
	float3 getNormal() const
	{
		return ( pEnd->pos - pOrigin->pos ).norm();
	}
	float getLength() const
	{
		return ( pEnd->pos - pOrigin->pos ).mod();
	}
};
struct HalfEdge
{
	Edge *pEdge;
	HalfEdge *pNext , *pPrev;
	bool orient;
	Face *pFace;
	Face *getAdjacentFace() const
	{
		return pEdge->aHalfEdges[ 0 ] == this ? pEdge->aHalfEdges[ 1 ]->pFace : pEdge->aHalfEdges[ 0 ]->pFace;
	}
	HalfEdge( Edge *pEdge , Face *pFace , bool orient ) :
		pEdge( pEdge ) ,
		pFace( pFace ) ,
		orient( orient )
	{
		pEdge->aHalfEdges.push_back( this );
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
	float3 getCenter() const
	{
		return ( pEdge->pEnd->pos + pEdge->pOrigin->pos ) / 2;
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
	float dist = 0.0f;
	Face *pFromFace;
	float3 getCenter() const
	{
		return ( loop[ 0 ]->getOrigin() + loop[ 1 ]->getOrigin() + loop[ 2 ]->getOrigin() ) / 3.0f;
	}
	Edge *getFromEdge()
	{
		for( auto hedge : loop )
		{
			if( hedge->getAdjacentFace() == pFromFace )
			{
				return hedge->pEdge;
			}
		}
		return nullptr;
	}
	void draw( DrawList &drawList) const
	{
		drawList.pushTriangle( loop[ 0 ]->getOrigin() , loop[ 1 ]->getOrigin() , loop[ 2 ]->getOrigin() );
	}
	bool collide( float3 const &pos , float3 const &v , float3 &proj )
	{
		auto p0 = loop[ 0 ]->getOrigin();
		auto p1 = loop[ 1 ]->getOrigin();
		auto p2 = loop[ 2 ]->getOrigin();
		if( p0.dist2( p1 ) < MathUtil< float >::EPS
			|| p1.dist2( p2 ) < MathUtil< float >::EPS 
			|| p2.dist2( p0 ) < MathUtil< float >::EPS )
		{
			return false;
		}
		auto norm = ( ( p1 - p0 ) ^ ( p2 - p0 ) ).norm();
		auto dr = pos - p0;
		auto perpDist = dr * norm;
		auto linearDist = -perpDist / ( v * norm );
		if( linearDist < 0.0f )
		{
			return false;
		}
		proj = pos + v * linearDist;
		auto area = ( ( p1 - p0 ) ^ ( p2 - p0 ) ).mod();
		return fabsf( (
			( ( proj - p0 ) ^ ( proj - p1 ) ).mod() +
			( ( proj - p1 ) ^ ( proj - p2 ) ).mod() +
			( ( proj - p2 ) ^ ( proj - p0 ) ).mod()
			) / area - 1.0f ) < 1.0e-3f;
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
std::vector< Vertex* > vertices;
std::vector< Face* > faces;
int main()
{
	GLFWwindow* window;
	GLuint vertex_buffer , line_buffer , index_buffer , vertex_shader , fragment_shader , program;
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
	glGenBuffers( 1 , &line_buffer );
	glBindBuffer( GL_ARRAY_BUFFER , line_buffer );
	glBufferData( GL_ARRAY_BUFFER , 1024 , nullptr , GL_STATIC_DRAW );
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
	glEnable( GL_POLYGON_OFFSET_LINE );

	//glDisable( GL_BLEND );
	float cameraPhi = 0.0f , cameraTheta = 0.0f;
	float cameraZoom = 15.0f;
	double xlastPos = 0;
	double ylastPos = 0;
	int mouseDown = 0;
	glPointSize( 10.0f );
	float3 points[ 2 ] = { {0.0f , 0.0f , 0.0f } , { 0.0f , 0.0f , 0.0f } };
	Face *aFaces[ 2 ] = { nullptr };
	int pointIndex = 0;
	struct Collision
	{
		float3 norm , pos;
		float t , length , dt;
		float3 getPos() const
		{
			return pos + norm * t;
		}
	};
	std::vector< Collision > collisions;
	while( !glfwWindowShouldClose( window ) )
	{
		int width , height;
		glfwGetFramebufferSize( window , &width , &height );
		glViewport( 0 , 0 , width , height );
		glClearColor( 0.5f , 0.5f , 0.5f , 1.0f );
		glClearDepth( 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		double xpos , ypos;
		glfwGetCursorPos( window , &xpos , &ypos );
		
		int mouseState = glfwGetMouseButton( window , GLFW_MOUSE_BUTTON_LEFT );
		float3 cameraPos = float3{ cosf( cameraTheta ) * cosf( cameraPhi ) ,cosf( cameraTheta ) * sinf( cameraPhi ),sinf( cameraTheta ) } *cameraZoom;
		if( mouseState == GLFW_PRESS )
		{
			if( !mouseDown )
			{
				float3 proj;
				float3 cameraLook = -cameraPos.norm();
				float3 cameraLeft = ( cameraLook ^ float3( 0.0f , 0.0f , 1.0f ) ).norm();
				float3 cameraUp = cameraLook ^ cameraLeft;
				float u = xpos / width * 2.0f - 1.0f;
				float v = ypos / height * 2.0f - 1.0f;
				float3 ray = ( cameraLook * 1.0f / MathUtil< float >::tan( 0.7f ) + cameraLeft * u + cameraUp * v ).norm();
				float minDist = 1000.0f;
				Face *pCollidedFace = nullptr;
				for( auto pFace : faces )
				{
					float3 lproj;
					if( pFace->collide( cameraPos , ray , lproj ) && lproj.dist2( cameraPos ) < minDist )
					{
						minDist = lproj.dist2( cameraPos );
						pCollidedFace = pFace;
						proj = lproj;
					}
				}
				if( pCollidedFace )
				{
					points[ pointIndex ] = proj;
					aFaces[ pointIndex ] = pCollidedFace;
					if( aFaces[ 0 ] && aFaces[ 1 ] )
					{
						collisions.clear();
						for( auto pFace : faces )
						{
							pFace->dist = 9999.0f;
							pFace->pFromFace = nullptr;
						}
						std::deque< Face* > faceQ;
						faceQ.push_back( aFaces[ 0 ] );
						aFaces[ 0 ]->dist = 0.0f;
						[ & ]()
						{
							while( !faceQ.empty() )
							{
								Face *seed = faceQ.front();
								faceQ.pop_front();
								float3 center = seed == aFaces[ 0 ] ? points[ 0 ] : seed == aFaces[ 1 ] ? points[ 1 ] : seed->getCenter();
								for( auto &hedge : seed->loop )
								{
									Face *adjFace = hedge->getAdjacentFace();
									float dist = seed->dist + hedge->getCenter().dist( center) + adjFace->getCenter().dist( hedge->getCenter() );
									if( dist < adjFace->dist )
									{
										adjFace->dist = dist;
										adjFace->pFromFace = seed;
										/*if( adjFace == aFaces[ 1 ] )
										{
											return;
										}*/
										faceQ.push_back( adjFace );
									}
								}
							}
						}( );
						Face *pFace = aFaces[ 1 ];
						while( pFace != nullptr && pFace != aFaces[ 0 ] )
						{
							auto edge = pFace->getFromEdge();
							collisions.push_back( { edge->getNormal() , edge->pOrigin->pos , edge->getLength() * 0.5f , edge->getLength() } );
							pFace = pFace->pFromFace;
						}
					}
				}
				pointIndex = ( pointIndex + 1 ) & 0x1;
				
			}
			mouseDown = 1;
		} else if( mouseState == GLFW_RELEASE )
		{
			mouseDown = 0;
		}
		if( mouseDown )
		{
			auto dx = -(xpos - xlastPos)/width;
			auto dy = (ypos - ylastPos)/height;
			cameraPhi += dx;
			cameraTheta += dy;
		}
		xlastPos = xpos;
		ylastPos = ypos;
		glEnable( GL_DEPTH_TEST );
		glUseProgram( program );
		auto viewProj = Camera::perspectiveLookAt( cameraPos ,
			{ 0.0f,0.0f,0.0f } , { 0.0f,0.0f,1.0f } , 0.01f , 1000.0f , 1.4f , 1.4f );
		glUniformMatrix4fv( 1 , 1 , GL_TRUE , viewProj._data );
		glUniform4f( 0 , 1.0f , 1.0f , 1.0f , 1.0f );
		glBindBuffer( GL_ARRAY_BUFFER , vertex_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , index_buffer );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0 , 3 , GL_FLOAT , GL_FALSE , 12 , 0 );
		glDepthFunc( GL_LESS );
		glPolygonOffset( 0.0f , 0.0f );
		glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
		glDrawElements( GL_TRIANGLES , drawList.aIndices.size() , GL_UNSIGNED_INT , 0 );
		glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
		glDepthFunc( GL_LEQUAL );
		glPolygonOffset( -1.0f , 0.0f );
		glUniform4f( 0 , 0.0f , 0.0f , 0.0f , 1.0f );
		glDrawElements( GL_TRIANGLES , drawList.aIndices.size() , GL_UNSIGNED_INT , 0 );

		
		//glUseProgram( 0 );
		
		glDisable( GL_DEPTH_TEST );
		glUniform4f( 0 , 1.0f , 0.0f , 0.0f , 1.0f );
		auto der = []( Collision const &ci , Collision const &cj )
		{
			return cj.t * ( cj.norm * ci.norm ) + ci.norm * cj.pos;
		};
		if( aFaces[ 0 ] && aFaces[ 1 ] )
		{
			for( int iter = 0; iter < 10; iter++ )
			{
				if( collisions.size() > 1 )
				{
#pragma omp parallel for
					for( int i = 0; i < collisions.size(); i++ )
					{
						float3 point0 , point1;
						if( i == 0 )
						{
							point0 = points[ 1 ];
							point1 = collisions[ i + 1 ].getPos();
						} else if( i == collisions.size() - 1 )
						{
							point0 = collisions[ i - 1 ].getPos();
							point1 = points[ 0 ];
						} else
						{
							point0 = collisions[ i - 1 ].getPos();
							point1 = collisions[ i + 1 ].getPos();
						}
						

						float B = collisions[ i ].t + collisions[ i ].norm * collisions[ i ].pos;
						float dist0 = collisions[ i ].getPos().dist( point0 ) + 0.001f;
						float dt0 = collisions[ i ].norm * point0 - B;
						float dist1 = collisions[ i ].getPos().dist( point1 ) + 0.001f;
						float dt1 = collisions[ i ].norm * point1 - B;
						collisions[ i ].dt = dt0 / dist0 + dt1 / dist1;
					}
				} else if( collisions.size() )
				{
					float dist0 = collisions[ 0 ].getPos().dist( points[ 0 ] );
					float dt0 = collisions[ 0 ].norm * points[ 0 ] - (
						collisions[ 0 ].t + collisions[ 0 ].norm * collisions[ 0 ].pos
						);
					float dist1 = collisions[ 0 ].getPos().dist( points[ 1 ] );
					float dt1 = collisions[ 0 ].norm * points[ 1 ] -(
						collisions[ 0 ].t + collisions[ 0 ].norm * collisions[ 0 ].pos
						);
					collisions[ 0 ].dt = dt0 / dist0 + dt1 / dist1;
				}
				for( auto &col : collisions )
				{
					col.t += col.dt * 0.01f;
					col.t = fmaxf( 0.0f , fminf( col.t , col.length ) );
				}
			}
			std::vector< float > lines;
			lines.push_back( points[ 1 ].x );
			lines.push_back( points[ 1 ].y );
			lines.push_back( points[ 1 ].z );
			for( auto &col : collisions )
			{
				float3 center = col.getPos();
				lines.push_back( center.x );
				lines.push_back( center.y );
				lines.push_back( center.z );
			}
			lines.push_back( points[ 0 ].x );
			lines.push_back( points[ 0 ].y );
			lines.push_back( points[ 0 ].z );
			glBindBuffer( GL_ARRAY_BUFFER , line_buffer );
			glBufferData( GL_ARRAY_BUFFER , lines.size() * 4 , &lines[ 0 ] , GL_STATIC_DRAW );
			glEnableVertexAttribArray( 0 );
			glVertexAttribPointer( 0 , 3 , GL_FLOAT , GL_FALSE , 12 , 0 );
			glDrawArrays( GL_LINE_STRIP , 0 , lines.size() / 3 );
		}
		glBegin( GL_POINTS );
		glVertex3f( points[ 0 ].x , points[ 0 ].y , points[ 0 ].z );
		glVertex3f( points[ 1 ].x , points[ 1 ].y , points[ 1 ].z );
		glEnd();
		glfwSwapBuffers( window );
		glfwPollEvents();
	}
	glfwDestroyWindow( window );
	glfwTerminate();
	exit( EXIT_SUCCESS );
	return 0;
}