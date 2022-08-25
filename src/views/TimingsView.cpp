#include "decision-graph/views/TimingsView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include "qwt_plot.h"
#include "qwt_plot_histogram.h"

#include <QVBoxLayout>

// ----------------------------------------------------------------------------
TimingsView::TimingsView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent)
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
void TimingsView::onCurrentFighterChanged() {}
void TimingsView::onNewSession() {}
void TimingsView::onDataAdded() {}
void TimingsView::onDataCleared() {}
void TimingsView::onQueryCompiled(int queryIdx) {}

// ----------------------------------------------------------------------------
void TimingsView::onQueryApplied() 
{
    struct SeqRef
    {
        SeqRef(const States& states, const Sequence& seq)
            : states(states), seq(seq)
        {}

        struct Hasher {
            typedef uint32_t HashType;
            HashType operator()(const SeqRef& ref) const {
                State::HasherNoSideData h;
                const State& first = ref.states[ref.seq.firstIdx()];
                const State& last = ref.states[ref.seq.lastIdx()];
                return rfcommon::hash32_combine(h(first), h(last));
            }
        };

        struct Compare {
            bool operator()(const SeqRef& a, const SeqRef& b) {
                State::CompareNoSideData c;
                const State& aFirst = a.states[a.seq.firstIdx()];
                const State& aLast = a.states[a.seq.lastIdx()];
                const State& bFirst = b.states[b.seq.firstIdx()];
                const State& bLast = b.states[b.seq.lastIdx()];
                return c(aFirst, bFirst) && c(aLast, bLast);
            }
        };

        const States& states;
        const Sequence& seq;
    };

    const States& states = model_->fighterStates(model_->currentFighter());

    rfcommon::HashMap<SeqRef, int, SeqRef::Hasher, SeqRef::Compare> sequenceFrequencies;
    for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
        for (const auto& seq : model_->matches(queryIdx))
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
        const auto& mostCommonFirstState = states[mostCommon->seq.firstIdx()];
        const auto& mostCommonLastState = states[mostCommon->seq.lastIdx()];

        for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
            for (const auto& seq : model_->matches(queryIdx))
                if (states[seq.firstIdx()].compareWithoutSideData(mostCommonFirstState)
                    && states[seq.lastIdx()].compareWithoutSideData(mostCommonLastState))
                {
                    const auto& first = states[seq.firstIdx()];
                    const auto& last = states[seq.lastIdx()];
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
