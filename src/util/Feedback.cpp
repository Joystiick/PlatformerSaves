#include "Feedback.hpp"
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/PauseLayer.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <hooks/PauseLayer.hpp>
#include <hooks/PlayLayer.hpp>
#include <save/SaveHistoryManager.hpp>
#include <ui/SaveHistoryMenuPopup.hpp>

using namespace geode::prelude;

namespace util::feedback {

PSPauseLayer* getOpenPauseLayer() {
    auto* scene = CCScene::get();
    if (!scene) {
        return nullptr;
    }
    auto* pauseLayer = typeinfo_cast<PauseLayer*>(scene->getChildByID("PauseLayer"));
    return pauseLayer ? static_cast<PSPauseLayer*>(pauseLayer) : nullptr;
}

void refreshPauseLayerUI(PSPlayLayer* playLayer) {
    if (!playLayer) {
        return;
    }

    auto* pauseLayer = getOpenPauseLayer();
    if (!pauseLayer) {
        return;
    }

    bool const saving = playLayer->m_fields->m_savingState == SavingState::Ready;
    bool const canManualSave = playLayer->canSave() && saving && !playLayer->isSpeedrunMode();
    pauseLayer->setSaveButtonEnabled(canManualSave);
    pauseLayer->setLoadButtonEnabled(saving);
    pauseLayer->setRewindButtonEnabled(saving && playLayer->canRewind());
    pauseLayer->updateLastSavedLabel(playLayer);
}

void playSaveSound(bool isLoad) {
    if (!Mod::get()->getSettingValue<bool>("play-save-sounds")) {
        return;
    }
    auto* engine = FMODAudioEngine::get();
    if (!engine) {
        return;
    }
    engine->playEffect(isLoad ? "btn_02.wav" : "btn_01.wav");
}

void showSaveNotification(bool manual) {
    if (manual) {
        if (Mod::get()->getSettingValue<bool>("show-manual-save-notifications")) {
            Notification::create("Game saved", NotificationIcon::Success, 1.5f)->show();
        }
    } else if (Mod::get()->getSettingValue<bool>("show-autosave-notifications")) {
        Notification::create("Checkpoint saved", NotificationIcon::Success, 1.5f)->show();
    }
}

void showLoadNotification() {
    if (Mod::get()->getSettingValue<bool>("show-manual-save-notifications")) {
        Notification::create("Save loaded", NotificationIcon::Success, 1.5f)->show();
    }
}

bool canOpenLoadMenu(PSPlayLayer* playLayer) {
    if (!playLayer || !playLayer->savesEnabled()) {
        return false;
    }
    if (!playLayer->m_isPlatformer || playLayer->m_isPracticeMode) {
        return false;
    }
    if (playLayer->m_fields->m_savingState != SavingState::Ready) {
        return false;
    }
    return SaveHistoryManager::get().hasValidSave(playLayer->m_level);
}

void openLoadMenu(PSPlayLayer* playLayer) {
    if (!canOpenLoadMenu(playLayer)) {
        return;
    }
    SaveHistoryMenuPopup::create()->show();
}

}
