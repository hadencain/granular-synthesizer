#pragma once
#include <array>
#include <atomic>
#include <juce_core/juce_core.h>
#include "../dsp/GranularParams.h"

struct ModSlot
{
    ModSource source = ModSource::LFO1;
    ModTarget target = ModTarget::GrainSize;
    float     depth  = 0.0f;   // [-1, +1]
    bool      active = false;
};

struct ModSources
{
    std::array<float, 4> lfo     {};   // [-1, +1]
    std::array<float, 2> env     {};   // [0, 1]
    float velocity    = 0.0f;
    float aftertouch  = 0.0f;
    float cc1         = 0.0f;
    float envFollower = 0.0f;
    float random      = 0.0f;
};

class ModMatrix
{
public:
    static constexpr int MAX_SLOTS = 64;

    // Message thread: write a slot
    void setSlot(int index, ModSource src, ModTarget tgt, float depth, bool active);
    ModSlot getSlot(int index) const;
    int  getSlotCount() const noexcept { return MAX_SLOTS; }

    // Audio thread: pull pending changes, then evaluate
    void updateSources(const ModSources& sources) noexcept;
    void syncPending() noexcept;    // call at start of each block

    // Returns summed mod value in [-1, +1] for a target
    float evaluate(ModTarget target) const noexcept;

    void reset();

    // Serialization helpers
    juce::ValueTree toValueTree() const;
    void fromValueTree(const juce::ValueTree& tree);

private:
    std::array<ModSlot, MAX_SLOTS> slots;
    std::array<ModSlot, MAX_SLOTS> pendingSlots;
    std::atomic<bool> slotsDirty { false };
    juce::CriticalSection pendingLock;

    std::array<float, static_cast<int>(ModSource::COUNT)> sourceValues {};
};
