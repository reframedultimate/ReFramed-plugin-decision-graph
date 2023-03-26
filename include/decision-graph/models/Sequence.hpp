#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include <variant>

namespace rfcommon {
    class MotionLabels;
}

/*!
 * The entire list of states, spanning multiple sessions if more than one was
 * loaded, is stored in this class. Some metadata is stored to make it easier
 * to format the data later (player name, fighter name, fighter ID).
 */
class States : public rfcommon::Vector<State>
{
public:
    States(rfcommon::FighterID fighterID, const rfcommon::String& playerName, const rfcommon::String& fighterName);
    ~States();

    const rfcommon::String playerName;
    const rfcommon::String fighterName;
    const rfcommon::FighterID fighterID;
};

/*!
 * Given the complete list of states for a particular fighter (see class "States"),
 * a "range" references a sub-section within that state list. This is simply
 * two indices into the states array: The start index and the end index. The
 * end index is exclusive, meaning if startIdx==endIdx then the range is empty.
 */
class Range
{
public:
    Range(int startIdx, int endIdx);
    ~Range();

    int startIdx, endIdx;
};

/*!
 * Very similar to "Range", except a sequence references a set of specific states
 * rather than a linear range. This occurs when states are merged. For example,
 * if an "aerial nair" is followed by a "landing nair", for the purpose of some
 * visualizations, these are the same thing. So they would be merged into a
 * single state "nair". Since this would make the range of states referenced in
 * the state array non-linear, so instead of storing a start and end index, an
 * array of indices is stored instead.
 * 
 * The indices will always be strictly increasing in order. A sequence should
 * never reference states out of order.
 */
class Sequence
{
public:
    Sequence();
    ~Sequence();

    rfcommon::SmallVector<int, 8> idxs;
};

rfcommon::String toString(const States& states, const Range& range, const rfcommon::MotionLabels* labels);
rfcommon::String toString(const States& states, const Sequence& seq, const rfcommon::MotionLabels* labels);
