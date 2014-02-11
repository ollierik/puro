// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Passage.h"

Passage::Passage(respool<Passage>* pool, uint16_t capacity) {
    pool_ = pool;
	size_ = 0;
	list_.reserve(capacity);
    
    PassageEntry prototype;
    prototype.time = prototype.value = 0;
    list_.assign(capacity, prototype);
}

float
Passage::GetTime(uint16_t index) {
	float f = list_[index].time;
	return f;
}

float
Passage::GetValue(uint16_t index) {
	float f = list_[index].value;
	return f;
}

void
Passage::SetSize(uint16_t size) {
	size_ = size;
}

void
Passage::SetEntry(uint16_t index, float time, float value) {
	//std::cout << "Set PassageEntry at index:" << index
			//<< " time:" << time << " value:" << value << std::endl;
    int c = list_.capacity();
	list_[index].value = value;
	list_[index].time = time;
}

void Passage::RegisterRefrence() {
    n_refrencees_++;
}

void Passage::RemoveRefrence() {
    n_refrencees_--;
    if (n_refrencees_ == 0) {
        pool_->setinactive(this);
    }
}