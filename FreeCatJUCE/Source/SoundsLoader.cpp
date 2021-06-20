/*
  ==============================================================================

    SoundsLoader.cpp
    Created: 18 Jun 2021 9:58:24pm
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#include "SoundsLoader.h"

void SoundsLoader::load() {
    sounds.clear();
    // set up our AudioFormatManager class to be able to read ogg files
    audioFormatManager.registerFormat(new juce::OggVorbisAudioFormat(), true);
    // allow our sound to be played on all notes
    juce::BigInteger allNotes;
    allNotes.setRange(0, 128, true);

    // for loop to load the audios
    for (int i =0; i< paths.size(); i++)
    {
        //std::cout << "Loading sound " +paths[i]+"\n";
        juce::File* file = new juce::File(paths[i]);
        
        audioFormatReader = audioFormatManager.createReaderFor(*file);
        // the numbers at the end are parameters of SamplerSound that are not relevant in this use case
        juce::SamplerSound* sound = new juce::SamplerSound("defaultName", *audioFormatReader, allNotes, 60, 0.0, 0.0, 10.0);
        sounds.push_back(sound);
    }
    loaded->setVariable(true);
    std::cout << "All sounds loaded!\n";
}
