#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <algorithm>

#define argumentGet(_x) getCmdOption(argv, argv + argc, _x)
#define argumentExist(_x) cmdOptionExists(argv, argv + argc, _x)

/*
 * following 2 functions are shamelessly stolen from the
 * the following stack overflow entry:
 *
 * https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
 */

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}


#endif // ARGUMENTS_H
