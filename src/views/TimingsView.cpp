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

// ----------------------------------------------------------------------------
void TimingsView::onQueryApplied() 
{
    struct SeqRangeRef
    {
        SeqRangeRef(const Sequence& seq, const SequenceRange& range)
            : seq(seq), range(range)
        {}

        struct Hasher {
            typedef uint32_t HashType;
            HashType operator()(const SeqRangeRef& ref) const {
                State::HasherNoSideData h;
                const State& first = ref.seq.states[ref.range.startIdx];
                const State& last = ref.seq.states[ref.range.endIdx - 1];
                return rfcommon::hash32_combine(h(first), h(last));
            }
        };

        struct Compare {
            bool operator()(const SeqRangeRef& a, const SeqRangeRef& b) {
                State::CompareNoSideData c;
                const State& aFirst = a.seq.states[a.range.startIdx];
                const State& aLast = a.seq.states[a.range.endIdx - 1];
                const State& bFirst = b.seq.states[b.range.startIdx];
                const State& bLast = b.seq.states[b.range.endIdx - 1];
                return c(aFirst, bFirst) && c(aLast, bLast);
            }
        };

        const Sequence& seq;
        const SequenceRange& range;
    };

    const Sequence& seq = model_->fighterSequence(model_->currentFighter());

    rfcommon::HashMap<SeqRangeRef, int, SeqRangeRef::Hasher, SeqRangeRef::Compare> sequenceFrequencies;
    for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
        for (const auto& range : model_->matches(queryIdx))
            sequenceFrequencies.insertOrGet(SeqRangeRef(seq, range), 0)->value()++;

    const SeqRangeRef* mostCommonSequence = nullptr;
    int highestCount = 0;
    for (const auto it : sequenceFrequencies)
        if (highestCount < it->value())
        {
            highestCount = it->value();
            mostCommonSequence = &it->key();
        }

    // Frames, Frequency
    rfcommon::HashMap<int, int> histogram;
    if (mostCommonSequence)
    {
        const auto& mostCommonFirstState = seq.states[mostCommonSequence->range.startIdx];
        const auto& mostCommonLastState = seq.states[mostCommonSequence->range.endIdx - 1];

        for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
            for (const auto& range : model_->matches(queryIdx))
                if (seq.states[range.startIdx].compareWithoutSideData(mostCommonFirstState)
                    && seq.states[range.endIdx - 1].compareWithoutSideData(mostCommonLastState))
                {
                    const auto& first = seq.states[range.startIdx];
                    const auto& last = seq.states[range.endIdx - 1];
                    int diffFrames = last.sideData.frameIndex.index() - first.sideData.frameIndex.index();
                    histogram.insertOrGet(diffFrames / 3, 0)->value()++;
                }

        auto seqName = toString(mostCommonSequence->seq, mostCommonSequence->range, model_->fighterID(model_->currentFighter()), labels_);
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
