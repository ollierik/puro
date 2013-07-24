/*
 * Idea.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: oek
 */

#include "Idea.h"

Idea::Idea() {
	std::cout << "Idea" << std::endl;
	Initialize();
	audio_ = new Passage(128);
	envelope_ = new Passage(128);
}

Idea::~Idea() {
  delete audio_;
  delete envelope_;
}

void
Idea::Initialize() {
	association_ = 0;
	material_ = 0;
}

Tag
Idea::GetAssociation() {
	return association_;
}
Tag
Idea::GetMaterial() {
	return material_;
}

Passage*
Idea::GetAudioPassage() {
	return audio_;
}
Passage*
Idea::GetEnvelopePassage() {
	return envelope_;
}

void
Idea::SetAssociation(Tag association) {
	association_ = association;
}
void
Idea::SetMaterial(Tag material) {
	material_ = material;
}
/*
void
Idea::SetAudio(Passage* audio) {
	audio_ = audio;
}
void
Idea::SetEnvelope(Passage* envelope) {
	envelope_ = envelope;
}
*/
bool
Idea::IsValid() {
	bool is_valid = (association_) && (material_)
			&& (audio_->GetSize()>1) && (envelope_->GetSize()>1);
	std::cout<< "Is idea valid: " << is_valid << std::endl;
	return is_valid;
}
