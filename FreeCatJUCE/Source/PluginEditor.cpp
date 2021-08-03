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
    setSize (1250, 700);
    
    // Start OSC sender
    if (! sender.connect ("127.0.0.1", sendUDPport))   // [4]
        this->showConnectionErrorMessage("Error: could not connect to UDP port "+ std::to_string(sendUDPport));
    
    // Sound map
    addAndMakeVisible(map);
    
    // Time to repaint map
    juce::Timer::startTimerHz(60);
    
    // Listen to loading variable changes (a callback function is triggered)
    loading->addChangeListener(this);
    
    // Grain size
    addAndMakeVisible (grainSizeLabel);
    grainSizeLabel.setText ("Grain size:", juce::dontSendNotification);
    grainSizeLabel.attachToComponent (&grainSizeText, true);
    grainSizeLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    grainSizeLabel.setJustificationType (juce::Justification::right);
    addAndMakeVisible (grainSizeText);
    grainSizeText.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
    grainSizeText.setInputRestrictions(6, "0123456789");
    grainSizeText.setText(std::to_string(grainSize->getVariable()));
    grainSizeText.onReturnKey = [this] {
        // Send OSC messages
        grainSize->setVariable(grainSizeText.getText().getIntValue());
        std::cout << "\nGrain size to be sent via OSC: " << grainSize->getVariable() << "\n";
        if (! sender.send("/juce/grain", grainSize->getVariable()))
            this->showConnectionErrorMessage("Error: could not send OSC message.");
    };
    
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
        std::cout << "\nText query to be sent via OSC: " << queryText.getText() + "\n";
        this->sendOSCtext(queryText.getText());
        this->startLoading();
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
        // Send OSC messages
        std::cout << "\nList of sound IDs to be sent via OSC: " << queryByExampleID_1.getText()+", "+ queryByExampleID_2.getText()+", "+ queryByExampleID_3.getText()+", "+ queryByExampleID_4.getText() + "\n";
        this->sendOSCexamples(queryByExampleID_1.getText(), queryByExampleID_2.getText(), queryByExampleID_3.getText(), queryByExampleID_4.getText());
        this->startLoading();
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

    // Show loading message
    if (loading->getVariable())
    {
        g.setColour (juce::Colours::black);
        g.setFont (40.0f);
        // Animate loading text to avoid the impression of a frozen screen
        if (juce::Time::getApproximateMillisecondCounter() % 100 == 0)
        {
            loadingCounter++;
        }
            
        if (loadingCounter==0)
            g.drawSingleLineText ("Loading, please wait...", 80, getHeight()-100);
        else if (loadingCounter==1)
            g.drawSingleLineText ("Loading, please wait.", 80, getHeight()-100);
        else if (loadingCounter==2)
            g.drawSingleLineText ("Loading, please wait..", 80, getHeight()-100);
        
        // reset loading counter
        if (loadingCounter>1)
            loadingCounter = 0;
    }
    
    // Ask for another query when it is too specific
    if (anotherQuery->getVariable() && !loading->getVariable())
    {
        juce::uint32 now = juce::Time::getMillisecondCounter();
        if ((now-anotherQuery->getStartTime()) < 7000) // show message for seven seconds
        {
            g.setColour (juce::Colours::black);
            g.setFont (20.0f);
            g.drawMultiLineText ("Your text query returned too few results. Try another query.", 70, getHeight()-100, getWidth()/2-150);
        }
        else
        {
            anotherQuery->setVariable(false);
        }
    }
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
    grainSizeText.setBounds(100, 60, mapWidth-150, 20);
    titleLabel.setBounds(160,  150, getWidth() - 20,  30);
    queryText.setBounds(100, 200, mapWidth-160, 20);
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

void HelloSamplerAudioProcessorEditor::startLoading()
{
    // Blocks text editors and show loading message
    grainSizeText.setReadOnly(true);
    queryText.setReadOnly(true);
    queryByExampleID_1.setReadOnly(true);
    queryByExampleID_2.setReadOnly(true);
    queryByExampleID_3.setReadOnly(true);
    queryByExampleID_4.setReadOnly(true);
    loading->setVariable(true);
}

void HelloSamplerAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    // Callback that gets triggered when the loading boolean turns to false
    //std::cout << "Loading boolean has changed to false: " << loading->getVariable() << "\n";
    grainSizeText.setReadOnly(false);
    queryText.setReadOnly(false);
    queryByExampleID_1.setReadOnly(false);
    queryByExampleID_2.setReadOnly(false);
    queryByExampleID_3.setReadOnly(false);
    queryByExampleID_4.setReadOnly(false);
}

void HelloSamplerAudioProcessorEditor::showConnectionErrorMessage(const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
}
