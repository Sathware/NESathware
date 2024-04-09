#pragma once

#ifdef BUILD_DLL
	#define SathwareAPI _declspec(dllexport)
#else
	#define SathwareAPI _declspec(dllimport)
#endif

#pragma warning( disable: 4251 )

#if defined(DEBUG) || defined(_DEBUG)
#define ThrowIfFailed(result, string) if(FAILED(result)) throw Exception(string);
#else
#define ThrowIfFailed(result, string) 
#endif