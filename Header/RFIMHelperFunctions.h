/*
 * RFIMHelperFunctions.h
 *
 *  Created on: 13 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_RFIMHELPERFUNCTIONS_H_
#define HEADER_RFIMHELPERFUNCTIONS_H_

#include <stdint.h>

#include "RFIMMemoryBlock.h"

//Stats
//------------------------------------------------------
float CalculateMean(float* dataArray, uint64_t dataLength);

float CalculateStandardDeviation(float* dataArray, uint64_t dataLength);
float CalculateStandardDeviation(float* dataArray, uint64_t dataLength, float mean);

float CalculateMedian(float* dataArray, uint64_t dataLength);
float CalculateMeanAbsoluteDeviation(float* dataArray, float* workingSpace, uint64_t dataLength);
float CalculateModifiedZScore(float sample, float median);

//------------------------------------------------------


void CalculateMeanMatrices(RFIMMemoryBlock* RFIMStruct);


void CalculateCovarianceMatrix(RFIMMemoryBlock* RFIMStruct);


void EigenvalueSolver(RFIMMemoryBlock* RFIMStruct);


void EigenReductionAndFiltering(RFIMMemoryBlock* RFIMStruct);



#endif /* HEADER_RFIMHELPERFUNCTIONS_H_ */
