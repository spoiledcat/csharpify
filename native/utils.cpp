#include "utils.h"

#include <filesystem>
#include <iostream>
#include <string_view>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <codecvt>

std::string normalizePath(const std::string &messyPath)

int parseManagedSignature(const std::string &full_type, std::string &assembly, std::string &typeAndNamespace,
                          std::string &nmspace, std::string &type) {
    size_t pos = full_type.find(", ");
    typeAndNamespace = full_type.substr(0, pos);
    assembly = full_type.substr(pos + 2);
    nmspace = {};

    pos = typeAndNamespace.find('+');

    if (pos != std::string::npos) {
        nmspace = typeAndNamespace.substr(0, pos);
    }

    if (!nmspace.empty()) {
        const std::filesystem::path typeAsPath(nmspace);
        if (typeAsPath.has_extension()) {
            auto str = typeAsPath.string();
            auto ext = typeAsPath.extension().string();
            nmspace = str.substr(0, str.length() - ext.length());
            type = ext.substr(1);
            type += "/";
        }
    }
    type += typeAndNamespace.substr(pos + 1);

    return 0;
}


#if PLATFORM_WIN
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

std::wstring StringToUnicode(const std::string& str)
{
    if (str.empty()) return {};
    const int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
    std::wstring strTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), strTo.data(), size_needed);
    return strTo;
}

char_t* StringToUnicode(const char* str, int len)
{
    const int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
    char_t* ret = (char_t*)malloc(sizeof(char_t*) * size_needed);
    MultiByteToWideChar(CP_UTF8, 0, str, len, ret, size_needed);
    return ret;
}

wchar_t* StringToUnicode(const char* str, int len)
{
    const int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
    wchar_t* ret = (wchar_t*)malloc(sizeof(wchar_t*) * size_needed);
    MultiByteToWideChar(CP_UTF8, 0, str, len, ret, size_needed);
    return ret;
}
#else
std::string UnicodeToString(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    const int size_needed = wcstombs(nullptr, wstr.data(), wstr.size());
    std::string strTo(size_needed, 0);
    wcstombs(strTo.data(), wstr.data(), size_needed);
    return strTo;
}

std::string UnicodeToString(const std::u16string& wstr)
{
    if (wstr.empty()) return {};
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
    return (conversion.to_bytes(wstr.data()));
}

std::wstring StringToUnicode(const std::string& str)
{
    if (str.empty()) return {};
    const int size_needed = mbstowcs(nullptr, str.data(), str.size());
    std::wstring strTo(size_needed, 0);
    mbstowcs(strTo.data(), str.data(), size_needed);
    return strTo;
}

wchar_t* StringToUnicode(const char* str, int len)
{
    const int size_needed = mbstowcs(nullptr, str, len);
    wchar_t* ret = (wchar_t*)malloc(sizeof(wchar_t*) * size_needed);
    mbstowcs(ret, str, size_needed);
    return ret;
}
#endif