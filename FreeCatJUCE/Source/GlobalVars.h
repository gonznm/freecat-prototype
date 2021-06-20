/*
  ==============================================================================

    GlobalVars.h
    Created: 19 Jun 2021 11:18:12pm
    Author:  Gonzalo Nieto Montero

  ==============================================================================
*/

#pragma once


class SharedVariable_loaded : public juce::ChangeBroadcaster
{
public:
    bool loaded { false };
    
    void setVariable(bool newValue)
    {
        loaded = newValue;
        sendChangeMessage();
    }
    bool getVariable()
    {
        return loaded;
    }
};

class SharedVariable_mouseClicked : public juce::ChangeBroadcaster
{
public:
    bool mouseClicked { false };
    
    void setVariable(bool newValue)
    {
        mouseClicked = newValue;
        sendChangeMessage();
    }
    bool getVariable()
    {
        return mouseClicked;
    }
};

class SharedVariable_x : public juce::ChangeBroadcaster
{
public:
    std::vector<float> x_points;
    
    void clear()
    {
        x_points.clear();
    }
    void append(float newValue)
    {
        x_points.push_back(newValue);
        sendChangeMessage();
    }
    std::vector<float> getVector()
    {
        return x_points;
    }
};

class SharedVariable_y : public juce::ChangeBroadcaster
{
public:
    std::vector<float> y_points;
    
    void clear()
    {
        y_points.clear();
    }
    void append(float newValue)
    {
        y_points.push_back(newValue);
        sendChangeMessage();
    }
    std::vector<float> getVector()
    {
        return y_points;
    }
};

class SharedVariable_closestIndex : public juce::ChangeBroadcaster
{
public:
    int closest_sound_index;
    
    void setVariable(int newValue)
    {
        closest_sound_index = newValue;
        sendChangeMessage();
    }
    int getVariable()
    {
        return closest_sound_index;
    }
};

