// Cobalt8M Controller (Studio 87): plugin entry. The processor lives in Processor.h, the editor (the six mockup pages) in Editor.h.
#include "Processor.h"
#include "Editor.h"

juce::AudioProcessorEditor* CtlProcessor::createEditor() { return new ctled::CtlEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CtlProcessor(); }
