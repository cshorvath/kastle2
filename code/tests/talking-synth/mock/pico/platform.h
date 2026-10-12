#pragma once
#include <thread>
inline void tight_loop_contents() { std::this_thread::yield(); }
