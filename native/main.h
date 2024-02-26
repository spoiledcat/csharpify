#ifndef __CSHARPIFY_MAIN_H__
#define __CSHARPIFY_MAIN_H__

int imgui_main(int, char**);

#include <filesystem>

std::string normalizePath(const std::string& messyPath) {
    std::filesystem::path canonicalPath = std::filesystem::absolute(std::filesystem::weakly_canonical(std::filesystem::path(messyPath)));
    return canonicalPath.make_preferred().string();
}

#endif