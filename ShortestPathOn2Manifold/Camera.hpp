#ifndef RCAMERA_H
#define RCAMERA_H
#include "math/vec.hpp"
#include "math/mat.hpp"
#include "math/Math.hpp"
using namespace Math;
struct Camera
{
	float nearplane = 0.1f;
	float farplane = 1000.0f;
	float3 local_x , local_y , local_z , pos;
	float fovx = 1.4f;
	float fovy = 1.4f;
	f4x4 calculateViewProj()
	{
		float    h , w , Q;
		w = ( float )1.0f / MathUtil< float >::tan( fovx * 0.5 );
		h = ( float )1.0f / MathUtil< float >::tan( fovy * 0.5 );
		Q = farplane / ( farplane - nearplane );
		f4x4 proj_matrix;
		proj_matrix( 0 , 0 ) = w;
		proj_matrix( 1 , 1 ) = h;
		proj_matrix( 2 , 2 ) = Q;
		proj_matrix( 2 , 3 ) = -Q * nearplane;
		proj_matrix( 3 , 2 ) = 1.0f;
		float3 const &z = local_z;
		float3 const &x = local_x;
		float3 const &y = local_y;
		f4x4 view = f4x4(
			x.x , y.x , local_z.x , 0.0f ,
			x.y , y.y , local_z.y , 0.0f ,
			x.z , y.z , local_z.z , 0.0f ,
			-x * pos , -y * pos , -local_z * pos , 1.0f ).trans();
		f4x4 cam_matrix =
			MatUtil::wrap( MatUtil::mat3rows( local_x , local_y , local_z ) ) *
			MatUtil::translate( -pos )
			;
		return proj_matrix * cam_matrix;
	}
	void lookAt( const float3 &pos , const float3 &sight_point , const float3 up_dir = { 0.0f , 0.0f , 1.0f } )
	{
		local_z = ( sight_point - pos ).norm();
		local_x = ( local_z ^ up_dir ).norm();
		local_y = local_x ^ local_z;
		this->pos = pos;
	}

	void setPos( const float3 pos )
	{
		this->pos = pos;
	}
	void setAngle( const float phi , const float theta , const float3 up_dir = { 0.0f , 0.0f , 1.0f } )
	{
		local_z = float3( MathUtil< float >::sin( theta ) * MathUtil< float >::cos( phi ) , MathUtil< float >::sin( theta ) * MathUtil< float >::sin( phi ) , MathUtil< float >::cos( theta ) );
		local_x = ( local_z ^ up_dir ).norm();
		local_y = local_x ^ local_z;
	}
	void setAspect( float ax , float ay )
	{
		fovx = ax;
		fovy = ay;
	}
	void setPerspective( const float nearp , const float farp , const float aspectx , const float aspecty )
	{
		nearplane = nearp;
		farplane = farp;
		fovx = aspectx;
		fovy = aspecty;
	}
	static f4x4 perspectiveLookAt( const float3 &pos , const float3 &sight_point , const float3 &up_dir , const float nearplane , const float farplane , const float fovx , const float fovy )
	{
		float3 local_z = ( sight_point - pos ).norm();
		float3 local_x = ( local_z ^ up_dir ).norm();
		float3 local_y = local_x ^ local_z;
		float    h , w , Q;
		w = ( float )1.0f / MathUtil< float >::tan( fovx * 0.5 );
		h = ( float )1.0f / MathUtil< float >::tan( fovy * 0.5 );
		Q = farplane / ( farplane - nearplane );
		f4x4 proj_matrix;
		proj_matrix( 0 , 0 ) = w;
		proj_matrix( 1 , 1 ) = h;
		proj_matrix( 2 , 2 ) = Q;
		proj_matrix( 2 , 3 ) = -Q * nearplane;
		proj_matrix( 3 , 2 ) = 1.0f;
		float3 const &z = local_z;
		float3 const &x = local_x;
		float3 const &y = local_y;
		f4x4 view = f4x4(
			x.x , y.x , local_z.x , 0.0f ,
			x.y , y.y , local_z.y , 0.0f ,
			x.z , y.z , local_z.z , 0.0f ,
			-x * pos , -y * pos , -local_z * pos , 1.0f ).trans();
		f4x4 cam_matrix =
			MatUtil::wrap( MatUtil::mat3rows( local_x , local_y , local_z ) ) *
			MatUtil::translate( -pos )
			;
		return proj_matrix * cam_matrix;
	}
	static f4x4 perpLookUp1x1( const float3 &pos , const float3 &look , const float3 &up )
	{
		f4x4 view , proj;
		float farp = 1000.0f , nearp = 1.0f;
		float Q;
		Q = farp / ( farp - nearp );
		proj = f4x4( 0.0f );
		proj( 0 , 0 ) = 1.0;
		proj( 1 , 1 ) = 1.0;
		proj( 2 , 2 ) = Q;
		proj( 3 , 2 ) = -Q * nearp;
		proj( 2 , 3 ) = 1.0f;
		float3 x = -( look ^ up );
		float3 y = up;
		view = f4x4(
			x.x , y.x , look.x , 0.0f ,
			x.y , y.y , look.y , 0.0f ,
			x.z , y.z , look.z , 0.0f ,
			-x * pos , -y * pos , -look * pos , 1.0f );
		return view * proj;
	}
	static f4x4 orthographic( const float3 &pos , const float3 &sight_point , const float3 &up_dir , const float dx , const float dy , const float dz )
	{
		float h , w , Q;
		float3 local_z = ( sight_point - pos ).norm();
		float3 local_x = ( local_z ^ up_dir ).norm();
		float3 local_y = local_x ^ local_z;
		f4x4 proj_matrix;
		proj_matrix( 0 , 0 ) = 1.0f / dx;
		proj_matrix( 1 , 1 ) = 1.0f / dy;
		proj_matrix( 2 , 2 ) = 1.0f / dz;
		proj_matrix( 3 , 3 ) = 1.0f;
		float3 const &z = local_z;
		float3 const &x = local_x;
		float3 const &y = local_y;
		f4x4 const view_matrix = f4x4(
			x.x , y.x , z.x , 0.0f ,
			x.y , y.y , z.y , 0.0f ,
			x.z , y.z , z.z , 0.0f ,
			-x * pos , -y * pos , -z * pos , 1.0f ).trans();
		return proj_matrix.trans() * view_matrix;
	}
	static void genCubeCamera( f4x4 *out , const float3 &pos )
	{
		out[ 0 ] = Camera::perpLookUp1x1( pos , float3( 1.0f , 0.0f , 0.0f ) , float3( 0.0f , 1.0f , 0.0f ) );
		out[ 1 ] = Camera::perpLookUp1x1( pos , float3( -1.0f , 0.0f , 0.0f ) , float3( 0.0f , 1.0f , 0.0f ) );
		out[ 2 ] = Camera::perpLookUp1x1( pos , float3( 0.0f , 1.0f , 0.0f ) , float3( 0.0f , 0.0f , -1.0f ) );
		out[ 3 ] = Camera::perpLookUp1x1( pos , float3( 0.0f , -1.0f , 0.0f ) , float3( 0.0f , 0.0f , 1.0f ) );
		out[ 4 ] = Camera::perpLookUp1x1( pos , float3( 0.0f , 0.0f , 1.0f ) , float3( 0.0f , 1.0f , 0.0f ) );
		out[ 5 ] = Camera::perpLookUp1x1( pos , float3( 0.0f , 0.0f , -1.0f ) , float3( 0.0f , 1.0f , 0.0f ) );
	}
	float3 getCameraRay( float2 const &k ) const
	{
		return ( local_z + local_x * k.x * MathUtil< float >::tan( fovx * 0.5f ) + local_y * k.y * MathUtil< float >::tan( fovy * 0.5f ) ).norm();
	}
	bool fristrumTest( float3 const &p , float size ) const
	{
		float3 np = p - pos;
		float z = np * local_z;
		float x = np * local_x;
		float y = np * local_y;
		if( z < nearplane || z > farplane ) return false;
		if( MathUtil< float >::abs( x / z ) > MathUtil< float >::tan( fovx / 2.0f + 0.2f ) ) return false;
		if( MathUtil< float >::abs( y ) / z > MathUtil< float >::tan( fovy / 2.0f + 0.2f ) ) return false;
		return true;
	}
	bool fristrumTest2d( float2 const &p ) const
	{
		return false;
	}
};
#endif // RCAMERA_H
