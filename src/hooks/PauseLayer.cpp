#include "PauseLayer.hpp"
#include <hooks/PlayLayer.hpp>
#include <ui/SaveHistoryMenuPopup.hpp>
#include <util/platform.hpp>

using namespace geode::prelude;
using namespace persistenceAPI;
using namespace util::platform;

namespace {
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

    m_fields->m_saveCheckpointsSprite = CCSprite::create("saveButton.png"_spr);
    m_fields->m_saveCheckpointsSprite->setScale(0.5f);
    m_fields->m_saveCheckpointsButton = CCMenuItemSpriteExtra::create(
        m_fields->m_saveCheckpointsSprite,
        this,
        menu_selector(PSPauseLayer::onSaveCheckpoints)
    );
    m_fields->m_saveCheckpointsButton->setID("save-button"_spr);

    m_fields->m_loadSaveSprite = CCSprite::create("loadButton.png"_spr);
    m_fields->m_loadSaveSprite->setScale(0.5f);
    m_fields->m_loadSaveButton = CCMenuItemSpriteExtra::create(
        m_fields->m_loadSaveSprite,
        this,
        menu_selector(PSPauseLayer::onLoadSave)
    );
    m_fields->m_loadSaveButton->setID("load-save-button"_spr);

    setSaveButtonEnabled(l_playLayer->canSave() && l_playLayer->m_fields->m_savingState == SavingState::Ready);
    setLoadButtonEnabled(l_playLayer->m_fields->m_savingState == SavingState::Ready);

    l_leftButtonMenu->addChild(m_fields->m_saveCheckpointsButton);
    l_leftButtonMenu->addChild(m_fields->m_loadSaveButton);
    l_leftButtonMenu->updateLayout();
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
    if (l_playLayer && l_playLayer->savesEnabled() && !m_fields->m_cancelSave && l_playLayer->canSave()) {
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
    }

    PauseLayer::onRestartFull(i_sender);
}

void PSPauseLayer::onSaveCheckpoints(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (l_playLayer && l_playLayer->m_fields->m_savingState == SavingState::Ready) {
        if (l_playLayer->startSaveGame(true)) {
            setSaveButtonEnabled(false);
        }
    }
}

void PSPauseLayer::onLoadSave(CCObject* i_sender) {
    PSPlayLayer* l_playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!l_playLayer || l_playLayer->m_fields->m_savingState != SavingState::Ready) {
        return;
    }

    SaveHistoryMenuPopup::create()->show();
}
