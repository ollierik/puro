/*
 * Worker.h
 *
 *  Created on: Jul 15, 2013
 *      Author: oek
 */

#ifndef WORKER_H_
#define WORKER_H_

class Worker {
	MainFrame *instance_;
	// TODO processing buffer for FFT
public:
	Worker(MainFrame* instance);
	virtual ~Worker();
	void PrepareDrop(Idea* onset, Drop* drop);
	void Tick();
};

#endif /* WORKER_H_ */
