/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 */
class ProcessorBase : public juce::AudioProcessor
{
public:
  //==============================================================================
  ProcessorBase()
      : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo()).withOutput("Output", juce::AudioChannelSet::stereo()))
  {
  }

  //==============================================================================
  void prepareToPlay(double, int) override {}
  void releaseResources() override {}
  void processBlock(juce::AudioSampleBuffer &, juce::MidiBuffer &) override {}

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override { return nullptr; }
  bool hasEditor() const override { return false; }

  //==============================================================================
  const juce::String getName() const override { return {}; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  double getTailLengthSeconds() const override { return 0; }

  //==============================================================================
  int getNumPrograms() override { return 0; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String &) override {}

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &) override {}
  void setStateInformation(const void *, int) override {}

private:
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBase)
};

class FilterProcessor : public ProcessorBase
{
public:
  FilterProcessor(std::string type, float frequency, float Q, float gainFactor)
      : type(type), frequency(frequency), Q(Q), gainFactor(gainFactor)
  {
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override
  {
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    
    juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
    if (type == "peakFilter")
    {
      coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(spec.sampleRate, frequency, Q, juce::Decibels::decibelsToGain(gainFactor));
    }
    else if (type == "lowShelfFilter")
    {
      coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(spec.sampleRate, frequency, Q, juce::Decibels::decibelsToGain(gainFactor));
    }
    else if (type == "highShelfFilter")
    {
      coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(spec.sampleRate, frequency, Q, juce::Decibels::decibelsToGain(gainFactor));
    }
    filter.coefficients = coefficients;

    filter.prepare(spec);
  }

  void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &) override
  {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
  }

  void reset() override
  {
    filter.reset();
  }

  const juce::String getName() const override { return "Filter"; }

private:
  juce::dsp::IIR::Filter<float> filter;
  juce::dsp::ProcessSpec spec;
  std::string type;
  float frequency, Q, gainFactor;
};

class ReverbProcessor : public ProcessorBase
{
public:
  ReverbProcessor(float roomSize, float damping, float wetLevel, float width)
      : roomSize(roomSize), damping(damping), wetLevel(wetLevel), width(width)
  {
    reverbParams.roomSize = roomSize;
    reverbParams.damping = damping;
    reverbParams.wetLevel = wetLevel;
    reverbParams.dryLevel = 1 - wetLevel;
    reverbParams.width = width;
    reverb.setParameters(reverbParams);
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override
  {
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    reverb.prepare(spec);
  }

  void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &) override
  {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
  }

  void reset() override
  {
    reverb.reset();
  }

  const juce::String getName() const override { return "Reverb"; }

private:
  juce::dsp::Reverb reverb;
  juce::dsp::Reverb::Parameters reverbParams;
  float roomSize, damping, wetLevel, width;
};

class CompressorProcessor : public ProcessorBase
{
public:
  CompressorProcessor(float threshold, float ratio, float attack, float release)
      : threshold(threshold), ratio(ratio), attack(attack), release(release)
  {
    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setAttack(attack);
    compressor.setRelease(release);
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override
  {
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    compressor.prepare(spec);
  }

  void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &) override
  {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);
  }

  void reset() override
  {
    compressor.reset();
  }

  const juce::String getName() const override { return "Compressor"; }

private:
  juce::dsp::Compressor<float> compressor;
  float threshold, ratio, attack, release;
};

class DelayLineProcessor : public ProcessorBase
{
public:
  DelayLineProcessor(float delayTime, float maximumDelayInSamples)
      : delayTime(delayTime), maximumDelayInSamples(maximumDelayInSamples)
  {
    delayLine.setDelay(delayTime);
    delayLine.setMaximumDelayInSamples(maximumDelayInSamples);
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override
  {
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    delayLine.prepare(spec);
  }

  void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &) override
  {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    delayLine.process(context);
  }

  void reset() override
  {
    delayLine.reset();
  }

  const juce::String getName() const override { return "DelayLine"; }

private:
  juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
  float delayTime, maximumDelayInSamples;
};

class PhaserProcessor : public ProcessorBase
{
public:
  PhaserProcessor(float rate, float depth, float centreFrequency, float feedback, float mix)
      : rate(rate), depth(depth), centreFrequency(centreFrequency), feedback(feedback), mix(mix)
  {
    phaser.setRate(rate);
    phaser.setDepth(depth);
    phaser.setCentreFrequency(centreFrequency);
    phaser.setFeedback(feedback);
    phaser.setMix(mix);
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override
  {
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    phaser.prepare(spec);
  }

  void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &) override
  {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    phaser.process(context);
  }

  void reset() override
  {
    phaser.reset();
  }

  const juce::String getName() const override { return "Phaser"; }

private:
  juce::dsp::Phaser<float> phaser;
  float rate, depth, centreFrequency, feedback, mix;
};

class ChorusProcessor : public ProcessorBase
{
public:
  ChorusProcessor(float rate, float depth, float centreDelay, float feedback, float mix)
      : rate(rate), depth(depth), centreDelay(centreDelay), feedback(feedback), mix(mix)
  {
    chorus.setRate(rate);
    chorus.setDepth(depth);
    chorus.setCentreDelay(centreDelay);
    chorus.setFeedback(feedback);
    chorus.setMix(mix);
  }

  void prepareToPlay(double sampleRate, int samplesPerBlock) override
  {
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    chorus.prepare(spec);
  }

  void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &) override
  {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    chorus.process(context);
  }

  void reset() override
  {
    chorus.reset();
  }

  const juce::String getName() const override { return "Chorus"; }

private:
  juce::dsp::Chorus<float> chorus;
  float rate, depth, centreDelay, feedback, mix;
};

class SemanticEQAudioProcessor : public juce::AudioProcessor
{
public:
  //==============================================================================
  SemanticEQAudioProcessor();
  ~SemanticEQAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //==============================================================================
  void initialiseGraph();

  void connectAudioNodes();

  void connectMidiNodes();
  
  void processText(const juce::String &text);

  void processInOrder(juce::dsp::AudioBlock<float> &block);

private:
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SemanticEQAudioProcessor)
  std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;
  juce::AudioProcessorGraph::Node::Ptr audioInputNode;
  juce::AudioProcessorGraph::Node::Ptr audioOutputNode;
  juce::AudioProcessorGraph::Node::Ptr midiInputNode;
  juce::AudioProcessorGraph::Node::Ptr midiOutputNode;
  juce::dsp::ProcessSpec spec;
};
