#include "mem/cache/replacement_policies/lw_rp.hh"

#include <cassert>
#include <memory>
#include <vector>

#include "params/LWRP.hh"
#include "debug/CacheRepl.hh"
#include "mem/cache/cache_blk.hh"

LWRP::LWRP(const Params *p)
    : BaseReplacementPolicy(p),
      Weight(nullptr)
{
}

void
LWRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    std::shared_ptr<LWReplData> LW_replacement_data =
            std::static_pointer_cast<LWReplData>(
                replacement_data);
    // Reset last touch timestamp and frequency
    LW_replacement_data->lastTouchTick = Tick(0);
    LW_replacement_data->frequency = 0;
}

void
LWRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<LWReplData> LW_replacement_data =
            std::static_pointer_cast<LWReplData>(
                replacement_data);
    // Update last touch timestamp and frequency
    LW_replacement_data->lastTouchTick = curTick();
    LW_replacement_data->frequency++;
}

void
LWRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<LWReplData> LW_replacement_data =
            std::static_pointer_cast<LWReplData>(
                replacement_data);
    // Set last touch timestamp and frequency
    LW_replacement_data->lastTouchTick = curTick();
    LW_replacement_data->frequency = 1;
}

bool comparefreq(WL_entry *w1, WL_entry *w2)
{
    return (w1->frequency < w2->frequency);
}

bool comparerecency(WL_entry *w1, WL_entry *w2)
{
    return (w1->lastTouchTick < w2->lastTouchTick);
}

bool compareweight(WL_entry *w1, WL_entry *w2)
{
    return (w1->weight < w2->weight);
}

ReplaceableEntry*
LWRP::getVictim(const ReplacementCandidates& candidates) const
{

    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    ReplaceableEntry* victim;
    std::vector<WL_entry*> WL;
    WL_entry *e;
    int i = 2;
    int j = 1;
    uint32_t freq_before = 0;
    int freq_j = 0;
    uint32_t set;
    ReplaceableEntry *from_recent, *from_freq;
    double weight;
    std::map<uint32_t,double>::iterator it;

    set = candidates[0]->getSet();
    it = Weight->find(set);
    if (it == Weight->end()) {
        weight = 1;
        Weight->insert(std::make_pair(set, weight));
    }
    else {
        weight = it->second;
    }


    for (const auto& candidate : candidates) {
        std::shared_ptr<LWReplData> candidate_replacement_data =
            std::static_pointer_cast<LWReplData>(
                candidate->replacementData);

        // If invalid entry, evict it
        if ((candidate_replacement_data->lastTouchTick == Tick(0)) &&
             candidate_replacement_data->frequency == 0) {
            return candidate;
        }

        e = new WL_entry();
        e->repl = candidate;
        e->lastTouchTick = candidate_replacement_data->lastTouchTick;
        e->frequency = candidate_replacement_data->frequency;
        e->weight = 0;
        WL.push_back(e);
    }

    // score by recency
    std::sort(WL.begin(), WL.end(), comparerecency);
    from_recent = WL.front()->repl;

    for (const auto& x : WL) {
        x->weight = x->weight + (double) i * weight;
        i++;
    }

    // score by frequency
    std::sort(WL.begin(), WL.end(), comparefreq);
    from_freq = WL.front()->repl;

    for (const auto& x : WL) {
        if (x->frequency != freq_before) {
            freq_before = x->frequency;
            freq_j = j;
        }
        if (x->frequency != 1) {
            x->weight = x->weight + (double) freq_j;
        }
        j++;
    }

    std::sort(WL.begin(), WL.end(), compareweight);
    
    victim = WL.front()->repl;

    // prefer clean block
    for (const auto& x : WL) {
        if (x->weight < (double) WL.size() * 3.5 / 2 + 1) {
            if (static_cast<CacheBlk*>(x->repl)->isDirty() == false) {
                WL.clear();
                return x->repl;
            }
        }
    }

    WL.clear();

    // adjust weight for biasing
    if (from_recent != from_freq) {
        if (victim == from_recent) {
            weight += 0.01;
            if (weight > 2.5) {
                weight = 2.5;
            }
        }
        else {
            weight -= 0.01;
            if (weight < 1.5) {
                weight = 1.5;
            }
        }
        Weight->find(set)->second = weight;
    }

    return victim;
}

std::shared_ptr<ReplacementData>
LWRP::instantiateEntry()
{
    Weight = new std::map<uint32_t, double>;
    return std::shared_ptr<ReplacementData>(new LWReplData());
}

LWRP*
LWRPParams::create()
{
    return new LWRP(this);
}
