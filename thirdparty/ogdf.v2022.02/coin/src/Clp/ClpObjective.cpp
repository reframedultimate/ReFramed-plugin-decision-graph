/* $Id: ClpObjective.cpp 1665 2011-01-04 17:55:54Z lou $ */
// Copyright (C) 2002, International Business Machines
// Corporation and others.  All Rights Reserved.
// This code is licensed under the terms of the Eclipse Public License (EPL).

#include "coin/CoinPragma.hpp"
#include "coin/ClpSimplex.hpp"
#include "coin/ClpObjective.hpp"

//#############################################################################
// Constructors / Destructor / Assignment
//#############################################################################

//-------------------------------------------------------------------
// Default Constructor
//-------------------------------------------------------------------
ClpObjective::ClpObjective () :
     offset_(0.0),
     type_(-1),
     activated_(1)
{

}

//-------------------------------------------------------------------
// Copy constructor
//-------------------------------------------------------------------
ClpObjective::ClpObjective (const ClpObjective & source) :
     offset_(source.offset_),
     type_(source.type_),
     activated_(source.activated_)
{

}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
ClpObjective::~ClpObjective ()
{

}

//----------------------------------------------------------------
// Assignment operator
//-------------------------------------------------------------------
ClpObjective &
ClpObjective::operator=(const ClpObjective& rhs)
{
     if (this != &rhs) {
          offset_ = rhs.offset_;
          type_ = rhs.type_;
          activated_ = rhs.activated_;
     }
     return *this;
}
/* Subset clone.  Duplicates are allowed
   and order is as given.
*/
ClpObjective *
ClpObjective::subsetClone (int,
                           const int * ) const
{
     std::cerr << "subsetClone not supported - ClpObjective" << std::endl;
     abort();
     return NULL;
}
/* Given a zeroed array sets nonlinear columns to 1.
   Returns number of nonlinear columns
*/
int
ClpObjective::markNonlinear(char *)
{
     return 0;
}
