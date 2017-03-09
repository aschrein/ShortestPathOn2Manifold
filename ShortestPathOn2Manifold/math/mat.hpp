#ifndef MAT_H
#define MAT_H
#include "math/vec.hpp"
namespace Math
{
	template< typename T , int N , int M >
	struct TMatrix
	{
		union
		{
			T _data[ N * M ];
			T __data[ N ][ M ];
		};
	public:
		
		CALLMOD TMatrix( T d )
		{
			ito( N )
				jto( M )
				__data[ i ][ j ] = i == j ? d : T( 0 );
		}
		CALLMOD TMatrix():
			TMatrix( 0.0f )
		{

		}
		CALLMOD TMatrix( TMatrix const & ) = default;
		CALLMOD TMatrix( TMatrix && ) = default;
		CALLMOD TMatrix &operator=( TMatrix const & ) = default;
		CALLMOD TMatrix &operator=( TMatrix && ) = default;
		template< typename F , typename ...D >
		CALLMOD TMatrix( F f , D ...arg )
		{
			unpacker< N * M , T , F , D... >::unpack( this->_data , N * M , f , arg... );
		}
		CALLMOD T& operator()( int i , int j )
		{
			return __data[ i ][ j ];
		}
		CALLMOD T const &operator()( int i , int j ) const
		{
			return __data[ i ][ j ];
		}
		CALLMOD TVector< M , T > &row( int i )
		{
			return *reinterpret_cast< TVector< M , T >* >( __data[ i ] );
		}
		CALLMOD TVector< M , T > const & row( int i ) const
		{
			return *reinterpret_cast< TVector< M , T > const * >( __data[ i ] );
		}
		CALLMOD TVector< N , T > collumn( int j ) const
		{
			TVector< N , T > out;
			ito( N )
				out[ i ] = this->__data[ i ][ j ];
			return out;
		}
		CALLMOD void print() const
		{
			for( int i = 0; i < N; i++ )
			{
				for( int j = 0; j < M; j++ )
					OS::IO::log( this->__data[ i ][ j ] , " | " );
				OS::IO::log( "\n" );
			}
		}
		CALLMOD TMatrix &operator-=( const TMatrix &a )
		{
			ito( N )
				jto( M )
				this->__data[ i ][ j ] -= a( i , j );
			return *this;
		}
		CALLMOD TMatrix operator-( const TMatrix &a ) const
		{
			TMatrix out = *this;
			return out -= a;
		}
		CALLMOD TMatrix &operator+=( const TMatrix &a )
		{
			ito( N )
				jto( M )
				this->__data[ i ][ j ] += a( i , j );
			return *this;
		}
		CALLMOD TMatrix operator+( const TMatrix &a ) const
		{
			TMatrix out = *this;
			return out += a;
		}
		CALLMOD TMatrix &operator*=( const TMatrix &a )
		{
			*this = *this * a;
			return *this;
		}
		CALLMOD TMatrix operator*( const TMatrix &a ) const
		{
			auto tmp = a.trans();
			TMatrix out;
			ito( N )
				jto( M )
					out( i , j ) = row( i ) * tmp.row( j );
			return out;
		}
		CALLMOD TMatrix< T , M , N > trans() const
		{
			TMatrix< T , M , N > out;
			ito( N )
				jto( M )
				out( j , i ) = ( *this )( i , j );
			return out;
		}
		template< typename T , int R >
		CALLMOD TMatrix< T , N , R > operator*( const TMatrix< T , M , R > &a ) const
		{
			auto tmp = a.trans();
			TMatrix< T , N , R > out;
			ito( N )
				jto( M )
					out( i , j ) = row( i ) * tmp.row( j );
			return out;
		}
		CALLMOD TMatrix &operator*=( T const &b )
		{
			ito( N )
				jto( M )
				_data[ i ][ j ] *= b;
			return *this;
		}
		CALLMOD TMatrix operator*( T const &b ) const
		{
			TMatrix out = *this;
			return out *= b;
		}
		CALLMOD TVector< M , T > operator*( TVector< M , T > const &v ) const
		{
			TVector< N , T > out;
			ito( N )
				out[ i ] = v * row( i );
			return out;
		}
	};
	typedef TMatrix< float , 4 , 4 > f4x4;
	typedef TMatrix< float , 3 , 3 > f3x3;
	typedef TMatrix< float , 2 , 2 > f2x2;
	template< typename T , int N , int M >
	CALLMOD TVector< M , T > operator*( TVector< N , T > const &v , TMatrix< T , N , M > const &m )
	{
		TVector< M , T > out;
		ito( M )
			out[ i ] = v * m.collumn( i );
		return out;
	}
	template< typename T , int N , int M >
	CALLMOD TVector< M , T > &operator*=( TVector< N , T > &v , TMatrix< T , N , M > const &m )
	{
		TVector< M , T > out;
		ito( M )
			out[ i ] = v * m.collumn( i );
		v = out;
		return v;
	}
	namespace MatUtil
	{
		template< typename T >
		CALLMOD TMatrix< T , 3 , 3 > rotate( const float3 &nax , T ang )
		{
			float sa = MathUtil<T>::sin( ang ) , ca = MathUtil<T>::cos( ang );
			float oa = 1.0 - ca;
			return TMatrix< T , 3 , 3 >(
				ca + oa * nax.x * nax.x ,
				oa * nax.y * nax.x + sa * nax.z ,
				oa * nax.z * nax.x - sa * nax.y ,
				oa * nax.x * nax.y - sa * nax.z ,
				ca + oa * nax.y * nax.y ,
				oa * nax.z * nax.y + sa * nax.x ,
				oa * nax.x * nax.z + sa * nax.y ,
				oa * nax.y * nax.z - sa * nax.x ,
				ca + oa * nax.z * nax.z
				);
		}
		template< typename T >
		CALLMOD TMatrix< T , 4 , 4 > wrap( TMatrix< T , 3 , 3 > const &src )
		{
			TMatrix< T , 4 , 4 > out;
			ito( 3 )
			{
				jto( 3 )
				{
					out( i , j ) = src( i , j );
				}
			}
			out( 3 , 3 ) = T( 1 );
			return out;
		}
		template< typename T >
		CALLMOD TMatrix< T , 2 , 2 > rotate( T ang )
		{
			float sa = MathUtil<T>::sin( ang ) , ca = MathUtil<T>::cos( ang );
			return TMatrix< T , 2 , 2 >(
				ca , sa ,
				-sa , ca
				);
		}
		template< typename T >
		CALLMOD TVector< 3 , T > proj( const TVector< 3 , T > &a , TMatrix< T , 4 , 4 > const &mat )
		{
			auto r = TVector< 4 , T >( a.x , a.y , a.z , 1.0f ) * mat;
			r /= r.w;
			return TVector< 3 , T >( r.x , r.y , r.z );
		}
		template< typename T , int N >
		CALLMOD TMatrix< T , N , N > inv( TMatrix< T , N , N > const &m )
		{
			T tmpmat[ N * N * 2 ];
			ito( N )
			{
				jto( N )
				{
					tmpmat[ i * 2 * N + j ] = m( i , j );
				}
				jto( N )
				{
					tmpmat[ i * 2 * N + N + j ] = i == j ? T( 1 ) : T( 0 );
				}
			}
			for( int n = 0; n < N; n++ )
			{
				T ann = tmpmat[ n * N * 2 + n ];
				if( MathUtil< T >::abs( ann ) < MathUtil< T >::EPS )
				{
					int new_index = n;
					T new_index_mod = MathUtil< T >::abs( ann );
					for( int k = n + 1; k < N; k++ )
					{
						T tmp = MathUtil< T >::abs( tmpmat[ k * N * 2 + n ] );
						if( tmp > new_index_mod )
						{
							new_index = k;
							new_index_mod = tmp;
						}
					}
					if( new_index == n )
					{
						return TMatrix< T , N , N >();
					}
					for( int j = n; j < N * 2; j++ )
					{
						T tmp = tmpmat[ n * N * 2 + j ];
						tmpmat[ n * N * 2 + j ] = tmpmat[ new_index * N * 2 + j ];
						tmpmat[ new_index * N * 2 + j ] = tmp;
					}
					ann = tmpmat[ n * N * 2 + n ];
				}
				T anni = T( 1 ) / ann;
				for( int j = n; j < N * 2; j++ )
				{
					tmpmat[ n * N * 2 + j ] *= anni;
				}
				for( int i = n + 1; i < N; i++ )
				{
					T tmp = tmpmat[ i * N * 2 + n ];
					for( int j = n + 1; j < 2 * N; j++ )
					{
						tmpmat[ i * N * 2 + j ] -= tmpmat[ n * N * 2 + j ] * tmp;
					}
				}
			}
			for( int n = N - 1; n >= 0; n-- )
			{
				for( int i = 0; i < n; i++ )
				{
					T tmp = tmpmat[ i * N * 2 + n ];
					for( int j = N; j < 2 * N; j++ )
					{
						tmpmat[ i * N * 2 + j ] -= tmpmat[ n * N * 2 + j ] * tmp;
					}
				}
			}
			TMatrix< T , N , N > out;
			ito( N )
			{
				jto( N )
				{
					out( i , j ) = tmpmat[ i * N * 2 + j + N ];
				}
			}
			return out;
		}
		template< typename T >
		CALLMOD TMatrix< T , 3 , 3 > mat3rows( TVector< 3 , T > const &x , TVector< 3 , T > const &y , TVector< 3 , T > const &z )
		{
			TMatrix< T , 3 , 3 > out(
				x.x , x.y , x.z ,
				y.x , y.y , y.z ,
				z.x , z.y , z.z
			);
			return out;
		}
		template< typename T >
		CALLMOD TMatrix< T , 4 , 4 > translate( TVector< 3 , T > const &trans )
		{
			TMatrix< T , 4 , 4 > out;
			out( 0 , 3 ) = trans.x;
			out( 1 , 3 ) = trans.y;
			out( 2 , 3 ) = trans.z;
			out( 3 , 3 ) = T( 1 );
			ito( 3 )
			{
				out( i , i ) = T( 1 );
			}
			return out;
		}
		template< typename T >
		CALLMOD TMatrix< T , 4 , 4 > scale( TVector< 3 , T > const &trans )
		{
			TMatrix< T , 4 , 4 > out;
			out( 0 , 0 ) = trans.x;
			out( 1 , 1 ) = trans.y;
			out( 2 , 2 ) = trans.z;
			out( 3 , 3 ) = T( 1 );
			return out;
		}
	}
}
#endif
