#include "utils.h"

#include <limits>

void skipComments(std::istream & inp)
{
	auto oldmask = inp.exceptions();
	inp.exceptions(std::ios::goodbit);
	char c;
	while (inp.get(c))
	{
		if (std::isspace(c))
		{
			// ignore
		}
		else if (c == '#')
		{
			// ignore until the end of line
			inp.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		else
		{
			// put it back and return
			inp.putback(c);
			break;
		}
	}
	// clear failbit if there was EOF
	inp.clear();
	inp.exceptions(oldmask);
}
