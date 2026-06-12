#pragma once
#include <Geode/Geode.hpp>
#include <string>

namespace util::savepackage {
    bool exportLevelSaves(GJGameLevel* level, std::string const& destPath);
    bool importLevelSaves(GJGameLevel* level, std::string const& srcPath, bool merge);
}
