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
                       ),
#endif
window (grainSize->getVariable(), juce::dsp::WindowingFunction<float>::hann) // hamming window used to window grain in the time domain
{
    // Start listening to OSC messages
    // specify here on which UDP port number to receive incoming OSC messages
    if (! connect (receiveUDPport))
        this->showConnectionErrorMessage ("Error: could not connect to UDP port " + std::to_string(receiveUDPport));
    OSCReceiver::addListener (this, "/juce");
    // Listen to closest_sound_index variable changes (a callback function is )
    closest_sound_index->addChangeListener(this);
    // Initialize buffer where grains are stored
    grainBuffer.setSize(1, grainSize->getVariable());
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
    
    if(mouseClicked->getVariable() && loaded->getVariable())
    {
        // Fill grain buffer (only at the very first block this should be done at the beginning)
        if (firstCycle)
        {
            this->calculateGrain();
            firstCycle=false;
        }
        // Write grain to the output buffer
        const float* grainReader = grainBuffer.getReadPointer(0); // grainBuffer gets updated calling calculateGrain()
        // Fill one channel buffer, which will be copied to the others (mono)
        float* writePointer = buffer.getWritePointer(0);
        for (int sampleCount=0; sampleCount<buffer.getNumSamples(); sampleCount++)
        {
            writePointer[sampleCount] = grainReader[grainSamplePos];
            
            /* "Debugging"
            if (sampleCount==0)
            {
                std::cout << "Starting buffer... (buffer position " << sampleCount << ", value " << writePointer[sampleCount] << ", grain position " << grainSamplePos << ")\n";
            }
            if (grainSamplePos==0)
            {
                std::cout << "\nGrain first sample: " << grainReader[grainSamplePos] << " (position " << grainSamplePos << ")\n";
                std::cout << "Output (first): " << writePointer[sampleCount] << " (position " << sampleCount << ")\n";
            }
            if (grainSamplePos==grainBuffer.getNumSamples())
            {
                std::cout << "Grain last sample: " << grainReader[grainSamplePos] << " (position " << grainSamplePos << ")\n";
                std::cout << "Output (last): " << writePointer[sampleCount] << " (position " << sampleCount << ")\n";
            }
            */
            
            if (grainSamplePos < grainBuffer.getNumSamples())
            {
                grainSamplePos += 1;
            }
            else
            {
                // choose a new grain because one has just finished
                grainSamplePos = 0;
                this->calculateGrain();
            }
        }
        // Copy this written buffer to the other channels (mono)
        if (buffer.getNumChannels() > 1)
        {
            for (int channel=1; channel <buffer.getNumChannels(); channel++)
            {
                buffer.copyFrom(channel, 0, buffer, 0, 0, buffer.getNumSamples());
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

// OSC receiver: ids, paths, x, y, targetLoudness, startSamples, loudnessValues
// Example –> [1234, "/path/audio", 1.5, 1.2, 45.5, "0 4410 8820", "48.0 54.2 44.1"]
void HelloSamplerAudioProcessor::oscMessageReceived (const juce::OSCMessage& message)
{
    // clear all vectors when the message has finished
    if (message.size()==1)
    {
        if (message[0].getString().compare("Start")==0)
        {
            std::cout << "\nStart message received: "+message[0].getString()+"\n";
            
            receivedOSCmessages = 0;
            loader.ids.clear();
            loader.paths.clear();
            x_points->clear();
            y_points->clear();
            loader.targetLoudness.clear();
            loader.startSamples.clear();
            loader.loudnessValues.clear();
            std::cout << "All vectors used to store temporary info of sounds cleared.";
        }
        if (message[0].getString().compare("Finished")==0)
        {
            std::cout << "\nFinished message received: "+message[0].getString()+"\n";
            std::cout << "Number of messages received: "+ std::to_string(receivedOSCmessages) +"\n";
            std::cout << "Number of sounds received: "+std::to_string(x_points->getVector().size())+"\n\n";
            loader.load();
        }
    }
    else
    {
        std::cout << "\n* Number of arguments of the OSC message: "+std::to_string(message.size())+"\n";
        receivedOSCmessages +=1;
        // decode message based on the order of the arguments
        int i=0;
        while (i<message.size())
        {
            int ID = message[i].getInt32();
            loader.ids.push_back(ID);
            i++;
            juce::String path = message[i].getString();
            loader.paths.push_back(path);
            i++;
            float x = message[i].getFloat32();
            x_points->append(x);
            i++;
            float y = message[i].getFloat32();
            y_points->append(y);
            i++;
            float l = message[i].getFloat32();
            loader.targetLoudness.push_back(l);
            i++;
            // grains start samples come as a string of integers separated by spaces, this needs to be splitted
            juce::String startSamples_str = message[i].getString();
            loader.startSamples.push_back(this->string2intVector(startSamples_str));
            i++;
            // grains loudness values come as a string of integers separated by spaces, this needs to be splitted
            juce::String loudnessValues_str = message[i].getString();
            loader.loudnessValues.push_back(this->string2floatVector(loudnessValues_str));
            i++;
            
            // Print info in console
            std::cout << "\nSound number "+std::to_string(loader.ids.size())+"\n";
            std::cout << "ID: "+std::to_string(ID)+"\n";
            std::cout << "Path: "+path+"\n";
            std::cout << "Coordinate X: "+std::to_string(x)+"\n";
            std::cout << "Coordinate Y: "+std::to_string(y)+"\n";
            std::cout << "Loudness: "+std::to_string(l)+"\n";
            std::cout << "Grains start samples: "+startSamples_str+"\n";
            std::cout << "Grains loudness values: "+loudnessValues_str+"\n";
        }
    }
}

void HelloSamplerAudioProcessor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    // Callback that gets triggered when the mouse moves to a new sound (the closest_sound_index changes)
    // For "debugging":
    //std::cout << "\nSound index has changed ("<< closest_sound_index->getVariable() <<")\n\n";
    // This may be glitchy and not necessary
    //this->calculateGrain();
}

void HelloSamplerAudioProcessor::calculateGrain()
{
    int startSample = this->getGrainStartSample(closest_sound_index->getVariable());
    // get audio data from the closest sound to click
    auto* data = loader.sounds[closest_sound_index->getVariable()]->getAudioData();
    const float* const inL = data->getReadPointer (0);
    const float* const inR = data->getNumChannels() > 1 ? data->getReadPointer (1) : nullptr;
    
    // clean the buffer where the grain is going to be loaded
    grainBuffer.clear();
    float* grainWriter = grainBuffer.getWritePointer (0, 0); // why doesn't this work? -> float* in = 0;
    //std::cout << "Chosen start sample: " << startSample << "(sound index:" << closest_sound_index->getVariable() << ")\n";
    // Fill auxiliary buffer with the data of the grain
    for (int sampleCount=0; sampleCount<grainBuffer.getNumSamples(); sampleCount++)
    {
        if (inR != nullptr)
        {
            grainWriter[sampleCount] = (inL[startSample+sampleCount] + inR[startSample+sampleCount]) * 0.5f;
        }
        else
        {
            grainWriter[sampleCount] = inL[startSample+sampleCount];
        }
    }
    // Window grain with a hamming window
    //std::cout << "First grain sample pre-windowing: " << grainWriter[0] << "\n";
    window.multiplyWithWindowingTable (grainWriter, grainSize->getVariable());
    //std::cout << "First grain sample post-windowing: " << grainWriter[0] << "\n";
}

int HelloSamplerAudioProcessor::getGrainStartSample(int snd_idx)
{
    int startSample = 0;
    // get loudness indexes that are inside a certain margin (+-6 dB) around the target loudness
    std::vector<float> candidates;
    for(int i=0; i<loader.loudnessValues[snd_idx].size(); i++)
    {
        if ( loader.loudnessValues[snd_idx][i] > loader.targetLoudness[snd_idx]-6 && loader.loudnessValues[snd_idx][i] < loader.targetLoudness[snd_idx]+6 )
        {
            // use these indexes to get the start samples of these grains
            candidates.push_back(loader.startSamples[snd_idx][i]);
        }
        // if there are no values, just take one that is >(targetLoudness-12)
        else if (loader.loudnessValues[snd_idx][i] > loader.targetLoudness[snd_idx]-12)
        {
            candidates.push_back(loader.startSamples[snd_idx][i]);
        }
    }
    // make a random choice between these ones
    int random_idx = rand() % candidates.size() + 0;
    startSample  = candidates[random_idx];

    return startSample;
}

//=============================================================================
// Util functions
std::vector<int> HelloSamplerAudioProcessor::string2intVector(juce::String str)
{
    juce::StringArray str_split;
    str_split.addTokens(str, " ", "\"");
    std::vector<int> output;
    for (int i=0; i<str_split.size(); i++)
    {
        output.push_back(str_split[i].getIntValue());
    }
    return output;
}

std::vector<float> HelloSamplerAudioProcessor::string2floatVector(juce::String str)
{
    juce::StringArray str_split;
    str_split.addTokens(str, " ", "\"");
    std::vector<float> output;
    for (int i=0; i<str_split.size(); i++)
    {
        output.push_back(str_split[i].getFloatValue());
    }
    return output;
}

void HelloSamplerAudioProcessor::showConnectionErrorMessage (const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
}
