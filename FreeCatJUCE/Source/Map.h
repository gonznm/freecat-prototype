/*
  ==============================================================================

    Map.h
    Created: 16 Jun 2021 11:43:24am
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalVars.h"

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
    float computeDistance(juce::Point<int>, juce::Point<float>);
    int getClosestSound(juce::Point<int>);
    
    float size { 10 };
    bool isEntered { false };
    juce::Point<int> mousePos { 0, 0 };

private:
    juce::SharedResourcePointer<SharedVariable_loaded> loaded;

    juce::SharedResourcePointer<SharedVariable_mouseClicked> mouseClicked;
    juce::SharedResourcePointer<SharedVariable_x> x_points;
    juce::SharedResourcePointer<SharedVariable_y> y_points;
    juce::SharedResourcePointer<SharedVariable_closestIndex> closest_sound_index;
    
    std::vector<float> Xpoints;
    std::vector<float> Ypoints;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    //HelloSamplerAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Map)
};
