/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Map.h"

//==============================================================================
/**
*/
class HelloSamplerAudioProcessorEditor  : public juce::AudioProcessorEditor
, public juce::Timer
{
public:
    HelloSamplerAudioProcessorEditor (HelloSamplerAudioProcessor& p);
    ~HelloSamplerAudioProcessorEditor() override;
    void timerCallback() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void showConnectionErrorMessage(const juce::String& messageText);

private:
    juce::Label titleLabel;
    juce::Label inputLabel;
    juce::TextEditor inputText;
    Map map;
    
    juce::OSCSender sender;
    int sendUDPport { 9002 };
    void sendOSC(juce::String query);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HelloSamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloSamplerAudioProcessorEditor)
};


