# pragma once

namespace chowdsp {

/**
 * Base class for plugin processors.
 * 
 * Children must override prepareToPlay
 * and releaseResources (from AudioProcessor),
 * as well as processBlock, and addParameters.
 * */
template<class Processor>
class PluginBase : public juce::AudioProcessor
{
public:
    PluginBase();
    ~PluginBase();

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; } 
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    virtual void processBlock (juce::AudioBuffer<float>&) = 0;

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override { return new foleys::MagicPluginEditor (magicState); }

    void getStateInformation (juce::MemoryBlock& data) override { magicState.getStateInformation (data); }
    void setStateInformation (const void* data, int sizeInBytes) override { magicState.setStateInformation (data, sizeInBytes, getActiveEditor()); }

protected:
    using Parameters = std::vector<std::unique_ptr<juce::RangedAudioParameter>>;
    juce::AudioProcessorValueTreeState vts;
    foleys::MagicProcessorState magicState { *this, vts };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginBase)
};

// check is template class has addParameters
template<typename T>
class HasAddParameters
{
    typedef char one;
    typedef long two;

    template <typename C> static one test (decltype(&C::addParameters));
    template <typename C> static two test (...);

public:
    enum { value = sizeof (test<T> (0)) == sizeof (char) };
};

template<class Processor>
PluginBase<Processor>::PluginBase() :
    AudioProcessor (BusesProperties()
        .withInput ("Input", AudioChannelSet::stereo(), true)
        .withOutput ("Output", AudioChannelSet::stereo(), true)),
    vts (*this, nullptr, Identifier ("Parameters"), createParameterLayout())
{
}

template<class Processor>
PluginBase<Processor>::~PluginBase() {}

template<class Processor>
juce::AudioProcessorValueTreeState::ParameterLayout PluginBase<Processor>::createParameterLayout()
{
    Parameters params;

    static_assert (HasAddParameters<Processor>::value,
        "Processor class MUST contain a static addParameters function!");
    Processor::addParameters (params);

    return { params.begin(), params.end() };
}

template<class Processor>
bool PluginBase<Processor>::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
{
    // only supports mono and stereo (for now)
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // input and output layout must be the same
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    
    return true;
}

template<class Processor>
void PluginBase<Processor>::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}

template<class Processor>
void PluginBase<Processor>::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    processBlock (buffer);
}

} // chowdsp
