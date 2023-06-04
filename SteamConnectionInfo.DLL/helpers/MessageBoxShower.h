#pragma once
#include <string>
#include <WinUser.h>

class MessageBoxShower
{
public:
	static void ShowError(const std::string& title, const std::string& body) {
		MessageBox(NULL, body.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
	}
};