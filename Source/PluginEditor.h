/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TheMorphvolverAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    TheMorphvolverAudioProcessorEditor (TheMorphvolverAudioProcessor&);
    ~TheMorphvolverAudioProcessorEditor();

	AudioFormatManager formatManager;
	
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

	void openButtonClicked();
	void modeButtonClicked();
	
private:
	
	TextButton openButton;
	ToggleButton modeButton;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TheMorphvolverAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMorphvolverAudioProcessorEditor)
};
