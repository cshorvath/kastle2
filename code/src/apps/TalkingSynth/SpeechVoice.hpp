// SPDX-License-Identifier: MIT
#pragma once
#include "PresetFile.hpp"
#include "pico/critical_section.h"
#include "pico/platform.h"
#include <cmath>
extern "C" {
#include "sam/stream.h"
}
namespace kastle2::talking {
class SpeechVoice {
    static constexpr uint32_t kSize = 512;
    critical_section_t lock_{};
    uint8_t ring_[kSize]{};
    uint32_t read_=0, write_=0, generation_=0, rendering_=0;
    uint16_t pitch_=256;
    Phrase selected_{};
    bool playing_=false, done_=false, stopped_=false;
    uint32_t phase_=0, output_rate_=44100;
    int16_t sample_=0;
    struct RenderContext { SpeechVoice *voice; uint32_t generation; };
    static int Sink(void *context, uint8_t sample) {
        auto &c=*static_cast<RenderContext *>(context);
        auto &v=*c.voice;
        for (;;) {
            critical_section_enter_blocking(&v.lock_);
            if (v.generation_!=c.generation) { critical_section_exit(&v.lock_); return 0; }
            if (v.write_-v.read_<kSize) {
                v.ring_[v.write_++ & (kSize-1)]=sample;
                critical_section_exit(&v.lock_); return 1;
            }
            critical_section_exit(&v.lock_);
            tight_loop_contents();
        }
    }
    static uint16_t Pitch(void *context) {
        auto &v=*static_cast<RenderContext *>(context)->voice;
        critical_section_enter_blocking(&v.lock_);
        const auto p=v.pitch_;
        critical_section_exit(&v.lock_); return p;
    }
public:
    uint32_t underruns=0, errors=0;
    void Init(uint32_t output_rate) { critical_section_init(&lock_); output_rate_=output_rate; }
    void SetSpeed(float multiplier) { // Wave Bard pitch multiplier; never changes sample read rate.
        const auto p=static_cast<uint16_t>(std::fmin(4.0f,std::fmax(0.25f,multiplier))*256);
        critical_section_enter_blocking(&lock_); pitch_=p; critical_section_exit(&lock_);
    }
    void Trigger(const Phrase &phrase) {
        critical_section_enter_blocking(&lock_);
        selected_=phrase; ++generation_; read_=write_=0;
        playing_=true; done_=false; phase_=0; sample_=0;
        critical_section_exit(&lock_);
    }
    int16_t Process() {
        critical_section_enter_blocking(&lock_);
        phase_+=22050;
        if (phase_>=output_rate_) {
            phase_-=output_rate_;
            if (read_!=write_) sample_=static_cast<int16_t>((int(ring_[read_++ & (kSize-1)])-128)*256);
            else { sample_=0; if (done_) playing_=false; else if (playing_) ++underruns; }
        }
        const auto result=sample_;
        critical_section_exit(&lock_); return result;
    }
    bool RenderNext() {
        critical_section_enter_blocking(&lock_);
        if (stopped_ || generation_==rendering_) { critical_section_exit(&lock_); return false; }
        rendering_=generation_;
        RenderContext context{this,rendering_};
        const Phrase phrase=selected_;
        critical_section_exit(&lock_);
        const int result=SamSpeak(phrase.text,phrase.speed,phrase.mouth,phrase.throat,phrase.sing,Sink,Pitch,&context);
        critical_section_enter_blocking(&lock_);
        if (generation_==context.generation) { done_=true; if (!result) ++errors; }
        critical_section_exit(&lock_);
        return true;
    }
    uint32_t BufferedSamples() {
        critical_section_enter_blocking(&lock_);
        const auto count=write_-read_;
        critical_section_exit(&lock_); return count;
    }
    bool IsRenderingComplete() {
        critical_section_enter_blocking(&lock_);
        const bool complete=done_;
        critical_section_exit(&lock_); return complete;
    }
    bool IsPlaying() {
        critical_section_enter_blocking(&lock_);
        const bool playing=playing_;
        critical_section_exit(&lock_); return playing;
    }
    void Stop() {
        critical_section_enter_blocking(&lock_);
        stopped_=true; ++generation_; read_=write_=0; sample_=0; playing_=false;
        critical_section_exit(&lock_);
    }
    void Worker() {
        for (;;) {
            critical_section_enter_blocking(&lock_);
            const bool stopped=stopped_;
            critical_section_exit(&lock_);
            if (stopped) return;
            if (!RenderNext()) tight_loop_contents();
        }
    }
};
}
