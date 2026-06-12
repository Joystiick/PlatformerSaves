#include "SaveHistoryMenuPopup.hpp"
#include <hooks/PlayLayer.hpp>

using namespace geode::prelude;

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
    if (!Popup::init(340.f, 280.f)) {
        return false;
    }

    setTitle("Load Save");

    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!playLayer || !playLayer->m_level) {
        return true;
    }

    m_entries = SaveHistoryManager::get().getEntries(playLayer->m_level);
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
    }

    buildList();
    return true;
}

void SaveHistoryMenuPopup::buildList() {
    auto size = m_mainLayer->getContentSize();
    float rowHeight = 36.f;
    float contentHeight = std::max(size.height - 60.f, rowHeight * static_cast<float>(m_entries.size()));

    m_listMenu = CCMenu::create();
    m_listMenu->setPosition({0.f, 0.f});
    m_listMenu->setContentSize({size.width - 20.f, contentHeight});

    auto& manager = SaveHistoryManager::get();
    bool newestFirst = Mod::get()->getSettingValue<bool>("save-history-sort");

    for (size_t displayIndex = 0; displayIndex < m_entries.size(); displayIndex++) {
        size_t oldestFirstIndex = newestFirst
            ? m_entries.size() - 1 - displayIndex
            : displayIndex;

        auto label = manager.formatEntryLabel(
            m_entries[displayIndex],
            displayIndex + 1
        );

        auto* rowLabel = CCLabelBMFont::create(label.c_str(), "goldFont.fnt");
        rowLabel->setScale(0.55f);
        rowLabel->setAnchorPoint({0.f, 0.5f});
        rowLabel->setPosition({10.f, contentHeight - rowHeight * (displayIndex + 0.5f)});

        auto* loadBtn = ButtonSprite::create("Load", 50.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.6f);
        auto* loadItem = CCMenuItemSpriteExtra::create(
            loadBtn,
            this,
            menu_selector(SaveHistoryMenuPopup::onLoadEntry)
        );
        loadItem->setUserData(reinterpret_cast<void*>(oldestFirstIndex));
        loadItem->setPosition({size.width - 55.f, contentHeight - rowHeight * (displayIndex + 0.5f)});

        m_listMenu->addChild(loadItem);
        m_mainLayer->addChild(rowLabel);
    }

    m_scrollView = CCScrollView::create({size.width - 20.f, size.height - 70.f}, true);
    m_scrollView->setPosition({10.f, 10.f});
    m_scrollView->setContentOffset({0.f, size.height - 70.f - contentHeight});
    m_scrollView->setContentSize({size.width - 20.f, contentHeight});
    m_scrollView->addChild(m_listMenu);
    m_mainLayer->addChild(m_scrollView);
}

void SaveHistoryMenuPopup::onLoadEntry(CCObject* sender) {
    auto* item = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!item) {
        return;
    }
    performLoad(reinterpret_cast<size_t>(item->getUserData()));
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

    size_t newerCount = entries.size() - oldestFirstIndex - 1;
    auto doLoad = [=]() {
        if (playLayer->loadFromHistoryIndex(oldestFirstIndex)) {
            onClose(nullptr);
        }
    };

    if (newerCount > 0 && Mod::get()->getSettingValue<bool>("confirm-load-truncate")) {
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

    doLoad();
}

void SaveHistoryMenuPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);
}
