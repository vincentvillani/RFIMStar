/*
 * UnitTests.h
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_UNITTESTS_H_
#define HEADER_UNITTESTS_H_


#include "RFIMStar.h"


void RunAllUniTests();


//Unit test to check that calculating the mean of a signal works correctly
void MeanUnitTest();

//Make sure the the covariance matrix is being calculated correctly
void CovarianceMatrixUnitTest();

void EigenvectorSolverUnitTest();

//Pack and unpack filterbank files without doing RFIM
//Tests to see if the unpack and pack functionality works correctly
//The input and output filterbank files should be identical
void PackingUnpackingUnitTest();


#endif /* HEADER_UNITTESTS_H_ */
