#include "decision-graph/views/TimingsView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include "qwt_plot.h"
#include "qwt_plot_histogram.h"

#include <QVBoxLayout>

// ----------------------------------------------------------------------------
TimingsView::TimingsView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
    , relativePlot_(new QwtPlot)
    , relativeData_(new QwtPlotHistogram)
{
    relativePlot_->setPalette(Qt::white);
    relativePlot_->setTitle("Relative Timings");
    relativePlot_->setAxisAutoScale(QwtPlot::xBottom);
    relativePlot_->setAxisAutoScale(QwtPlot::yLeft);

    relativeData_->attach(relativePlot_);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(relativePlot_);
    setLayout(l);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
TimingsView::~TimingsView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void TimingsView::onNewSessions() {}
void TimingsView::onClearAll() {}
void TimingsView::onDataAdded() {}
void TimingsView::onPOVChanged() {}
void TimingsView::onQueriesChanged() {}
void TimingsView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void TimingsView::onQueriesApplied()
{
    struct SeqRef
    {
        SeqRef(const States& states, const Sequence& seq)
            : states(states), seq(seq)
        {}

        struct Hasher {
            typedef uint32_t HashType;
            HashType operator()(const SeqRef& ref) const {
                HashType hash = 0;
                for (int idx : ref.seq.idxs)
                {
                    const State& state = ref.states[idx];
                    const uint32_t motion_l = state.motion.lower();
                    const uint16_t status = state.status.value();
                    const uint8_t motion_h = state.motion.upper();
                    const uint32_t a = motion_l;
                    const uint32_t b = (status << 8) | motion_l;
                    hash = rfcommon::hash32_combine(hash, rfcommon::hash32_combine(a, b));
                }
                return hash;
            }
        };

        struct Compare {
            bool operator()(const SeqRef& a, const SeqRef& b) const {
                if (a.seq.idxs.count() != b.seq.idxs.count())
                    return false;
                for (int i = 0; i != a.seq.idxs.count(); ++i)
                {
                    const State& sa = a.states[a.seq.idxs[i]];
                    const State& sb = b.states[b.seq.idxs[i]];
                    if (sa.motion != sb.motion) return false;
                    if (sa.status != sb.status) return false;
                }
                return true;
            }
        };

        const States& states;
        const Sequence& seq;
    };

    const States& states = model_->fighterStates(model_->playerPOV());

    rfcommon::HashMap<SeqRef, int, SeqRef::Hasher, SeqRef::Compare> sequenceFrequencies;
    for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
        for (const auto& seq : model_->mergedMatches(queryIdx))
            sequenceFrequencies.insertOrGet(SeqRef(states, seq), 0)->value()++;

    const SeqRef* mostCommon = nullptr;
    int highestCount = 0;
    for (const auto it : sequenceFrequencies)
        if (highestCount < it->value())
        {
            highestCount = it->value();
            mostCommon = &it->key();
        }

    // Frames, Frequency
    rfcommon::HashMap<int, int> histogram;
    if (mostCommon)
    {
        for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
            for (const auto& seq : model_->mergedMatches(queryIdx))
                if (SeqRef::Compare()(SeqRef(states, seq), *mostCommon))
                {
                    const auto& first = states[seq.idxs.front()];
                    const auto& last = states[seq.idxs.back()];
                    int diffFrames = last.sideData.frameIndex.index() - first.sideData.frameIndex.index();
                    histogram.insertOrGet(diffFrames / 3, 0)->value()++;
                }

        auto seqName = toString(states, mostCommon->seq, labels_);
        relativePlot_->setTitle(QString("Relative Timings for ") + seqName.cStr());
    }

    QVector<QwtIntervalSample> samples;
    for (const auto& it : histogram)
    {
        qreal frames = it->key() * 3.0;
        samples.push_back(QwtIntervalSample(it->value(), frames - 3.0/2.1, frames + 3.0/2.1));
    }
    relativeData_->setSamples(samples);
    relativePlot_->replot();
}
