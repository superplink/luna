#pragma once
#include <chrono>
#include <iostream>


// https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
template <typename F>
void log_time_action (F fun) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    
    auto t1 = high_resolution_clock::now();
    fun();
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_float = t2 - t1;

    std::cout << ms_float.count() << "ms\n";
}


template <typename F>
double time_action (F fun) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    
    auto t1 = high_resolution_clock::now();
    fun();
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_float = t2 - t1;
    
    return ms_float.count();
}



