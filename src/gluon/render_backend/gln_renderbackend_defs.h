#pragma once

#ifdef _WIN32
#	ifdef GLUON_RENDERBACKEND_MAKELIB
#		define GLUON_RENDERBACKEND_EXPORT
#	else
#		ifdef GLUON_RENDERBACKEND_MAKEDLL
#			define GLUON_RENDERBACKEND_EXPORT __declspec(dllexport)
#		else
#			define GLUON_RENDERBACKEND_EXPORT __declspec(dllimport)
#		endif
#	endif
#else
#	define GLUON_RENDERBACKEND_EXPORT
#endif
