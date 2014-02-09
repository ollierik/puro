// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#ifndef IDEA_H_
#define IDEA_H_

#include "Puro.h"
#include "Passage.h"

/*
 * Represents a prototype of a Drop.
 */

class Idea {
	Tag association_;
	Tag material_;
	Passage* audio_;
	Passage* envelope_;
    Time time_offset_;
    
public:
	Idea();
	~Idea();
	void Initialize();

	Tag GetAssociation();
	Tag GetMaterial();
	Passage* GetAudioPassage();
	Passage* GetEnvelopePassage();

	void SetAssociation(Tag association);
	void SetMaterial(Tag material);
	//void SetAudio(Passage* audio);
	//void SetEnvelope(Passage* envelope);
    
    Time GetTimeOffset();
    void SetTimeOffset(Time current);

	bool IsValid(); // has it been initialized with parameters?
};

#endif /* IDEA_H_ */
