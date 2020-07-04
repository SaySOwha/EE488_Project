/**
 * Copyright (c) 2018 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Daniel CarFBlho
 */

#include "mem/cache/replacement_policies/fb_rp.hh"

#include <cassert>
#include <memory>
#include <vector>

#include "params/FBRP.hh"
#include "debug/CacheRepl.hh"
#include "mem/cache/cache_blk.hh"

FBRP::FBRP(const Params *p)
    : BaseReplacementPolicy(p),
      Weight(nullptr)
{
}

void
FBRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    std::shared_ptr<FBReplData> FB_replacement_data =
            std::static_pointer_cast<FBReplData>(
                replacement_data);
    // Reset last touch timestamp and frequency
    FB_replacement_data->lastTouchTick = Tick(0);
    FB_replacement_data->frequency = 0;
}

void
FBRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<FBReplData> FB_replacement_data =
            std::static_pointer_cast<FBReplData>(
                replacement_data);
    // Update last touch timestamp and frequency
    FB_replacement_data->lastTouchTick = curTick();
    FB_replacement_data->frequency++;
}

void
FBRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<FBReplData> FB_replacement_data =
            std::static_pointer_cast<FBReplData>(
                replacement_data);
    // Set last touch timestamp and frequency
    FB_replacement_data->lastTouchTick = curTick();
    FB_replacement_data->frequency = 1;
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
FBRP::getVictim(const ReplacementCandidates& candidates) const
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
        std::shared_ptr<FBReplData> candidate_replacement_data =
            std::static_pointer_cast<FBReplData>(
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
    

    for (const auto& x : WL) {
        DPRINTF(CacheRepl, "addr: 0x%x, tick: %d, freq: %d, score: %d, dirty: %d, set: %d, weight: %lf\n", std::static_pointer_cast<FBReplData>(
                x->repl->replacementData)->addr, x->lastTouchTick, x->frequency, x->weight, static_cast<CacheBlk*>(x->repl)->isDirty(), x->repl->getSet(), weight);
    }

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
    DPRINTF(CacheRepl, "Replacement victim: %s, addr: 0x%x\n\n", victim->print(), std::static_pointer_cast<FBReplData>(
                victim->replacementData)->addr);

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
FBRP::instantiateEntry()
{
    Weight = new std::map<uint32_t, double>;
    return std::shared_ptr<ReplacementData>(new FBReplData());
}

FBRP*
FBRPParams::create()
{
    return new FBRP(this);
}
