//
//  resource.cpp
//  puro
//
//  Created by Olli Erik Keskinen on 20/01/14.
//  Copyright (c) 2014 Olli Erik Keskinen. All rights reserved.
//

#include "resource.h"



template <class A_Type> A_Type calc<A_Type>::multiply(A_Type x,A_Type y) {
    return x*y;
}
template <class A_Type> A_Type calc<A_Type>::add(A_Type x, A_Type y)
{
    return x+y;
}
