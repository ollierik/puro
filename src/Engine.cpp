// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Engine.h"
#include "Drop.h"
#include "PuroBase.h"
#include "Onset.h"

Engine::Engine(PuroBase* instance) {
	//std::cout << "Engine" << std::endl;
	instance_ = instance;
}

void
Engine::AddOnset(Onset* onset) {
    /*
	DropBundle bundle;
	bundle.drop = drop;
	bundle.index = 0;
	drops_in_use_.push_back(bundle);
     */
    onsets_in_use_.push_back(onset);
}

// TODO ADD TIMING
void
Engine::GetAudioOutput(uint32_t n, float* buffer) {
	instance_->Tick();
	// iterator running
	//std::list<struct DropBundle>::iterator running = drops_in_use_.begin();
	std::list<Onset*>::iterator running = onsets_in_use_.begin();

	for (uint32_t k=0; k<n; k++)
		buffer[k] = 0;

	while (running != onsets_in_use_.end()) {
        Onset* onset = *running;
		uint32_t n_summed = onset->drop_->GetAudio(onset->index_, n, buffer);
		onset->index_ += n_summed; // this is done outside of drop
		if (n_summed != n) {
			instance_->ReturnDepletedDrop(onset->drop_);
            delete onset;
			running = onsets_in_use_.erase(running);
		}
		else {
			running++;
        }
	}
}
