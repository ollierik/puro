// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.


#pragma once

#include "Puro.h"
#include <vector>
#include "respool.h"

//#define PASSAGE_MAX_SIZE 128

struct PassageEntry {
	float time;
	float value;
};

//////////////////////////////////////////////

class Passage {
	uint16_t size_;
    std::vector<PassageEntry> list_;
    uint16_t n_refrencees_;
    respool<Passage> * pool_;
public:
	Passage(respool<Passage>* pool, uint16_t capacity);
	uint16_t GetSize() { return size_; };
	float GetTime(uint16_t index);
	float GetValue(uint16_t index);
	void SetSize(uint16_t size);
	void SetEntry(uint16_t index, float time, float value);
    //uint16_t GetNumberOfRefrencees() { return n_refrencees_; };
    void RegisterRefrence();
    void RemoveRefrence();
    
    static uint32_t GetPassageMaxSize() { return 128; };
};