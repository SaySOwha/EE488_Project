
#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LW_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LW_RP_HH__

#include "mem/cache/replacement_policies/base.hh"

struct LWRPParams;

struct WL_entry
{
    ReplaceableEntry *repl;
    Tick lastTouchTick;
    uint32_t frequency;
    double weight;
};


class LWRP : public BaseReplacementPolicy
{
  private:
    std::map<uint32_t, double> *Weight;



  protected:
    /** LW-specific implementation of replacement data. */
    struct LWReplData : ReplacementData
    {
        /** Tick on which the entry was last touched. */
        Tick lastTouchTick;
        uint32_t frequency;

        /**
         * Default constructor. Invalidate data.
         */
        LWReplData() : lastTouchTick(0), frequency(0) {}
    };

  public:
    /** Convenience typedef. */
    typedef LWRPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    LWRP(const Params *p);

    /**
     * Destructor.
     */
    ~LWRP() {}

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                              const override;

    /**
     * Touch an entry to update its replacement data.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using VA timestamps.
     *
     * @param candidates Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;                                                      

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_VA_RP_HH__
