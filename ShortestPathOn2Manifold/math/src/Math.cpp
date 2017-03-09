#include "../Math.hpp"
#include <cmath>
#include <random>
#include <time.h>
using namespace Math;
template class MathUtil< float >;
template class MathUtil< int32_t >;
template class MathUtil< uint32_t >;
template class MathUtil< int64_t >;
template class MathUtil< uint64_t >;
template<>
int32_t MathUtil< int32_t >::abs( int32_t const &val )
{
	return labs( val );
}
template<>
uint32_t MathUtil< uint32_t >::abs( uint32_t const &val )
{
	return val;
}
template<>
int64_t MathUtil< int64_t >::abs( int64_t const &val )
{
	return labs( val );
}
template<>
uint64_t MathUtil< uint64_t>::abs( uint64_t const &val )
{
	return val;
}
template<>
int32_t MathUtil< int32_t >::max( int32_t const &a , int32_t const &b )
{
	return a > b ? a : b;
}
template<>
uint32_t MathUtil< uint32_t >::min( uint32_t const &a , uint32_t const &b )
{
	return a < b ? a : b;
}
template<>
uint32_t MathUtil< uint32_t >::max( uint32_t const &a , uint32_t const &b )
{
	return a > b ? a : b;
}
template<>
float MathUtil< float >::sqrt( float const &val )
{
	return sqrtf( val );
}
template<>
float MathUtil< float >::abs( float const &val )
{
	return fabs( val );
}
template<> float MathUtil< float >::sqr( float const &val )
{
	return val * val;
}
template<> float MathUtil< float >::invsqrt( float const &val )
{
	return 1.0f / sqrtf( val );
}
template<> float MathUtil< float >::sin( float const &val )
{
	return sinf( val );
}
template<> float MathUtil< float >::invsin( float const &val )
{
	return 1.0f / sinf( val );
}
template<> float MathUtil< float >::cos( float const &val )
{
	return cosf( val );
}
template<> float MathUtil< float >::invcos( float const &val )
{
	return 1.0f / cosf( val );
}
template<> float MathUtil< float >::tan( float const &val )
{
	return tanf( val );
}
template<> float MathUtil< float >::atan2( float const &y , float const &x )
{
	return atan2f( y , x );
}
template<> float MathUtil< float >::min( float const &y , float const &x )
{
	return fminf( y , x );
}
template<> float MathUtil< float >::max( float const &y , float const &x )
{
	return fmax( y , x );
}
template<> float MathUtil< float >::pow( float const &val , float const &pow )
{
	return powf( val , pow );
}
template<> float MathUtil< float >::wrap( float const &val , float const &min , float const &max )
{
	return val > max ? max : val < min ? min : val;
}
template<> float MathUtil< float >::findRootBiject( Func1D f , float const &x0 , float const &x1 , int max_depth )
{
	return NAN;
}
template<> float MathUtil< float >::findRootNewton( Func1D f , Func1D dfdx , float const &x0 , float const &x1 , int max_depth )
{
	return NAN;
}
template<> float MathUtil< float >::findRootUniformMarching( Func1D f , Func2D root_finder , float const &x0 , float const &x1 , int march_count )
{
	return NAN;
}
static float getPi()
{
	srand( clock() );
	return acosf( -1.0f );
}
const float MathUtil< float >::PI( getPi() );
const float MathUtil< float >::invPI( 1.0f / acosf( -1.0f ) );
const float MathUtil< float >::PI2( 2 * acosf( -1.0f ) );
const float MathUtil< float >::invPI2( 0.5f * 1.0f / acosf( -1.0f ) );
const float MathUtil< float >::EPS( FLT_EPSILON );
const float MathUtil< float >::SQREPS( FLT_EPSILON * FLT_EPSILON );
const float MathUtil< float >::MAX( FLT_MAX );
const float MathUtil< float >::NaN( NAN );
template<> float MathUtil< float >::randomUniform()
{
	return float( rand() ) / RAND_MAX;
}