/*
  ==============================================================================

    Map.cpp
    Created: 16 Jun 2021 11:43:24am
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Map.h"

//==============================================================================
Map::Map()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

Map::~Map()
{
}

void Map::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    g.fillAll(juce::Colours::black);
    
    for (int i = 0; i < n_points; i++)
    {
        juce::Rectangle<float> area { Xpoints[i], Ypoints[i], size, size };
        g.setColour(isEntered ? juce::Colours::red : juce::Colours::lightgoldenrodyellow);
        g.fillEllipse(area);
    }
}

void Map::mouseEnter (const juce::MouseEvent &event)
{
    isEntered = true;
}

void Map::mouseExit (const juce::MouseEvent &event)
{
    isEntered = false;
}

void Map::mouseDrag (const juce::MouseEvent & e)
{
    int x = e.getPosition().getX();
    int y = e.getPosition().getY();
    
    if (x>=0 && x<=getWidth() && y>=0 && y<=getHeight())
    {
        mousePos = e.getPosition();
        // actualizar aquí las variables del pluginprocessor
    }
}

void Map::mouseDown (const juce::MouseEvent & e)
{
    mouseClicked = true;
}

void Map::mouseUp (const juce::MouseEvent & e)
{
    mouseClicked = false;
}


void Map::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
