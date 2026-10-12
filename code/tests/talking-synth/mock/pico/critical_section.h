#pragma once
#include <mutex>
struct critical_section_t { std::mutex mutex; };
inline void critical_section_init(critical_section_t *) {}
inline void critical_section_enter_blocking(critical_section_t *c) { c->mutex.lock(); }
inline void critical_section_exit(critical_section_t *c) { c->mutex.unlock(); }
