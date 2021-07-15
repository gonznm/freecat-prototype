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
    // Set the editor's size
    setSize (800, 800);
    
    // Start OSC sender
    if (! sender.connect ("127.0.0.1", sendUDPport))   // [4]
        this->showConnectionErrorMessage("Error: could not connect to UDP port "+ std::to_string(sendUDPport));
    
    // Sound map
    addAndMakeVisible(map);
    juce::Timer::startTimerHz(60);
    // Title
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    titleLabel.setText ("FreeCat", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    titleLabel.setJustificationType (juce::Justification::centred);
    // Input text label
    addAndMakeVisible (inputLabel);
    inputLabel.setText ("Text query:", juce::dontSendNotification);
    inputLabel.attachToComponent (&inputText, true);
    inputLabel.setColour (juce::Label::textColourId, juce::Colours::blue);
    inputLabel.setJustificationType (juce::Justification::right);
    // Actual input text box
    addAndMakeVisible (inputText);
    inputText.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    inputText.onReturnKey = [this] {
        std::cout << inputText.getText() + "\n";
        this->sendOSC(inputText.getText());
        //inputLabel.hideEditor(true);
    };

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
    //g.setColour (juce::Colours::black);
    //g.setFont (15.0f);
    //g.drawSingleLineText ("Mouse: " +std::to_string(map.mousePos.getX())+", "+ std::to_string(map.mousePos.getY()), 5, getHeight()-5, juce::Justification::left);
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
    int mapWidth = 600;
    int mapHeight = 600;
    map.setBounds(getWidth()/2 - mapWidth/2, getHeight()/2 - mapHeight/2, 600, 600);
    titleLabel.setBounds(10,  10, getWidth() - 20,  30);
    inputText.setBounds(getWidth()/2 - mapWidth/2 + 80, 50, mapWidth-80, 20);
}

void HelloSamplerAudioProcessorEditor::sendOSC(juce::String query)
{
    // create and send an OSC message with an address and a float value:
    if (! sender.send("/juce", query))
        this->showConnectionErrorMessage("Error: could not send OSC message.");
}

void HelloSamplerAudioProcessorEditor::showConnectionErrorMessage(const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
}
