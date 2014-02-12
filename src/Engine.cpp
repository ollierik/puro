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

Engine::Engine(PuroBase* base) {
	//std::cout << "Engine" << std::endl;
	base_ = base;
    time_ = 0;
}

void
Engine::AddOnset(Onset* onset) {
    /*
     DropBundle bundle;
     bundle.drop = drop;
     bundle.index = 0;
     drops_in_use_.push_back(bundle);
     */
    onsets_.push_back(onset);
}

// TODO ADD TIMING
void
Engine::GetAudioOutput(uint32_t n, float* buffer) {
    
    dout << "### TICK ###" << dndl;
	base_->Tick();
    time_ += n;
    
	std::list<Onset*>::iterator iter = onsets_.begin();
    
	for (uint32_t k=0; k<n; k++)
		buffer[k] = 0;
    
	while (iter != onsets_.end()) {
        Onset* onset = *iter;
        
        // TIMING CHECK
        if (onset->time_ > time_) {
            
            // THIS SHOULD BE PASSED
            dout << "Time: " << onset->time_ << "<" << time_ << dndl;
            dout << "pass drop" << dndl;
            iter++;
            
        } else {
            // THIS SHOULD BE RUN
            uint32_t n_summed = onset->drop_->GetAudio(onset->index_, n, buffer);
            onset->index_ += n_summed; // this is done outside of drop
            // if these aren't equal, the drop has depleted
            if (n_summed != n) {
                base_->ReturnDepletedDrop(onset->drop_);
                delete onset;
                iter = onsets_.erase(iter);
            }
            else {
                iter++;
            }
        } // was run
    } // end of while
}
