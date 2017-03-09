#pragma once
#include <functional>
#undef max
#undef min
namespace Math
{
	template< typename T >
	class MathUtil
	{
	public:
		typedef std::function< T( T const & ) > Func1D;
		typedef std::function< T( T const & , T const & ) > Func2D;
		typedef std::function< T( Func1D , T const & , T const & , int ) > RootFinder1D;
		static T abs( T const & );
		static T sqrt( T const & );
		static T sqr( T const & );
		static T invsqrt( T const & );
		static T sin( T const & );
		static T invsin( T const & );
		static T cos( T const & );
		static T invcos( T const & );
		static T tan( T const & );
		static T atan2( T const &y , T const &x );
		static T min( T const &y , T const &x );
		static T max( T const &y , T const &x );
		static T pow( T const &val , T const &pow );
		static T wrap( T const &val , T const &min , T const &max );
		static T findRootBiject( Func1D f , T const &x0 , T const &x1 , int max_depth );
		static T findRootNewton( Func1D f , Func1D dfdx , T const &x0 , T const &x1 , int max_depth );
		static T findRootUniformMarching( Func1D f , Func2D root_finder , T const &x0 , T const &x1 , int march_count );
		static T randomUniform();
		static const T PI;
		static const T invPI;
		static const T PI2;
		static const T invPI2;
		static const T EPS;
		static const T SQREPS;
		static const T MAX;
		static const T NaN;
	};
}
