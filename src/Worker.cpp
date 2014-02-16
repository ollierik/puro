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
#include "Passage.h"

Worker::Worker(PuroBase* instance) {
	//std::cout << "Worker" << std::endl;
	base_ = instance;
}

void
Worker::PrepareDrop(Onset* onset) {
	//drop->Initialize(onset->GetAssociation(), onset->GetMaterial());
    
    
    onset->drop_->ProcessAudio(onset->material_, onset->audio_passage_);
    onset->drop_->ProcessEnvelope(onset->envelope_passage_);
    
    onset->audio_passage_->RemoveReference();
    onset->audio_passage_ = 0;
    onset->envelope_passage_->RemoveReference();
    onset->envelope_passage_ = 0;
	//drop->ProcessAudio();
	//drop->ProcessEnvelope();
    dout << "Drop Prepared" << dndl;
}

/*
 Get free drod, get next onset, prepare it, schedule it"
*/
void
Worker::Tick() {
    
    while (base_->HasFreeDrops()) {
        Onset* onset = base_->GetNextOnset();
        if (onset == 0)
            break;
        onset->drop_ = base_->PopFreeDrop();
        PrepareDrop(onset);
        base_->ScheduleOnset(onset);
    }
}
