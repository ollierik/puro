// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "MainFrame.h"
#include "Engine.h"
#include "AudioStorage.h"
#include "Worker.h"
#include "Interpreter.h"
#include "Idea.h"
#include "Drop.h"

MainFrame::MainFrame(uint16_t n_ideas, uint16_t n_drops) {

	//std::cout << "MainFrame" << std::endl;

	audio_storage_ = new AudioStorage();
	engine_ = new Engine(this);
	interpreter_ = new Interpreter(this);
	worker_ = new Worker(this);

	ideas_.reserve(n_ideas);
	for (uint16_t i = 0; i < n_ideas; ++i) {
		Idea* new_idea = new Idea();
		ideas_.push_back(*new_idea);
		ideas_free_.push(&ideas_[i]);
	}

	drops_.reserve(n_drops);
	for (uint16_t i = 0; i < n_drops; ++i) {
		Drop* new_drop = new Drop(this, GetBufferMaxLength());
		drops_.push_back(*new_drop);
		drops_free_.push(&drops_[i]);
	}

}

MainFrame::~MainFrame() {
	delete interpreter_;
	delete engine_;
	delete audio_storage_;
	delete worker_;
}

Idea*
MainFrame::GetIdea(Tag association) {
	//std::cout << "Get idea " << association << std::endl;
	Idea* idea = ideas_in_use_[association];
	if (idea == 0) {
		//std::cout << "   Use NEW idea " << std::endl;
		idea = ideas_free_.front();
		ideas_free_.pop();
		ideas_in_use_[association] = idea;
		idea->SetAssociation(association);
	}
	return idea;
}

float* MainFrame::GetAudioData(Tag material) {
	return audio_storage_->GetData(material);
}

uint32_t MainFrame::GetAudioSize(Tag material) {
	return audio_storage_->GetSize(material);
}

void MainFrame::LoadAudioMaterial(Tag material, char* path) {
	audio_storage_->LoadFile(material, path);
}

void MainFrame::OnsetDrop(Idea* idea) {

	//std::cout << "Onset drop, n of onsets now: " << onsets_.size() << std::endl;
	// TODO TIME
	// LOGIC TO INSERTING TO RIGHT SPOT WHEN TIME IS ADDED
	if (idea->IsValid()) {
		onsets_.push(idea);
		//std::cout << "n of onsets afterwards: " << onsets_.size() << std::endl;
	}
}

Idea*
MainFrame::GetNextOnset() {
	if (onsets_.empty())
		return 0;
	Idea* next = onsets_.front();
	onsets_.pop();
	return next;
}

// TODO CHECKS
Drop*
MainFrame::PopFreeDrop() {
	//std::cout << "Pop free drop, n: " << drops_free_.size() << std::endl;
	Drop* drop = drops_free_.front();
	drops_free_.pop();
	return drop;
}

// TODO TIME
void
MainFrame::ScheduleDrop(Drop* drop) {
	engine_->AddDrop(drop);
}

void
MainFrame::ReturnDepletedDrop(Drop* drop) {
	//std::cout << "Drop depleted" << std::endl;
	drops_free_.push(drop);
}


uint32_t
MainFrame::GetBufferMaxLength() {
	return 262144;
}

uint16_t
MainFrame::GetPassageMaxLength() {
	return 128;
}

uint32_t
MainFrame::GetMaterialSampleRate(Tag material) {
	return (uint32_t)audio_storage_->GetSampleRate(material);
}

Engine*
MainFrame::GetEngine() {
	return engine_;
}

Interpreter*
MainFrame::GetInterpreter() {
	return interpreter_;
}

void
MainFrame::Tick() {
	//std::cout << "MAINFRAME TICK" << std::endl;
	worker_->Tick();
}
