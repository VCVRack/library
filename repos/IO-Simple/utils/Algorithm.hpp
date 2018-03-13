#if!defined ALGORITHM_HPP
#define ALGORITHM_HPP
#include <functional>

template<class T, class Compare>
constexpr const T& clamp( const T& v, const T& lo, const T& hi, Compare comp )
{
	return assert( !comp(hi, lo) ), comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
	return clamp( v, lo, hi, std::less<T>() );
}

#endif
