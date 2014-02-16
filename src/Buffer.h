// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#pragma once

class Buffer {
	uint32_t capacity_;
	uint32_t size_;
	float* data_;
public:
	Buffer(uint32_t capacity);
	~Buffer();
	void Clear();
	void Clear(uint32_t);
	uint32_t GetSize();
	uint32_t SetSize(uint32_t size);
	float GetValue(uint32_t index);
	void SetValue(uint32_t index, float value);
};