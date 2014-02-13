// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.


#pragma once

class Worker {
	PuroBase* base_;
	// TODO processing buffer for FFT
public:
	Worker(PuroBase* instance);
	void PrepareDrop(Drop* drop);
	void Tick();
};
