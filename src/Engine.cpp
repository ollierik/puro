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
    running_time_ = 0;
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
    running_time_ += n;
    
	std::list<Onset*>::iterator iter = onsets_.begin();
    
	for (uint32_t k=0; k<n; k++)
		buffer[k] = 0;
    
	while (iter != onsets_.end()) {
        Onset* onset = *iter;
        
        // TIMING CHECK, if running time has not passed onset time
        if (onset->time_ > running_time_) {
            
            // THIS SHOULD BE PASSED
            dout << "Time: " << onset->time_ << "<" << running_time_ << dndl;
            dout << "pass drop" << dndl;
            iter++;
            
        } else {
            // THIS SHOULD BE RUN
            
            // TODO
            // OFFSET
            
            uint32_t n_summed;
            //uint64_t a = running_time_ - n;
            uint32_t offset = 0;
            if (running_time_ - n < onset->time_) {
                offset = n - uint32_t(running_time_ - onset->time_);
                n_summed = onset->drop_->GetAudio(buffer, onset->index_, n-offset, offset);
                //dout << "##################################################\nOffset: " << offset << dndl;
            } else {
                n_summed = onset->drop_->GetAudio(buffer, onset->index_, n);
            }
            
            onset->index_ += n_summed; // this is done outside of drop
            // if these aren't equal, the drop has depleted
            if (n_summed != n-offset) {
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
