#pragma once
#include <math/vec.hpp>
#include <os/log.hpp>
#include <test_util\TestUtil.hpp>
#include <math/mat.hpp>
using namespace Math;
using namespace OS::IO;
bool vectorTest()
{
	{
		RETURN_ASSERT( sizeof( f3 ) == 12 );
		{
			f3 a( 0.0f , 1.0f , 2.0f );
			RETURN_ASSERT( a.x == 0.0f );
			f3 b = { 0.0f , 1.0f , 777.0f };
			RETURN_ASSERT( b.z == 777.0f );
			auto c = 5 * a + b * 4;
			c.x = 666.0f;
			RETURN_ASSERT( c.x == 666.0f );
			RETURN_ASSERT( c.y == 9.0f );
		}
		{
			f3 n = { 1.0f , 0.0f , 0.0f };
			f3 t = { 0.0f , 1.0f , 0.0f };
			RETURN_ASSERT( n * t == 0.0f );
			f3 b = n ^ t;
			RETURN_ASSERT( b == f3( 0.0f , 0.0f , 1.0f ) );
		}
		OS::IO::log( "f3 test success\n" );
	}
	{
		f2 n = { 1.0f , 0.0f };
		RETURN_ASSERT( !n == f2( 0.0f , 1.0f ) );
		OS::IO::log( "f2 test success\n" );
	}
	{
		RETURN_ASSERT( sizeof( f2x2 ) == 16 );
		{
			f2x2 rot = rotation< float >( MathUtil< float >::PI / 6 );
			f2 test = { 1.0f , 0.0f };
			test *= rot;
			RETURN_ASSERT( MathUtil< float >::abs( test.y - 0.5f ) < MathUtil< float >::EPS );
			test = { 1.0f , 0.0f };
			test = rot * test;
			RETURN_ASSERT( MathUtil< float >::abs( test.y + 0.5f ) < MathUtil< float >::EPS );
			OS::IO::log( "f2x2 test success\n" );
		}
	}
	return true;
}