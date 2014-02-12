// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Onset.h"

Onset::Onset(Time time, Drop* drop) {
    time_ = time;
    drop_ = drop;
    index_ = 0;
}