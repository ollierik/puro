// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "PuroBase.h"
#include "Engine.h"
#include "AudioStorage.h"
#include "Worker.h"
#include "Interpreter.h"
#include "Idea.h"
#include "Drop.h"

PuroBase::PuroBase(uint16_t n_ideas,  uint16_t n_drops, uint16_t n_audio_passages, uint16_t n_envelope_passages) {
    
	//std::cout << "PuroBase" << std::endl;
    
	audio_storage_ = new AudioStorage();
	engine_ = new Engine(this);
	interpreter_ = new Interpreter(this);
	worker_ = new Worker(this);
    
    Passage audio_passage_prototype(&audio_passages_, 128);
    Passage envelope_passage_prototype(&envelope_passages_, 128);
    
    audio_passages_.assign(20, &audio_passage_prototype);
    envelope_passages_.assign(20, &envelope_passage_prototype);
    
    //Passage passage_prototype(128);
    //= new respool<Passage>(n_audio_passages, &passage_prototype);
    //envelope_passages_ = new respool<Passage>(n_envelope_passages, &passage_prototype);
    
    //debug_msg("HELLO DEBUG!");
    
	ideas_.reserve(n_ideas);
	for (uint16_t i = 0; i < n_ideas; ++i) {
		Idea* new_idea = new Idea();
		ideas_.push_back(*new_idea);
		ideas_free_.push(&ideas_[i]);
	}
    
	//drops_.reserve(n_drops);
	for (uint16_t i = 0; i < n_drops; ++i) {
		Drop* new_drop = new Drop(this, GetBufferMaxLength());
		//drops_.push_back(*new_drop);
		drops_free_.push(new_drop);
	}
}

PuroBase::~PuroBase() {
	delete interpreter_;
	delete engine_;
	delete audio_storage_;
	delete worker_;
}

Idea*
PuroBase::GetIdea(Tag association) {
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

Passage *
PuroBase::GetFreeAudioPassage() {
    auto inactive = audio_passages_.getinactive();
    return inactive;
}

Passage *
PuroBase::GetFreeEnvelopePassage() {
    return envelope_passages_.getinactive();
}

void
PuroBase::AssociateAudioPassage(Tag idea, Passage* filled) {
    Idea* idea_to_use = GetIdea(idea);
    idea_to_use->SetAudioPassage(filled);
}

void
PuroBase::AssociateEnvelopePassage(Tag idea, Passage* filled) {
    Idea* idea_to_use = GetIdea(idea);
    idea_to_use->SetEnvelopePassage(filled);
}

float *
PuroBase::GetAudioData(Tag material) {
	return audio_storage_->GetData(material);
}

uint32_t
PuroBase::GetAudioSize(Tag material) {
	return audio_storage_->GetSize(material);
}

void
PuroBase::LoadAudioMaterial(Tag material, char* path) {
	audio_storage_->LoadFile(material, path);
}

void
PuroBase::OnsetDrop(Tag association, Time relative) {
    
    Idea* idea = GetIdea(association);
    
	if (idea->IsValid()) {
        //Drop* drop = PopFreeDrop();
        //if (drop == 0) return;
        
        //drop->Initialize(idea);
        Time absolute = relative + idea->GetTimeOffset();
        
        Onset* onset = new Onset(absolute);
        
        // ARRANGE CHRONOLOGICALLY
        auto it = onsets_.begin();
        while (it != onsets_.end()) {
            //if (absolute < (*it)->GetOnsetTime()) {
            if (absolute < (*it)->time_) {
                break;
            }
            it++;
        }
        onsets_.insert(it, onset);
		//std::cout << "n of onsets afterwards: " << onsets_.size() << std::endl;
	}
}

bool
PuroBase::HasFreeDrops() {
    bool empty = onsets_.empty();
    return !empty;
}

Onset*
PuroBase::GetNextOnset() {
	if (onsets_.empty()) {
		return 0;
    }
	Onset* next = onsets_.front();
	onsets_.pop_front();
	return next;
}

// TODO CHECKS
Drop*
PuroBase::PopFreeDrop() {
	//std::cout << "Pop free drop, n: " << drops_free_.size() << std::endl;
    if (drops_free_.empty()) {
        dout << "NO FREE DROPS" << dndl;
        return 0;
    }
	Drop* drop = drops_free_.front();
	drops_free_.pop();
	return drop;
}

// TODO TIME
void
PuroBase::ScheduleOnset(Onset* onset) {
	engine_->AddOnset(onset);
}

void
PuroBase::ReturnDepletedDrop(Drop* drop) {
	//std::cout << "Drop depleted" << std::endl;
    dout << "*******************************\nReturn Depleted" << dndl;
	drops_free_.push(drop);
}

uint32_t
PuroBase::GetBufferMaxLength() {
	return 262144;
}

uint16_t
PuroBase::GetPassageMaxLength() {
	return 128;
}

uint32_t
PuroBase::GetMaterialSampleRate(Tag material) {
	return (uint32_t)audio_storage_->GetSampleRate(material);
}

Engine*
PuroBase::GetEngine() {
	return engine_;
}

Interpreter*
PuroBase::GetInterpreter() {
	return interpreter_;
}


void
PuroBase::SyncIdea(Tag association) {
    Idea* idea_to_use = this->GetIdea(association);
    Time current_time = this->GetTime();
    idea_to_use->SetTimeOffset(current_time);
}

Time
PuroBase::GetTime() {
    return 0;
}

void
PuroBase::Tick() {
	//std::cout << "MAINFRAME TICK" << std::endl;
	worker_->Tick();
}
