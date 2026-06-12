#include "PauseLayer.hpp"
#include <hooks/PlayLayer.hpp>
#include <save/SaveHistoryManager.hpp>
#include <ui/SaveHistoryMenuPopup.hpp>
#include <ui/SaveStatsPopup.hpp>
#include <util/Feedback.hpp>
#include <util/platform.hpp>

using namespace geode::prelude;
using namespace persistenceAPI;
using namespace util::platform;

namespace {
    constexpr float kPauseButtonHeight = 49.f;

    cocos2d::CCSprite* createPauseButtonSprite(char const* resource) {
        auto* sprite = CCSprite::create(resource);
        if (!sprite) {
            return nullptr;
        }
        auto const size = sprite->getContentSize();
        if (size.height > 0.f) {
            sprite->setScale(kPauseButtonHeight / size.height);
        }
        sprite->setContentSize({kPauseButtonHeight, kPauseButtonHeight});
        return sprite;
    }

    void setMenuSpriteEnabled(cocos2d::CCSprite* sprite, CCMenuItemSpriteExtra* button, bool enabled) {
        if (!sprite || !button) {
            return;
        }
        sprite->setColor(enabled ? ccc3(255, 255, 255) : ccc3(127, 127, 127));
        button->m_bEnabled = enabled;
    }
}

void PSPauseLayer::setSaveButtonEnabled(bool enabled) {
    setMenuSpriteEnabled(m_fields->m_saveCheckpointsSprite, m_fields->m_saveCheckpointsButton, enabled);
}

void PSPauseLayer::setLoadButtonEnabled(bool enabled) {
    setMenuSpriteEnabled(m_fields->m_loadSaveSprite, m_fields->m_loadSaveButton, enabled);
}

void PSPauseLayer::setRewindButtonEnabled(bool enabled) {
    setMenuSpriteEnabled(m_fields->m_rewindSprite, m_fields->m_rewindButton, enabled);
}

void PSPauseLayer::updateLastSavedLabel(PSPlayLayer* playLayer) {
    if (!m_fields->m_lastSavedLabel || !playLayer || !playLayer->m_level) {
        return;
    }

    auto latest = SaveHistoryManager::get().getLatest(playLayer->m_level);
    if (!latest) {
        m_fields->m_lastSavedLabel->setString("Last saved: —");
        return;
    }

    auto const prefix = SaveHistoryManager::get().reasonPrefix(latest->reason);
    auto const text = latest->name.empty()
        ? fmt::format("Last saved: {} cp{}", prefix, latest->checkpointCount)
        : fmt::format("Last saved: {} {}", prefix, latest->name);
    m_fields->m_lastSavedLabel->setString(text.c_str());
}

void PSPauseLayer::customSetup() {
    PauseLayer::customSetup();

    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!l_playLayer || !l_playLayer->savesEnabled() || !l_playLayer->m_isPlatformer || l_playLayer->m_isPracticeMode) {
        return;
    }

    CCMenu* l_leftButtonMenu = static_cast<CCMenu*>(getChildByID("left-button-menu"));
    if (!l_leftButtonMenu) {
        return;
    }

    m_fields->m_saveCheckpointsSprite = createPauseButtonSprite("saveButton.png"_spr);
    m_fields->m_saveCheckpointsButton = CCMenuItemSpriteExtra::create(
        m_fields->m_saveCheckpointsSprite,
        this,
        menu_selector(PSPauseLayer::onSaveCheckpoints)
    );
    m_fields->m_saveCheckpointsButton->setID("save-button"_spr);
    m_fields->m_saveCheckpointsButton->setContentSize({kPauseButtonHeight, kPauseButtonHeight});

    m_fields->m_loadSaveSprite = createPauseButtonSprite("loadButton.png"_spr);
    m_fields->m_loadSaveButton = CCMenuItemSpriteExtra::create(
        m_fields->m_loadSaveSprite,
        this,
        menu_selector(PSPauseLayer::onLoadSave)
    );
    m_fields->m_loadSaveButton->setID("load-save-button"_spr);
    m_fields->m_loadSaveButton->setContentSize({kPauseButtonHeight, kPauseButtonHeight});

    auto* rewindSprite = ButtonSprite::create("<<", 40.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.5f);
    m_fields->m_rewindSprite = rewindSprite;
    m_fields->m_rewindButton = CCMenuItemSpriteExtra::create(
        rewindSprite,
        this,
        menu_selector(PSPauseLayer::onRewind)
    );
    m_fields->m_rewindButton->setID("rewind-button"_spr);

    auto* statsSprite = ButtonSprite::create("Stats", 50.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.45f);
    m_fields->m_statsButton = CCMenuItemSpriteExtra::create(
        statsSprite,
        this,
        menu_selector(PSPauseLayer::onStats)
    );
    m_fields->m_statsButton->setID("stats-button"_spr);

    l_leftButtonMenu->addChild(m_fields->m_saveCheckpointsButton);
    l_leftButtonMenu->addChild(m_fields->m_loadSaveButton);
    l_leftButtonMenu->addChild(m_fields->m_rewindButton);
    l_leftButtonMenu->addChild(m_fields->m_statsButton);
    l_leftButtonMenu->updateLayout();

    auto const winSize = CCDirector::sharedDirector()->getWinSize();
    m_fields->m_lastSavedLabel = CCLabelBMFont::create("Last saved: —", "Pusab.fnt"_spr);
    m_fields->m_lastSavedLabel->setScale(0.35f);
    m_fields->m_lastSavedLabel->setAnchorPoint({0.f, 1.f});
    m_fields->m_lastSavedLabel->setPosition({10.f, winSize.height - 10.f});
    m_fields->m_lastSavedLabel->setID("last-saved-label"_spr);
    addChild(m_fields->m_lastSavedLabel);

    util::feedback::refreshPauseLayerUI(l_playLayer);
}

void PSPauseLayer::onEdit(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (l_playLayer && l_playLayer->m_fields->m_savingState != SavingState::Ready) {
        Notification::create("Cannot edit level while the game is being saved", NotificationIcon::Warning, 1.0f)->show();
        return;
    }

    PauseLayer::onEdit(i_sender);
}

void PSPauseLayer::tryQuit(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (l_playLayer && l_playLayer->m_fields->m_savingState != SavingState::Ready) {
        Notification::create("Cannot exit level while the game is being saved", NotificationIcon::Warning, 1.0f)->show();
        return;
    }

    PauseLayer::tryQuit(i_sender);
}

void PSPauseLayer::onQuit(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (l_playLayer && l_playLayer->savesEnabled() && !m_fields->m_cancelSave && l_playLayer->canSave() && !l_playLayer->isSpeedrunMode()) {
        l_playLayer->m_fields->m_exitAfterSave = true;
        createQuickPopup("Exit Level",
            "Are you sure you want to <cr>exit without saving</c>?",
            "Exit",
            "Save",
            [&](FLAlertLayer*, bool i_btn2) {
                if (i_btn2) {
                    onSaveCheckpoints(i_sender);
                } else {
                    m_fields->m_cancelSave = true;
                    PauseLayer::onQuit(i_sender);
                    m_fields->m_cancelSave = false;
                }
                hideAndLockCursor(true);
            }
        );
        return;
    }

    PauseLayer::onQuit(i_sender);
}

void PSPauseLayer::onRestartFull(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (l_playLayer && l_playLayer->savesEnabled()) {
        l_playLayer->m_fields->m_normalModeCheckpoints->removeAllObjects();
        l_playLayer->m_fields->m_activatedCheckpoints.clear();
        l_playLayer->m_fields->m_deathCount = 0;
    }

    PauseLayer::onRestartFull(i_sender);
}

void PSPauseLayer::onSaveCheckpoints(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!l_playLayer || l_playLayer->isSpeedrunMode()) {
        return;
    }
    if (l_playLayer->m_fields->m_savingState != SavingState::Ready) {
        return;
    }
    if (!l_playLayer->canSave()) {
        Notification::create("Already saved at this checkpoint", NotificationIcon::Info, 1.5f)->show();
        return;
    }
    if (l_playLayer->startSaveGame(true)) {
        setSaveButtonEnabled(false);
    }
}

void PSPauseLayer::onLoadSave(CCObject* i_sender) {
    util::feedback::openLoadMenu(static_cast<PSPlayLayer*>(PlayLayer::get()));
}

void PSPauseLayer::onRewind(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!l_playLayer) {
        return;
    }
    l_playLayer->rewindFromHistory();
}

void PSPauseLayer::onStats(CCObject* i_sender) {
    SaveStatsPopup::create()->show();
}
