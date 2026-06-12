#pragma once
#include <Geode/Geode.hpp>
#include <optional>
#include <string>
#include <vector>

enum class SaveReason {
    Checkpoint,
    Manual
};

struct SaveHistoryEntry {
    int slot = 0;
    int64_t timestamp = 0;
    SaveReason reason = SaveReason::Checkpoint;
    int checkpointCount = 0;
};

class SaveHistoryManager {
public:
    static SaveHistoryManager& get();

    std::vector<SaveHistoryEntry> getEntries(GJGameLevel* level);
    std::optional<SaveHistoryEntry> getLatest(GJGameLevel* level);
    int getNextSlot(GJGameLevel* level);
    bool appendEntry(GJGameLevel* level, int slot, SaveReason reason, int checkpointCount);
    bool truncateAfterIndex(GJGameLevel* level, size_t oldestFirstIndex);
    bool hasValidSave(GJGameLevel* level);
    void removeAllSaves(GJGameLevel* level);

    std::string formatEntryLabel(const SaveHistoryEntry& entry, size_t displayNumber) const;
    std::string reasonToString(SaveReason reason) const;

private:
    SaveHistoryManager() = default;

    bool loadHistory(GJGameLevel* level, matjson::Value& outHistory);
    bool saveHistory(GJGameLevel* level, const matjson::Value& history);
    SaveHistoryEntry entryFromJson(const matjson::Value& value) const;
    matjson::Value entryToJson(const SaveHistoryEntry& entry) const;
};
