/*
 * Idea.h
 *
 *  Created on: Jul 15, 2013
 *      Author: oek
 */

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

	bool IsValid(); // has it been initialized with parameters?
};

#endif /* IDEA_H_ */
