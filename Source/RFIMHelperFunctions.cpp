/*
 * RFIMHelperFunctions.cpp
 *
 *  Created on: 13 May 2016
 *      Author: vincentvillani
 */

#include "../Header/RFIMHelperFunctions.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>

#ifdef BUILD_WITH_MKL
#include <mkl.h>
#include <mkl_lapacke.h>
#endif



#define POWER_THRESHOLD (0.3f)

float CalculateMean(float* dataArray, uint64_t dataLength)
{
#ifdef BUILD_WITH_MKL

	float result = cblas_sasum(dataLength, dataArray, 1);

	return result / dataLength;
#else
	float result = 0;

	for(uint64_t i = 0; i < dataLength; ++i)
	{
		result += dataArray[i];
	}

	return result / dataLength;
#endif

}


float CalculateStandardDeviation(float* dataArray, uint64_t dataLength)
{

	float mean = CalculateMean(dataArray, dataLength);

	float result = 0;

	for(uint64_t i = 0; i < dataLength; ++i)
	{
		//Square the difference and sum it
		result += powf((dataArray[i] - mean), 2.0f);
	}

	return sqrtf(result / dataLength);
}




float CalculateStandardDeviation(float* dataArray, uint64_t dataLength, float mean)
{
	float result = 0;

	for(uint64_t i = 0; i < dataLength; ++i)
	{
		//Square the difference and sum it
		result += powf((dataArray[i] - mean), 2.0f);
	}

	return sqrtf(result / dataLength);
}




int compareFunction(const void* a, const void* b)
{
	float floata = (*(float*)a);
	float floatb = (*(float*)b);

	if(floata > floatb)
		return 1;
	else if(floata < floatb)
		return -1;
	else
		return 0;
}

float CalculateMedian(float* dataArray, uint64_t dataLength)
{
	//Sort the data
	qsort(dataArray, dataLength, sizeof(float), &compareFunction);

	//return the median of the sorted data
	return dataArray[ (uint64_t)(dataLength / 2.0f) ];
}


float CalculateMeanAbsoluteDeviation(float* dataArray, float* workingSpace, uint64_t dataLength)
{
	//Copy the data into the working space
	memcpy(workingSpace, dataArray, dataLength * sizeof(float));

	float median1 = CalculateMedian(workingSpace, dataLength);

	//printf("Median1: %f\n", median1);

	//Now get the absolute deviations from that median
	for(uint64_t i = 0; i < dataLength; ++i)
	{
		workingSpace[i] = fabs(workingSpace[i] - median1);
	}

	//Find the median of the absolute deviations
	float median2 = CalculateMedian(workingSpace, dataLength);

	//printf("Median2: %f\n", median2);

	return median2;

}



//Used to detect outliers in a dataset
//If the absolute value of a modified z-score is higher than 3.5 it can be considered a potential outlier
//http://www.itl.nist.gov/div898/handbook/eda/section3/eda35h.htm
float CalculateModifiedZScore(float sample, float median, float meanAbsoluteDeviation)
{
	return  fabs( (0.6745f * (sample - median)) / meanAbsoluteDeviation );
}





void CalculateMeanMatrices(RFIMMemoryBlock* rfimMemBlock)
{

#ifdef BUILD_WITH_MKL

	float alphaOne = 1.0f / rfimMemBlock->h_numberOfSamples;
	float alphaTwo = 1;
	float beta = 0;

	uint64_t signalMatrixOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_numberOfSamples;
	uint64_t meanVecOffset = rfimMemBlock->h_valuesPerSample;
	uint64_t covarianceMatrixOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_valuesPerSample;


	//Compute the mean vector
	//We use the same d_onevec each time
	for(uint64_t i = 0; i < rfimMemBlock->h_batchSize; ++i)
	{

		//Calculate meanVec
		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasTrans,
				1, rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_numberOfSamples,
				alphaOne, rfimMemBlock->h_oneVec, 1,
				rfimMemBlock->h_inputSignal + (i * signalMatrixOffset), rfimMemBlock->h_valuesPerSample, beta,
				rfimMemBlock->h_meanVec + (i * meanVecOffset), 1);


		//Calculate mean outer product matrix
		cblas_sgemm(CblasColMajor, CblasTrans, CblasNoTrans,
				rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_valuesPerSample, 1,
				alphaTwo, rfimMemBlock->h_meanVec + (i * meanVecOffset), 1,
				rfimMemBlock->h_meanVec + (i * meanVecOffset), 1, beta,
				rfimMemBlock->h_covarianceMatrix + (i * covarianceMatrixOffset), rfimMemBlock->h_valuesPerSample);


	}
#endif


}


void CalculateCovarianceMatrix(RFIMMemoryBlock* rfimMemBlock)
{

#ifdef BUILD_WITH_MKL


	//Calculate the mean matrices
	CalculateMeanMatrices(rfimMemBlock);


	//Calculate the covariance matrices
	//Take the outer product of the signal with itself
	float alpha = 1.0f / rfimMemBlock->h_numberOfSamples;
	float beta = -1;

	uint64_t signalOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_numberOfSamples;
	uint64_t covarianceMatrixOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_valuesPerSample;




	for(uint64_t i = 0; i < rfimMemBlock->h_batchSize; ++i)
	{

		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasTrans,
				rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_numberOfSamples,
				alpha, rfimMemBlock->h_inputSignal + (i * signalOffset), rfimMemBlock->h_valuesPerSample,
				rfimMemBlock->h_inputSignal + (i * signalOffset), rfimMemBlock->h_valuesPerSample, beta,
				rfimMemBlock->h_covarianceMatrix + (i * covarianceMatrixOffset), rfimMemBlock->h_valuesPerSample);

	}


#endif

}


void EigenvalueSolver(RFIMMemoryBlock* RFIMStruct)
{


#ifdef BUILD_WITH_MKL

	//SYEV
	//Compute the SVD for each covariance matrix
	//EIGENVALUES AND VECTORS ARE FROM LOWEST TO HIGHEST
	//EIGENVALUES ARE PLACED IN RFIMStruct->h_S
	//EIGENVECTORS ARE PLACED IN RFIMStruct->h_covarianceMatrix
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{

		int info = LAPACKE_ssyev(LAPACK_COL_MAJOR, 'V', 'U', RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset),
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset));


		//Check to see if everything went ok
		if(info != 0)
		{
			//If info = -i, the i-th parameter had an illegal value
			//If info = i, then sgesdd did not converge, updating process failed
			fprintf(stderr, "Host_EigenvalueSolver: SVD computation didn't converge. Info: %d\n", info);
			exit(1);
		}
	}


	/*
	//SYEVR
	lapack_int info;
	//const char* sChar = 'S';
	//const char* charArg = (const char*)malloc(sizeof(const char));
	const char* test = "S";
	float abstol = slamch(test);
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{
		LAPACKE_ssyevr(LAPACK_COL_MAJOR, 'V', 'A', 'U',
				RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset),
				RFIMStruct->h_valuesPerSample,
				0, 0, 0, 0, abstol, &info, RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset),
				RFIMStruct->h_U + (i * RFIMStruct->h_UBatchOffset),
				RFIMStruct->h_valuesPerSample,
				(lapack_int*)RFIMStruct->h_VT);
		if(info != RFIMStruct->h_valuesPerSample)
		{
			//If info = -i, the i-th parameter had an illegal value
			//If info = i, then sgesdd did not converge, updataing process failed
			fprintf(stderr, "Host_EigenvalueSolver: Eigen computation didn't converge. Info: %d\n", info);
			exit(1);
		}
	}
	*/

	//SVD
	/*
	int info;
	//Compute the SVD for each covariance matrix
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{
		info =  LAPACKE_sgesdd(LAPACK_COL_MAJOR, 'A',
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset),
				RFIMStruct->h_U + (i * RFIMStruct->h_UBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_VT + (i * RFIMStruct->h_VTBatchOffset), RFIMStruct->h_valuesPerSample);
		//Check to see if everything went ok
		if(info != 0)
		{
			//If info = -i, the i-th parameter had an illegal value
			//If info = i, then sgesdd did not converge, updataing process failed
			fprintf(stderr, "Host_EigenvalueSolver: SVD computation didn't converge. Info: %d\n", info);
			exit(1);
		}
		/*
		//Tell the device to solve the eigenvectors
		cusolverStatus = cusolverDnSgesvd(*RFIMStruct->cusolverHandle, 'A', 'A',
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_S + (i * RFIMStruct->h_SBatchOffset),
				RFIMStruct->d_U + (i * RFIMStruct->h_UBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_VT + (i * RFIMStruct->h_VTBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_eigenWorkingSpace + (i * RFIMStruct->h_eigenWorkingSpaceBatchOffset),
				RFIMStruct->h_singleEigWorkingSpaceByteSize,
				NULL,
				RFIMStruct->d_devInfo + (i * RFIMStruct->h_devInfoBatchOffset));
	}
	*/

#endif

}


void EigenReductionAndFiltering(RFIMMemoryBlock* RFIMStruct, RFIMConfiguration* RFIMConfiguration)
{

#ifdef BUILD_WITH_MKL


	float alpha = 1;
	float beta = 0;

	uint64_t totalDimensionsRemoved = 0;


	//Values per sample = number of beams
	//Number of samples = number of samples in a window
	//batch size = number of freq channels


	//For each freq channel
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{


		//For each eigenvector * eigenvalue pair, multiply them and go through and see how many elements are larger than some threshold
		//A good threshold is probably around 4 or 5. (This is because RFI *tends* to have an eigenvalue larger than around 2 ~ 2.5

		//Count how many elements in the eigenvectors are above the threshold, if it's than three or less, we
		//leave it and just mark it in the mask file.

		uint32_t eigenVectorsToRemove = 0;

		//If it's greater than three, zero it out (the eigenvalue) and repeat this process for the other eigenvectors
		//Remember MKL returns eigenvectors from lowest to highest, so go backwards through the matrix
		for(uint32_t col = RFIMStruct->h_valuesPerSample - 1; col > 0; --col)
		{

			//Is this column even worth looking at?
			//Is it's eigenvalue above the threshold?
			if(RFIMStruct->h_S[ (i * RFIMStruct->h_SBatchOffset) + col ] < RFIMConfiguration->powerThreshold)
			{
				break;
			}

			//TODO: REMOVE THIS IF ADDING CODE COMMENTED BELOW BACK!
			eigenVectorsToRemove += 1;


			/*
			//Figure out the location of the column we are looking at
			float* currentCol = RFIMStruct->h_covarianceMatrix +
					(i * RFIMStruct->h_covarianceMatrixBatchOffset) + (col * RFIMStruct->h_valuesPerSample);



			for(uint32_t row = 0; row < RFIMStruct->h_valuesPerSample; ++row)
			{
				//Multiply each eigenvector element with it's corresponding eigenvalue
				RFIMStruct->h_outputSignal[row] = currentCol[row] * RFIMStruct->h_S[col];
			}

			float columnMean = CalculateMean(RFIMStruct->h_outputSignal, RFIMStruct->h_valuesPerSample);
			float columnMedian = currentCol[ RFIMStruct->h_valuesPerSample / 2 ];
			float columnMeanAbsoluteDeviation = CalculateMeanAbsoluteDeviation(RFIMStruct->h_outputSignal, RFIMStruct->h_outputSignal + RFIMStruct->h_valuesPerSample,
					RFIMStruct->h_valuesPerSample);

			//printf("Mean: %f\n", columnMean);
			//printf("Median: %f\n", columnMedian);
			//printf("MAD: %f\n", columnMeanAbsoluteDeviation);



			uint32_t numberOfPowerEigenvectorElements = 0;

			//Use the first RFIMStruct->h_valuesPerSample output signal elements as working space because allocating memory takes time...
			for(uint32_t row = 0; row < RFIMStruct->h_valuesPerSample; ++row)
			{


				//Check if it's an outlier
				float modifiedZScore = CalculateModifiedZScore(RFIMStruct->h_outputSignal[row], columnMedian, columnMeanAbsoluteDeviation);
				//printf("Sample: %f\n", RFIMStruct->h_outputSignal[row]);
				//printf("ModifiedZ: %f\n", modifiedZScore);

				//exit(1);

				//This is a potential outlier
				if( modifiedZScore > 3.5f)
				{
					numberOfPowerEigenvectorElements += 1;
				}

			}




			//Decision time...
			//To zero or not to zero

			//If greater than 3, zero
			if(numberOfPowerEigenvectorElements > 3)
			{
				eigenVectorsToRemove += 1;
			}
			//There is something(s) that is above the power threshold, but not enough to zero
			//Set the mask flag for this freq channel, because something interesting might be going on...
			else if(numberOfPowerEigenvectorElements == 1)
			{
				//TODO: Set the mask

				break;
			}
			//Nothing is going on here, just leave
			else
			{
				break;
			}

			*/


		}


		//eigenVectorsToRemove = 1;

		//If we are not removing anything we will just do the matrix multiplications for no reason...
		//Skip to the next freq channel
		if(eigenVectorsToRemove == 0)
			continue;


		totalDimensionsRemoved += eigenVectorsToRemove;



		//1. Put the signal into the eigenvector space. E(t)S
		cblas_sgemm(CblasColMajor, CblasTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha, RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample);




		//IF ZEROING
		if(RFIMConfiguration->rfimMode == ZERO)
		{
			//Set the appropriate number of columns to zero
			for(uint32_t col = RFIMStruct->h_valuesPerSample - 1; col > (RFIMStruct->h_valuesPerSample - 1) - eigenVectorsToRemove; --col)
			{

				//Figure out the location of the column we are looking at
				float* currentCol = RFIMStruct->h_covarianceMatrix +
						(i * RFIMStruct->h_covarianceMatrixBatchOffset) + (col * RFIMStruct->h_valuesPerSample);

				//Set to zero
				memset(currentCol, 0, sizeof(float) * RFIMStruct->h_valuesPerSample);
			}
		}
		else if(RFIMConfiguration->rfimMode == ATTENUATE)
		{

			float mean = CalculateMean(RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset),
					RFIMStruct->h_valuesPerSample - eigenVectorsToRemove);

			//TODO: I think this is ok?
			//Attenuate the appropriate eigenvectors
			for(uint32_t col = RFIMStruct->h_valuesPerSample - 1; col > (RFIMStruct->h_valuesPerSample - 1) - eigenVectorsToRemove; --col)
			{
				//Figure out the location of the column we are looking at
				float* currentCol = RFIMStruct->h_covarianceMatrix +
						(i * RFIMStruct->h_covarianceMatrixBatchOffset) + (col * RFIMStruct->h_valuesPerSample);

				//attenuation factor = mean / eigenvalue
				float attenuationFactor = mean / RFIMStruct->h_S[ (i * RFIMStruct->h_SBatchOffset) + col ];

				//Attenuate each element in the eigenvector
				for(uint32_t row = 0; row < RFIMStruct->h_valuesPerSample; ++row)
				{
					currentCol[row] *= attenuationFactor;
				}
			}

		}
		else
		{
			fprintf(stderr, "EigenReductionAndFiltering: RFIM mode not set...\n");
			exit(1);
		}






		//Do the inverse multiplication to bring it back to the original space
		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha, RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample);

	}



	//if(totalDimensionsRemoved > 0)
	//	printf("Dimensions removed: %llu\n", totalDimensionsRemoved);




#endif

}




