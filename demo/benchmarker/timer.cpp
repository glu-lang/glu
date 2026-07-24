#include <chrono>
#include <vector>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::milliseconds;

// Structure to represent a timer interval
struct TimerInterval {
    TimePoint startTime;
    TimePoint endTime;
    bool _isRunning;

    TimerInterval();

    /// @brief Mark the interval as complete
    void stop();

    /// @brief Check if the interval is still running
    /// @return True if active, false otherwise
    bool isRunning() const;

    /// @brief Get the elapsed time for this interval
    /// @return Duration in milliseconds
    Duration getElapsed() const;
};

/// @brief Singleton timer class for tracking time intervals
class Timer {
    std::vector<TimerInterval> _intervals;

    Timer();

public:
    Timer(Timer const &) = delete;
    Timer &operator=(Timer const &) = delete;

    /// @brief Get the singleton instance
    /// @return Reference to the global Timer instance
    static Timer &getInstance();

    /// @brief Start a new timer interval
    /// @return Index of the newly created interval
    size_t start();

    /// @brief Stop a specific timer interval
    /// @param index The index of the interval to stop
    /// @return True if interval was found and stopped, false otherwise
    bool stop(size_t index);

    /// @brief Get elapsed time for a specific interval
    /// @param index The index of the interval
    /// @return Duration in milliseconds, or 0 if index is invalid
    Duration getElapsed(size_t index) const;

    /// @brief Get total elapsed time across all intervals
    /// @return Total duration in milliseconds
    Duration getTotalElapsed() const;

    /// @brief Count currently active intervals
    /// @return Number of running intervals
    size_t countActive() const;

    /// @brief Get all intervals
    /// @return Vector of all timer intervals
    std::vector<TimerInterval> const &getIntervals() const;

    /// @brief Reset all intervals
    void reset();
};

// TimerInterval method definitions
TimerInterval::TimerInterval()
    : startTime(Clock::now()), endTime(), _isRunning(true)
{
}

void TimerInterval::stop()
{
    endTime = Clock::now();
    _isRunning = false;
}

bool TimerInterval::isRunning() const
{
    return _isRunning;
}

Duration TimerInterval::getElapsed() const
{
    TimePoint end = _isRunning ? Clock::now() : endTime;
    return std::chrono::duration_cast<Duration>(end - startTime);
}

// Timer method definitions
Timer::Timer() { }

Timer &Timer::getInstance()
{
    static Timer *_instance = nullptr;
    if (!_instance) {
        _instance = new Timer();
    }
    return *_instance;
}

size_t Timer::start()
{
    _intervals.emplace_back();
    return _intervals.size() - 1;
}

bool Timer::stop(size_t index)
{
    if (index >= _intervals.size()) {
        return false;
    }
    if (!_intervals[index].isRunning()) {
        return false;
    }
    _intervals[index].stop();
    return true;
}

Duration Timer::getElapsed(size_t index) const
{
    if (index >= _intervals.size()) {
        return Duration(0);
    }
    return _intervals[index].getElapsed();
}

Duration Timer::getTotalElapsed() const
{
    Duration total(0);
    for (auto const &interval : _intervals) {
        total += interval.getElapsed();
    }
    return total;
}

size_t Timer::countActive() const
{
    size_t count = 0;
    for (auto const &interval : _intervals) {
        if (interval.isRunning()) {
            count++;
        }
    }
    return count;
}

std::vector<TimerInterval> const &Timer::getIntervals() const
{
    return _intervals;
}

void Timer::reset()
{
    _intervals.clear();
}
