#pragma once
#include <string>
#include <format>
#include "SathwareEngine.h"

class SathwareAPI Exception
{
public:
	Exception(const std::wstring& exceptionMessage, const long errorCode = 0) noexcept
		: message(exceptionMessage), errorCode(errorCode)
	{
		//message = message + std::format(L"  Error Code: {}", errorCode);
	}

	Exception(const wchar_t* exceptionMessage, const long errorCode = 0) noexcept
		: message(exceptionMessage), errorCode(errorCode)
	{
		//message = message + std::format(L"  Error Code: {}", errorCode);
	}

	Exception(const Exception& other) noexcept
		: message(other.message), errorCode(other.errorCode)
	{}

	Exception& operator=(const Exception& other) noexcept
	{
		message = other.message;
		errorCode = other.errorCode;
		return *this;
	}

	const wchar_t* const what() const noexcept
	{
		return message.c_str();
	}

private:
	std::wstring message;
	long errorCode;

	// Prevent dangling pointer 
	std::wstring output;
};