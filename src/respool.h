//
//  resource.h
//  puro
//
//  Created by Olli Erik Keskinen on 20/01/14.
//  Copyright (c) 2014 Olli Erik Keskinen. All rights reserved.
//

#ifndef __puro__resource__
#define __puro__resource__

template <class T>
class respool {
    
    std::vector<T> data_;
    std::vector<T *> refs_;
    unsigned int first_inactive_;
    
public:
    respool(unsigned int size, T prototype) {
        
        data_ = std::vector<T> (size, prototype);
        refs_.reserve(size);
        for (unsigned int i = 0; i < size; i++) {
            refs_[i] = &data_[i];
        }
        first_inactive_ = refs_.size();
    }
    
    T* getinactive();
    void setinactive(T* free);
    
};

template <class T>
T* respool::getinactive() {
    if (first_inactive_ == 0)
        return 0;
    first_inactive_--;
    // pop first free
    Passage* free = refs_[first_inactive_];
    refs_[first_inactive_] = 0;
    return free;
}

template<class T>
void respool::setinactive(T* free) {
    refs_[first_inactive_] = free;
    first_inactive_++;
}

#endif /* defined(__puro__resource__) */