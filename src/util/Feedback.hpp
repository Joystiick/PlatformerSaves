#pragma once
#include <Geode/Geode.hpp>

class PSPlayLayer;
class PSPauseLayer;

namespace util::feedback {
    PSPauseLayer* getOpenPauseLayer();
    void refreshPauseLayerUI(PSPlayLayer* playLayer);
    void playSaveSound(bool isLoad);
    void showSaveNotification(bool manual);
    void showLoadNotification();
    bool canOpenLoadMenu(PSPlayLayer* playLayer);
    void openLoadMenu(PSPlayLayer* playLayer);
}
