/*
MIT License

Copyright (c) 2024 Marek Mach (Bastl Instruments)
Copyright (c) 2024 Vaclav Mach (Bastl Instruments)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include "common/EnumTools.hpp"
#include "common/controls/DirtyInputsHandler.hpp"
#include "common/controls/FancyMode.hpp"
#include "common/controls/FancyPot.hpp"
#include "common/core/App.hpp"
#include "common/core/Hardware.hpp"
#include "common/core/midi/Message.hpp"
#include "common/core/midi/NoteSender.hpp"
#include "common/dsp/control/AdsrEnv.hpp"
#include "common/dsp/control/EnvelopeFollower.hpp"
#include "common/dsp/effects/HardClipper.hpp"
#include "common/dsp/effects/SoftClipper.hpp"
#include "common/dsp/filters/DjFilterStereo.hpp"
#include "common/dsp/synthesis/Oscillator.hpp"
#include "common/dsp/utility/AdvancedDynamicDelayLine.hpp"
#include "common/dsp/utility/AutoFreeze.hpp"
#include "common/dsp/utility/Quantizer.hpp"
#include "common/dsp/utility/Slewer.hpp"
#include "common/fastcode.hpp"
#include "common/peripherals/WS2812.hpp"
#include "PresetFile.hpp"
#include "SpeechVoice.hpp"
#include "cc.hpp"

namespace kastle2
{

class AppTalkingSynth : public virtual App
{
public:
    void Init();

    void DeInit();

    FASTCODE void AudioLoop(q15_t *input, q15_t *output, size_t size);

    void UiLoop();

    void SecondCoreWorker();

    void MemoryInitialization();

    void MidiCallback(midi::Message *msg);

    uint8_t GetId()
    {
        return 0x71; // Custom application ID; Bastl IDs are reserved.
    }

private:
    bool inited_ = false;

    static constexpr Hardware::AnalogInput CV_PITCH_FREE = Hardware::AnalogInput::PITCH_1;
    static constexpr Hardware::AnalogInput CV_PITCH_STEP = Hardware::AnalogInput::PITCH_2;
    static constexpr Hardware::AnalogInput CV_SAMPLE = Hardware::AnalogInput::PARAM_1;
    static constexpr Hardware::AnalogInput CV_ENVELOPE = Hardware::AnalogInput::PARAM_3;
    static constexpr Hardware::AnalogInput CV_BANK = Hardware::AnalogInput::MODE;

    static constexpr size_t kMemBank = Memory::ADDR_APP_SPACE + 0x0;
    static constexpr size_t kMemAudioRouting = Memory::ADDR_APP_SPACE + 0x1;
    static constexpr size_t kMemPotFine = Memory::ADDR_APP_SPACE + 0x2;
    static constexpr size_t kMemPotFilter = Memory::ADDR_APP_SPACE + 0x3;
    static constexpr size_t kMemPotFX = Memory::ADDR_APP_SPACE + 0x4;
    static constexpr size_t kMemPotScale = Memory::ADDR_APP_SPACE + 0x5;
    static constexpr size_t kMemPotRoot = Memory::ADDR_APP_SPACE + 0x6;
    static constexpr size_t kMemPotLengthMod = Memory::ADDR_APP_SPACE + 0x7;
    static constexpr size_t kMemPotBankMod = Memory::ADDR_APP_SPACE + 0x8;

    enum class Pot
    {
        SAMPLE,
        SAMPLE_MOD,
        PITCH,
        PITCH_MOD,
        PITCH_SCALE,
        PITCH_ROOT,
        PITCH_FINE,
        PITCH_QUANTIZED,
        ENVELOPE,
        ENVELOPE_MOD,
        FX,
        FILTER,
        AUDIO_ROUTE,
        COUNT
    };

    enum class PitchSource
    {
        PATCH,
        MIDI
    };

    PitchSource pitch_source_ = PitchSource::PATCH;

    void UpdateQuantizedPot();

    FancyMode bank_select_ = FancyMode(FancyMode::Config{
        .memory_addr = kMemBank,
        .midi_cc = cc::BANK,
        .midi_attenuator_cc = cc::BANK_MOD,
        .midi_output_cc = cc::OUT_BANK});

    int32_t fx_volume_compensation_ = Q15_ZERO;

    EnumArray<Pot, std::unique_ptr<FancyPot>> pots_;

    float base_pitch_prev_ = 0;

    float trigger_base_pitch_prev_ = 0;

    int32_t quantizer_scale_prev_ = 0;

    int32_t quantizer_root_prev_ = 0;

    int32_t pitch_note_cv_ = 0;

    int32_t pitch_note_cv_prev_ = 0;

    bool quantization_enabled_ = false;

    int32_t trigger_detect_pitch_prev_ = 0;

    size_t sample_bank_selected_ = 0;

    size_t sample_num_selected_ = 0;

    size_t sample_num_selected_prev_ = 0;

    static constexpr int32_t kSampleChangeThreshold = 100;

    int32_t sample_cv_;

    int32_t prev_sample_cv_;

    bool sample_play_on_change_;

    int32_t sticky_sample_mod_ = INT32_MAX;

    int32_t sticky_sample_continous_mod_ = INT32_MAX;

    int32_t lfo_mod_octave_prev_ = 0;
    int32_t lfo_mod_scale_prev_ = 0;
    int32_t lfo_mod_root_prev_ = 0;

    int32_t prev_base_envelope_ = 0;

    int32_t envelope_cv_;

    size_t ui_indicate_change_time_ = 0;

    size_t ui_loop_counter_ = 0;

    static constexpr size_t kUiLoopCounterMax = 8;

    q15_t delay_feedback_ = Q15_ZERO;

    q15_t delay_mix_ = Q15_ZERO;

    int32_t delay_stereo_left_ = 0;

    int32_t delay_stereo_right_ = 0;

    int32_t prev_delay_left_val_ = 0;

    int32_t prev_delay_right_val_ = 0;

    q15_t fx_compressor_max_ = Q15_ZERO;

    size_t delay_ticks_ = 0;

    AdsrEnv envelope_;

    AdsrEnv envelope_indicator_;

    AdsrEnv envelope_out_;

    talking::SpeechVoice speech_;

    Quantizer quantizer_;

    DjFilterStereo filter_;

    Slewer filter_volume_compensation_slewer_;

    SoftClipper playback_clipper_;

    SoftClipper fx_input_clipper_;

    SoftClipper soft_clipper_;

    std::unique_ptr<AdvancedDynamicDelayLine<q15least_t>> delay_left_;

    std::unique_ptr<AdvancedDynamicDelayLine<q15least_t>> delay_right_;

    EnvelopeFollower fx_compressor_;

    void inline UiIndicateChange();

    FASTCODE void ProcessEffects(size_t index);

    size_t buffer_size_ = 0;

    q15_t *input_buffer_;

    q15_t *output_buffer_;

    inline void UpdateSelectedBank();

    inline void UpdateSelectedSample();

    inline size_t GetPatchedSample();

    inline size_t GetContinousPatchedSample();

    inline void Trigger(bool manual = false);

    inline void TriggerCheck();

    FASTCODE void ActualTrigger(bool force);

    inline uint32_t GetColor(size_t bank) const;

    EdgeDetector trigger_ = EdgeDetector(EdgeDetector::Type::RISING);

    bool triggered_ = false;

    bool trigger_was_manual_ = false;

    uint64_t trigger_spacing_us_;

    absolute_time_t last_trigger_time_;

    bool input_audio_through_fx_ = false;

    int32_t compress_amount_ = Q15_MAX;

    talking::PresetFile presets_;
    Quantizer::Scale scales_[32]{};
    TriggerGenerator::Rhythm rhythms_[64]{};

    bool LoadPresets();

    Oscillator chorus_lfo_;

    int32_t chorus_lfo_val_ = 0;

    DirtyInputsHandler<4> dirty_inputs_handler_{
        std::array<Hardware::AnalogInput, 4>{
            Hardware::AnalogInput::PITCH_2, // NOTE
            Hardware::AnalogInput::PARAM_1, // SAMPLE MOD
            Hardware::AnalogInput::MODE,    // BANK
            Hardware::AnalogInput::PARAM_3, // LENGTH
        }};

    // MIDI constants

    static constexpr uint32_t kMidiMinNote = 48;

    static constexpr uint32_t kMidiTriggerOriginalPitchBelow = 24;

    static constexpr uint32_t kMidiNoteControlRepeat = 12;

    static constexpr uint32_t kMidiBaseNote = 72;

    static constexpr float kPitchBendRange = 7.0f;

    // MIDI variables

    uint32_t midi_note_ = 0;

    float midi_pitch_bend_multiplier_ = 1.0f;

    uint8_t patch_pitch_midi_note_ = 0;

    int32_t prev_forward_length_cc_ = INT32_MAX;

    int32_t prev_backward_length_cc_ = INT32_MAX;

    bool next_time_send_note_on_ = false;

    midi::NoteSender note_sender_;

    void SendMidiLength(bool force);

    void SendMidiCombo(bool force);

    void SendMidiBank(bool force);

    void SendMidiSample(bool force);
};
}
