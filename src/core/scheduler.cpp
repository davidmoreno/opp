#include "scheduler.hpp"
#include "extra/thread_barrier.hpp"
#include <fmt/format.h>

#include <boost/fiber/barrier.hpp>
#include <boost/fiber/algo/work_stealing.hpp>
#include <boost/fiber/detail/context_spmc_queue.hpp>

namespace opp{
  thread_local uint32_t tid;


  // class worker : public boost::fibers::algo::algorithm {
  // public:
  //   worker();
  //   ~worker();
  //
  //   worker( worker const&) = delete;
  //   worker( worker &&) = delete;
  //   worker & operator=( worker const&) = delete;
  //   worker & operator=( worker &&) = delete;
  //
  //
  //   virtual void awakened( boost::fibers::context *) noexcept;
  //   virtual boost::fibers::context * pick_next() noexcept;
  //   virtual bool has_ready_fibers() const noexcept;
  //   virtual void suspend_until( std::chrono::steady_clock::time_point const&) noexcept;
  //   virtual void notify() noexcept;
  // };

  scheduler::scheduler(uint32_t nworkers){
    _running = true;
    thread_barrier barrier(nworkers);
    boost::fibers::use_scheduling_algorithm< boost::fibers::algo::work_stealing >(nworkers, true);
    tid = 0;
    for (uint32_t i=0; i<nworkers-1; ++i){
      workers.emplace_back([this, &barrier, i, nworkers](){
        tid = i+1;
        boost::fibers::use_scheduling_algorithm< boost::fibers::algo::work_stealing >(nworkers, true);
        barrier.wait();

        // this is what makes all the other fibers to run, and this thread to wait.
        std::unique_lock<std::mutex> lk(running_mutex);
        running_cond.wait( lk, [this](){
          fmt::print(stderr, "{} exit if true, it is {}\n", tid, !this->_running);
          return !this->_running;
        });
        fprintf(stderr, "EOT %d\n", tid);
      });
    }
    barrier.wait();
  }

  scheduler::~scheduler(){
    fprintf(::stderr, "~scheduler()\n");
    _running = false;
    running_cond.notify_all();
    for (std::thread & t: workers) { /*< wait for threads to terminate >*/
      t.join();
    }
  }

}
