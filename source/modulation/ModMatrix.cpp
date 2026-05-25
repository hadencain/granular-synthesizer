#include "ModMatrix.h"

void ModMatrix::setSlot(int index, ModSource src, ModTarget tgt,
                         float depth, bool active)
{
    if (index < 0 || index >= MAX_SLOTS) return;
    {
        juce::ScopedLock sl(pendingLock);
        pendingSlots[index] = { src, tgt, depth, active };
    }
    slotsDirty.store(true, std::memory_order_release);
}

ModSlot ModMatrix::getSlot(int index) const
{
    if (index < 0 || index >= MAX_SLOTS) return {};
    juce::ScopedLock sl(pendingLock);
    return pendingSlots[index];
}

void ModMatrix::syncPending() noexcept
{
    bool expected = true;
    if (slotsDirty.compare_exchange_strong(expected, false,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed))
    {
        juce::ScopedTryLock stl(pendingLock);
        if (stl.isLocked())
            slots = pendingSlots;
        else
            slotsDirty.store(true); // retry next block
    }
}

void ModMatrix::updateSources(const ModSources& s) noexcept
{
    sourceValues[static_cast<int>(ModSource::LFO1)]        = s.lfo[0];
    sourceValues[static_cast<int>(ModSource::LFO2)]        = s.lfo[1];
    sourceValues[static_cast<int>(ModSource::LFO3)]        = s.lfo[2];
    sourceValues[static_cast<int>(ModSource::LFO4)]        = s.lfo[3];
    sourceValues[static_cast<int>(ModSource::Env1)]        = s.env[0];
    sourceValues[static_cast<int>(ModSource::Env2)]        = s.env[1];
    sourceValues[static_cast<int>(ModSource::Velocity)]    = s.velocity;
    sourceValues[static_cast<int>(ModSource::Aftertouch)]  = s.aftertouch;
    sourceValues[static_cast<int>(ModSource::CC1)]         = s.cc1;
    sourceValues[static_cast<int>(ModSource::EnvFollower)] = s.envFollower;
    sourceValues[static_cast<int>(ModSource::Random)]      = s.random;
}

float ModMatrix::evaluate(ModTarget target) const noexcept
{
    float sum = 0.0f;
    for (const auto& slot : slots)
    {
        if (slot.active && slot.target == target)
            sum += sourceValues[static_cast<int>(slot.source)] * slot.depth;
    }
    return juce::jlimit(-1.0f, 1.0f, sum);
}

void ModMatrix::reset()
{
    for (auto& s : slots)    s = {};
    for (auto& s : pendingSlots) s = {};
    sourceValues.fill(0.0f);
    slotsDirty.store(false);
}

juce::ValueTree ModMatrix::toValueTree() const
{
    juce::ValueTree tree("ModMatrix");
    juce::ScopedLock sl(pendingLock);
    for (int i = 0; i < MAX_SLOTS; ++i)
    {
        const auto& slot = pendingSlots[i];
        if (!slot.active) continue;
        juce::ValueTree slotTree("Slot");
        slotTree.setProperty("index",  i, nullptr);
        slotTree.setProperty("source", static_cast<int>(slot.source), nullptr);
        slotTree.setProperty("target", static_cast<int>(slot.target), nullptr);
        slotTree.setProperty("depth",  slot.depth, nullptr);
        slotTree.setProperty("active", slot.active, nullptr);
        tree.appendChild(slotTree, nullptr);
    }
    return tree;
}

void ModMatrix::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.isValid()) return;
    {
        juce::ScopedLock sl(pendingLock);
        for (auto& s : pendingSlots) s = {};
        for (auto child : tree)
        {
            const int idx = child.getProperty("index", -1);
            if (idx < 0 || idx >= MAX_SLOTS) continue;
            pendingSlots[idx] = {
                static_cast<ModSource>(static_cast<int>(child.getProperty("source", 0))),
                static_cast<ModTarget>(static_cast<int>(child.getProperty("target", 0))),
                static_cast<float>(child.getProperty("depth", 0.0f)),
                static_cast<bool>(child.getProperty("active", false))
            };
        }
    }
    slotsDirty.store(true);
}
