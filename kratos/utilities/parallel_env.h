//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher
//

// System includes
#include <thread>

// External includes
#ifdef KRATOS_SMP_OPENMP
#include <omp.h>
#endif

// Project includes
#include "includes/lock_object.h"

InitializeSharedMemParallel()
{
    msLock = LockObject();
    msMainThreadId = std::this_thread::get_id();
}

class GlobalLock
{
GlobalLock() noexcept
{
    msLock.SetLock();
}

~GlobalLock() noexcept
{
    msLock.UnSetLock();
}

private:
static LockObject msLock; // can it be here? => who initializes? => maybe with singleton like in GiDIO? => creation must be threadsafe!!! => hence maybe better done outside...
};


   static inline int GetNumThreads()
    {
#ifdef KRATOS_SMP_OPENMP
        return omp_get_max_threads();
#elif KRATOS_SMP_CXX11
        // this might still not always work, could still return 0...
        // => do we even want to support it?
        // => probably better to use the specified number and not the may available!
        // (aka OMP_NUM_THREADS in the case of OpenMP)
        return std::thread::hardware_concurrency();
#else
        return 1;
#endif
    }

    /// Wrapper for omp_get_num_threads().
    /**
     @return Number of OpenMP threads in the current team.
     */
    static int GetCurrentNumberOfThreads()
    {
#ifdef KRATOS_SMP_OPENMP
        return omp_get_num_threads();
#elif KRATOS_SMP_CXX11
        https://bitbucket.org/blaze-lib/blaze/wiki/Cpp%20Thread%20Parallelization
        // this probably has to be a custom mechanism! => not provided by the standard
        /*from blaze (ThreadBackend.h):

        inline size_t ThreadBackend<TT,MT,LT,CT>::initPool()
        {
        const char* env = std::getenv( "BLAZE_NUM_THREADS" );

        if( env == nullptr )
            return 1UL;
        else return max( 1, atoi( env ) );
        }
        #if (defined _MSC_VER)
        #  pragma warning(pop)
        #endif
        */

        return ???;
#else
        return 1;
#endif
    }

    /// Wrapper for omp_get_num_procs().
    /**
     @return Number of processors available to the device.
     */
    static int GetNumberOfProcessors()
    {
#ifdef KRATOS_SMP_OPENMP
        return omp_get_num_procs();
#elif KRATOS_SMP_CXX11
        return ???;
#else
        return 1;
#endif
    }

    /// Wrapper for omp_get_thread_num().
    /**
     @return The thread number for this thread, 0 if scalar run.
     */
    static inline int ThisThread()
    {
#ifdef KRATOS_SMP_OPENMP
        return omp_get_thread_num();
#elif KRATOS_SMP_CXX11
        // this is a real blocker as not really possible with C++, due to how the parallelism works there!
        // maybe maybe we could provide this if we have a threadpool where we can map the thread id to an id starting from 0 (assigned from the threadpool)
        return ???;
#else
        return 0;
#endif
    }

    /// Wrapper for omp_in_parallel().
    /**
     @return Maximum number of OpenMP threads that will be used in
     parallel regions.
     */
    static inline bool IsInParallel()
    {
#ifdef KRATOS_SMP_OPENMP
        return omp_in_parallel();
#elif KRATOS_SMP_CXX11
        // if thread id of the current thread is different from the global one it means we are in parallel
        return std::this_thread::get_id() != msMainThreadId;
#else
        return false;
#endif
    }

#ifdef KRATOS_SMP_CXX11
static std::thread::id msMainThreadId;
#endif
static LockObject msLock // here or inside the lock?
