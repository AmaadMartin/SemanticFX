/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SemanticEQAudioProcessor::SemanticEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
#endif
      mainProcessor(new juce::AudioProcessorGraph()) ,
      parameters(*this, nullptr, juce::Identifier("PARAMETERS"),
                 {
                     std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f)
                    })
{
}

SemanticEQAudioProcessor::~SemanticEQAudioProcessor()
{
}

//==============================================================================
const juce::String SemanticEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SemanticEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SemanticEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SemanticEQAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SemanticEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SemanticEQAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int SemanticEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SemanticEQAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SemanticEQAudioProcessor::getProgramName(int index)
{
    return {};
}

void SemanticEQAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void SemanticEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    mainProcessor->setPlayConfigDetails(getMainBusNumInputChannels(),
                                        getMainBusNumOutputChannels(),
                                        sampleRate, samplesPerBlock);

    mainProcessor->prepareToPlay(sampleRate, samplesPerBlock);
    initialiseGraph();

    mixer.prepare(spec);
}

// Function to make a POST request to the create-conversation API
std::string SemanticEQAudioProcessor::createConversation() {
    std::string url = "http://localhost:5000/create-conversation";
    std::string command = "curl -X POST " + url;

    // Execute the command and get the response
    std::string response = "";

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe)
    {
        char buffer[128];
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                response += buffer;
        }
        pclose(pipe);
    }

    return response;
}

void SemanticEQAudioProcessor::initialiseGraph()
{
    mainProcessor->clear();

    audioInputNode = mainProcessor->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
    audioOutputNode = mainProcessor->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
    midiInputNode = mainProcessor->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode));
    midiOutputNode = mainProcessor->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode));

    connectAudioNodes();
    connectMidiNodes();
}

void SemanticEQAudioProcessor::connectAudioNodes()
{
    for (int channel = 0; channel < 2; ++channel)
        mainProcessor->addConnection({{audioInputNode->nodeID, channel},
                                      {audioOutputNode->nodeID, channel}});
}

void SemanticEQAudioProcessor::connectMidiNodes()
{
    mainProcessor->addConnection({{midiInputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex},
                                  {midiOutputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex}});
}

void SemanticEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SemanticEQAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

std::string SemanticEQAudioProcessor::getCurrentState(){
    return "";
}

void SemanticEQAudioProcessor::processText(const juce::String &text)
{
    if(threadId == ""){
        threadId = createConversation();
    }

    // Make API call to /get-params endpoint
    std::string url = "http://localhost:5000/get-params";
    std::string query = text.toStdString();
    std::string currentState = " ";
    std::string command = "curl -X POST -H \"Content-Type: application/json\" -d '{\"query\": \"" + query + "\", \"current_state\": \"" + currentState + "\", \"thread_id\": " + threadId + "}' " + url;

    // Execute the command and get the response
    std::string response = "";
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe)
    {
        char buffer[128];
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
                response += buffer;
        }
        pclose(pipe);
    }

    // Parse the JSON response
    juce::var result = juce::JSON::parse(response);
    if (result.isObject())
    {
        juce::var parameters = result["run_parameters"];
        // Print the parameters
        // std::cout << "Parameters: " << parameters.toString() << std::endl;

        // Set the parameters for each effect
        if (parameters.isObject())
        {
            juce::var jsonEffects = parameters["effects"];
            if (jsonEffects.isArray())
            {
                for (auto connection : mainProcessor->getConnections())
                    mainProcessor->removeConnection(connection);

                // Previous node to connect from, starting with the input node
                juce::AudioProcessorGraph::Node::Ptr prevNode = audioInputNode;

                 for (int i = 0; i < jsonEffects.size(); ++i)
                 {
                     juce::var effect = jsonEffects[i];

                     if (effect.isObject())
                     {
                         juce::AudioProcessorGraph::Node::Ptr effectNode;
                         juce::String effectName = effect["type"].toString();
                         if (effectName == "peakFilter")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<FilterProcessor>("peakFilter", effect["centreFrequency"], effect["Q"], effect["gainFactor"]));
                            // continue;
                         } else if (effectName == "lowShelfFilter")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<FilterProcessor>("lowShelfFilter", effect["cutOffFrequency"], effect["Q"], effect["gainFactor"]));
                            // continue;
                         } else if (effectName == "highShelfFilter")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<FilterProcessor>("highShelfFilter", effect["cutOffFrequency"], effect["Q"], effect["gainFactor"]));
                            // continue;
                         } else if (effectName == "reverb")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<ReverbProcessor>(effect["roomSize"], effect["damping"], effect["wetLevel"], effect["width"]));
                         } else if (effectName == "compressor")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<CompressorProcessor>(effect["threshold"], effect["ratio"], effect["attack"], effect["release"]));
                            // continue;
                         } else if (effectName == "delayLine")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<DelayLineProcessor>(effect["delay"], effect["maximumDelayInSamples"]));
                            // continue;
                         } else if (effectName == "phaser")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<PhaserProcessor>(effect["rate"], effect["depth"], effect["centerFrequency"], effect["feedback"], effect["mix"]));
                            // continue;
                         } else if (effectName == "chorus")
                         {
                            effectNode = mainProcessor->addNode(std::make_unique<ChorusProcessor>(effect["rate"], effect["depth"], effect["centreDelay"], effect["feedback"], effect["mix"]));
                            // continue;
                         }
                         // Connect the previous node to this effect node
                         for(int channel = 0; channel < 2; ++channel)
                             mainProcessor->addConnection({ { prevNode->nodeID, channel }, { effectNode->nodeID, channel } });
                         prevNode = effectNode;
                     }
                 }

                // Connect the last effect to the output node
                for (int channel = 0; channel < 2; ++channel)
                    mainProcessor->addConnection({{prevNode->nodeID, channel}, {audioOutputNode->nodeID, channel}});
                connectMidiNodes();

                for (auto node : mainProcessor->getNodes()) // [10]
                    node->getProcessor()->enableAllBuses();
            }
        }

        juce::var messages = result["messages"];
        juce::String formattedMessages;
        if(messages.isArray()){
            for (auto& messageObj : *messages.getArray()){
                if (messageObj.isObject()) {
                    juce::String sender = messageObj["role"].toString();
                    juce::String message = messageObj["value"].toString();

                    if (sender == "user"){
                            formattedMessages += "You: " + message + "\n\n";    
                        
                    } else {
                        formattedMessages += "SemanticEQ: " + message + "\n\n";
                    }
                }
            }
        }

        juce::MessageManager::callAsync([this, formattedMessages]() {
            if (auto* editor = dynamic_cast<SemanticEQAudioProcessorEditor*>(getActiveEditor())){
                editor->setMessageContent(formattedMessages);
            }
        });
    }


}

void SemanticEQAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // juce::dsp::AudioBlock<float> block(buffer);
    mixer.setWetMixProportion(parameters.getRawParameterValue("mix")->load());
    mixer.pushDrySamples(buffer);

    mainProcessor->processBlock(buffer, midiMessages);

    mixer.mixWetSamples(buffer);
}

//==============================================================================
bool SemanticEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *SemanticEQAudioProcessor::createEditor()
{
    return new SemanticEQAudioProcessorEditor(*this);
}

//==============================================================================
void SemanticEQAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SemanticEQAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new SemanticEQAudioProcessor();
}
