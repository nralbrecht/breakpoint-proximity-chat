#pragma once

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include <DspFilters/Dsp.h>

#include "Logger.h"
#include "PluginConfig.h"


class RadioAudioProcessor {
private:
    double m_freq_low;
    double m_freq_hi;

    double m_o_freq_low;
    double m_o_freq_hi;

    float m_vol_follow = 0.0f;
    float m_vol_follow_r = 0.0f;

    std::unique_ptr<Dsp::Filter> f_m = std::make_unique< Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass <4>, 1, Dsp::DirectFormII> >(1024);
    std::unique_ptr<Dsp::Filter> f_s = std::make_unique< Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass <4>, 2, Dsp::DirectFormII> >(1024);

    std::unique_ptr<Dsp::Filter> f_m_o = std::make_unique< Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass <4>, 1, Dsp::DirectFormII> >(1024);
    std::unique_ptr<Dsp::Filter> f_s_o = std::make_unique< Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass <4>, 2, Dsp::DirectFormII> >(1024);

	// last freq settings
	std::pair<double, double> m_last_eq_in;		// low, high
	std::pair<double, double> m_last_eq_out;	// low, high

	std::vector<float> m_samples;
	std::vector<float> m_samples_r;

    void do_process(float *samples, int frame_count, float &volFollow, float destruction);
    float clampf(float min, float value, float max);
public:
    RadioAudioProcessor();

    void process(short* samples, int frame_count, int channels, float destruction);
};
