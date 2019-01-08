/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheMorphvolverAudioProcessorEditor::TheMorphvolverAudioProcessorEditor (TheMorphvolverAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	addAndMakeVisible (openButton);
	openButton.setButtonText ("Open");
	openButton.onClick = [this] { openButtonClicked(); };
	
	
	addAndMakeVisible (modeButton);
	modeButton.setButtonText ("Convolver");
	modeButton.onClick = [this] { modeButtonClicked(); };
	setSize (500, 300);
}

TheMorphvolverAudioProcessorEditor::~TheMorphvolverAudioProcessorEditor()
{
}

//==============================================================================

void TheMorphvolverAudioProcessorEditor::openButtonClicked()
{
	if (processor.flag == 0) {	formatManager.registerBasicFormats(); }
	FileChooser chooser ("Select a file to convolve...", {});
	
	if (chooser.browseForFileToOpen())
	{
		processor.m_convCounter = 0;
		File soundFile (chooser.getResult());
		std::unique_ptr<AudioFormatReader> reader (formatManager.createReaderFor (soundFile));
		if (reader.get() != nullptr)
		{
			
			processor.m_convBuffer.setSize (reader->numChannels, processor.m_convBuffersize);// (int)  reader->lengthInSamples);
			reader->read (&processor.m_convBuffer,
						  0,
						  processor.m_convBuffersize,//(int)  reader->lengthInSamples,
						  44100,
						  true,
						  true);
			
			processor.flag =1;
			processor.m_fifoIndex = 0; //when a file is loaded, sets the fifo counter to zero to synch with the convfifo.
			processor.m_convfifoIndex = 0; //when a file is loaded, sets the fifo counter to zero to synch with the convfifo.
			processor.blockReadyForFFT = false;
			processor.convblockReadyForFFT = false;
			processor.counter = 0;
			
		}
	}
}//openButtonClicked

void TheMorphvolverAudioProcessorEditor::modeButtonClicked(){
	
	if (processor.mode == processor.procmode::convolver){
		processor.mode = processor.procmode::multiplier;
		modeButton.setButtonText ("Multiplier");
	}
	else{
		processor.mode = processor.procmode::convolver;
		modeButton.setButtonText ("Convolver");
	}
	
	
}

void TheMorphvolverAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour (Colours::white);
    g.setFont (15.0f);
	
}

void TheMorphvolverAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	openButton.setBounds(Rectangle<int>(Point<int>(50,50),Point<int>(150,150)));
	modeButton.setBounds(Rectangle<int>(Point<int>(300,50),Point<int>(400,150)));
}
