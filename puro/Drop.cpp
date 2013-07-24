/*
 * Drop.cpp
 *
 *  Created on: Jul 11, 2013
 *      Author: oek
 */

#include "Puro.h"
#include "Buffer.h"
#include "Drop.h"
#include "Passage.h"
#include "MainFrame.h"

Drop::Drop(MainFrame *instance, uint32_t buffer_size) {
	// TODO Auto-generated constructor stub

	std::cout << "Drop" << std::endl;
	instance_ = instance;
	idea_ = 0;
	material_ = 0;
	envelope_ = new Buffer(buffer_size);
	audio_ = new Buffer(buffer_size);
}

Drop::~Drop() {
	delete audio_;
	delete envelope_;
}

void
Drop::Initialize(Tag idea, Tag material) {
	idea_ = idea;
	material_ = material;
	audio_->Clear();
	envelope_->Clear();
}


int32_t
Drop::ProcessAudio(Passage* audio) {
	std::cout << "Prepare drop" << std::endl;
	// [0] 14000 2250 16250 linear
	// [0] 14000 2250 15125 half-time
	// [0] 14000 2250 11750 inverse

	std::cout << "mat size: " << instance_->GetAudioSize(material_) << std::endl;

	if (audio->GetSize() < 2)
		return 0; // kill-signal

	// GET MATERIAL
	float* material_data = instance_->GetAudioData(material_);
	std::cout << "Got material data:" << material_data << std::endl;
	if (material_data == 0)
		return 0; // kill-signal
	uint32_t material_size = instance_->GetAudioSize(material_);
	uint32_t sample_rate = GetMaterialSampleRate();

	// RESIZE BUFFER
	float t0 = audio->GetTime(0);
	float t1 = audio->GetTime(audio->GetSize()-1);
	uint32_t duration = (uint32_t)(sample_rate*(t1-t0)); // this is absolute
	audio_->SetSize(duration);

	/////////////////////////////////////////////////
	// INSERT SPLITTING FUNCTIONALITY HERE!
	/////////////////////////////////////////////////

	// FILL BUFFER SEGMENT AT A TIME
	for (uint16_t i=1; i < audio->GetSize(); i++) {

		uint32_t t0 = audio->GetTime(i-1) * sample_rate;
		uint32_t t1 = audio->GetTime(i) * sample_rate;
		float s0 = audio->GetValue(i-1) * sample_rate;
		float s1 = audio->GetValue(i) * sample_rate;
		std::cout << "i: " << i << "/" << audio->GetSize()-1 << " from s0: " << s0 << " to s1: " << s1 << std::endl;
		uint32_t t = t0;

		float x;
		float coef = (s1-s0)/(float)(t1-t0);

		// GET SAMPLE POINTS FOR THE SEGMENT
		while (t<t1) {
			x = (float)(t-t0);
			uint32_t index = (uint32_t)(s0 + x*coef);

			float f;
			if (index < material_size)
				f = material_data[index]; // INTERPOLATE HERE
			else
				f = 0.f;

			audio_->SetValue(t, f);
			t++;
		}

	}
/*
	for (uint32_t a=0; a<duration; a++) {
		float f = audio_->GetValue(a);
		std::cout << "a: " << a << " f: " << f << std::endl;
	}
*/
	return 1;
}

int32_t
Drop::ProcessEnvelope(Passage* envelope) {
	// [0] 0.25 0.75 0.5 1.0 0.75 0.75 1.0 0.0

	if (envelope->GetSize() < 2)
		return 0; // kill-signal

	// FILL BUFFER SEGMENT AT A TIME
	for (uint16_t i=1; i < envelope->GetSize(); i++) {

		uint32_t fx = envelope->GetTime(i-1) * GetDurationInSamples();
		uint32_t tx = envelope->GetTime(i) * GetDurationInSamples();
		float fy = envelope->GetValue(i-1);
		float ty = envelope->GetValue(i);
		uint32_t x = fx;

		// GET ENVELOPE POINTS FOR THE SEGMENT
		while (x<tx) {
			float f = fy + (float)(x-fx)/(float)(tx-fx) * (ty-fy);
			envelope_->SetValue(x, f);
			x++;
		}
		// if the list is not properly formed to have 100% as the last time
		while (x < GetDurationInSamples()) {
			envelope_->SetValue(x, 0.f); // DEBUG WELL, NO LEAK HERE
			x++;
		}
	}
	return 0;
}

uint32_t
Drop::GetMaterialSampleRate() {
	return instance_->GetMaterialSampleRate(material_);
}

uint32_t
Drop::GetDurationInSamples() {
	return audio_->GetSize();
}

/**
 * Caller is in charge of taking care, that the buffer has enough space and that
 * it has been initialized.
 * Caller should take care that correct index is given.
 */
uint32_t
Drop::GetAudio(uint32_t index, uint32_t n, float* buffer) {
	uint32_t last = index + n - 1;
	uint32_t duration = GetDurationInSamples();
	if (last > duration)
		n = duration - index;

	std::cout<< "Get Audio, index: " << index << " n: " << n << " duration: " << duration << std::endl;

	for (uint32_t i=0; i<n; i++) {
		float f = audio_->GetValue(index + i) * envelope_->GetValue(index + i);
		//std::cout << "i: " << i << " f: " << f << std::endl;
		buffer[i] += f;
	}
	std::cout << "n summed: " << n << std::endl;
	return n;
}
