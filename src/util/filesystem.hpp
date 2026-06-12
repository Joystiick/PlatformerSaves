#pragma once
#include <Geode/Geode.hpp>

#define PSF_EXT ".psf"

namespace util::filesystem {
    inline std::string getParentDirectoryFromPath(const std::string& i_filePath)
    {
        size_t l_pos = i_filePath.find_last_of("\\/");
        return (std::string::npos == l_pos)
            ? ""
            : i_filePath.substr(0, l_pos);
    }

    inline std::string cleanLevelName(GJGameLevel* i_level) {
        std::string l_cleanLevelName = i_level->m_levelName;
        l_cleanLevelName.erase(std::remove(l_cleanLevelName.begin(), l_cleanLevelName.end(), '.'), l_cleanLevelName.end());
        l_cleanLevelName.erase(std::remove(l_cleanLevelName.begin(), l_cleanLevelName.end(), '/'), l_cleanLevelName.end());
        l_cleanLevelName.erase(std::remove(l_cleanLevelName.begin(), l_cleanLevelName.end(), '\\'), l_cleanLevelName.end());
        return l_cleanLevelName;
    }

    inline std::string getLevelSaveDirectory(GJGameLevel* i_level) {
        std::string l_filePath = geode::Mod::get()->getSaveDir().generic_string();

        switch(i_level->m_levelType) {
            case GJLevelType::Main:
                l_filePath.append(fmt::format("/saves/local/{}", i_level->m_levelID.value()));
                break;
            case GJLevelType::Editor:
                l_filePath.append(fmt::format("/saves/editor/{}_rev{}", cleanLevelName(i_level).c_str(), i_level->m_levelRev));
                break;
            case GJLevelType::Saved:
            default:
                l_filePath.append(fmt::format("/saves/online/{}", i_level->m_levelID.value()));
                break;
        }
        return l_filePath;
    }

    inline std::string getHistoryFilePath(GJGameLevel* i_level) {
        return getLevelSaveDirectory(i_level) + "/history.json";
    }

    inline std::string getSaveFilePath(GJGameLevel* i_level, int i_slot, bool i_checkExists = false) {
        std::string l_filePath = getLevelSaveDirectory(i_level);
        l_filePath.append(fmt::format("/slot{}{}", i_slot, PSF_EXT));

        if (i_checkExists && !std::filesystem::exists(l_filePath)) {
            return "";
        }
        return l_filePath;
    }

    inline bool validSaveExists(GJGameLevel* i_level) {
        if (std::filesystem::exists(getHistoryFilePath(i_level))) {
            return true;
        }
        for (int i = 0; i < 4; i++) {
            if (getSaveFilePath(i_level, i, true) != "") {
                return true;
            }
        }
        return false;
    }

    inline void removeSaveFile(GJGameLevel* i_level, int i_slot) {
        std::string l_filePath = getSaveFilePath(i_level, i_slot, true);
        if (l_filePath != "") {
            std::filesystem::remove(l_filePath);
        }
    }
}
