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
    std::string name;
    double timePlayed = 0.0;
    int attempts = 0;
    int deaths = 0;
    std::string branchId = "main";
};

struct SaveHistoryAppendParams {
    int slot = 0;
    SaveReason reason = SaveReason::Checkpoint;
    int checkpointCount = 0;
    std::string name;
    double timePlayed = 0.0;
    int attempts = 0;
    int deaths = 0;
    std::string branchId;
};

class SaveHistoryManager {
public:
    static constexpr int HISTORY_SCHEMA_VERSION = 2;
    static constexpr char const* DEFAULT_BRANCH = "main";
    static constexpr char const* SPEEDRUN_BRANCH = "speedrun";
    static constexpr char const* kSpeedrunBranch = SPEEDRUN_BRANCH;

    static SaveHistoryManager& get();

    std::vector<SaveHistoryEntry> getEntries(GJGameLevel* level);
    std::vector<SaveHistoryEntry> getEntries(GJGameLevel* level, bool speedrunOnly);
    std::vector<SaveHistoryEntry> getEntriesForBranch(GJGameLevel* level, std::string const& branchId);
    std::optional<SaveHistoryEntry> getLatest(GJGameLevel* level);
    std::optional<SaveHistoryEntry> getLatestForBranch(GJGameLevel* level, std::string const& branchId);
    int getNextSlot(GJGameLevel* level);
    bool appendEntry(GJGameLevel* level, SaveHistoryAppendParams const& params);
    bool truncateAfterIndex(GJGameLevel* level, size_t oldestFirstIndex);
    bool deleteEntry(GJGameLevel* level, size_t oldestFirstIndex);
    bool renameEntry(GJGameLevel* level, size_t oldestFirstIndex, std::string const& name);
    void enforceHistoryCap(GJGameLevel* level);
    bool forkBranchOnLoad(GJGameLevel* level, size_t oldestFirstIndex, int& outSlot, std::string& outBranchId);
    std::string getActiveBranch(GJGameLevel* level);
    std::string getActiveBranchId(GJGameLevel* level);
    bool setActiveBranch(GJGameLevel* level, std::string const& branchId);
    bool hasValidSave(GJGameLevel* level);
    void removeAllSaves(GJGameLevel* level);

    std::string formatEntryLabel(const SaveHistoryEntry& entry, size_t displayNumber) const;
    std::string reasonPrefix(SaveReason reason) const;
    std::string reasonToString(SaveReason reason) const;

private:
    SaveHistoryManager() = default;

    bool loadHistory(GJGameLevel* level, matjson::Value& outHistory);
    bool saveHistory(GJGameLevel* level, const matjson::Value& history);
    void migrateHistory(matjson::Value& history);
    bool forkLoadAtIndex(GJGameLevel* level, size_t oldestFirstIndex, int& outSlot, std::string& outBranchId);
    SaveHistoryEntry entryFromJson(const matjson::Value& value) const;
    matjson::Value entryToJson(const SaveHistoryEntry& entry) const;
    std::vector<SaveHistoryEntry> entriesFromHistory(matjson::Value const& history) const;
};
