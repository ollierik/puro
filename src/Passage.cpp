// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Passage.h"

Passage::Passage(uint16_t capacity) {
	capacity_ = capacity;
	size_ = 0;
	list_ = new PassageEntry[capacity_];
}

uint16_t
Passage::GetSize() {
	return size_;
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
	list_[index].value = value;
	list_[index].time = time;
}

//////////////////////////////////////////////////////////////

PassageContainer::PassageContainer(uint16_t n_passages, uint16_t passage_capacity) {
    passages_.reserve(n_passages);
	for (uint16_t i = 0; i < n_passages; ++i) {
		Passage* new_passage = new Passage(passage_capacity);
		passages_.push_back(*new_passage);
	}
}




