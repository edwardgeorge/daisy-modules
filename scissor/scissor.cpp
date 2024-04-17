#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
Oscillator osc;
Switch toggle;

void configure_osc() {
	float coarse = fmap(hw.GetAdcValue(CV_1), 12, 84);
	float fine = fmap(hw.GetAdcValue(CV_2), 0, 10);
	float cvmod = fmap(hw.GetAdcValue(CV_5), 0, 60); // 1v/8
	float midi_nn = fclamp(coarse + fine + cvmod, 0.f, 127.f);
    float freq  = mtof(midi_nn);
	osc.SetFreq(freq);
	float pw_knob = fmap(hw.GetAdcValue(CV_3), 0.1, 0.9);
	float pw_mod = fmap(hw.GetAdcValue(CV_6), -0.9, 0.9);
	float pw = fclamp(pw_knob + pw_mod, 0.1, 0.9);
	osc.SetPw(pw);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	toggle.Debounce();
	bool toggle_on = toggle.Pressed();
	configure_osc();
	for (size_t i = 0; i < size; i++)
	{
		float sig = osc.Process();
		OUT_L[i] = sig < 0 ? IN_R[i] : IN_L[i];
		OUT_R[i] = toggle_on ? sig : sig < 0 ? IN_L[i] : IN_R[i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	// hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
	osc.Init(hw.AudioSampleRate());
	osc.SetWaveform(Oscillator::WAVE_SQUARE);
	osc.SetAmp(1);
	osc.SetPw(0.5);
	toggle.Init(DaisyPatchSM::B8, hw.AudioCallbackRate());
	hw.StartAudio(AudioCallback);
	while(1) {}
}
