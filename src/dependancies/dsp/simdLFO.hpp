#pragma once

using simd::float_4;

template <typename T>
struct LowFrequencyOscillator {
	T phase = 0.f;
	T pw = 0.5f;
	T freq = 1.f;
	bool polarPat = false;
	bool invert = false;
	T resetState = T::mask();

	void setPitch(T pitch) {
		pitch = simd::fmin(pitch, 16.f);
		freq = simd::pow(2.f, pitch);
	}
	void setPulseWidth(T pw_) {
		const T pwMin = 0.01f;
		this->pw = clamp(pw_, pwMin, 1.f - pwMin);
	}
	//Edited to include internal button state
	void setReset(T resetInternal, T resetExternal) {
		resetExternal = simd::rescale(resetExternal, 0.1f, 2.f, 0.f, 1.f);
		T on = ((resetInternal > 0.f) | (resetExternal > 0.f));
		T off = ((resetInternal <= 0.f) | (resetExternal <= 0.f));
		T triggered = ~resetState & on;
		resetState = simd::ifelse(off, 0.f, resetState);
		resetState = simd::ifelse(on, T::mask(), resetState);
		phase = simd::ifelse(triggered, 0.f, phase);
	}
	void step(float dt) {
		T deltaPhase = simd::fmin(freq * dt, 0.5f);
		phase += deltaPhase;
		phase -= (phase >= 1.f) & 1.f;
	}
	T sin() {
		T p = phase;
		if (!polarPat) p -= 0.25f;
		T v = simd::sin(2 * M_PI * p);
		if (invert) v *= -1.f;
		if (!polarPat) v += 1.f;
		return v;
	}
	T saw() {
		T p = phase;
		if (!polarPat) p -= 0.5f;
		T v = 2.f * (p - simd::round(p));
		if (invert) v *= -1.f;
		if (!polarPat) v += 1.f;
		return v;
	}
	T tri() {
		T p = phase;
		if (polarPat) p += 0.25f;
		T v = 4.f * simd::fabs(p - simd::round(p)) - 1.f;
		if (invert) v *= -1.f;
		if (!polarPat) v += 1.f;
		return v;
	}
	T sqr() {
		T v = simd::ifelse(phase < pw, 1.f, -1.f);
		if (invert) v *= -1.f;
		if (!polarPat) v += 1.f;
		return v;
	}
	T light() {
		return simd::sin(2 * T(M_PI) * phase);
	}
};
