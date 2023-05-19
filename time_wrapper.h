#include <chrono>

template<typename TFunction>
class TimerWrapper {
public:
    TimerWrapper(TFunction function, clock_t& elapsedTime)
        : call(function), start_time_(::clock()), elapsed_time_(elapsedTime) {
    }

    ~TimerWrapper() {
        const clock_t endTime_ = ::clock();
        const clock_t diff = (endTime_ - start_time_);
        elapsed_time_ += diff;
    }

    TFunction call;

private:
    const clock_t start_time_;
    clock_t& elapsed_time_;
};

template<typename TFunction>
TimerWrapper<TFunction> TestTime(TFunction function, clock_t& elapsedTime) {
    return TimerWrapper<TFunction>(function, elapsedTime);
}
