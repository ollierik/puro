// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "PuroBase.h"
#include "Puro.h"
#include "Drop.h"
#include "Worker.h"

Worker::Worker(PuroBase* instance) {
	//std::cout << "Worker" << std::endl;
	instance_ = instance;
}

Worker::~Worker() {
	// TODO Auto-generated destructor stub
}

void
Worker::PrepareDrop(Drop* drop) {
	//drop->Initialize(onset->GetAssociation(), onset->GetMaterial());
	drop->ProcessAudio();
	drop->ProcessEnvelope();
}

void
Worker::Tick() {

	Drop* onset = instance_->GetNextOnset();
	if (onset==0) {
		//std::cout << "no onset" << std::endl;
		return;
	}

	//std::cout << "Worker Tick" << std::endl;

	PrepareDrop(onset);
	instance_->ScheduleDrop(free_drop);
}
