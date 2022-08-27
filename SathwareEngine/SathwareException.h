#pragma once
#include <string>
#include "SathwareEngine.h"

class SathwareAPI Exception
{
public:
	Exception(const std::wstring& exceptionMessage) noexcept
		: message(exceptionMessage)
	{}

	Exception(const wchar_t* exceptionMessage) noexcept
		: message(exceptionMessage)
	{}

	Exception(const Exception& other) noexcept
		: message(other.message)
	{}

	Exception& operator=(const Exception& other) noexcept
	{
		message = other.message;
		return *this;
	}

	const wchar_t* const what() const noexcept
	{
		return message.c_str();
	}

private:
	std::wstring message;
};