/*
 * RFIMHelperFunctions.h
 *
 *  Created on: 13 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_RFIMHELPERFUNCTIONS_H_
#define HEADER_RFIMHELPERFUNCTIONS_H_


#include "RFIMMemoryBlock.h"


void CalculateMeanMatrices(RFIMMemoryBlock* RFIMStruct);


void CalculateCovarianceMatrix(RFIMMemoryBlock* RFIMStruct);


void EigenvalueSolver(RFIMMemoryBlock* RFIMStruct);


void EigenReductionAndFiltering(RFIMMemoryBlock* RFIMStruct);


#endif /* HEADER_RFIMHELPERFUNCTIONS_H_ */
