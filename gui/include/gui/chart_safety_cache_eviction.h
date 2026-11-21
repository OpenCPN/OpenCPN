/* GPL-2.0-or-later. Preserve ordered eviction without rescanning pinned keys.
 */
#ifndef OCPN_CHART_SAFETY_CACHE_EVICTION_H
#define OCPN_CHART_SAFETY_CACHE_EVICTION_H
#include <new>
#include <set>

namespace ocpn::chart_safety {
// The caller holds the cache's existing lock and reports key mutations.
// Allocate the small eligibility index only when an eviction is first needed.
// The selected key is exactly the first unpinned key in the ordered cache.
template <class Key>
class PinnedCacheEvictionIndex {
public:
  void Reset() {
    eligible_.clear();
    ready_ = false;
  }
  void Pinned(const Key& key) {
    if (ready_) eligible_.erase(key);
  }
  void Erased(const Key& key) {
    if (ready_) eligible_.erase(key);
  }
  template <class Pins>
  void Stored(const Key& key, const Pins& pins) {
    if (!ready_ || pins.find(key) != pins.end()) return;
    try {
      eligible_.insert(key);
    } catch (const std::bad_alloc&) {
      Reset();
    }
  }
  template <class Cache, class Pins>
  typename Cache::iterator First(Cache& cache, const Pins& pins) {
    if (!ready_) {
      try {
        for (const auto& entry : cache)
          if (pins.find(entry.first) == pins.end())
            eligible_.insert(entry.first);
        ready_ = true;
      } catch (const std::bad_alloc&) {
        // Optional acceleration must not make memory pressure a failure.
        Reset();
        for (auto it = cache.begin(); it != cache.end(); ++it)
          if (pins.find(it->first) == pins.end()) return it;
        return cache.end();
      }
    }
    while (!eligible_.empty()) {
      auto candidate = cache.find(*eligible_.begin());
      if (candidate != cache.end() && pins.find(candidate->first) == pins.end())
        return candidate;
      eligible_.erase(eligible_.begin());
    }
    return cache.end();
  }

private:
  std::set<Key> eligible_;
  bool ready_ = false;
};
}  // namespace ocpn::chart_safety
#endif
