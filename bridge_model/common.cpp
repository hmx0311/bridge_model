#include <Windows.h>

#include "common.h"

std::pair<const void*, int> getResource(const char* name, const char* type)
{
	HRSRC rc_info = FindResourceA(nullptr, name, type);
	if (rc_info == nullptr)
	{
		printf("\nERROR: Can't Find Resource %s\n", name);
		return { nullptr, 0 };
	}
	HGLOBAL rc_data = LoadResource(nullptr, rc_info);
	if (rc_data == nullptr)
	{
		printf("\nERROR: Can't Load Resource %s\n", name);
		return { nullptr, 0 };
	}
	return { LockResource(rc_data), SizeofResource(nullptr, rc_info) };
}

std::pair<const void*, int> getResource(int name, const char* type)
{
	return getResource(MAKEINTRESOURCEA(name), type);
}
