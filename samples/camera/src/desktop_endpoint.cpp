

#include "camera_sample.h"

#include <exception>

int main()
{
	try
	{
		gs::init();
		gs::startInfiniteLoop();
		return 0;
	}
	catch (const std::exception& e)
	{
		return 1;
	}
	catch (...)
	{
		return 2;
	}
}