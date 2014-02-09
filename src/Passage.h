// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#ifndef PASSAGE_H_
#define PASSAGE_H_

#include "Puro.h"
#include <list>

//#define PASSAGE_MAX_SIZE 128

struct PassageEntry {
	float time;
	float value;
};

class Passage {
	uint16_t capacity_;
	uint16_t size_;
	PassageEntry* list_;
    uint16_t n_refrencees_;
public:
	Passage(uint16_t capacity);
	uint16_t GetSize();
	float GetTime(uint16_t index);
	float GetValue(uint16_t index);
	void SetSize(uint16_t size);
	void SetEntry(uint16_t index, float time, float value);
    
    static uint32_t GetPassageMaxSize() { return 128; };
};

class PassageContainer {
    std::list<Passage> passages_;
public:
    PassageContainer(uint16_t n_passages, uint16_t passage_capacity);
    Passage* GetFreePassage();
    void ReturnPassage();
};

#endif /* PASSAGE_H_ */

