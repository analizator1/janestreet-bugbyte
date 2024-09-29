#ifndef _UTILS_H_
#define _UTILS_H_

#include <vector>
#include <ostream>

template<class T>
std::ostream & operator<<(std::ostream & out, std::vector<T> const & vec)
{
	out << "{ ";
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (it != vec.begin())
		{
			out << ", ";
		}
		out << *it;
	}
	out << " }";
	return out;
}

#endif // _UTILS_H_
