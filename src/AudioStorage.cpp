// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "AudioStorage.h"

icstdsp::AudioFile*
AudioStorage::GetFile(Tag association) {

	icstdsp::AudioFile* file = 0;

	for (uint16_t i = 0; i<files_.size(); i++) {
		AudioBundle* bundle = &files_[i];
		if (bundle->tag == association)
			file = bundle->file;
	}
	return file;
}

int
AudioStorage::LoadFile(Tag association, char* path) {

	//std::cout << "Load file" << std::endl;

	icstdsp::AudioFile* new_file = new icstdsp::AudioFile();

	new_file->Load(path);
	AudioBundle bundle = { association, new_file };

	files_.push_back(bundle);

	return 0;
}

float*
AudioStorage::GetData(Tag association) {

	icstdsp::AudioFile* file = GetFile(association);
	if (file == 0)
		return 0;
	return file->GetSafePt();
}

uint32_t
AudioStorage::GetSize(Tag association) {

	icstdsp::AudioFile* file = GetFile(association);
	if (file == 0)
		return 0;
	return file->GetSize();
}

uint32_t
AudioStorage::GetSampleRate(Tag association) {
	icstdsp::AudioFile* file = GetFile(association);
	if (file == 0)
		return 0;
	return file->GetRate();
}

