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
	base_ = instance;
}

void
Worker::PrepareDrop(Drop* drop) {
	//drop->Initialize(onset->GetAssociation(), onset->GetMaterial());
	drop->ProcessAudio();
	drop->ProcessEnvelope();
    dout << "Drop Prepared" << dndl;
}

void
Worker::Tick() {
    
	Onset* onset = base_->GetNextOnset();
    while (onset != 0) {
        PrepareDrop(onset->drop_);
        base_->ScheduleOnset(onset);
        onset = base_->GetNextOnset();
    }
}
