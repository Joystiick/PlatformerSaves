#pragma once
#include <Geode/ui/Popup.hpp>
#include <save/SaveHistoryManager.hpp>
#include <vector>

class SaveHistoryMenuPopup : public geode::Popup {
protected:
    std::vector<SaveHistoryEntry> m_entries;
    std::vector<size_t> m_oldestFirstIndices;
    cocos2d::extension::CCScrollView* m_scrollView = nullptr;
    cocos2d::CCMenu* m_listMenu = nullptr;

    bool init() override;
    void buildList();
    void onLoadEntry(cocos2d::CCObject* sender);
    void onDeleteEntry(cocos2d::CCObject* sender);
    void onRenameEntry(cocos2d::CCObject* sender);
    void onExport(cocos2d::CCObject* sender);
    void onImport(cocos2d::CCObject* sender);
    void performLoad(size_t oldestFirstIndex);
    void onClose(cocos2d::CCObject* sender) override;

public:
    static SaveHistoryMenuPopup* create();
};
