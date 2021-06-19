/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloSamplerAudioProcessor::HelloSamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Start listening to OSC messages
    // specify here on which UDP port number to receive incoming OSC messages
    if (! connect (UDPport))
        this->showConnectionErrorMessage ("Error: could not connect to UDP port " + std::to_string(UDPport));
    OSCReceiver::addListener (this, "/juce");
}

HelloSamplerAudioProcessor::~HelloSamplerAudioProcessor()
{
}

//==============================================================================
const juce::String HelloSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HelloSamplerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HelloSamplerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HelloSamplerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HelloSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HelloSamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HelloSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HelloSamplerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HelloSamplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void HelloSamplerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HelloSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    //synth.setCurrentPlaybackSampleRate(sampleRate);
}

void HelloSamplerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HelloSamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void HelloSamplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    if(mouseClicked)
    {
        // function to get closest sound
        auto* data = loader.sounds[2]->getAudioData();
        // function to get segment
        const float* const inL = data->getReadPointer (0);
        const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;
        
        float* outL = buffer.getWritePointer (0, 0);
        float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1, 0) : nullptr;
        
        int numSamples = buffer.getNumSamples();
        
        // Fill the buffer
        while(--numSamples>=0)
        {
            if (outR != nullptr)
            {
                *outL++ += inL[samplePos];
                *outR++ += inR[samplePos];
            }
            else
            {
                *outL++ += (inL[samplePos] + inR[samplePos]) * 0.5f;
            }
            
            if (samplePos < data->getNumSamples())
            {
                samplePos += 1;
            }
            else
            {
                break;
            }
        }
    }
}

//================================================ ==============================
bool HelloSamplerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HelloSamplerAudioProcessor::createEditor()
{
    return new HelloSamplerAudioProcessorEditor (*this);
}

//==============================================================================
void HelloSamplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HelloSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloSamplerAudioProcessor();
}

//==============================================================================
// My added functions

// OSC receiver: get ids, paths, x, y, loudness
// 1234, "/path/audio", 1.5, 1.2, 45.5
void HelloSamplerAudioProcessor::oscMessageReceived (const juce::OSCMessage& message)
{
    std::cout << "Size: "+std::to_string(message.size())+"\n";
    int i=0;
    while (i<message.size())
    {
        std::cout << "\nSound number "+std::to_string(loader.ids.size()+1)+"\n";
        int ID = message[i].getInt32();
        std::cout << "ID: "+std::to_string(ID)+"\n";
        loader.ids.push_back(ID);
        i++;
        juce::String path = message[i].getString();
        std::cout << "Path: "+path+"\n";
        loader.paths.push_back(path);
        i++;
        float x = message[i].getFloat32();
        std::cout << "Coordinate X: "+std::to_string(x)+"\n";
        loader.x_points.push_back(x);
        i++;
        float y = message[i].getFloat32();
        std::cout << "Coordinate Y: "+std::to_string(y)+"\n";
        loader.y_points.push_back(y);
        i++;
        float l = message[i].getFloat32();
        std::cout << "Loudness: "+std::to_string(l)+"\n";
        loader.loudness.push_back(l);
        i++;
    }
    std::cout << "\nTotal number of sounds: "+std::to_string(loader.ids.size())+"\n\n";

    loader.load();
}

void HelloSamplerAudioProcessor::showConnectionErrorMessage (const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
}
