/*
  ==============================================================================

    Map.cpp
    Created: 16 Jun 2021 11:43:24am
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#include <JuceHeader.h>
//#include <Math.h>
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
    
    if(loaded->getVariable())
    {
        Xpoints = x_points->getVector(); // ugly but works
        Ypoints = y_points->getVector();
        for (int i = 0; i < Xpoints.size(); i++)
        {
            juce::Rectangle<float> area { Xpoints[i] * (getWidth()-size), Ypoints[i] * (getHeight()-size), size, size };
            //juce::Rectangle<float> area { 0.617253f*getWidth(), 0.178460f*getHeight(), size, size };
            g.setColour(isEntered ? juce::Colours::red : juce::Colours::lightgoldenrodyellow);
            g.fillEllipse(area);
        }
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
        if (loaded->getVariable())
        {
            int candidate = getClosestSound(mousePos);
            if (closest_sound_index->getVariable()!=candidate)
                closest_sound_index->setVariable(candidate);
        }
    }
}

int Map::getClosestSound(juce::Point<int> click)
{
    // Returns the index of the closest sound (euclidean distance)
    float shortest_distance = -1;
    float shortest_distance_index = -1;
    for (int i = 0; i < Xpoints.size(); i++)
    {
        juce::Point<float> point { Xpoints[i]*(getWidth()-size),Ypoints[i]*(getHeight()-size)};
        int distance = computeDistance(click, point);
        if ( (shortest_distance < 0) || (distance < shortest_distance) )
        {
            shortest_distance = distance;
            shortest_distance_index = i;
        }
    }
    return shortest_distance_index;
}

float Map::computeDistance(juce::Point<int> p1, juce::Point<float> p2)
{
    // euclidean distance between ttwo points
    return sqrt( pow(p1.getX()-p2.getX(),2)+pow(p1.getY()-p2.getY(),2) );
}

void Map::mouseDown (const juce::MouseEvent & e)
{
    mouseClicked->setVariable(true);
}

void Map::mouseUp (const juce::MouseEvent & e)
{
    mouseClicked->setVariable(false);
}


void Map::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
