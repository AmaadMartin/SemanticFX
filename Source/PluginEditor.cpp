/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SemanticEQAudioProcessorEditor::SemanticEQAudioProcessorEditor(SemanticEQAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), valueTreeState(p.parameters)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
    // eqInterpolationSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    // eqInterpolationSlider.setRange(-1.0, 1.0, 0.01);
    // eqInterpolationSlider.setValue(0, juce::dontSendNotification);
    // eqInterpolationSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    // eqInterpolationSlider.addListener(this);
    // addAndMakeVisible(eqInterpolationSlider);

    //Initialize and configure chat
    chat.setMultiLine(true);
    chat.setReadOnly(true);
    chat.setScrollbarsShown(true);
    chat.setCaretVisible(false);
    chat.setPopupMenuEnabled(true);
    addAndMakeVisible(chat);

    // Initialize and configure text editor
    textEditor.setMultiLine(false);
    textEditor.setReturnKeyStartsNewLine(false);
    textEditor.setReadOnly(false);
    textEditor.setScrollbarsShown(true);
    textEditor.setCaretVisible(true);
    textEditor.setPopupMenuEnabled(true);
    textEditor.setText("");
    addAndMakeVisible(textEditor);

    // Initialize and configure generate button
    generateButton.setButtonText("Generate");
    generateButton.addListener(this);
    addAndMakeVisible(generateButton);

    mixSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);

    // Add the slider to the editor
    addAndMakeVisible(mixSlider);
    mixSliderAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "mix", mixSlider));

}

SemanticEQAudioProcessorEditor::~SemanticEQAudioProcessorEditor()
{
}

//==============================================================================
void SemanticEQAudioProcessorEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void SemanticEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto area = getLocalBounds();
    const int buttonHeight = 30;  // Height for buttons and text editor
    const int sliderHeight = 40;  // Height for sliders

    // 1. Message Display (Chat) - Half of the height of the plugin window
    chat.setBounds(area.removeFromTop(area.proportionOfHeight(0.5)));

    // 2. Text Input - Below the chat
    textEditor.setBounds(area.removeFromTop(buttonHeight));

    // 3. Generate Button - Below the text input
    generateButton.setBounds(area.removeFromTop(buttonHeight));

    // 4. Mix Slider - Fill the remaining area
    mixSlider.setBounds(area);

    // If you have more components, adjust their bounds similarly
}

void SemanticEQAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &eqInterpolationSlider)
    {
        // audioProcessor.setInterpolation(eqInterpolationSlider.getValue());
    }
}

void SemanticEQAudioProcessorEditor::setMessageContent(const juce::String& newText) {
    chat.setText(newText, juce::dontSendNotification);
}

void SemanticEQAudioProcessorEditor::buttonClicked(juce::Button *button)
{
    if (button == &generateButton)
    {
        auto text = textEditor.getText();
        audioProcessor.processText(text);
    }
}
