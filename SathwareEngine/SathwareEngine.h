#pragma once

#ifdef BUILD_DLL
	#define SathwareAPI _declspec(dllexport)
#else
	#define SathwareAPI _declspec(dllimport)
#endif

#pragma warning( disable: 4251 )