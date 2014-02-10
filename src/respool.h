//
//  resource.h
//  puro
//
//  Created by Olli Erik Keskinen on 20/01/14.
//  Copyright (c) 2014 Olli Erik Keskinen. All rights reserved.
//

#pragma once

template <class T>
class respool {
    
    std::vector<T> data_;
    std::vector<T *> refs_;
    unsigned int first_inactive_;
    
public:
    respool();
    void assign(unsigned int size, T prototype);
    T* getinactive();
    void setinactive(T* free);
};

/*
 template<typename T> CircBuff<T>::CircBuf<T>(int i) {
 data = new T[i];
 }
 */

template <class T>
void
respool<T>::assign(unsigned int size, T prototype) {
    
    data_.assign(size, prototype);
    refs_.reserve(size);
    for (unsigned int i = 0; i < size; i++) {
        refs_[i] = &data_[i];
    }
    first_inactive_ = refs_.size();
}

template <class T>
T* respool<T>::getinactive() {
    if (first_inactive_ == 0)
        return 0;
    first_inactive_--;
    // pop first free
    T* free = refs_[first_inactive_];
    refs_[first_inactive_] = 0;
    return free;
}

template<class T>
void
respool<T>::setinactive(T* free) {
    refs_[first_inactive_] = free;
    first_inactive_++;
}