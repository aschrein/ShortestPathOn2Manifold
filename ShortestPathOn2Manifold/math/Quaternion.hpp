#pragma once
#include <engine/math\mat.hpp>
namespace Math
{
	template< typename T >
	struct TQuaternion
	{
		union
		{
			T __data[ 4 ];
			struct
			{
				T x , y , z , w;
			};
		};
		CALLMOD TQuaternion( T x , T y , T z , T w ) :
			x( x ) , y( y ) , z( z ) , w( w )
		{}
		CALLMOD TQuaternion() :
			TQuaternion( T( 0 ) , T( 0 ) , T( 0 ) , T( 0 ) )
		{}
		CALLMOD TQuaternion( TQuaternion const & ) = default;
		CALLMOD TQuaternion( TQuaternion && ) = default;
		CALLMOD TQuaternion &operator=( TQuaternion const & ) = default;
		CALLMOD TQuaternion &operator=( TQuaternion && ) = default;
		CALLMOD TQuaternion( TVector< 3 , T > const &axis , T angle )
		{
			auto cos = MathUtil< T >::cos( angle / 2 );
			auto sin = MathUtil< T >::sin( angle / 2 );
			x = axis.x * sin;
			y = axis.y * sin;
			z = axis.z * sin;
			w = cos;
		}
		CALLMOD TQuaternion( TVector< 3 , T > const &axis )
		{
			x = axis.x;
			y = axis.y;
			z = axis.z
			w = T( 1 );
		}
		CALLMOD TQuaternion operator+( TQuaternion const &a ) const
		{
			return TQuaternion( *this ) += a;
		}
		CALLMOD TQuaternion &operator+=( TQuaternion const &a )
		{
			x += a.x;
			y += a.y;
			z += a.z;
			w += a.w;
			return *this;
		}
		CALLMOD TQuaternion operator-( TQuaternion const &a ) const
		{
			return TQuaternion( *this ) -= a;
		}
		CALLMOD TQuaternion &operator-=( TQuaternion const &a )
		{
			x -= a.x;
			y -= a.y;
			z -= a.z;
			w -= a.w;
			return *this;
		}
		CALLMOD TQuaternion &operator*=( T const &a )
		{
			x *= a;
			y *= a;
			z *= a;
			w *= a;
			return *this;
		}
		CALLMOD TQuaternion operator*( T const &a ) const
		{
			return TQuaternion( *this ) *= a;
		}
		CALLMOD TQuaternion &operator/=( T const &a )
		{
			auto ia = T( 1 ) / a;
			return *this *= ia;
		}
		CALLMOD TQuaternion operator/( T const &a ) const
		{
			return TQuaternion( *this ) /= a;
		}
		CALLMOD TQuaternion operator*( TQuaternion const &a ) const
		{
			return TQuaternion(
				x * a.w + w * a.x + y * a.z - z * a.y ,
				y * a.w + w * a.y - x * a.z + z * a.x ,
				z * a.w + w * a.z + x * a.y - y * a.x ,
				w * a.w - x * a.x - y * a.y - z * a.z
				);
		}
		CALLMOD TQuaternion &operator*=( TQuaternion const &a )
		{
			*this = *this * a;
			return *this;
		}
		CALLMOD TQuaternion operator/( TQuaternion const &a ) const
		{
			return ( *this * !a ) / a.mod2();
		}
		CALLMOD TQuaternion operator/=( TQuaternion const &a )
		{
			return *this = *this * a;
		}
		CALLMOD T mod2() const
		{
			return x * x + y * y + z * z + w * w;
		}
		CALLMOD T mod() const
		{
			return MathUtil< T >::sqrt( mod2() );
		}
		CALLMOD TMatrix< T , 3 , 3 > getRotationMatrix() const
		{
			return TMatrix< T , 3 , 3 >(
				1 - 2 * y*y - 2 * z*z , 2 * x*y - 2 * z*w , 2 * x*z + 2 * y*w ,
				2 * x*y + 2 * z*w , 1 - 2 * x*x - 2 * z*z , 2 * y*z - 2 * x*w ,
				2 * x*z - 2 * y*w , 2 * y*z + 2 * x*w , 1 - 2 * x*x - 2 * y * y
				);
		}
		CALLMOD TQuaternion operator!() const
		{
			return TQuaternion( -x , -y , -z , w );
		}
		TVector< 3 , T > rotate( TVector< 3 , T > const &a ) const
		{
			auto res = *this * TQuaternion( a.x , a.y , a.z , 0 ) / *this;
			return{ res.x , res.y , res.z };
		}
	};
	template< typename T >
	inline TQuaternion< T > operator*( T const &a , TQuaternion< T > const &b )
	{
		return b * a;
	}
	typedef TQuaternion< float > qf;
	template< typename T >
	struct TDoubleQuaternion
	{
		union
		{
			T __data[ 8 ];
			struct
			{
				TQuaternion< T > r , t;
			};
		};
		CALLMOD TDoubleQuaternion() :
			r( T( 1 ) , T( 0 ) , T( 0 ) , T( 0 ) ) ,
			t( T( 0 ) , T( 0 ) , T( 0 ) , T( 0 ) )
		{}
		CALLMOD TDoubleQuaternion( TDoubleQuaternion const & ) = default;
		CALLMOD TDoubleQuaternion( TDoubleQuaternion && ) = default;
		CALLMOD TDoubleQuaternion &operator=( TDoubleQuaternion const & ) = default;
		CALLMOD TDoubleQuaternion &operator=( TDoubleQuaternion && ) = default;
	};
	typedef TDoubleQuaternion< float > dqf;
}