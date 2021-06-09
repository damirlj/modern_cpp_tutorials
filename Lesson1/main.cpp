#include <chrono>
#include <sstream>
#include <string>
#include <functional>


#include "ConsoleLogger.h"
#include "AOThread.h"





int testAOT(int tasks)
{
    using namespace std;
    using namespace std::chrono_literals;
    using namespace utils::aot;


    AOThread<void> aoThread
    {    "t_testAOT"
        , utils::thread::ThreadWrapper::schedule_t::normal
        , 0
    };

    utils::log::ConsoleLogger logger;

    auto task = [&logger](int d, std::chrono::milliseconds timeout) mutable
    {
      logger.log("Sleeping: %llu[ms]", timeout.count());

      std::this_thread::sleep_for(timeout);

    };

    future<void> futures[tasks];

    for (auto i = 0; tasks > i; ++i)
    {
        job_t<void> t {std::bind(task, i + 1, 1s * (i + 1) )};
        futures[i] = aoThread.enqueue(std::move(t));
    }

    for (auto i = 0; tasks > i; ++i)
    {
        futures[i].get();
    }

    return 0;
}



int main()
{
    return testAOT(4);
}
