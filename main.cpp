#include "uuid.h"
#include <3rdparty/loguru/debug.h>

int main()
{
	/* our default verbosity is to print only gWarn statements */
	loguru::g_stderr_verbosity = 3;
	/* override default verbosity if we have 'DEBUG' env var set */
	char *var = getenv("DEBUG");
	if (var)
		loguru::g_stderr_verbosity = std::stoi(var);

	gWarn("getting started");

	return 0;
}
