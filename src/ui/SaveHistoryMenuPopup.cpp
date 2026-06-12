#include "SaveHistoryMenuPopup.hpp"
#include <Geode/ui/TextInput.hpp>
#include <functional>
#include <hooks/PlayLayer.hpp>
#include <util/SavePackageIO.hpp>

using namespace geode::prelude;

namespace {
class RenameSavePopup : public Popup {
    TextInput* m_input = nullptr;
    GJGameLevel* m_level = nullptr;
    size_t m_index = 0;
    std::function<void()> m_onRenamed;

public:
    static RenameSavePopup* create(GJGameLevel* level, size_t index, std::function<void()> onRenamed) {
        auto* ret = new RenameSavePopup();
        if (ret && ret->init(level, index, std::move(onRenamed))) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(GJGameLevel* level, size_t index, std::function<void()> onRenamed) {
        if (!Popup::init(300.f, 160.f)) {
            return false;
        }
        m_level = level;
        m_index = index;
        m_onRenamed = std::move(onRenamed);
        setTitle("Rename Save");

        m_input = TextInput::create(240.f, "Save name", "bigFont.fnt");
        m_input->setPosition({m_mainLayer->getContentWidth() / 2.f, m_mainLayer->getContentHeight() / 2.f + 10.f});
        m_mainLayer->addChild(m_input);

        auto* menu = CCMenu::create();
        menu->setPosition({m_mainLayer->getContentWidth() / 2.f, 35.f});
        auto* okBtn = ButtonSprite::create("OK", 60.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.6f);
        auto* okItem = CCMenuItemSpriteExtra::create(okBtn, this, menu_selector(RenameSavePopup::onOk));
        menu->addChild(okItem);
        m_mainLayer->addChild(menu);
        return true;
    }

    void onOk(CCObject*) {
        auto const name = m_input->getString();
        if (!name.empty()) {
            SaveHistoryManager::get().renameEntry(m_level, m_index, name);
        }
        onClose(nullptr);
        if (m_onRenamed) {
            m_onRenamed();
        }
    }

    void onClose(CCObject* sender) override {
        Popup::onClose(sender);
    }
};
}

SaveHistoryMenuPopup* SaveHistoryMenuPopup::create() {
    auto* ret = new SaveHistoryMenuPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SaveHistoryMenuPopup::init() {
    if (!Popup::init(400.f, 320.f)) {
        return false;
    }

    setTitle("Load Save");

    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!playLayer || !playLayer->m_level) {
        return true;
    }

    auto allEntries = SaveHistoryManager::get().getEntries(playLayer->m_level);
    bool const speedrun = playLayer->isSpeedrunMode();
    for (size_t i = 0; i < allEntries.size(); i++) {
        if (speedrun && allEntries[i].branchId != SaveHistoryManager::SPEEDRUN_BRANCH) {
            continue;
        }
        m_oldestFirstIndices.push_back(i);
        m_entries.push_back(allEntries[i]);
    }

    auto* bottomMenu = CCMenu::create();
    bottomMenu->setPosition({m_mainLayer->getContentWidth() / 2.f, 22.f});

    auto* exportBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Export", 60.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.5f),
        this,
        menu_selector(SaveHistoryMenuPopup::onExport)
    );
    auto* importBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Import", 60.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.5f),
        this,
        menu_selector(SaveHistoryMenuPopup::onImport)
    );
    bottomMenu->addChild(exportBtn);
    bottomMenu->addChild(importBtn);
    bottomMenu->alignItemsHorizontallyWithPadding(12.f);
    m_mainLayer->addChild(bottomMenu);

    if (m_entries.empty()) {
        auto* empty = CCLabelBMFont::create("No saves yet", "Pusab.fnt"_spr);
        empty->setScale(0.45f);
        empty->setPosition(m_mainLayer->getContentSize() / 2.f);
        m_mainLayer->addChild(empty);
        return true;
    }

    bool newestFirst = Mod::get()->getSettingValue<bool>("save-history-sort");
    if (newestFirst) {
        std::reverse(m_entries.begin(), m_entries.end());
        std::reverse(m_oldestFirstIndices.begin(), m_oldestFirstIndices.end());
    }

    buildList();
    return true;
}

void SaveHistoryMenuPopup::buildList() {
    auto size = m_mainLayer->getContentSize();
    float rowHeight = 44.f;
    float contentHeight = std::max(size.height - 90.f, rowHeight * static_cast<float>(m_entries.size()));

    m_listMenu = CCMenu::create();
    m_listMenu->setPosition({0.f, 0.f});
    m_listMenu->setContentSize({size.width - 20.f, contentHeight});

    auto& manager = SaveHistoryManager::get();

    for (size_t displayIndex = 0; displayIndex < m_entries.size(); displayIndex++) {
        size_t oldestFirstIndex = m_oldestFirstIndices[displayIndex];
        auto const& entry = m_entries[displayIndex];

        auto label = manager.formatEntryLabel(entry, displayIndex + 1);
        if (!entry.name.empty()) {
            label = fmt::format("{} | {:.1f}s | {} deaths", label, entry.timePlayed, entry.deaths);
        } else {
            label = fmt::format("{} | {:.1f}s | {} deaths", label, entry.timePlayed, entry.deaths);
        }

        auto* rowLabel = CCLabelBMFont::create(label.c_str(), "goldFont.fnt");
        rowLabel->setScale(0.45f);
        rowLabel->setAnchorPoint({0.f, 0.5f});
        rowLabel->setPosition({8.f, contentHeight - rowHeight * (displayIndex + 0.5f)});

        float btnY = contentHeight - rowHeight * (displayIndex + 0.5f);

        auto* loadBtn = ButtonSprite::create("Load", 42.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.55f);
        auto* loadItem = CCMenuItemSpriteExtra::create(loadBtn, this, menu_selector(SaveHistoryMenuPopup::onLoadEntry));
        loadItem->setUserData(reinterpret_cast<void*>(oldestFirstIndex));
        loadItem->setPosition({size.width - 48.f, btnY});

        auto* delBtn = ButtonSprite::create("X", 28.f, true, "goldFont.fnt", "GJ_button_04.png", 0.f, 0.55f);
        auto* delItem = CCMenuItemSpriteExtra::create(delBtn, this, menu_selector(SaveHistoryMenuPopup::onDeleteEntry));
        delItem->setUserData(reinterpret_cast<void*>(oldestFirstIndex));
        delItem->setPosition({size.width - 95.f, btnY});

        auto* renBtn = ButtonSprite::create("R", 28.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.55f);
        auto* renItem = CCMenuItemSpriteExtra::create(renBtn, this, menu_selector(SaveHistoryMenuPopup::onRenameEntry));
        renItem->setUserData(reinterpret_cast<void*>(oldestFirstIndex));
        renItem->setPosition({size.width - 130.f, btnY});

        m_listMenu->addChild(loadItem);
        m_listMenu->addChild(delItem);
        m_listMenu->addChild(renItem);
        m_mainLayer->addChild(rowLabel);
    }

    m_scrollView = CCScrollView::create({size.width - 20.f, size.height - 100.f}, m_listMenu);
    m_scrollView->setDirection(kCCScrollViewDirectionVertical);
    m_scrollView->setPosition({10.f, 40.f});
    m_scrollView->setContentOffset({0.f, size.height - 100.f - contentHeight});
    m_scrollView->setContentSize({size.width - 20.f, contentHeight});
    m_mainLayer->addChild(m_scrollView);
}

void SaveHistoryMenuPopup::onLoadEntry(CCObject* sender) {
    auto* item = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!item) {
        return;
    }
    performLoad(reinterpret_cast<size_t>(item->getUserData()));
}

void SaveHistoryMenuPopup::onDeleteEntry(CCObject* sender) {
    auto* item = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!item || !playLayer) {
        return;
    }
    size_t const index = reinterpret_cast<size_t>(item->getUserData());
    createQuickPopup("Delete Save", "Delete this save entry permanently?", "Cancel", "Delete",
        [this, playLayer, index](FLAlertLayer*, bool btn2) {
            if (btn2 && SaveHistoryManager::get().deleteEntry(playLayer->m_level, index)) {
                onClose(nullptr);
                SaveHistoryMenuPopup::create()->show();
            }
        }
    );
}

void SaveHistoryMenuPopup::onRenameEntry(CCObject* sender) {
    auto* item = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!item || !playLayer) {
        return;
    }
    size_t const index = reinterpret_cast<size_t>(item->getUserData());
    RenameSavePopup::create(playLayer->m_level, index, [this]() {
        onClose(nullptr);
        SaveHistoryMenuPopup::create()->show();
    })->show();
}

void SaveHistoryMenuPopup::onExport(CCObject* sender) {
    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!playLayer || !playLayer->m_level) {
        return;
    }
    auto const dest = Mod::get()->getSaveDir() / "exports" / fmt::format("{}", playLayer->m_level->m_levelID.value());
    if (util::savepackage::exportLevelSaves(playLayer->m_level, dest.string())) {
        Notification::create(fmt::format("Exported to {}", dest.string()), NotificationIcon::Success, 2.5f)->show();
    } else {
        Notification::create("Export failed", NotificationIcon::Error, 1.5f)->show();
    }
}

void SaveHistoryMenuPopup::onImport(CCObject* sender) {
    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!playLayer || !playLayer->m_level) {
        return;
    }
    auto const src = Mod::get()->getSaveDir() / "exports" / fmt::format("{}", playLayer->m_level->m_levelID.value());
    createQuickPopup("Import Saves", "Import saves from export folder? Existing entries are kept (merge).", "Cancel", "Import",
        [playLayer, src](FLAlertLayer*, bool btn2) {
            if (btn2) {
                if (util::savepackage::importLevelSaves(playLayer->m_level, src.string(), true)) {
                    Notification::create("Saves imported", NotificationIcon::Success, 1.5f)->show();
                } else {
                    Notification::create("Import failed — export folder missing?", NotificationIcon::Error, 2.f)->show();
                }
            }
        }
    );
}

void SaveHistoryMenuPopup::performLoad(size_t oldestFirstIndex) {
    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!playLayer) {
        return;
    }

    auto entries = SaveHistoryManager::get().getEntries(playLayer->m_level);
    if (oldestFirstIndex >= entries.size()) {
        return;
    }

    auto const loadBehavior = Mod::get()->getSettingValue<std::string>("load-behavior");
    size_t newerCount = entries.size() - oldestFirstIndex - 1;
    auto doLoad = [this, playLayer, oldestFirstIndex]() {
        if (playLayer->loadFromHistoryIndex(oldestFirstIndex)) {
            onClose(nullptr);
        }
    };

    if (loadBehavior == "truncate" && newerCount > 0 && Mod::get()->getSettingValue<bool>("confirm-load-truncate")) {
        createQuickPopup(
            "Load Save",
            fmt::format(
                "Loading this save will <cr>delete {} newer save{}</c>. Continue?",
                newerCount,
                newerCount == 1 ? "" : "s"
            ),
            "Cancel",
            "Load",
            [=](FLAlertLayer*, bool btn2) {
                if (btn2) {
                    doLoad();
                }
            }
        );
        return;
    }

    if (loadBehavior == "branch" && newerCount > 0) {
        createQuickPopup(
            "Branch Load",
            fmt::format(
                "Load this save on a <cg>new branch</c>? {} newer save{} will be kept.",
                newerCount,
                newerCount == 1 ? "" : "s"
            ),
            "Cancel",
            "Load",
            [=](FLAlertLayer*, bool btn2) {
                if (btn2) {
                    doLoad();
                }
            }
        );
        return;
    }

    doLoad();
}

void SaveHistoryMenuPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);
}
