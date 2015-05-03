#pragma once

namespace Feather
{
    enum AtomicOrdering
    {
        atomicNone,             ///< No atomic guarantees
        atomicUnordered,        ///< The set of values that can be read is governed by the happens-before partial order. Can model Java non-volatile shared variables
        atomicMonotonic,        ///< C++0x memory_order_relaxed
        atomicAcquire,          ///< C++0x memory_order_acquire
        atomicRelease,          ///< C++0x memory_order_release
        atomicAcquireRelease,   ///< C++0x memory_order_acq_rel
        atomicSeqConsistent,    ///< C++0x memory_order_seq_cst; Java volatile
    };
}
