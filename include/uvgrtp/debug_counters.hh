#pragma once

/* DEBUG-ONLY allocation counters, not part of upstream uvgRTP.
 *
 * Added to investigate a leak measured in uvgRTP_ros: glibc's live heap grows
 * roughly in proportion to message throughput across every transport, which
 * points at the shared send path rather than at any one codec. Reading the code
 * was inconclusive -- frame_queue::deinit_transaction looks complete -- so
 * these count what actually happens instead.
 *
 * Header-only with function-local statics so the diff is tiny and there is no
 * static-initialisation order to reason about. Intended to be dropped, or kept
 * behind a build flag, once the leak is found.
 */

#include <atomic>
#include <cstdint>

namespace uvgrtp {
namespace debug {

inline std::atomic<uint64_t>& transactions_allocated()
{
    static std::atomic<uint64_t> value{0};
    return value;
}

inline std::atomic<uint64_t>& transactions_freed()
{
    static std::atomic<uint64_t> value{0};
    return value;
}

inline std::atomic<uint64_t>& tmp_buffers_allocated()
{
    static std::atomic<uint64_t> value{0};
    return value;
}

inline std::atomic<uint64_t>& tmp_buffers_freed()
{
    static std::atomic<uint64_t> value{0};
    return value;
}

inline std::atomic<uint64_t>& frames_enqueued()
{
    static std::atomic<uint64_t> value{0};
    return value;
}

} // namespace debug
} // namespace uvgrtp
