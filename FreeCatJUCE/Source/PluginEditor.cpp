/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloSamplerAudioProcessorEditor::HelloSamplerAudioProcessorEditor (HelloSamplerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    
    addAndMakeVisible(map);
    juce::Timer::startTimerHz(60);
}

HelloSamplerAudioProcessorEditor::~HelloSamplerAudioProcessorEditor()
{
    juce::Timer::stopTimer(); 
}

//==============================================================================
void HelloSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);

    // You can add your drawing code here!
    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
    g.drawSingleLineText ("Mouse: " +std::to_string(map.mousePos.getX())+", "+ std::to_string(map.mousePos.getY()), 5, getHeight()-5, juce::Justification::left);
}

void HelloSamplerAudioProcessorEditor::timerCallback()
{
    map.repaint();
    repaint();
}

void HelloSamplerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    map.setBounds(getWidth()/2 -200, getHeight()/2 -200, 400, 400);
}
