#pragma once

#ifdef _WIN32
#	ifdef GLUON_API_MAKEDLL
#		define GLUON_API_EXPORT __declspec(dllexport)
#	else
#		define GLUON_API_EXPORT __declspec(dllimport)
#	endif
#else
#	define GLUON_API_EXPORT
#endif
