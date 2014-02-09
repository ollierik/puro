// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.


#ifndef WORKER_H_
#define WORKER_H_

class Worker {
	PuroBase *instance_;
	// TODO processing buffer for FFT
public:
	Worker(PuroBase* instance);
	virtual ~Worker();
	void PrepareDrop(Idea* onset, Drop* drop);
	void Tick();
};

#endif /* WORKER_H_ */
