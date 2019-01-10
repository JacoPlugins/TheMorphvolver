
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheMorphvolverAudioProcessor::TheMorphvolverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
 , m_forwardFFT(m_fftorder), m_inverseFFT(m_fftorder)
{
}

TheMorphvolverAudioProcessor::~TheMorphvolverAudioProcessor()
{
}

//==============================================================================
const String TheMorphvolverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TheMorphvolverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TheMorphvolverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TheMorphvolverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TheMorphvolverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TheMorphvolverAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TheMorphvolverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TheMorphvolverAudioProcessor::setCurrentProgram (int index)
{
}

const String TheMorphvolverAudioProcessor::getProgramName (int index)
{
    return {};
}

void TheMorphvolverAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void TheMorphvolverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	

}

void TheMorphvolverAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TheMorphvolverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void TheMorphvolverAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	

//define input channels and buffersize
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
	int buffersize = buffer.getNumSamples();
	

	
//check and clean up
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i){
        buffer.clear (i, 0, buffer.getNumSamples());
	}


//push to fifo
	if (1){
		auto* channelData = buffer.getReadPointer (0);
		for (int i = 0; i< buffersize; ++i){
			pushSampleToFIFO(channelData[i]);
			
		}
	}

//if there is a file to convolve loaded, push to convfifo
	if (flag){
		for (int i = 0; i< buffersize; i++){
			pushSampleToConvFIFO();
		}
	}

	//check wether buffersize is constant Logger::writeToLog(String("bs BEFORE fft= ") + String(static_cast<int>(buffersize)));
	//Logger::writeToLog(String("convfifo before assign= ") + String(static_cast<int>(m_convfifoIndex)));

//when the fifos are full, start computing circular convolution
	if (blockReadyForFFT and convblockReadyForFFT){
		
		blockReadyForFFT = false;
		convblockReadyForFFT = false;
		
		if (mode == procmode::convolver){ //perform FFT on one channel and perform circular convolution
			
			m_forwardFFT.perform(m_fftdata, m_fftdataT, false);
			m_forwardFFT.perform(m_convfftdata, m_convfftdataT, false);
			zeromem (procChannelData, sizeof (procChannelData));
			for (int k = 0; k < m_fftsize ; ++k){
				procChannelDataT[k] = m_fftdataT[k] * m_convfftdataT[k];
			}
			m_inverseFFT.perform(procChannelDataT,procChannelData,true);
			
		}// if (mode == convolver)
		
		
	}// if (blocksready)

	//increase the counter for the OUT buffer
	if (counter + buffersize >= m_fftsize ){
		counter = 0;
	}else{
		counter = counter + buffersize;
	}
	
	//push the processed samples to the OUTPUT audio buffer
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData2 = buffer.getWritePointer (channel);
		if (mode == procmode::multiplier){
			for (int a = 0; a < buffersize; a++){
				channelData2[a] = m_convfftdata[a + counter].real() * m_fftdata[a + counter].real();
				}
		}
		if (mode == procmode::convolver){
			for (int a = 0; a < buffersize; a++){
				channelData2[a] = procChannelData[a + counter].real();
			}
		}

		
	}//for channel
	
	

	
	
}//processblock

//========================================================================
void TheMorphvolverAudioProcessor::pushSampleToFIFO(float sample){
	
	if (m_fifoIndex == m_fftsize - 1 )    // if the fft buffer is full
	{
		m_fifo[m_fifoIndex].real( sample );
		
		if (!blockReadyForFFT) //when the fifo is full copy it to the fftData
		{
			zeromem (m_fftdata, sizeof (m_fftdata));
			std::memcpy (m_fftdata, m_fifo, sizeof (m_fifo)/2);
			blockReadyForFFT = true;
			m_fifoIndex = 0;

		}
		

	}else{
		
		m_fifo[m_fifoIndex].real( sample ); //push sample into FFT fifo put fifoindex to zero after filled fifo in ::processBlock
		m_fifoIndex++;
	}
}//pushSampleToFifo

//========================================================================
void TheMorphvolverAudioProcessor::pushSampleToConvFIFO(){

	if (m_convCounter == m_convBuffer.getNumSamples()){ // first check if we exceeded the sample file to convolve with audio stream
		m_convCounter = 0;
	}
	
	if (m_convfifoIndex == m_fftsize - 1 )    // if the fft buffer is full
	{
		m_convfifo[m_convfifoIndex].real( m_convBuffer.getSample(0, m_convCounter ));
		m_convCounter++;
		if (!convblockReadyForFFT) //when the fifo is full copy it to the fftData
		{
			zeromem (m_convfftdata, sizeof (m_convfftdata));
			std::memcpy (m_convfftdata, m_convfifo, sizeof (m_convfifo)/2.);
			convblockReadyForFFT = true;
						m_convfifoIndex = 0;
		}
		
	}else{
		
		m_convfifo[m_convfifoIndex].real(m_convBuffer.getSample(0, m_convCounter));
		m_convfifoIndex++; //put to zero after filled convfifo in ::processBlock
		m_convCounter++;
		
	}//if/elsee

}//pushSampleToConvFifo


//==============================================================================
bool TheMorphvolverAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* TheMorphvolverAudioProcessor::createEditor()
{
    return new TheMorphvolverAudioProcessorEditor (*this);
}

//==============================================================================
void TheMorphvolverAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TheMorphvolverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheMorphvolverAudioProcessor();
}
