#include "Monitor.hpp"

namespace utils
{
    class Event final
    {
        public:

            explicit Event(bool autoReset) noexcept : autoReset_{autoReset}
            {}

            inline void wait() noexcept
            {
                auto lock = sync_.wait([this] { return flag_; });
                if (autoReset_) flag_ = false;
            }

            inline [[nodiscard]] bool wait_for(std::chrono::milliseconds timeout) noexcept
            {
                auto [result, lock] = sync_.wait_for(timeout, [this]{ return flag_; });
                if (autoReset_) flag_ = false;
                return result;
            }

            inline void signal() noexcept { sync_.notify_one([this]{ flag_ = true; }); }
            inline void broadcast() noexcept { sync_.notify_all([this] { flag_ = true; }); }

        private:
            const bool autoReset_;
            bool flag_ {false};
            Monitor<> sync_ {};
    };
}