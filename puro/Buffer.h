/*
 * Buffer.h
 *
 *  Created on: Jul 11, 2013
 *      Author: oek
 */

#ifndef BUFFER_H_
#define BUFFER_H_

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

#endif /* BUFFER_H_ */
