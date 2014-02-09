// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Interpreter.h"
#include "Idea.h"
#include "PuroBase.h"

Interpreter::Interpreter(PuroBase* instance) {
	//std::cout << "Interpreter" << std::endl;
	instance_ = instance;
}

// TODO ACTUALLY INTERFACE THESE
void
Interpreter::SetMaterial(Tag idea, Tag material) {
	Idea* idea_to_use = instance_->GetIdea(idea);
	idea_to_use->SetMaterial(material);
}

void
Interpreter::SetSync(Tag idea) {
    instance_->SyncIdea(idea);
}

void
Interpreter::SetAudioPassage(Tag idea, uint16_t n_data, float* data) {

	Idea* idea_to_use = instance_->GetIdea(idea);
	Passage* passage_to_use = idea_to_use->DetachAudioPassage();
	FloatListToPassage(passage_to_use, n_data, data);
}

void
Interpreter::SetEnvelopePassage(Tag idea, uint16_t n_data, float* data) {

	Idea* idea_to_use = instance_->GetIdea(idea);
	Passage* passage_to_use = idea_to_use->GetEnvelopePassage();
	FloatListToPassage(passage_to_use, n_data, data);
}

void
Interpreter::OnsetDropFromIdea(Tag idea, Time time) {
	//std::cout << "Onset drop from idea" << std::endl;
	instance_->OnsetDrop(idea, time);
}
void
Interpreter::LoadAudioMaterial(Tag association, char* path_to_file) {
	instance_->LoadAudioMaterial(association, path_to_file);
}
