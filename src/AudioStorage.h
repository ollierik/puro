// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#ifndef AUDIOSTORAGE_H_
#define AUDIOSTORAGE_H_

#include "Puro.h"
#include "ICST/common.h"
#include "ICST/AudioFile.h"
#include <vector>

struct AudioBundle {
	Tag tag;
	icstdsp::AudioFile* file;
};

class AudioStorage {
	std::vector<AudioBundle> files_;
	icstdsp::AudioFile* GetFile(Tag association);
public:
	int LoadFile(Tag association, char* path);
	float* GetData(Tag association);
	uint32_t GetSize(Tag association);
	uint32_t GetSampleRate(Tag association);
};

#endif /* AUDIOSTORAGE_H_ */
