/*
 * Drop.h
 *
 *  Created on: Jul 11, 2013
 *      Author: oek
 */

#ifndef DROP_H_
#define DROP_H_

#include "Puro.h"

class Drop {
	MainFrame* instance_;
	Tag idea_;
	Tag material_;
	//Drop* next_;
	Buffer* audio_;
	Buffer* envelope_;

	uint32_t GetMaterialSampleRate();
	uint32_t GetDurationInSamples();
public:
	Drop(MainFrame* instance, uint32_t buffer_size);
	~Drop();
	uint32_t GetAudio(uint32_t index, uint32_t n, float* buffer);
	void Initialize(Tag idea, Tag material);
	int32_t ProcessAudio(Passage* audio);
	int32_t ProcessEnvelope(Passage* envelope);
};

#endif /* DROP_H_ */
