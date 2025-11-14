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

int recommended_limit(int speaker_count)
{
    return 120 / speaker_count;
}

} // namespace glutalk_cpp
