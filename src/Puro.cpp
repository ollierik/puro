// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Puro.h"
#include <iostream>
#include "PuroBase.h"
#include "Passage.h"

Tag CharsToTag(char* name) {
	Tag tag=0;
	uint16_t i = 0;
	while (i<8 && name[i]!='\0') {
		Tag in = (Tag)name[i];
		tag <<= 8;
		tag += in;
		i++;
	}
	return tag;
}

void
FloatListToPassage(Passage* passage_to_use, uint16_t n_data, float* data) {
	//std::cout << "FloatListToPassage n_data: " << n_data << std::endl;
	// [0] 14000 2250 16250 linear

	uint16_t size = n_data / 2 + 1;
	passage_to_use->SetSize(size);

	float time = 0;
	float value = data[0];
	passage_to_use->SetEntry(0, time, value);
	for (uint16_t ri=1, wi=1; ri+1<n_data; ri+=2, wi++) {
		time = data[ri];
		value = data[ri+1];
		passage_to_use->SetEntry(wi, time, value);
	}
}


Puro::Puro() {
	mainframe_ = new PuroBase(8, 16, 16, 16);
}

Puro::~Puro() {
	delete mainframe_;
}

Engine*
Puro::GetEngine() {
	return mainframe_->GetEngine();
}

Interpreter*
Puro::GetInterpreter() {
	return mainframe_->GetInterpreter();
}

const char*
Puro::HelloWorld() {
	return "Hello World";
}
