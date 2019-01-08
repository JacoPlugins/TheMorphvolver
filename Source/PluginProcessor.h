

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
 *** This Plugin is meant to output the result of convolution of the input signal with a given audio sample loaded by the user.
 The aim is not to produce a conventional convolution reverb by convolving input data with an impulse response. Here the "impulse response" can be any audio file.
 In order to do this, TheMorphvolver computes the FFT of both the incoming audio and the loaded audio file (our "impulse response"). Then it multiply their spectra and takes the inverse fourier transform of the multiplied spectra and output the result.
 To achieve this, TheMorphvolver utilises 3 buffers (FIFO):
 
 - m_fftdata[m_fftsize * 2] = {0};  : the buffer to store the incoming signal data to perform the FFT on.
 - m_convfftdata[m_fftsize * 2] = {0};  : the buffer to store the loaded audio data to perform the FFT on.
 - float procChannelData[m_fftsize * 2] = {0}; : the buffer to store the processed (IFFTd) data to send to output.
 
 These buffers has to be twice the length of our actual decided fftsize because I am using the ::performRealOnlyForwardTransform method of the dsp::FFT class (or its respective inverse).
 
*** The Morphvolver also implement a simpler multiplier mode which simply outputs the multiplication of the loaded sound file and the input signal.
 
*/
class TheMorphvolverAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    TheMorphvolverAudioProcessor();
    ~TheMorphvolverAudioProcessor();

	//String procmode = "multiplier";
	
	enum procmode { convolver,
					multiplier
			};
	int mode = procmode::multiplier;//procmode::convolver;
	
	int counter = 0;
	float m_Wet = 0.1;
	int Ncalls = 0;
	
	dsp::FFT m_forwardFFT;
	dsp::FFT m_inverseFFT;
	
	enum
	{
		m_fftorder = 10,
		m_fftsize  = 1 << m_fftorder
	};
	
	AudioBuffer<float> m_convBuffer;
	int m_convBuffersize =  44100 * 2 ;
	int m_convCounter = 0;
	
	//arrays to store data for the fft of the audio stream
	dsp::Complex<float> m_fftdata[m_fftsize ] = {0,0};
	dsp::Complex<float> m_fftdataT[m_fftsize ] = {0,0};
	dsp::Complex<float> m_fifo[m_fftsize] = {0,0};
	bool blockReadyForFFT = false;
	int m_fifoIndex = 0;
	
	//arrays to store data for the fft of the sample to convolve with audio stream
	dsp::Complex<float> m_convfftdata[m_fftsize ] = {0,0};
	dsp::Complex<float> m_convfftdataT[m_fftsize ] = {0,0};
	dsp::Complex<float> m_convfifo[m_fftsize] = {0,0};
	bool convblockReadyForFFT = false;
	int m_convfifoIndex = 0;
	
	dsp::Complex<float> procChannelData[m_fftsize ] = {0,0};
	dsp::Complex<float> procChannelDataT[m_fftsize ] = {0,0};
	
	int flag = 0;
	int first_time_call = 0;
	
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
	int Nprocbl = 0;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

	void pushSampleToFIFO(float sample);
	void pushSampleToConvFIFO();
    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheMorphvolverAudioProcessor)
};
