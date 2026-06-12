#include "PauseLayer.hpp"
#include <hooks/PlayLayer.hpp>
#include <ui/SaveHistoryMenuPopup.hpp>
#include <util/platform.hpp>

using namespace geode::prelude;
using namespace persistenceAPI;
using namespace util::platform;

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

    m_fields->m_saveCheckpointsSprite = CircleButtonSprite::createWithSprite("saveButton.png"_spr, 1.5, CircleBaseColor::Green, CircleBaseSize::Medium);
    m_fields->m_saveCheckpointsSprite->setScale(0.66);
    CCSize l_contentSize = m_fields->m_saveCheckpointsSprite->getContentSize();
    m_fields->m_saveCheckpointsSprite->setContentSize({l_contentSize.width, 49});
    m_fields->m_saveCheckpointsButton = CCMenuItemSpriteExtra::create(
        m_fields->m_saveCheckpointsSprite,
        this,
        menu_selector(PSPauseLayer::onSaveCheckpoints)
    );
    m_fields->m_saveCheckpointsButton->setID("save-button"_spr);

    if (!l_playLayer->canSave() && l_playLayer->m_fields->m_savingState == SavingState::Ready) {
        m_fields->m_saveCheckpointsSprite->setColor({127,127,127});
        if (m_fields->m_saveCheckpointsSprite->getChildren()->count() > 0) {
            static_cast<CCSprite*>(m_fields->m_saveCheckpointsSprite->getChildren()->objectAtIndex(0))->setColor({127,127,127});
        }
        m_fields->m_saveCheckpointsButton->m_bEnabled = false;
    }

    m_fields->m_loadSaveSprite = CircleButtonSprite::createWithSprite("loadButton.png"_spr, 1.5, CircleBaseColor::Green, CircleBaseSize::Medium);
    m_fields->m_loadSaveSprite->setScale(0.66);
    m_fields->m_loadSaveSprite->setContentSize({l_contentSize.width, 49});
    m_fields->m_loadSaveButton = CCMenuItemSpriteExtra::create(
        m_fields->m_loadSaveSprite,
        this,
        menu_selector(PSPauseLayer::onLoadSave)
    );
    m_fields->m_loadSaveButton->setID("load-save-button"_spr);

    if (l_playLayer->m_fields->m_savingState != SavingState::Ready) {
        m_fields->m_loadSaveSprite->setColor({127,127,127});
        if (m_fields->m_loadSaveSprite->getChildren()->count() > 0) {
            static_cast<CCSprite*>(m_fields->m_loadSaveSprite->getChildren()->objectAtIndex(0))->setColor({127,127,127});
        }
        m_fields->m_loadSaveButton->m_bEnabled = false;
    }

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
            m_fields->m_saveCheckpointsSprite->setColor({127,127,127});
            if (m_fields->m_saveCheckpointsSprite->getChildren()->count() > 0) {
                static_cast<CCSprite*>(m_fields->m_saveCheckpointsSprite->getChildren()->objectAtIndex(0))->setColor({127,127,127});
            }
            m_fields->m_saveCheckpointsButton->m_bEnabled = false;
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
