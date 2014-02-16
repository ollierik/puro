// puro - Audio sampler library
//
// Copyright (c) 2013 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.

#pragma once

#include "Puro.h"

class Onset {
public:
    Time time_;
    Drop* drop_;
    uint32_t index_;
    Tag material_;
    Passage* audio_passage_;
    Passage* envelope_passage_;
    
    Onset(Time time, Tag material, Passage* audio_passage_, Passage* envelope_passage_);
};