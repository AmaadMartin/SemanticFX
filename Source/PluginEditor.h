/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 */
class SemanticEQAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Slider::Listener,
                                       public juce::Button::Listener
{
public:
  SemanticEQAudioProcessorEditor(SemanticEQAudioProcessor &);
  ~SemanticEQAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;
  void sliderValueChanged(juce::Slider *slider) override;
  void buttonClicked(juce::Button *button) override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  SemanticEQAudioProcessor &audioProcessor;
  juce::Slider eqInterpolationSlider;
  juce::TextEditor textEditor;
  juce::TextButton generateButton;

  juce::AudioProcessorValueTreeState &valueTreeState;

  juce::Slider mixSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixSliderAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SemanticEQAudioProcessorEditor)
};
