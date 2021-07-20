/*
  ==============================================================================

    SoundsLoader.h
    Created: 18 Jun 2021 9:58:24pm
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "SharedVars.h"

class SoundsLoader
{
public:
    void load();
    
    // vector where sounds are loaded
    std::vector<juce::SamplerSound*> sounds;
    // other necessary data of the sounds
    std::vector<int> ids;
    std::vector<juce::String> paths;
    std::vector<float> targetLoudness;
    std::vector<std::vector<float>> loudnessValues;
    
private:
    juce::SharedResourcePointer<SharedVariable_loaded> loaded;
    // manager object that finds an appropriate way to decode various audio files.  Used with SampleSound objects.
    juce::AudioFormatManager audioFormatManager;
    juce::AudioFormatReader* audioFormatReader { nullptr };
};

