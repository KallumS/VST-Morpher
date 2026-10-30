#include "MorpherProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new morpher::MorpherProcessor();
}
