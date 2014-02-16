// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#include "Onset.h"

Onset::Onset(Time time, Tag material, Passage* audio_passage, Passage* envelope_passage) {
    time_ = time;
    drop_ = 0;
    index_ = 0;
    material_ = material;
    audio_passage_ = audio_passage;
    envelope_passage_ = envelope_passage;
}