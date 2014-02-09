//
//  resource.h
//  puro
//
//  Created by Olli Erik Keskinen on 20/01/14.
//  Copyright (c) 2014 Olli Erik Keskinen. All rights reserved.
//

#ifndef __puro__resource__
#define __puro__resource__

class resource {
    
    std::vector<Passage> data_;
    std::vector<Passage *> refs_;
    unsigned int first_inactive_;
    
public:
    resource(unsigned int size, Passage prototype) {
        
        data_ = std::vector<Passage> (size, prototype)
        refs_.reserve(size);
        for (unsigned int i = 0; i < size; i++) {
            refs_[i] = &data[i];
        }
        first_inactive_ = refs_.size();
    }
    
    Passage* getFree() {
        if (first_inactive_ == 0)
            return 0;
        first_inactive_--;
        // pop first free
        Passage* free = refs_[first_inactive_];
        refs_[first_inactive_] = 0;
        return free;
    }
    
    void setFree(Passage* free) {
        refs_[first_inactive_] = free;
        first_inactive_++;
    }
    
};

#endif /* defined(__puro__resource__) */