#include "SaveHistoryManager.hpp"
#include <util/filesystem.hpp>
#include <fstream>

using namespace geode::prelude;

namespace {
    matjson::Value makeEmptyHistory() {
        return matjson::makeObject({
            {"schema", SaveHistoryManager::HISTORY_SCHEMA_VERSION},
            {"activeBranch", std::string(SaveHistoryManager::DEFAULT_BRANCH)},
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

std::string SaveHistoryManager::reasonPrefix(SaveReason reason) const {
    return reason == SaveReason::Manual ? "[M]" : "[C]";
}

SaveHistoryEntry SaveHistoryManager::entryFromJson(const matjson::Value& value) const {
    SaveHistoryEntry entry;
    entry.slot = value["slot"].asInt().unwrapOr(0);
    entry.timestamp = value["timestamp"].asInt().unwrapOr(0);
    auto const reason = value["reason"].asString().unwrapOr("checkpoint");
    entry.reason = reason == "manual" ? SaveReason::Manual : SaveReason::Checkpoint;
    entry.checkpointCount = value["checkpointCount"].asInt().unwrapOr(0);
    entry.name = value["name"].asString().unwrapOr("");
    entry.timePlayed = value["timePlayed"].asDouble().unwrapOr(0.0);
    entry.attempts = value["attempts"].asInt().unwrapOr(0);
    entry.deaths = value["deaths"].asInt().unwrapOr(0);
    entry.branchId = value["branchId"].asString().unwrapOr(DEFAULT_BRANCH);
    return entry;
}

matjson::Value SaveHistoryManager::entryToJson(const SaveHistoryEntry& entry) const {
    return matjson::makeObject({
        {"slot", entry.slot},
        {"timestamp", entry.timestamp},
        {"reason", reasonToString(entry.reason)},
        {"checkpointCount", entry.checkpointCount},
        {"name", entry.name},
        {"timePlayed", entry.timePlayed},
        {"attempts", entry.attempts},
        {"deaths", entry.deaths},
        {"branchId", entry.branchId}
    });
}

void SaveHistoryManager::migrateHistory(matjson::Value& history) {
    int schema = history["schema"].asInt().unwrapOr(1);
    if (schema >= HISTORY_SCHEMA_VERSION) {
        if (!history.contains("activeBranch")) {
            history["activeBranch"] = DEFAULT_BRANCH;
        }
        return;
    }

    auto entries = getEntriesArray(history);
    std::vector<matjson::Value> migrated;
    for (auto const& value : entries) {
        auto entry = entryFromJson(value);
        if (entry.branchId.empty()) {
            entry.branchId = DEFAULT_BRANCH;
        }
        migrated.push_back(entryToJson(entry));
    }
    history["entries"] = migrated;
    history["schema"] = HISTORY_SCHEMA_VERSION;
    if (!history.contains("activeBranch")) {
        history["activeBranch"] = DEFAULT_BRANCH;
    }
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
    migrateHistory(outHistory);
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

std::vector<SaveHistoryEntry> SaveHistoryManager::entriesFromHistory(matjson::Value const& history) const {
    std::vector<SaveHistoryEntry> entries;
    if (auto arr = history["entries"].asArray()) {
        for (auto const& value : arr.unwrap()) {
            entries.push_back(entryFromJson(value));
        }
    }
    return entries;
}

std::vector<SaveHistoryEntry> SaveHistoryManager::getEntries(GJGameLevel* level) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return {};
    }
    return entriesFromHistory(history);
}

std::vector<SaveHistoryEntry> SaveHistoryManager::getEntries(GJGameLevel* level, bool speedrunOnly) {
    if (!speedrunOnly) {
        return getEntries(level);
    }
    return getEntriesForBranch(level, SPEEDRUN_BRANCH);
}

std::vector<SaveHistoryEntry> SaveHistoryManager::getEntriesForBranch(GJGameLevel* level, std::string const& branchId) {
    auto entries = getEntries(level);
    std::vector<SaveHistoryEntry> filtered;
    for (auto const& entry : entries) {
        if (entry.branchId == branchId) {
            filtered.push_back(entry);
        }
    }
    return filtered;
}

std::optional<SaveHistoryEntry> SaveHistoryManager::getLatest(GJGameLevel* level) {
    auto entries = getEntries(level);
    if (entries.empty()) {
        return std::nullopt;
    }
    return entries.back();
}

std::optional<SaveHistoryEntry> SaveHistoryManager::getLatestForBranch(GJGameLevel* level, std::string const& branchId) {
    auto entries = getEntriesForBranch(level, branchId);
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

std::string SaveHistoryManager::getActiveBranch(GJGameLevel* level) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return DEFAULT_BRANCH;
    }
    return history["activeBranch"].asString().unwrapOr(DEFAULT_BRANCH);
}

std::string SaveHistoryManager::getActiveBranchId(GJGameLevel* level) {
    return getActiveBranch(level);
}

bool SaveHistoryManager::setActiveBranch(GJGameLevel* level, std::string const& branchId) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return false;
    }
    history["activeBranch"] = branchId;
    return saveHistory(level, history);
}

bool SaveHistoryManager::appendEntry(GJGameLevel* level, SaveHistoryAppendParams const& params) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return false;
    }

    auto entries = getEntriesArray(history);
    entries.push_back(entryToJson({
        .slot = params.slot,
        .timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count(),
        .reason = params.reason,
        .checkpointCount = params.checkpointCount,
        .name = params.name,
        .timePlayed = params.timePlayed,
        .attempts = params.attempts,
        .deaths = params.deaths,
        .branchId = params.branchId.empty() ? getActiveBranch(level) : params.branchId
    }));
    history["entries"] = entries;
    if (!saveHistory(level, history)) {
        return false;
    }
    enforceHistoryCap(level);
    return true;
}

void SaveHistoryManager::enforceHistoryCap(GJGameLevel* level) {
    int cap = Mod::get()->getSettingValue<int64_t>("max-history-entries");
    if (cap <= 0) {
        return;
    }

    matjson::Value history;
    if (!loadHistory(level, history)) {
        return;
    }

    auto entries = entriesFromHistory(history);
    while (static_cast<int>(entries.size()) > cap) {
        util::filesystem::removeSaveFile(level, entries.front().slot);
        entries.erase(entries.begin());
    }

    std::vector<matjson::Value> jsonEntries;
    for (auto const& entry : entries) {
        jsonEntries.push_back(entryToJson(entry));
    }
    history["entries"] = jsonEntries;
    saveHistory(level, history);
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

    matjson::Value history;
    if (!loadHistory(level, history)) {
        return false;
    }
    std::vector<matjson::Value> jsonEntries;
    for (auto const& entry : entries) {
        jsonEntries.push_back(entryToJson(entry));
    }
    history["entries"] = jsonEntries;
    return saveHistory(level, history);
}

bool SaveHistoryManager::deleteEntry(GJGameLevel* level, size_t oldestFirstIndex) {
    auto entries = getEntries(level);
    if (oldestFirstIndex >= entries.size()) {
        return false;
    }

    util::filesystem::removeSaveFile(level, entries[oldestFirstIndex].slot);
    entries.erase(entries.begin() + static_cast<ptrdiff_t>(oldestFirstIndex));

    matjson::Value history;
    if (!loadHistory(level, history)) {
        return false;
    }
    std::vector<matjson::Value> jsonEntries;
    for (auto const& entry : entries) {
        jsonEntries.push_back(entryToJson(entry));
    }
    history["entries"] = jsonEntries;
    return saveHistory(level, history);
}

bool SaveHistoryManager::renameEntry(GJGameLevel* level, size_t oldestFirstIndex, std::string const& name) {
    matjson::Value history;
    if (!loadHistory(level, history)) {
        return false;
    }

    auto entries = getEntriesArray(history);
    if (oldestFirstIndex >= entries.size()) {
        return false;
    }

    auto entry = entryFromJson(entries[oldestFirstIndex]);
    entry.name = name;
    entries[oldestFirstIndex] = entryToJson(entry);
    history["entries"] = entries;
    return saveHistory(level, history);
}

bool SaveHistoryManager::forkLoadAtIndex(GJGameLevel* level, size_t oldestFirstIndex, int& outSlot, std::string& outBranchId) {
    auto entries = getEntries(level);
    if (oldestFirstIndex >= entries.size()) {
        return false;
    }

    auto const& source = entries[oldestFirstIndex];
    auto const srcPath = util::filesystem::getSaveFilePath(level, source.slot, true);
    if (srcPath.empty()) {
        return false;
    }

    outSlot = getNextSlot(level);
    auto const destPath = util::filesystem::getSaveFilePath(level, outSlot, false);
    std::error_code ec;
    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        return false;
    }

    outBranchId = fmt::format("fork-{}", outSlot);
    setActiveBranch(level, outBranchId);

    return appendEntry(level, {
        .slot = outSlot,
        .reason = source.reason,
        .checkpointCount = source.checkpointCount,
        .name = source.name.empty() ? fmt::format("Fork #{}", outSlot) : source.name,
        .timePlayed = source.timePlayed,
        .attempts = source.attempts,
        .deaths = source.deaths,
        .branchId = outBranchId
    });
}

bool SaveHistoryManager::forkBranchOnLoad(GJGameLevel* level, size_t oldestFirstIndex, int& outSlot, std::string& outBranchId) {
    return forkLoadAtIndex(level, oldestFirstIndex, outSlot, outBranchId);
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
    auto const prefix = reasonPrefix(entry.reason);
    auto const namePart = entry.name.empty()
        ? fmt::format("{} cp{}", prefix, entry.checkpointCount)
        : fmt::format("{} {}", prefix, entry.name);
    return fmt::format("#{} — {} — {} attempt{}", displayNumber, namePart, entry.attempts, entry.attempts == 1 ? "" : "s");
}
