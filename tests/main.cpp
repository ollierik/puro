// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Puro.h"
#include "Interpreter.h"
#include "Engine.h"
#include "Passage.h"

// for creating output file
#include "ICST/common.h"
#include "ICST/AudioFile.h"
#include <iostream>

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

void timing_test() {
    
    dout << "START TIMING TEST" << dndl;
    
	///////////////////////////////////////////////
	// PREPARE ENVIRONMENT
	///////////////////////////////////////////////

	Puro* puro = new Puro();
	Interpreter* interp = puro->GetInterpreter();
	Engine* engine = puro->GetEngine();

	Tag material = CharsToTag((char*)"lafille");

	interp->LoadAudioMaterial(material, (char*)"/Users/ollierik/lafille.wav");

	///////////////////////////////////////////////
	// PREPARE UNIT 1
	///////////////////////////////////////////////
	{
		Tag idea = CharsToTag((char*)"u1");

		interp->SetMaterial(idea, material);

		// AUDIO
		float audio_list[] = { 7.9, 0.1, 8.0 }; // linear
		interp->SetAudioPassage(idea, ARRLEN(audio_list), audio_list);

		// ENVELOPE
		//float envelope_list[5] = { 1.0, 0.5, 0.25, 1.0, 0 }; 	// sharp attack
		float envelope_list[5] = { 0, 0.5, 1.0, 1.0, 0 }; 	// trapezoid
		//float envelope_list[] = { 1.0, 1.0, 1.0 }; 	// const

		interp->SetEnvelopePassage(idea, ARRLEN(envelope_list), envelope_list);

        unsigned interval = 6000;
        for (int i=0; i<40; i++) {
            interp->OnsetDropFromIdea(idea, i*interval);
        }
        
		//float envelope_list2[5] = { 0.0, 0.5, 0.25, 1.0, 1.0 }; 	// sharp decline
		//interp->SetEnvelopePassage(idea, ARRLEN(envelope_list2), envelope_list2);
        
		//interp->OnsetDropFromIdea(idea, 66150);
        
        
	}

	///////////////////////////////////////////////
	// PREPARE OUTPUT FILE
	///////////////////////////////////////////////

	uint32_t n = 1024;
	uint32_t blocks = 500;
	icstdsp::AudioFile* out_file = new icstdsp::AudioFile();
	out_file->Create(n*blocks);
	float* buffer = out_file->GetSafePt();

	for (unsigned k=0; k<n*blocks; k++)
		buffer[k] = 0;

	for (unsigned b=0; b<blocks; b++) {
		engine->GetAudioOutput(n, &buffer[b*n]);
	}

	out_file->SaveWave((char*)"/Users/ollierik/outfile.wav");

	//for (unsigned k=0; k<n*blocks; k++)
	//	std::cout << buffer[k] << std::endl;

    dout << "Done!" << dndl;
    delete puro;
}

void output_test() {
    
    dout << "START TEST" << dndl;
    
	///////////////////////////////////////////////
	// PREPARE ENVIRONMENT
	///////////////////////////////////////////////

	Puro* puro = new Puro();
	Interpreter* interp = puro->GetInterpreter();
	Engine* engine = puro->GetEngine();

	Tag material = CharsToTag((char*)"lafille");

	interp->LoadAudioMaterial(material, (char*)"/Users/ollierik/lafille.wav");

	///////////////////////////////////////////////
	// PREPARE UNIT 1
	///////////////////////////////////////////////
	{
		Tag idea = CharsToTag((char*)"u1");

		interp->SetMaterial(idea, material);

		// AUDIO
		float audio_list[] = { 5.0, 4.0, 9.0 }; // linear
		interp->SetAudioPassage(idea, ARRLEN(audio_list), audio_list);

		// ENVELOPE
		float envelope_list[5] = { 0, 0.5, 1.0, 1.0, 0 }; 	// trapezoid
		//float envelope_list[] = { 1.0, 1.0, 1.0 }; 	// const

		interp->SetEnvelopePassage(idea, ARRLEN(envelope_list), envelope_list);

		interp->OnsetDropFromIdea(idea);
	}
	///////////////////////////////////////////////
	// PREPARE UNIT 2
	///////////////////////////////////////////////
	{
		Tag idea = CharsToTag((char*)"u2");

		interp->SetMaterial(idea, material);

		// AUDIO
		std::cout << "AUDIO" << std::endl;
		float audio_list[] = { 10.0, 4.0, 14.0 }; // linear
		interp->SetAudioPassage(idea, ARRLEN(audio_list), audio_list);

		// ENVELOPE
		std::cout << "ENVELOPE" << std::endl;
		//float envelope_list[5] = { 0, 0.5, 1.0, 1.0, 0 }; 	// trapezoid
		float envelope_list[] = { 1.0, 1.0, 1.0 }; 	// const

		interp->SetEnvelopePassage(idea, ARRLEN(envelope_list), envelope_list);

		interp->OnsetDropFromIdea(idea);
	}

	///////////////////////////////////////////////
	// PREPARE OUTPUT FILE
	///////////////////////////////////////////////

	uint32_t n = 1024;
	uint32_t blocks = 1600;
	icstdsp::AudioFile* out_file = new icstdsp::AudioFile();
	out_file->Create(n*blocks);
	float* buffer = out_file->GetSafePt();

	for (unsigned k=0; k<n*blocks; k++)
		buffer[k] = 0;

	for (unsigned b=0; b<blocks; b++) {
		engine->GetAudioOutput(n, &buffer[b*n]);
	}

	out_file->SaveWave((char*)"/Users/ollierik/outfile.wav");

	//for (unsigned k=0; k<n*blocks; k++)
	//	std::cout << buffer[k] << std::endl;

    dout << "Done!" << dndl;
}

int main() {
    timing_test();
}