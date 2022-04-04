#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/MotionsTable.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
SequenceSearchModel::SequenceSearchModel(const MotionsTable* motionsTable)
    : motionsTable_(motionsTable)
{}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setSession(rfcommon::Session* session)
{
    session_ = session;
    sequences_.resize(session_->fighterCount());

    for (int f = 0; f != session_->frameCount(); ++f)
        addFrame(f, session_->frame(f));

    // Be helpful and change the current fighter index if a character
    // matches the last fighter character
    if (currentFighterCharacter_.count() > 0 && currentFighterCharacter_ != fighterCharacter(currentFighter_))
    {
        for (int p = 0; p != session_->fighterCount(); ++p)
            if (currentFighterCharacter_ == fighterCharacter(p))
            {
                currentFighter_ = p;
                break;
            }
    }
    currentFighterCharacter_ = fighterCharacter(currentFighter_);

    session_->dispatcher.addListener(this);
    dispatcher.dispatch(&SequenceSearchListener::onSessionChanged);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::clearSession(rfcommon::Session* session)
{
    session_->dispatcher.removeListener(this);

    sequences_.clearCompact();
    session_.drop();

    dispatcher.dispatch(&SequenceSearchListener::onSessionChanged);
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::fighterCount() const
{
    return session_.notNull() ? session_->fighterCount() : 0;
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::fighterName(int fighterIdx) const
{
    return session_->name(fighterIdx).cStr();
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::fighterCharacter(int fighterIdx) const
{
    if (session_.isNull())
        return "(no session)";
    const rfcommon::FighterID fighterID = session_->fighterID(fighterIdx);
    const rfcommon::String* s = session_->mappingInfo().fighterID.map(fighterID);
    return s ? s->cStr() : "(unknown fighter)";
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setCurrentFighter(int fighterIdx)
{
    currentFighter_ = fighterIdx;
    currentFighterCharacter_ = fighterCharacter(fighterIdx);
    dispatcher.dispatch(&SequenceSearchListener::onCurrentFighterChanged);
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::frameCount() const
{
    return session_.notNull() ? session_->frameCount() : 0;
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::sequenceLength(int fighterIdx) const
{
    return fighterIdx < sequences_.count() ? sequences_[fighterIdx].states.count() : 0;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<rfcommon::String> SequenceSearchModel::availableLabels(int fighterIdx) const
{
    if (session_.isNull())
        return rfcommon::Vector<rfcommon::String>();

    rfcommon::Vector<rfcommon::String> l1, l2, l3, result;
    rfcommon::HashMap<rfcommon::FighterMotion::Type, int> map;
    for (int f = 0; f != session_->frameCount(); ++f)
    {
        const rfcommon::FighterState& state = session_->state(f, fighterIdx);
        if (map.insertNew(state.motion().value(), result.count()) == map.end())
            continue;

        if (const char* user = motionsTable_->motionToUserLabel(state.motion()))
            l1.emplace(user);
        else if (const char* label = motionsTable_->motionToLabel(state.motion()))
            l2.emplace(label);
        else
            l3.emplace(state.motion().toStdString().c_str());
    }

    result.push(std::move(l1));
    result.push(std::move(l2));
    result.push(std::move(l3));
    return result;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::setQuery(const char* queryStr)
{
    QueryASTNode* ast;
    Query* query;

    ast = Query::parse(queryStr);
    if (ast == nullptr)
    {
        queryError_ = "Parse error";
        return false;
    }
    ast->exportDOT("query-ast.dot");

    query = Query::compileAST(ast, motionsTable_);
    QueryASTNode::destroyRecurse(ast);
    if (query == nullptr)
    {
        queryError_ = "Compile error";
        return false;
    }
    query->exportDOT("query.dot", motionsTable_);

    query_.reset(query);
    dispatcher.dispatch(&SequenceSearchListener::onQueryChanged);
    return true;
}

// ----------------------------------------------------------------------------
Graph SequenceSearchModel::applyQuery(int* numMatches, int* numMatchedStates)
{
    *numMatches = 0;
    *numMatchedStates = 0;
    if (query_.get() == nullptr || session_.get() == nullptr)
        return Graph();

    rfcommon::Vector<SequenceRange> matchingSequences = query_->apply(sequences_[currentFighter_]);
    Graph graph = Graph::fromSequenceRanges(sequences_[currentFighter_], matchingSequences);
    graph.exportDOT("decision_graph_search.dot", currentFighter_, session_, motionsTable_);

    *numMatches = matchingSequences.count();
    for (const auto& range : matchingSequences)
        *numMatchedStates += range.endIdx - range.startIdx;

    return graph;
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addFrame(int frameIdx, const rfcommon::Frame& frame)
{
    for (int fighterIdx = 0; fighterIdx != session_->fighterCount(); ++fighterIdx)
    {
        const auto& fighterState = frame.fighter(fighterIdx);
        Sequence& seq = sequences_[fighterIdx];

        const bool inHitlag = [this, fighterIdx, frameIdx, &fighterState]() -> bool {
            if (session_->fighterCount() != 2)
                return false;

            const auto& opponentState = session_->state(frameIdx, 1 - fighterIdx);
            const auto& prevFighterState = frameIdx > 0 ? session_->state(frameIdx - 1, fighterIdx) : fighterState;
            if (opponentState.flags().attackConnected())
                return fighterState.hitstun() == prevFighterState.hitstun();

            return false;
        }();

        const bool inHitstun = [&fighterState]() -> bool {
            return fighterState.hitstun() > 0;
        }();

        const bool inShieldlag = [&fighterState]() -> bool {
            return fighterState.status() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        }();

        const bool opponentInHitlag = [this, fighterIdx, frameIdx, &fighterState]() -> bool {
            if (session_->fighterCount() != 2)
                return false;

            const auto& opponentState = session_->state(frameIdx, 1 - fighterIdx);
            const auto& prevOpponentState = frameIdx > 0 ? session_->state(frameIdx - 1, 1 - fighterIdx) : opponentState;
            if (fighterState.flags().attackConnected())
                return opponentState.hitstun() == prevOpponentState.hitstun();

            return false;
        }();

        const bool opponentInHitstun = [fighterIdx, &frame]() -> bool {
            if (frame.fighterCount() != 2)
                return false;

            const auto& opponentState = frame.fighter(1 - fighterIdx);
            return opponentState.hitstun() > 0;
        }();

        const bool opponentInShieldlag = [fighterIdx, &frame]() -> bool {
            if (frame.fighterCount() != 2)
                return false;

            const auto& opponentState = frame.fighter(1 - fighterIdx);
            return opponentState.status() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        }();

        const State state(
            fighterState.motion(),
            fighterState.status(),
            fighterState.hitStatus(),
            inHitlag, inHitstun, inShieldlag,
            opponentInHitlag, opponentInHitstun, opponentInShieldlag
        );

        // Only process state if it is meaningfully different from the previously
        // processed state
        if (seq.states.count() > 0 && state == seq.states.back())
            continue;

        // Add to sequence
        seq.states.push(state);

        /*
        // New unique state
        auto nodeLookupResult = data.stateLookup.insertOrGet(state, -1);
        if (nodeLookupResult->value() == -1)
        {
            data.graph.nodes.emplace(state);
            nodeLookupResult->value() = data.graph.nodes.count() - 1;
        }
        const int currentNodeIdx = nodeLookupResult->value();

        // Create a new edge to the previously added/visited node
        if (data.prevNodeIdx != -1)
        {
            auto edgeLookupResult = data.edgeLookup.insertOrGet(EdgeConnection(data.prevNodeIdx, currentNodeIdx), -1);

            // If edge does not exist yet, create it. Otherwise, add weight to
            // the existing edge
            if (edgeLookupResult->value() == -1)
            {
                data.graph.edges.emplace(data.prevNodeIdx, currentNodeIdx);
                data.graph.nodes[data.prevNodeIdx].outgoingEdges.push(data.graph.edges.count() - 1);
                data.graph.nodes[currentNodeIdx].incomingEdges.push(data.graph.edges.count() - 1);

                edgeLookupResult->value() = data.graph.edges.count() - 1;
            }
            else
                data.graph.edges[edgeLookupResult->value()].addWeight();
        }
        data.prevNodeIdx = currentNodeIdx;
        */
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::onRunningSessionNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame)
{
    addFrame(frameIdx, frame);
    dispatcher.dispatch(&SequenceSearchListener::onSequenceChanged);
}
