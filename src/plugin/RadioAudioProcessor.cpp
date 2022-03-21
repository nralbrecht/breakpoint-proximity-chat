#include "RadioAudioProcessor.h"


#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif
const double TWO_PI_OVER_SAMPLE_RATE = 2*M_PI/48000;

RadioAudioProcessor::RadioAudioProcessor() {
    Dsp::Params params;
    params[0] = 48000; // sample rate
    params[1] = 4; // order
    params[2] = 1600; // center frequency
    params[3] = 1300; // band width

    f_m->setParams(params);
    f_s->setParams(params);
    f_m_o->setParams(params);
    f_s_o->setParams(params);

	m_samples.reserve(480);
	m_samples_r.reserve(480);

    m_freq_low = PluginConfig::get()->getValue<double>("freq_low_in");
    m_freq_hi = PluginConfig::get()->getValue<double>("freq_hi_in");
    m_o_freq_low = PluginConfig::get()->getValue<double>("freq_low_out");
    m_o_freq_hi = PluginConfig::get()->getValue<double>("freq_hi_out");
}

float RadioAudioProcessor::clampf(float min, float value, float max) {
    if (value < min) {
        return min;
    }
    else if (value > max) {
        return max;
    }
    else {
        return value;
    }
}

void RadioAudioProcessor::do_process(float *samples, int frame_count, float &volFollow, float destruction)
{
    // ALL INPUTS AND OUTPUTS IN THIS ARE -1.0f and +1.0f

    // Find volume of current block of frames...
    float vol = 0.0f;
    // float min = 1.0f, max = -1.0f;
    for (int i = 0; i < frame_count; i++) {
       vol += (samples[i] * samples[i]);
    }
    vol /= (float)frame_count;

    // Fudge factor, inrease for more noise
    vol *= destruction;

    // Smooth follow from last frame, both multiplies add up to 1...
    volFollow = volFollow * 0.5f + vol * 0.5f;

    // Between -1.0f and 1.0f...
    float random = (((float)(rand()&32767)) / 16384.0f) - 1.0f;

    // Between 1 and 128...
    int count = (rand() & 127) + 1;
    float temp;
    for (int i = 0; i < frame_count; i++)
    {
        if (!count--)
        {
            // Between -1.0f and 1.0f...
            random = (((float)(rand() & 32767)) / 16384.0f) - 1.0f;
            // Between 1 and 128...
            count = (rand() & 127) + 1;
        }

        // Add random to inputs * by current volume;
        temp = samples[i] + random * volFollow;

        // Make it an integer between -60 and 60
        temp = trunc(temp * 40.0f);

        // Drop it back down but massively quantised and too high
        temp = (temp / 40.0f);
        temp *= 0.05f * destruction;
        temp += samples[i] * (1 - (0.05f * destruction));
        samples[i] = clampf(-1.0f, temp, 1.0f);
    }
}

double calculate_bandwidth(double low_frequency, double high_frequency) {
    return high_frequency - low_frequency;
}

double calculate_center_frequency(double low_frequency, double high_frequency) {
    return low_frequency + (calculate_bandwidth(low_frequency, high_frequency) / 2.0);
}

void update_filter_frequencies(
    double low_frequency,
    double high_frequency,
    std::pair<double, double>& last,
    const std::unique_ptr<Dsp::Filter>& mono_filter,
    const std::unique_ptr<Dsp::Filter>& stereo_filter)
{
    if (last.first == low_frequency && last.second == high_frequency)
        return;

    // calculate center frequency and bandwidth
    const auto new_center_frequency = calculate_center_frequency(low_frequency, high_frequency);
    const auto new_bandwidth = calculate_bandwidth(low_frequency, high_frequency);

    if (mono_filter) {
        mono_filter->setParam(2, new_center_frequency); // center frequency
        mono_filter->setParam(3, new_bandwidth); // bandwidth
    }
    if (stereo_filter) {
        stereo_filter->setParam(2, new_center_frequency); // center frequency
        stereo_filter->setParam(3, new_bandwidth); // bandwidth
    }

    last.first = low_frequency;
    last.second = high_frequency;
}

void RadioAudioProcessor::process(short *samples, int frame_count, int channels, float destruction) {
    try
    {
        // update filters if necessary
        update_filter_frequencies(m_freq_low, m_freq_hi, m_last_eq_in, f_m, f_s);
        update_filter_frequencies(m_o_freq_low, m_o_freq_hi, m_last_eq_out, f_m_o, f_s_o);

        if (channels == 1)
        {
            m_samples.resize(frame_count);
            for (int i=0; i < frame_count; ++i) {
                m_samples[i] = samples[i] / 32768.f;
            }

            float* audioData[1];
            audioData[0] = m_samples.data();
            f_m->process(frame_count, audioData);

            if (destruction > 0.0f) {
                do_process(m_samples.data(), frame_count, m_vol_follow, destruction);
            }

            f_m_o->process(frame_count, audioData);

            for (int i = 0; i < frame_count; ++i) {
                samples[i] = static_cast<int16_t>(m_samples[i] * 32768.f);
            }
        }
        else if (channels == 2)
        {
            // Extract from interleaved and convert to QVarLengthArray<float>
            m_samples.resize(frame_count);
            m_samples_r.resize(frame_count);

            for (int i = 0; i < frame_count; ++i)
            {
                m_samples[i] = samples[i*2] / 32768.f;
                m_samples_r[i] = samples[1 + (i*2)] / 32768.f;
            }

            float* audioData[2];
            audioData[0] = m_samples.data();
            audioData[1] = m_samples_r.data();
            f_s->process(frame_count, audioData);

            if (destruction > 0.0f) {
                do_process(m_samples.data(), frame_count, m_vol_follow, destruction);
                do_process(m_samples_r.data(), frame_count, m_vol_follow_r, destruction);
            }

            f_s_o->process(frame_count, audioData);

            for (int i = 0; i < frame_count; ++i)
            {
                samples[i*2] = static_cast<int16_t>(m_samples[i] * 32768.f);
                samples[1 + (i*2)] = static_cast<int16_t>(m_samples_r[i] * 32768.f);
            }
        }
        Logger::get()->LogF(LoggerLogLevel::Verbose, "RadioAudioProcessor::process done");
    }
    catch(const std::exception& e)
    {
        Logger::get()->LogF(LoggerLogLevel::Error, "RadioAudioProcessor::process error: %s", e.what());
    }
}
