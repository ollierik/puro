// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "MainFrame.h"
#include "Puro.h"
#include "Drop.h"
#include "Worker.h"
#include "Idea.h"

Worker::Worker(MainFrame* instance) {
	//std::cout << "Worker" << std::endl;
	instance_ = instance;
}

Worker::~Worker() {
	// TODO Auto-generated destructor stub
}

void
Worker::PrepareDrop(Idea* onset, Drop* drop) {
	drop->Initialize(onset->GetAssociation(), onset->GetMaterial());
	drop->ProcessAudio(onset->GetAudioPassage());
	drop->ProcessEnvelope(onset->GetEnvelopePassage());
}

void
Worker::Tick() {

	Idea* onset = instance_->GetNextOnset();
	if (onset==0) {
		//std::cout << "no onset" << std::endl;
		return;
	}

	//std::cout << "Onset: " << onset << std::endl;

	Drop* free_drop = instance_->PopFreeDrop();
	if (!free_drop) {
		//std::cout << "no free drop" << std::endl;
		return;
	}

	//std::cout << "Worker Tick" << std::endl;

	PrepareDrop(onset, free_drop);
	instance_->ScheduleDrop(free_drop);
}
