#ifndef CSHARPIFY_UTILS_H_
#define CSHARPIFY_UTILS_H_

#include <string>

#define RETURN_FAIL_IF_FALSE(exp, msg) { if (!(exp)) { printf(msg); return EXIT_FAILURE; } }

std::string normalizePath(const std::string &messyPath);

int parseManagedSignature(const std::string &full_type, std::string &assembly, std::string &typeAndNamespace,
                          std::string &nmspace, std::string &type);

std::string UnicodeToString(const std::wstring& wstr);
std::string UnicodeToString(const std::u16string& wstr);
std::wstring StringToUnicode(const std::string& str);
wchar_t* StringToUnicode(const char* str, int len);

#endif// CSHARPIFY_UTILS_H_