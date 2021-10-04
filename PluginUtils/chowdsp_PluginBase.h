#pragma once

namespace chowdsp
{
/**
 * Base class for plugin processors.
 * 
 * Derived classes must override `prepareToPlay` and `releaseResources`
 * (from `juce::AudioProcessor`), as well as `processAudioBlock`, and
 * `addParameters`.
*/
template <class Processor>
class PluginBase : public juce::AudioProcessor
{
public:
    PluginBase();
    PluginBase (juce::UndoManager* um);
    PluginBase (const juce::AudioProcessor::BusesProperties& layout);
    ~PluginBase();

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}
    virtual PresetManager& getPresetManager() { return *presetManager; }

    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override {}
    virtual void processAudioBlock (juce::AudioBuffer<float>&) = 0;

    bool hasEditor() const override { return true; }
#if CHOWDSP_USE_FOLEYS_CLASSES
    juce::AudioProcessorEditor* createEditor() override
    {
        return new foleys::MagicPluginEditor (magicState);
    }
#endif

    void getStateInformation (juce::MemoryBlock& data) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

protected:
    using Parameters = std::vector<std::unique_ptr<juce::RangedAudioParameter>>;
    juce::AudioProcessorValueTreeState vts;

#if CHOWDSP_USE_FOLEYS_CLASSES
    foleys::MagicProcessorState magicState { *this, vts };
#endif

    std::unique_ptr<chowdsp::PresetManager> presetManager;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginBase)
};

/** Class that uses SFINAE to ensure that the
 *  processor class has an `addParameters` function
*/
template <typename T>
class HasAddParameters
{
    typedef char one;
    typedef long two;

    template <typename C>
    static one test (decltype (&C::addParameters));
    template <typename C>
    static two test (...);

public:
    enum
    {
        value = sizeof (test<T> (nullptr)) == sizeof (char)
    };
};

template <class Processor>
PluginBase<Processor>::PluginBase() : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true).withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
                                      vts (*this, nullptr, juce::Identifier ("Parameters"), createParameterLayout())
{
}

template <class Processor>
PluginBase<Processor>::PluginBase (juce::UndoManager* um) : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true).withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
                                                            vts (*this, um, juce::Identifier ("Parameters"), createParameterLayout())
{
}

template <class Processor>
PluginBase<Processor>::PluginBase (const juce::AudioProcessor::BusesProperties& layout) : AudioProcessor (layout),
                                                                                          vts (*this, nullptr, juce::Identifier ("Parameters"), createParameterLayout())
{
}

template <class Processor>
PluginBase<Processor>::~PluginBase()
{
}

template <class Processor>
juce::AudioProcessorValueTreeState::ParameterLayout PluginBase<Processor>::createParameterLayout()
{
    Parameters params;

    static_assert (HasAddParameters<Processor>::value,
                   "Processor class MUST contain a static addParameters function!");
    Processor::addParameters (params);

    return { params.begin(), params.end() };
}

template <class Processor>
int PluginBase<Processor>::getNumPrograms()
{
    if (presetManager == nullptr)
        return 1;

    return presetManager->getNumPresets();
}

template <class Processor>
int PluginBase<Processor>::getCurrentProgram()
{
    if (presetManager == nullptr)
        return 0;

    return presetManager->getCurrentPresetIndex();
}

template <class Processor>
void PluginBase<Processor>::setCurrentProgram (int index)
{
    if (presetManager == nullptr)
        return;

    presetManager->loadPresetFromIndex (index);
}

template <class Processor>
const juce::String PluginBase<Processor>::getProgramName (int index)
{
    if (presetManager == nullptr)
        return {};

    return presetManager->getPresetName (index);
}

template <class Processor>
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

template <class Processor>
void PluginBase<Processor>::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
#if CHOWDSP_USE_FOLEYS_CLASSES
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
#endif
}

template <class Processor>
void PluginBase<Processor>::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    processAudioBlock (buffer);
}

template <class Processor>
void PluginBase<Processor>::getStateInformation (juce::MemoryBlock& data)
{
#if CHOWDSP_USE_FOLEYS_CLASSES
    magicState.getStateInformation (data);
#else
    auto state = vts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, data);
#endif
}

template <class Processor>
void PluginBase<Processor>::setStateInformation (const void* data, int sizeInBytes)
{
#if CHOWDSP_USE_FOLEYS_CLASSES
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());
#else
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (vts.state.getType()))
            vts.replaceState (juce::ValueTree::fromXml (*xmlState));
#endif
}

} // namespace chowdsp
