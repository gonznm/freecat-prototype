/*
  ==============================================================================

    Map.h
    Created: 16 Jun 2021 11:43:24am
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Map  : public juce::Component
{
public:
    Map();
    ~Map() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseEnter (const juce::MouseEvent &event) override;
    void mouseExit (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    
    float size { 10 };
    bool isEntered { false };
    juce::Point<int> mousePos { 0, 0 };
    bool mouseClicked;
    
    // this should come from python (OSC)
    int n_points { 2 };
    float Xpoints[2] = { 0, size };
    float Ypoints[2] = { 0, size };
    
    

private:
    // puntero a clase de procesor
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    //HelloSamplerAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Map)
};
