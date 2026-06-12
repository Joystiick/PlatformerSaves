#include "SaveStatsPopup.hpp"
#include <hooks/PlayLayer.hpp>
#include <save/SaveHistoryManager.hpp>

using namespace geode::prelude;

SaveStatsPopup* SaveStatsPopup::create() {
    auto* ret = new SaveStatsPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SaveStatsPopup::init() {
    if (!Popup::init(300.f, 220.f)) {
        return false;
    }

    setTitle("Run Stats");

    auto* playLayer = static_cast<PSPlayLayer*>(PlayLayer::get());
    if (!playLayer) {
        return true;
    }

    auto const size = m_mainLayer->getContentSize();
    float y = size.height - 55.f;
    float const lineHeight = 28.f;

    auto addLine = [&](char const* label, std::string const& value) {
        auto* text = CCLabelBMFont::create(
            fmt::format("{}: {}", label, value).c_str(),
            "goldFont.fnt"
        );
        text->setScale(0.55f);
        text->setAnchorPoint({0.f, 0.5f});
        text->setPosition({15.f, y});
        m_mainLayer->addChild(text);
        y -= lineHeight;
    };

    addLine("Deaths", std::to_string(playLayer->m_fields->m_deathCount));
    addLine("Attempts", std::to_string(playLayer->m_attempts));
    addLine("Time", fmt::format("{:.1f}s", playLayer->m_timePlayed));
    addLine("Checkpoints", std::to_string(playLayer->m_fields->m_normalModeCheckpoints->count()));

    if (playLayer->m_level) {
        auto latest = SaveHistoryManager::get().getLatest(playLayer->m_level);
        if (latest) {
            addLine("History entries", std::to_string(SaveHistoryManager::get().getEntries(playLayer->m_level).size()));
            addLine("Branch", SaveHistoryManager::get().getActiveBranch(playLayer->m_level));
        }
    }

    return true;
}

void SaveStatsPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);
}
