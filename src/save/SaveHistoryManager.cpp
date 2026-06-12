#include "SaveHistoryManager.hpp"
#include <util/filesystem.hpp>
#include <fstream>

using namespace geode::prelude;

namespace {
    matjson::Value makeEmptyHistory() {
        return matjson::makeObject({
            {"entries", matjson::Value(std::vector<matjson::Value>{})}
        });
    }

    std::vector<matjson::Value> getEntriesArray(matjson::Value& history) {
        if (auto result = history["entries"].asArray()) {
            return result.unwrap();
        }
        return {};
    }
}

SaveHistoryManager& SaveHistoryManager::get() {
    static SaveHistoryManager instance;
    return instance;
}

std::string SaveHistoryManager::reasonToString(SaveReason reason) const {
    switch (reason) {
        case SaveReason::Checkpoint: return "checkpoint";
        case SaveReason::Manual: return "manual";
        default: return "checkpoint";
    }
}

SaveHistoryEntry SaveHistoryManager::entryFromJson(const matjson::Value& value) const {
    SaveHistoryEntry entry;
    entry.slot = value["slot"].asInt().unwrapOr(0);
    entry.timestamp = value["timestamp"].asInt().unwrapOr(0);
    auto const reason = value["reason"].asString().unwrapOr("checkpoint");
    entry.reason = reason == "manual" ? SaveReason::Manual : SaveReason::Checkpoint;
    entry.checkpointCount = value["checkpointCount"].asInt().unwrapOr(0);
    return entry;
}

matjson::Value SaveHistoryManager::entryToJson(const SaveHistoryEntry& entry) const {
    return matjson::makeObject({
        {"slot", entry.slot},
        {"timestamp", entry.timestamp},
        {"reason", reasonToString(entry.reason)},
        {"checkpointCount", entry.checkpointCount}
    });
}

bool SaveHistoryManager::loadHistory(GJGameLevel* level, matjson::Value& outHistory) {
    auto const path = util::filesystem::getHistoryFilePath(level);
    if (!std::filesystem::exists(path)) {
        outHistory = makeEmptyHistory();
        return true;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto parsed = matjson::parse(content);
    if (!parsed) {
        outHistory = makeEmptyHistory();
        return true;
    }

    outHistory = parsed.unwrap();
    if (!outHistory.contains("entries") || !outHistory["entries"].isArray()) {
        outHistory["entries"] = matjson::Value(std::vector<matjson::Value>{});
    }
    return true;
}

bool SaveHistoryManager::saveHistory(GJGameLevel* level, const matjson::Value& history) {
    auto const dir = util::filesystem::getLevelSaveDirectory(level);
    if (!std::filesystem::exists(dir) && !std::filesystem::create_directories(dir)) {
        return false;
    }

    auto const path = util::filesystem::getHistoryFilePath(level);
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << history.dump(2);
    return true;
}

std::vector<SaveHistoryEntry> SaveHistoryManager::getEntries(GJGameLevel* level) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return {};
    }

    std::vector<SaveHistoryEntry> entries;
    for (auto const& value : getEntriesArray(history)) {
        entries.push_back(entryFromJson(value));
    }
    return entries;
}

std::optional<SaveHistoryEntry> SaveHistoryManager::getLatest(GJGameLevel* level) {
    auto entries = getEntries(level);
    if (entries.empty()) {
        return std::nullopt;
    }
    return entries.back();
}

int SaveHistoryManager::getNextSlot(GJGameLevel* level) {
    auto entries = getEntries(level);
    int maxSlot = -1;
    for (auto const& entry : entries) {
        if (entry.slot > maxSlot) {
            maxSlot = entry.slot;
        }
    }
    return maxSlot + 1;
}

bool SaveHistoryManager::appendEntry(GJGameLevel* level, int slot, SaveReason reason, int checkpointCount) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return false;
    }

    auto entries = getEntriesArray(history);
    entries.push_back(entryToJson({
        .slot = slot,
        .timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count(),
        .reason = reason,
        .checkpointCount = checkpointCount
    }));
    history["entries"] = entries;
    return saveHistory(level, history);
}

bool SaveHistoryManager::truncateAfterIndex(GJGameLevel* level, size_t oldestFirstIndex) {
    auto entries = getEntries(level);
    if (entries.empty() || oldestFirstIndex >= entries.size()) {
        return false;
    }

    for (size_t i = oldestFirstIndex + 1; i < entries.size(); i++) {
        util::filesystem::removeSaveFile(level, entries[i].slot);
    }

    entries.resize(oldestFirstIndex + 1);

    matjson::Value history = makeEmptyHistory();
    std::vector<matjson::Value> jsonEntries;
    for (auto const& entry : entries) {
        jsonEntries.push_back(entryToJson(entry));
    }
    history["entries"] = jsonEntries;
    return saveHistory(level, history);
}

bool SaveHistoryManager::hasValidSave(GJGameLevel* level) {
    auto latest = getLatest(level);
    if (!latest) {
        return false;
    }
    return util::filesystem::getSaveFilePath(level, latest->slot, true) != "";
}

void SaveHistoryManager::removeAllSaves(GJGameLevel* level) {
    for (auto const& entry : getEntries(level)) {
        util::filesystem::removeSaveFile(level, entry.slot);
    }

    auto const historyPath = util::filesystem::getHistoryFilePath(level);
    if (std::filesystem::exists(historyPath)) {
        std::filesystem::remove(historyPath);
    }
}

std::string SaveHistoryManager::formatEntryLabel(const SaveHistoryEntry& entry, size_t displayNumber) const {
    auto const reason = entry.reason == SaveReason::Manual ? "Manual save" : "Checkpoint";
    return fmt::format("#{} — {} — {} checkpoint{}", displayNumber, reason, entry.checkpointCount, entry.checkpointCount == 1 ? "" : "s");
}
