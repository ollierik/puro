// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Puro.h"
#include "Buffer.h"
#include "Drop.h"
#include "Passage.h"
#include "PuroBase.h"
#include "Idea.h"

Drop::Drop(PuroBase *instance, uint32_t buffer_size) {
	// TODO Auto-generated constructor stub

	//std::cout << "Drop" << std::endl;
	base_ = instance;
	idea_ = 0;
	material_ = 0;
	envelope_ = new Buffer(buffer_size);
	audio_ = new Buffer(buffer_size);
    //Passage audio_passage_ = new Passage(;
    //Passage envelope_passage_;
}

Drop::~Drop() {
	delete audio_;
	delete envelope_;
}

void
Drop::Initialize(Idea* idea) {
	idea_ = idea->GetAssociation();
	material_ = idea->GetMaterial();
    
    audio_passage_ = idea->GetAudioPassage();
    envelope_passage_ = idea->GetEnvelopePassage();
    audio_passage_->RegisterRefrence();
    envelope_passage_->RegisterRefrence();
    
	audio_->Clear();
	envelope_->Clear();
}


int32_t
Drop::ProcessAudio() {
	//std::cout << "Prepare drop" << std::endl;
	// [0] 14000 2250 16250 linear
	// [0] 14000 2250 15125 half-time
	// [0] 14000 2250 11750 inverse

	//std::cout << "mat size: " << base_->GetAudioSize(material_) << std::endl;

	if (audio_passage_->GetSize() < 2)
		return 0; // kill-signal

	// GET MATERIAL
	float* material_data = base_->GetAudioData(material_);
	//std::cout << "Got material data:" << material_data << std::endl;
	if (material_data == 0)
		return 0; // kill-signal
	uint32_t material_size = base_->GetAudioSize(material_);
	uint32_t sample_rate = GetMaterialSampleRate();

	// RESIZE BUFFER
	float t0 = audio_passage_->GetTime(0);
	float t1 = audio_passage_->GetTime(audio_passage_->GetSize()-1);
	uint32_t duration = (uint32_t)(sample_rate*(t1-t0)); // this is absolute
	audio_->SetSize(duration);

	/////////////////////////////////////////////////
	// INSERT SPLITTING FUNCTIONALITY HERE!
	/////////////////////////////////////////////////

	// FILL BUFFER SEGMENT AT A TIME
	for (uint16_t i=1; i < audio_passage_->GetSize(); i++) {

		uint32_t t0 = audio_passage_->GetTime(i-1) * sample_rate;
		uint32_t t1 = audio_passage_->GetTime(i) * sample_rate;
		float s0 = audio_passage_->GetValue(i-1) * sample_rate;
		float s1 = audio_passage_->GetValue(i) * sample_rate;
		//std::cout << "i: " << i << "/" << audio->GetSize()-1 << " from s0: " << s0 << " to s1: " << s1 << std::endl;
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
    audio_passage_->RemoveRefrence();
	return 1;
}

int32_t
Drop::ProcessEnvelope() {
	// [0] 0.25 0.75 0.5 1.0 0.75 0.75 1.0 0.0

	if (envelope_passage_->GetSize() < 2)
		return 0; // kill-signal

	// FILL BUFFER SEGMENT AT A TIME
	for (uint16_t i=1; i < envelope_passage_->GetSize(); i++) {

		uint32_t fx = envelope_passage_->GetTime(i-1) * GetDurationInSamples();
		uint32_t tx = envelope_passage_->GetTime(i) * GetDurationInSamples();
		float fy = envelope_passage_->GetValue(i-1);
		float ty = envelope_passage_->GetValue(i);
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
    envelope_passage_->RemoveRefrence();
	return 0;
}

uint32_t
Drop::GetMaterialSampleRate() {
	return base_->GetMaterialSampleRate(material_);
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
Drop::GetAudio(float* buffer, uint32_t index, uint32_t n, uint32_t offset) {
//Drop::GetAudio(float* buffer, uint32_t index, uint32_t n) {
	uint32_t last = index + n - 1;
	uint32_t duration = GetDurationInSamples();
	if (last > duration)
		n = duration - index;

	//std::cout<< "Get Audio, index: " << index << " n: " << n << " duration: " << duration << std::endl;

	//for (uint32_t i=offset; i<n; i++) {
	for (uint32_t i=0; i<n; i++) {
		//float f = audio_->GetValue(index + i) * envelope_->GetValue(index + i);
		float f = envelope_->GetValue(index + i);
		//std::cout << "i: " << i << " f: " << f << std::endl;
		buffer[i + offset] += f;
	}
	//std::cout << "n summed: " << n << std::endl;
	return n;
}

Time Drop::GetOnsetTime() {
    return onset_time_;
}
