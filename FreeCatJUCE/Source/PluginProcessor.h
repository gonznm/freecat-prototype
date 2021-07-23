/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SoundsLoader.h"
#include "SharedVars.h"

//==============================================================================
/**
*/
class HelloSamplerAudioProcessor  : public juce::AudioProcessor,
                                    private juce::OSCReceiver, private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>,
                                    public juce::ChangeListener
{
public:
    //==============================================================================
    HelloSamplerAudioProcessor();
    ~HelloSamplerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    
    int receiveUDPport { 9001 };
    bool firstCycle { false };
    
    void oscMessageReceived (const juce::OSCMessage& message) override;
    void showConnectionErrorMessage (const juce::String& messageText);
    
    std::vector<int> string2intVector(juce::String str);
    std::vector<float> string2floatVector(juce::String str);
    
    int getGrainStartSample(int snd_idx);
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    void calculateGrain();

private:
    juce::SharedResourcePointer<SharedVariable_grainSize> grainSize;
    juce::SharedResourcePointer<SharedVariable_loaded> loaded;
    juce::SharedResourcePointer<SharedVariable_loading> loading;

    juce::SharedResourcePointer<SharedVariable_mouseClicked> mouseClicked;
    juce::SharedResourcePointer<SharedVariable_x> x_points;
    juce::SharedResourcePointer<SharedVariable_y> y_points;
    juce::SharedResourcePointer<SharedVariable_closestIndex> closest_sound_index;
    
    SoundsLoader loader;
    int grainSamplePos { 0 };
    int receivedOSCmessages { 0 };
    juce::dsp::WindowingFunction<float> window;
    juce::AudioBuffer<float> grainBuffer;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloSamplerAudioProcessor)
};
