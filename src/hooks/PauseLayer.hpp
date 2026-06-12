#pragma once
#include "Geode/binding/PauseLayer.hpp"
#include "Geode/cocos/cocoa/CCObject.h"
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <sabe.persistenceapi/include/PersistenceAPI.hpp>

class $modify(PSPauseLayer, PauseLayer) {
public:
    struct Fields {
        cocos2d::CCSprite* m_saveCheckpointsSprite = nullptr;
        CCMenuItemSpriteExtra* m_saveCheckpointsButton = nullptr;
        cocos2d::CCSprite* m_loadSaveSprite = nullptr;
        CCMenuItemSpriteExtra* m_loadSaveButton = nullptr;
        bool m_cancelSave = false;
    };

    $override
    void customSetup();

    $override
    void onEdit(cocos2d::CCObject* i_sender);

    $override
    void tryQuit(cocos2d::CCObject* i_sender);

    $override
    void onRestartFull(cocos2d::CCObject* i_sender);

    $override
    void onQuit(cocos2d::CCObject* i_sender);

    void onSaveCheckpoints(cocos2d::CCObject* i_sender);
    void onLoadSave(cocos2d::CCObject* i_sender);
    void setSaveButtonEnabled(bool enabled);
};
