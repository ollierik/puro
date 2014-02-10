// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#pragma once

#include "Puro.h"

class Drop {
	PuroBase* base_;
	Tag idea_;
	Tag material_;
    Time onset_time_;
	//Drop* next_;
    Passage* audio_passage_;
    Passage* envelope_passage_;
	Buffer* audio_;
	Buffer* envelope_;

	uint32_t GetMaterialSampleRate();
	uint32_t GetDurationInSamples();
public:
	Drop(PuroBase* instance, uint32_t buffer_size);
	~Drop();
    
	uint32_t GetAudio(uint32_t index, uint32_t n, float* buffer);
    Time GetOnsetTime();
    
	//void Initialize(Tag idea, Tag material, Passage* audio, Passage* envelope);
    void Initialize(Idea* idea);
	int32_t ProcessAudio();
	int32_t ProcessEnvelope();
};
