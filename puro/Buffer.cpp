/*
 * Buffer.cpp
 *
 *  Created on: Jul 11, 2013
 *      Author: oek
 */

#include "Puro.h"
#include "Buffer.h"


Buffer::Buffer(uint32_t capacity) {
	capacity_ = capacity;
	size_ = 0;
	data_ = new float [capacity_];

	Clear();
}

Buffer::~Buffer() {
	delete[] data_;
}

uint32_t
Buffer::GetSize() {
	return size_;
}

uint32_t
Buffer::SetSize(uint32_t new_size) {
	if (new_size > capacity_)
		new_size = capacity_;

	while (new_size > size_) {
		data_[size_] = 0;
		size_++;
	}
	if (new_size < size_)
		size_ = new_size;
	return size_;
}

float
Buffer::GetValue(uint32_t index) {
	return data_[index];
}

void
Buffer::SetValue(uint32_t index, float value) {
	data_[index] = value;
}

void
Buffer::Clear() {
	Clear(capacity_);
}

void
Buffer::Clear(uint32_t to) {
	if (to > capacity_) to = capacity_;
	for (uint32_t k=0; k<to; k++)
		data_[k] = 0;
	SetSize(0);
}

