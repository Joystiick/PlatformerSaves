#include "PlayLayer.hpp"
#include <filesystem>
#include <hooks/PauseLayer.hpp>
#include <save/SaveHistoryManager.hpp>
#include <util/algorithm.hpp>
#include <util/filesystem.hpp>
#include <util/platform.hpp>

using namespace geode::prelude;
using namespace persistenceAPI;
using namespace util::platform;

bool PSPlayLayer::startSaveGame(bool manual) {
    if (m_fields->m_savingState != SavingState::Ready || m_isPracticeMode) return false;
    if (!manual && !canSave()) return false;

    m_fields->m_manualSave = manual;
    m_fields->m_pendingSaveReason = manual ? SaveReason::Manual : SaveReason::Checkpoint;
    m_fields->m_saveSlot = SaveHistoryManager::get().getNextSlot(m_level);
    m_fields->m_savingState = SavingState::Setup;

    CCScene* l_currentScene = CCScene::get();
    if (l_currentScene) {
        l_currentScene->runAction(
            CCSequence::create(
                CCDelayTime::create(0.0f),
                CCCallFunc::create(
                    this,
                    callfunc_selector(PSPlayLayer::saveGame)
                ),
                nullptr
            )
        );
    }
    return true;
}

void PSPlayLayer::writePSFHeader() {
    m_fields->m_stream.write(s_psfMagicAndVer,sizeof(s_psfMagicAndVer));
    // finishedSaving
    m_fields->m_stream.writeZero(sizeof(bool));
    PSFData l_psfData;
    l_psfData.data = 0;
    // save platform
    l_psfData.m_platform = static_cast<uint16_t>(m_fields->m_platform);
    // save original psf version
    l_psfData.m_originalVersion = m_fields->m_originalPSFVersion;
    // save updated from previous level version
    l_psfData.m_updatedFromPreviousLevelVersion = m_fields->m_updatedFromPreviousLevelVersion;
    // save low detail mode
    l_psfData.m_lowDetailMode = m_level->m_lowDetailMode && m_level->m_lowDetailModeToggled;
    m_fields->m_stream.write(reinterpret_cast<char*>(&l_psfData.data),sizeof(l_psfData.data));
    // unused bytes
    m_fields->m_stream.writeZero(16-(sizeof(s_psfMagicAndVer)+sizeof(bool)+sizeof(PSFData)));
    unsigned int l_levelStringHash = util::algorithm::hash_string(m_level->m_levelString.c_str());
    m_fields->m_stream << l_levelStringHash;
}

void PSPlayLayer::saveGame() {
    switch (m_fields->m_savingState) {
        case SavingState::Setup: {
            if (m_fields->m_normalModeCheckpoints->count() == 0) {
                m_fields->m_savingState = SavingState::Ready;
                break;
            }
            m_fields->m_remainingCheckpointSaveCount = 1;

            std::string l_filePath = getSaveFilePath(-1);
            std::string l_fileDirectory = util::filesystem::getParentDirectoryFromPath(l_filePath);
            if (!std::filesystem::exists(l_fileDirectory) && !std::filesystem::create_directories(l_fileDirectory)) {
                m_fields->m_savingState = SavingState::Ready;
                break;
            }

            int l_PAVersion = 2;
            if (m_fields->m_readPSFVersion > 0 && m_fields->m_readPSFVersion < 10) {
                l_PAVersion = 1;
            }
            if (!m_fields->m_stream.setFile(l_filePath, l_PAVersion, true)) {
                m_fields->m_savingState = SavingState::Ready;
                break;
            }

            showSavingProgressCircleSprite(true);

            writePSFHeader();

            m_fields->m_stream << m_fields->m_remainingCheckpointSaveCount;

            m_fields->m_savingState = SavingState::SaveCheckpoint;
            // falls through
        }
        case SavingState::SaveCheckpoint: {
            if (m_fields->m_remainingCheckpointSaveCount > 0) {
                saveCheckpointToStream(m_fields->m_normalModeCheckpoints->count()-m_fields->m_remainingCheckpointSaveCount);
                m_fields->m_remainingCheckpointSaveCount--;
                CCScene* l_currentScene = CCScene::get();
                if (l_currentScene) {
                    l_currentScene->runAction(
                        CCSequence::create(
                            CCDelayTime::create(0.0f),
                            CCCallFunc::create(
                                this,
                                callfunc_selector(PSPlayLayer::saveGame)
                            ),
                            nullptr
                        )
                    );
                }
                break;
            }
            if (m_fields->m_remainingCheckpointSaveCount == 0) {
                m_fields->m_savingState = SavingState::SaveActivatedCheckpoints;
            }
            // falls through
        }
        case SavingState::SaveActivatedCheckpoints: {
            unsigned int l_size = m_fields->m_activatedCheckpoints.size();
            m_fields->m_stream.write(reinterpret_cast<char*>(&l_size), 4);
            for (int i = 0; i < m_fields->m_activatedCheckpoints.size(); i++) {
                m_fields->m_activatedCheckpoints[i].save(m_fields->m_stream);
            }
            m_fields->m_savingState = SavingState::SaveExtraData;
            // falls through
        }
        case SavingState::SaveExtraData: {
            m_fields->m_stream << m_effectManager->m_persistentItemCountMap;

            m_fields->m_stream << m_effectManager->m_persistentTimerItemSet;

            m_fields->m_stream << m_attempts;

            m_fields->m_savingState = SavingState::Ready;
            // falls through
        }
        case SavingState::Ready: {
            if (m_fields->m_normalModeCheckpoints->count() > 0) {
                m_fields->m_lastSavedCheckpointTimestamp = static_cast<PSCheckpointObject*>(m_fields->m_normalModeCheckpoints->lastObject())->m_fields->m_timestamp;
            }
            m_fields->m_stream.seek(sizeof(s_psfMagicAndVer));
            bool o_finishedSaving = true;
            m_fields->m_stream.write((char*)&o_finishedSaving,sizeof(bool));
            endStream();

            SaveHistoryManager::get().appendEntry(
                m_level,
                m_fields->m_saveSlot,
                m_fields->m_pendingSaveReason,
                static_cast<int>(m_fields->m_normalModeCheckpoints->count())
            );

            showSavingProgressCircleSprite(false);
            showSavingSuccessSprite();

            if (Mod::get()->getSettingValue<bool>("show-save-notifications")) {
                auto const message = m_fields->m_pendingSaveReason == SaveReason::Manual
                    ? "Game saved"
                    : "Checkpoint saved";
                Notification::create(message, NotificationIcon::Success, 1.5f)->show();
            }

            m_fields->m_manualSave = false;

            if (m_fields->m_exitAfterSave) {
                m_fields->m_exitAfterSave = false;
                PauseLayer* l_pauseLayer = static_cast<PauseLayer*>(CCScene::get()->getChildByID("PauseLayer"));
                if (l_pauseLayer) {
                    l_pauseLayer->onQuit(this);
                }
            }
            break;
        }
    }
}

void PSPlayLayer::saveCheckpointToStream(unsigned int i_index) {
#if defined(PS_DEBUG) && defined(PS_DESCRIBE)
    static_cast<PSCheckpointObject*>(m_fields->m_normalModeCheckpoints->objectAtIndex(i_index))->describe();
#endif
    static_cast<PSCheckpointObject*>(m_fields->m_normalModeCheckpoints->objectAtIndex(i_index))->save(m_fields->m_stream);
}
