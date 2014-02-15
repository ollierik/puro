// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Idea.h"

Idea::Idea() {
	//std::cout << "Idea" << std::endl;
	Initialize();
}

void
Idea::Initialize() {
	association_ = 0;
	material_ = 0;
    audio_passage_ = 0;
    envelope_passage_ = 0;
}

Tag
Idea::GetAssociation() {
	return association_;
}
Tag
Idea::GetMaterial() {
	return material_;
}

Passage*
Idea::GetAudioPassage() {
	return audio_passage_;
}
Passage*
Idea::GetEnvelopePassage() {
	return envelope_passage_;
}

void
Idea::SetAssociation(Tag association) {
	association_ = association;
}
void
Idea::SetMaterial(Tag material) {
	material_ = material;
}

Time
Idea::GetTimeOffset() {
    return time_offset_;
}
void
Idea::SetTimeOffset(Time current) {
    time_offset_ = current;
}

void
Idea::SetAudioPassage(Passage* audio) {
    if (audio_passage_ != 0)
        audio_passage_->RemoveReference();
	audio_passage_ = audio;
}
void
Idea::SetEnvelopePassage(Passage* envelope) {
    if (envelope_passage_ != 0)
        envelope_passage_->RemoveReference();
	envelope_passage_ = envelope;
}

bool
Idea::IsValid() {
    if (!(audio_passage_ && envelope_passage_))
        return false;
	bool is_valid = (association_) && (material_)
			&& (audio_passage_->GetSize()>1) && (envelope_passage_->GetSize()>1);
	return is_valid;
}
