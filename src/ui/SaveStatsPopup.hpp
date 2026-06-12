#pragma once
#include <Geode/ui/Popup.hpp>

class SaveStatsPopup : public geode::Popup {
protected:
    bool init() override;
    void onClose(cocos2d::CCObject* sender) override;

public:
    static SaveStatsPopup* create();
};
