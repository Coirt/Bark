#pragma once


struct LowFrequencyOscillator {
	float phase = 0.f;
	float pw = 0.5f;
	float freq = 1.f;
	bool offset = false;
	bool invert = false;
	dsp::SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {}

	void setPitch(float pitch) {
		pitch = std::fmin(pitch, 8.f);
		freq = std::pow(2.f, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset / 0.01f)) {
			phase = 0.f;
		}
	}
	void step(float dt) {
		float deltaPhase = std::fmin(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.f)
			phase -= 1.f;
	}
	float sin() {
		if (offset)
			return 1.f - std::cos(2 * M_PI * phase) * (invert?-1.f:1.f);
		else
			return std::sin(2 * M_PI * phase) * (invert?-1.f:1.f);
	}
	float tri(float x) {
		return 4.f * std::fabs(x - std::round(x));
	}
	float tri() {
		if (offset)
			return tri(invert?phase - 0.5f:phase);
		else
			return -1.f + tri(invert?phase - 0.25f:phase - 0.75f);
	}
	float saw(float x) {
		return 2.f * (x - std::round(x));
	}
	float saw() {
		if (offset)
			return invert?2.f * (1.f - phase):2.f * phase;
		else
			return saw(phase) * (invert?-1.f:1.f);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert?1.f:-1.f;
		return offset?sqr + 1.f:sqr;
	}
	float light() {
		return std::sin(2 * M_PI * phase);
	}
};