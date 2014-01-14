// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Engine.h"
#include "Drop.h"
#include "MainFrame.h"

Engine::Engine(MainFrame* instance) {
	//std::cout << "Engine" << std::endl;
	instance_ = instance;
}

Engine::~Engine() {
}

void
Engine::AddDrop(Drop* drop) {
	DropBundle bundle;
	bundle.drop = drop;
	bundle.index = 0;
	drops_in_use_.push_back(bundle);
}

// TODO ADD TIMING
void
Engine::GetAudioOutput(uint32_t n, float* buffer) {
	instance_->Tick();
	// iterator running
	std::list<struct DropBundle>::iterator running = drops_in_use_.begin();

	for (uint32_t k=0; k<n; k++)
		buffer[k] = 0;

	while (running != drops_in_use_.end()) {
		uint32_t n_summed = running->drop->GetAudio(running->index, n, buffer);
		running->index += n_summed; // this is done outside of drop
		if (n_summed != n) {
			instance_->ReturnDepletedDrop(running->drop);
			running = drops_in_use_.erase(running);
		}
		else
			running++;
	}
}
