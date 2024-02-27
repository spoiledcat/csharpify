#ifndef CSHARPIFY_UTILS_H_
#define CSHARPIFY_UTILS_H_

#include <filesystem>

#define RETURN_FAIL_IF_FALSE(exp, msg) { if (!(exp)) { printf(msg); return EXIT_FAILURE; } }

std::string normalizePath(const std::string& messyPath) {
    std::filesystem::path canonicalPath = std::filesystem::absolute(std::filesystem::weakly_canonical(std::filesystem::path(messyPath)));
    return canonicalPath.make_preferred().string();
}


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

std::string UnicodeToString(const std::wstring& wstr)
{
	if (wstr.empty()) return {};
	const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), strTo.data(), size_needed, nullptr, nullptr);
	return strTo;
}
#endif

#endif// CSHARPIFY_UTILS_H_