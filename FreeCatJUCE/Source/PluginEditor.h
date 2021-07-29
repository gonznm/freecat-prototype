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
, public juce::Timer, public juce::ChangeListener
{
public:
    HelloSamplerAudioProcessorEditor (HelloSamplerAudioProcessor& p);
    ~HelloSamplerAudioProcessorEditor() override;
    void timerCallback() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void showConnectionErrorMessage(const juce::String& messageText);
    
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:
    // Title interface one (text)
    juce::Label titleLabel;
    // Title interface two (query by examples)
    juce::Label titleLabelTwo;
    // Sound map
    Map map;
    // Grain size text box
    juce::Label grainSizeLabel;
    juce::TextEditor grainSizeText;
    //Text query text boxes
    juce::Label queryLabel;
    juce::TextEditor queryText;
    // Query by examples text boxes
    juce::Label queryByExample_1;
    juce::TextEditor queryByExampleID_1;
    juce::Label queryByExample_2;
    juce::TextEditor queryByExampleID_2;
    juce::Label queryByExample_3;
    juce::TextEditor queryByExampleID_3;
    juce::Label queryByExample_4;
    juce::TextEditor queryByExampleID_4;
    
    juce::SharedResourcePointer<SharedVariable_grainSize> grainSize;
    juce::SharedResourcePointer<SharedVariable_loading> loading;
    int loadingCounter {0};
    void startLoading();
    
    juce::SharedResourcePointer<SharedVariable_anotherQuery> anotherQuery;
    
    juce::OSCSender sender;
    int sendUDPport { 9002 };
    void sendOSCtext(juce::String query);
    void sendOSCexamples(juce::String one, juce::String two, juce::String three, juce::String four);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HelloSamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloSamplerAudioProcessorEditor)
};


