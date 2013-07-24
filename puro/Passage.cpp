/*
 * Passage.cpp
 *
 *  Created on: Jul 12, 2013
 *      Author: oek
 */

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
	std::cout << "Set PassageEntry at index:" << index
			<< " time:" << time << " value:" << value << std::endl;
	list_[index].value = value;
	list_[index].time = time;
}
