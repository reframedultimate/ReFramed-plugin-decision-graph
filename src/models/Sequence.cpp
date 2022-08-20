#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/LabelMapper.hpp"

// ----------------------------------------------------------------------------
rfcommon::String Sequence::toString(rfcommon::FighterID fighterID, LabelMapper* labels) const
{
    rfcommon::String result;
    for (const auto& state : states)
    {
        rfcommon::String label;

        if (state.inHitlag() || state.inHitstun())
            label = "disadv";
        else
            label = labels->bestEffortStringHighestLayer(fighterID, state.motion());

        if (state.opponentInShieldlag())
            label += " os";
        else if (state.opponentInHitlag() || state.opponentInHitstun())
            label += " adv";

        if (result.length())
            result += " -> ";
        result += label;
    }
    return result;
}
