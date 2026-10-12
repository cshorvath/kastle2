#include "SpeechVoice.hpp"
#include <cassert>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdio>
using namespace kastle2::talking;
static int collect(void *p,uint8_t sample) { static_cast<std::vector<int16_t> *>(p)->push_back((int(sample)-128)*256); return 1; }
int main() {
    SpeechVoice voice; voice.Init(22050); // Direct-rate comparison avoids host scheduling assumptions.
    Phrase first{"A VERY LONG PHRASE",72,128,128,0,{}};
    Phrase second{"HELLO",72,128,128,0,{}};
    voice.Trigger(first);
    std::thread old([&]{assert(voice.RenderNext());});
    // Give the renderer time to fill the ring and block on its bounded capacity.
    const auto filledDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(voice.BufferedSamples()<512) { assert(std::chrono::steady_clock::now()<filledDeadline);std::this_thread::yield(); }
    voice.Trigger(second); old.join();
    assert(voice.Process()==0); // Old PCM and retained output sample were discarded.
    std::vector<int16_t> expected;
    assert(SamSpeak(second.text,72,128,128,0,collect,nullptr,&expected)==1);
    std::thread next([&]{assert(voice.RenderNext());});
    std::vector<int16_t> actual;
    // Process at a lower-than-real-time host rate, keeping the producer ahead.
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(voice.IsPlaying()) {
        assert(std::chrono::steady_clock::now()<deadline);
        while (!voice.BufferedSamples() && !voice.IsRenderingComplete()) {
            assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();
        }
        actual.push_back(voice.Process());
    }
    next.join();
    // One final silence closes the completed stream, not part of SAM's output.
    assert(actual.size()==expected.size()+1);
    assert(std::equal(expected.begin(),expected.end(),actual.begin()));
    assert(!voice.RenderNext());
    assert(voice.errors==0);
    puts("Voice tests passed: bounded producer cancellation and monophonic restart");
}
