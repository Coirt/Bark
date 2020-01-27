#pragma once
#include "../utility/fasttrigo.h"

using namespace FT;

//pseudo---------------------------------------
		//float constPowPan(float position) {
		//	PANPOS pos;
		//	/* pi/2: 1/4 cycle of a sinusoid */
		//	const double  piovr2 = 4.0 * atan(1.0) * 0.5;		//	M_PI_2 == 1.57079632679489661923
		//	const double  root2ovr2 = sqrt(2.0) * 0.5;			//	M_SQRT1_2 == 0.707106781186547524401
		//	params[PAN_PARAM].getValue();//±0.707106781186547524401
		//	/* scale position to fit the pi/2 range */
		//	double thispos = position * piovr2;			// paramvalue min == -M_SQRT1_2, max == M_SQRT1_2, thispos == ±1.1107207345395915617532801458281
		//	/* each channel uses a 1/4 of a cycle */
		//	double angle = thispos * 0.5;				// 1.1107207345395915617532801458281 / 2 == ±0.55536036726979578087664007291405

		//	pos.left = root2ovr2 * (cos(angle) - sin(angle));
		//	pos.right = root2ovr2 * (cos(angle) + sin(angle));
		//	return pos;
		//}
	//pseudo---------------------------------------


	/***Constant Power Pan ---
	Referenced Richard Dobson, Antonio Grazioli(Autodafe)/Alfredo Santamaria(AS)
	*/

	/*
	float cpPanL(float bal, float cv) {//Left Signal
		float position = bal + cv / 5, power;
		float thisPos = simd::clamp(position, -1.f, 1.f) * M_PI_2;	//min -1.57 max 1.57
		float angle = thisPos / 2;	//min -0.785 max 0.785
		power = M_SQRT1_2 * (simd::cos(angle) - simd::sin(angle));
		return power;
	}

	float cpPanR(float bal, float cv) {//Right Signal
		float position = bal + cv / 5, power;
		float thisPos = simd::clamp(position, -1.f, 1.f) * M_PI_2;
		float angle = thisPos / 2;
		power = M_SQRT1_2 * (simd::cos(angle) + simd::sin(angle));
		return power;
	}
	*/

inline float cpPanL(float bal, float cv) {//Left Signal
	float position = 0.f, thisPos = 0.f, angle = 0.f, power = 0.f;
	position = bal + cv / 5;
	thisPos = clamp(position, -1.f, 1.f) * M_PI_2;	//min -1.57 max 1.57
	angle = thisPos / 2;								//min -0.785 max 0.785
	//power = M_SQRT1_2 * (FT::cos(angle) - FT::sin(angle));
	//power = M_SQRT1_2 * (std::cos(angle) - std::sin(angle));
	//power = M_SQRT1_2 * (simd::cos(angle) - simd::sin(angle));
	//power = M_SQRT1_2 * (cos(angle) - sin(angle));
	//power = M_SQRT1_2 * (cosh(angle) - sinh(angle));
	//power = std::sin((1 - position) / 2 * thisPos);
	//power = sin((1 - position) / 2 * thisPos);
	power = FT::sin((1 - position) / 2);
	//power = std::sqrt((1 - position) / 2);
	//power = simd::sqrt((1 - position) / 2);
	return power;
}

inline float cpPanR(float bal, float cv) {//Right Signal
	float position = 0.f, thisPos = 0.f, angle = 0.f, power = 0.f;
	position = bal + cv / 5;
	thisPos = clamp(position, -1.f, 1.f) * M_PI_2;
	angle = thisPos / 2;
	//power = M_SQRT1_2 * (FT::cos(angle) + FT::sin(angle));				//cpu FTA: 3.0%, 0.68us | FT: 2.6%, 0.60us
	//power = M_SQRT1_2 * (std::cos(angle) + std::sin(angle));			//cpu: 3.0%, 0.69us
	//power = M_SQRT1_2 * (simd::cos(angle) + simd::sin(angle));			//2.2%, 0.51us (1 ch)
	//power = M_SQRT1_2 * (cos(angle) + sin(angle));						//cpu: 3.4%, 0.76us
	//power = M_SQRT1_2 * (cosh(angle) + sinh(angle));					//cpu: 2.8%, 0.62us	NOTE: Hyperbolic
	//power = std::sin((1 + position) / 2 * thisPos);					//cpu: 3.0%, 0.68us
	//power = sin((1 + position) / 2 * thisPos);							//cpu: %, us
	power = FT::sin((1 + position) / 2);									//cpu FT: 1.4%, 0.32us | FTA 1.5%, 0.33us
	//power = std::sqrt((1 + position) / 2);								//cpu: 0.4%, 0.10us NOTE: Extreme L/R is more linear while towards centre is similar to sin
	//power = simd::sqrt((1 + position) / 2);							//cpu: 0.4%, 0.09us	NOTE: ↑

	return power;
}
