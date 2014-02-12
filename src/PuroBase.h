// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#pragma once

#include <map>
#include <vector>
#include <queue>
#include <list>

#include "Puro.h"
#include "respool.h"
#include "Onset.h"

class PuroBase {
    
	Interpreter* interpreter_;
	Engine* engine_;
	AudioStorage* audio_storage_;
	Worker* worker_;
    
    Time time_;
    
	// Ideas
	std::vector<Idea> ideas_;
	std::map<Tag, Idea *> ideas_in_use_;
	std::queue<Idea *> ideas_free_;
    
	// Drops
	std::list<Onset *> onsets_; // unprepared, chronologically ordered from lowest
	//std::vector<Drop> drops_;
	std::queue<Drop *> drops_free_;
    
    // Passages
    //std::vector<Passage> audio_passages_;
    //std::vector<Passage> envelope_passages_;
    respool<Passage> audio_passages_;
    respool<Passage> envelope_passages_;
    
    
public:
    PuroBase(uint16_t n_ideas,  uint16_t n_drops, uint16_t n_audio_passages, uint16_t n_envelope_passages);
	~PuroBase();
	float* GetAudioData(Tag material);
	uint32_t GetAudioSize(Tag material);
	void LoadAudioMaterial(Tag material, char* path);
	Idea* GetIdea(Tag association);
    
    Passage* GetFreeAudioPassage();
    Passage* GetFreeEnvelopePassage();
    
    void AssociateAudioPassage(Tag idea, Passage* filled);
    void AssociateEnvelopePassage(Tag idea, Passage* filled);
    
	//void OnsetDrop(Idea* idea); // onset idea into drop NOW
	void OnsetDrop(Tag association, Time relative); // onset idea into drop
    
	Onset* GetNextOnset(); // get next oncoming onset
	Drop* PopFreeDrop(); // pop next free Drop
	void ScheduleOnset(Onset* onset); // add drop to Engines run queue
	void ReturnDepletedDrop(Drop* drop); // return depleted Drop from Engine
    
	uint32_t GetBufferMaxLength();
	uint16_t GetPassageMaxLength();
	uint32_t GetMaterialSampleRate(Tag material);
    Time GetTime();
    
	Engine* GetEngine();
	Interpreter* GetInterpreter();
    
    void SyncIdea(Tag idea);
    
	void Tick(); // for now just to run worker
};
