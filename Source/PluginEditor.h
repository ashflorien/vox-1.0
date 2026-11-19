#pragma once
#include "PluginProcessor.h"

class VoxOneAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit VoxOneAudioProcessorEditor(VoxOneAudioProcessor&);
    ~VoxOneAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    class HardwareLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        juce::Colour accent { 0xffe23d37 }, ink { 0xfff2eee8 }, panel { 0xff25262a };
        void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float,
                              const juce::Slider::SliderStyle, juce::Slider&) override;
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    } look;
    void applyTheme();
    void timerCallback() override;
    VoxOneAudioProcessor& processor;
    juce::Slider gate, comp, gain, saturation;
    juce::ComboBox skin, satType, meterMode;
    juce::TextButton power { "" };
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SA> gateAttachment, compAttachment, gainAttachment, satAttachment;
    std::unique_ptr<CA> skinAttachment, typeAttachment;
    int currentSkin = -1;
    float displayedMeter = -60.f;
    float displayedReduction = 0.f;
    juce::Colour bg { 0xff202125 }, panel { 0xff303136 }, ink { 0xfff2eee8 }, muted { 0xffbbb9b7 }, accent { 0xffe23d37 };
    // Design coordinates are in a 1200 x 760 logical grid; children use the same transform.
    juce::Rectangle<float> viewport() const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxOneAudioProcessorEditor)
};
