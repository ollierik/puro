/*
 * MainFrame.h
 *
 *  Created on: Jul 16, 2013
 *      Author: oek
 */

#ifndef MAINFRAME_H_
#define MAINFRAME_H_


#include <map>
#include <vector>
#include <queue>

#include "Puro.h"

class MainFrame {

	Interpreter* interpreter_;
	Engine* engine_;
	AudioStorage* audio_storage_;
	Worker* worker_;

	// Ideas
	std::vector<Idea> ideas_;
	std::map<Tag, Idea*> ideas_in_use_;
	std::queue<Idea*> ideas_free_;

	// Drops
	std::vector<Drop> drops_;
	std::queue<Drop*> drops_free_;

	std::queue<Idea*> onsets_; // chronologically ordered from lowest

public:
	MainFrame(uint16_t nIdeas, uint16_t nDrops);
	~MainFrame();
	float* GetAudioData(Tag material);
	uint32_t GetAudioSize(Tag material);

	void LoadAudioMaterial(Tag material, char* path);
	Idea* GetIdea(Tag association);
	void OnsetDrop(Idea* idea); // onset idea into drop NOW

	Idea* GetNextOnset(); // get next oncoming onset
	Drop* PopFreeDrop(); // pop next free Drop
	void ScheduleDrop(Drop* drop); // add drop to Engines run queue
	void ReturnDepletedDrop(Drop* drop); // return depleted Drop from Engine

	uint32_t GetBufferMaxLength();
	uint16_t GetPassageMaxLength();
	uint32_t GetMaterialSampleRate(Tag material);

	Engine* GetEngine();
	Interpreter* GetInterpreter();

	void Tick(); // for now just to run worker
};
#endif /* MAINFRAME_H_ */
