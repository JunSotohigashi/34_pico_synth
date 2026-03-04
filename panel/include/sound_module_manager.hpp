#pragma once

#include <cstdint>
#include "config.hpp"
#include "sound_unit.hpp"
#include "voice_allocator.hpp"
#include "panel_manager.hpp"

class SoundModuleManager {
private:
    SoundUnit* sound_units_[16];
    VoiceAllocator* voice_allocator_;
    PanelManager* panel_manager_;
    uint16_t serialized_[STREAM_LENGTH];

public:
    SoundModuleManager(VoiceAllocator* va, PanelManager* pm);
    ~SoundModuleManager();

    void update();
    void serialize(uint8_t unit_id, uint16_t* buf);

    VoiceAllocator* get_voice_allocator() { return voice_allocator_; }
};
