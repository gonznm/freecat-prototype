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
    setSize (1000, 700);
    
    // Start OSC sender
    if (! sender.connect ("127.0.0.1", sendUDPport))   // [4]
        this->showConnectionErrorMessage("Error: could not connect to UDP port "+ std::to_string(sendUDPport));
    
    // Sound map
    addAndMakeVisible(map);
    
    // Time to repaint map
    juce::Timer::startTimerHz(60);
    
    // Title interface one
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    titleLabel.setText ("Browse sounds in Freesound", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    titleLabel.setJustificationType (juce::Justification::left);
    // Input text query label
    addAndMakeVisible (queryLabel);
    queryLabel.setText ("Text query:", juce::dontSendNotification);
    queryLabel.attachToComponent (&queryText, true);
    queryLabel.setColour (juce::Label::textColourId, juce::Colours::blue);
    queryLabel.setJustificationType (juce::Justification::right);
    // Actual input text query box
    addAndMakeVisible (queryText);
    queryText.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    queryText.onReturnKey = [this] {
        std::cout << "Text query to be sent via OSC: " << queryText.getText() + "\n";
        this->sendOSCtext(queryText.getText());
        //queryLabel.hideEditor(true);
    };
    
    // Title interface two
    addAndMakeVisible (titleLabelTwo);
    titleLabelTwo.setFont (juce::Font (16.0f, juce::Font::bold));
    titleLabelTwo.setText ("Give four sound IDs as examples", juce::dontSendNotification);
    titleLabelTwo.setColour (juce::Label::textColourId, juce::Colours::black);
    titleLabelTwo.setJustificationType (juce::Justification::left);
    // Input text query by example one
    addAndMakeVisible (queryByExample_1);
    queryByExample_1.setText ("Top-left sound example:", juce::dontSendNotification);
    queryByExample_1.attachToComponent (&queryByExampleID_1, true);
    queryByExample_1.setColour (juce::Label::textColourId, juce::Colours::darkgreen);
    queryByExample_1.setJustificationType (juce::Justification::right);
    addAndMakeVisible (queryByExampleID_1);
    queryByExampleID_1.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    queryByExampleID_1.setInputRestrictions(6, "0123456789");
    // Input text query by example two
    addAndMakeVisible (queryByExample_2);
    queryByExample_2.setText ("Top-right sound example:", juce::dontSendNotification);
    queryByExample_2.attachToComponent (&queryByExampleID_2, true);
    queryByExample_2.setColour (juce::Label::textColourId, juce::Colours::darkgreen);
    queryByExample_2.setJustificationType (juce::Justification::right);
    addAndMakeVisible (queryByExampleID_2);
    queryByExampleID_2.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    queryByExampleID_2.setInputRestrictions(6, "0123456789");
    // Input text query by example three
    addAndMakeVisible (queryByExample_3);
    queryByExample_3.setText ("Bottom-left sound example:", juce::dontSendNotification);
    queryByExample_3.attachToComponent (&queryByExampleID_3, true);
    queryByExample_3.setColour (juce::Label::textColourId, juce::Colours::darkgreen);
    queryByExample_3.setJustificationType (juce::Justification::right);
    addAndMakeVisible (queryByExampleID_3);
    queryByExampleID_3.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    queryByExampleID_3.setInputRestrictions(6, "0123456789");
    // Input text query by example four
    addAndMakeVisible (queryByExample_4);
    queryByExample_4.setText ("Bottom-right sound example:", juce::dontSendNotification);
    queryByExample_4.attachToComponent (&queryByExampleID_4, true);
    queryByExample_4.setColour (juce::Label::textColourId, juce::Colours::darkgreen);
    queryByExample_4.setJustificationType (juce::Justification::right);
    addAndMakeVisible (queryByExampleID_4);
    queryByExampleID_4.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    queryByExampleID_4.setInputRestrictions(6, "0123456789");
    queryByExampleID_4.onReturnKey = [this] {
        std::cout << "List of sound IDs to be sent via OSC: " << queryByExampleID_1.getText()+", "+ queryByExampleID_2.getText()+", "+ queryByExampleID_3.getText()+", "+ queryByExampleID_4.getText() + "\n";
        this->sendOSCexamples(queryByExampleID_1.getText(), queryByExampleID_2.getText(), queryByExampleID_3.getText(), queryByExampleID_4.getText());
        //queryLabel.hideEditor(true);
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
    map.setBounds(getWidth()/2, getHeight()/2 - mapHeight/2, mapWidth, mapHeight);
    titleLabel.setBounds(160,  50, getWidth() - 20,  30);
    queryText.setBounds(100, 100, mapWidth-160, 20);
    titleLabelTwo.setBounds(160,  getHeight()/2 - 50, getWidth() - 20,  30);
    queryByExampleID_1.setBounds(200, getHeight()/2, mapWidth-300, 20);
    queryByExampleID_2.setBounds(200, getHeight()/2+21, mapWidth-300, 20);
    queryByExampleID_3.setBounds(200, getHeight()/2+(21*2), mapWidth-300, 20);
    queryByExampleID_4.setBounds(200, getHeight()/2+(21*3), mapWidth-300, 20);
}

void HelloSamplerAudioProcessorEditor::sendOSCtext(juce::String query)
{
    // create and send an OSC message with an address and a float value:
    if (! sender.send("/juce/text", query))
        this->showConnectionErrorMessage("Error: could not send OSC message.");
}

void HelloSamplerAudioProcessorEditor::sendOSCexamples(juce::String one, juce::String two, juce::String three, juce::String four)
{
    // create and send an OSC message with an address and a float value:
    if (! sender.send("/juce/examples", one, two, three, four))
        this->showConnectionErrorMessage("Error: could not send OSC message.");
}

void HelloSamplerAudioProcessorEditor::showConnectionErrorMessage(const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
}
