/*
 * Interpreter.h
 *
 *  Created on: Jul 17, 2013
 *      Author: oek
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "Puro.h"

class Interpreter {
	MainFrame* instance_;
public:
	Interpreter(MainFrame* instance);
	void SetMaterial(Tag idea, Tag material);
	void SetAudioPassage(Tag idea, uint16_t n_data, float* data);
	void SetEnvelopePassage(Tag idea, uint16_t n_data, float* data);
	void OnsetDropFromIdea(Tag idea);
	void LoadAudioMaterial(Tag association, char* path_to_file);

};

#endif /* INTERPRETER_H_ */
