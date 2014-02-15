// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Onset.h"

Onset::Onset(Time time) {
    time_ = time;
    drop_ = 0;
    index_ = 0;
    audio_passage_ = 0;
    envelope_passage_ = 0;
}