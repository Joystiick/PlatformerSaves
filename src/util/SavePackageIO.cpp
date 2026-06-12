#include "SavePackageIO.hpp"
#include <save/SaveHistoryManager.hpp>
#include <util/filesystem.hpp>

using namespace geode::prelude;

namespace {
    bool copyDirectory(std::filesystem::path const& src, std::filesystem::path const& dest, bool merge) {
        std::error_code ec;
        if (!std::filesystem::exists(src, ec)) {
            return false;
        }
        if (!merge && std::filesystem::exists(dest, ec)) {
            std::filesystem::remove_all(dest, ec);
        }
        std::filesystem::create_directories(dest, ec);
        for (auto const& entry : std::filesystem::directory_iterator(src, ec)) {
            auto const target = dest / entry.path().filename();
            if (entry.is_directory()) {
                if (!copyDirectory(entry.path(), target, true)) {
                    return false;
                }
            } else if (entry.is_regular_file()) {
                if (merge && std::filesystem::exists(target, ec)) {
                    continue;
                }
                std::filesystem::copy_file(entry.path(), target, std::filesystem::copy_options::overwrite_existing, ec);
                if (ec) {
                    return false;
                }
            }
        }
        return true;
    }
}

namespace util::savepackage {
    bool exportLevelSaves(GJGameLevel* level, std::string const& destPath) {
        auto const srcDir = util::filesystem::getLevelSaveDirectory(level);
        if (!std::filesystem::exists(srcDir)) {
            return false;
        }
        return copyDirectory(srcDir, std::filesystem::path(destPath), false);
    }

    bool importLevelSaves(GJGameLevel* level, std::string const& srcPath, bool merge) {
        auto const destDir = util::filesystem::getLevelSaveDirectory(level);
        if (!copyDirectory(std::filesystem::path(srcPath), destDir, merge)) {
            return false;
        }
        SaveHistoryManager::get().getEntries(level);
        return true;
    }
}
