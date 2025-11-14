// Provides access to std::chrono utilities for calendar calculations.
#include <chrono>

namespace glutalk_cpp {

struct PresenterSegment {
    char const *message;
    unsigned int minutes;
};

char const *getSpeaker()
{
    return "Clang++";
}

PresenterSegment getSegment()
{
    return { "C++ shapes the optimized core of GluTalk", 12 };
}

int getRecommendedLimit(int speaker_count)
{
    return 60 / speaker_count;
}

unsigned int getCurrentYear()
{
    using namespace std::chrono;
    auto today = floor<days>(system_clock::now());
    auto ymd = year_month_day(today);
    return static_cast<unsigned int>(static_cast<int>(ymd.year()));
}

} // namespace glutalk_cpp
