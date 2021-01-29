#include <iostream>
#include <cmath>
#include <cuda_runtime.h>

#include "../include/utils.h"
#include "../include/loadSaveImage.h"

static const int filterWidth = 9;
static const float filterSigma = 2.f;

void preProcess(uchar4 **h_inputImageRGBA, uchar4 **h_outputImageRGBA,
                uchar4 **d_inputImageRGBA, uchar4 **d_outputImageRGBA,
                unsigned char **d_redBlurred, unsigned char **d_red,
                unsigned char **d_greenBlurred, unsigned char **d_green,
                unsigned char **d_blueBlurred, unsigned char **d_blue,
                float **h_filter, float **d_filter,
                size_t &rows, size_t &cols,
                const std::string &filename){
    checkCudaErrors(cudaFree(0));
	
    loadImageRGBA(filename, h_inputImageRGBA, &rows, &cols);
	
    *h_outputImageRGBA = new uchar4[rows * cols];
	
    size_t numPixels = rows * cols;
    checkCudaErrors(cudaMalloc(d_inputImageRGBA, sizeof(uchar4) * numPixels));
    checkCudaErrors(cudaMalloc(d_outputImageRGBA, sizeof(uchar4) * numPixels));

    checkCudaErrors(cudaMemcpy(*d_inputImageRGBA, *h_inputImageRGBA, sizeof(uchar4) * numPixels, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset(*d_outputImageRGBA, 0, numPixels * sizeof(uchar4)));
	
    *h_filter = new float[filterWidth * filterWidth];

    float filterSum = 0.f; 
    for (int r = -filterWidth / 2; r <= filterWidth / 2; ++r){
        for (int c = -filterWidth / 2; c <= filterWidth / 2; ++c){
            float filterValue = expf(-(float)(c * c + r * r) / (2.f * filterSigma * filterSigma));
            (*h_filter)[(r + filterWidth / 2) * filterWidth + c + filterWidth / 2] = filterValue;
            filterSum += filterValue; // for normalization
        }
    }
	
    float normalizationFactor = 1.f / filterSum;
    for (int r = -filterWidth / 2; r <= filterWidth / 2; ++r)
        for (int c = -filterWidth / 2; c <= filterWidth / 2; ++c)
            (*h_filter)[(r + filterWidth / 2) * filterWidth + c + filterWidth / 2] *= normalizationFactor;
			
    checkCudaErrors(cudaMalloc(d_red, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMalloc(d_green, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMalloc(d_blue, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMalloc(d_redBlurred, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMalloc(d_greenBlurred, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMalloc(d_blueBlurred, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMemset(*d_redBlurred, 0, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMemset(*d_greenBlurred, 0, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMemset(*d_blueBlurred, 0, sizeof(unsigned char) * numPixels));
    checkCudaErrors(cudaMalloc(d_filter, sizeof(float) * filterWidth * filterWidth));
    checkCudaErrors(cudaMemcpy(*d_filter, *h_filter, sizeof(float) * filterWidth * filterWidth, cudaMemcpyHostToDevice));
}

void postProcess(const std::string &output_file, uchar4 *const h_outputImage, const uchar4 *const d_outputImage,
                 const int rows, const int cols){
    size_t numPixels = rows * cols;
    checkCudaErrors(cudaMemcpy(h_outputImage, d_outputImage, sizeof(uchar4) * numPixels, cudaMemcpyDeviceToHost));
    saveImageRGBA(h_outputImage, rows, cols, output_file);
}

__global__ void gaussian_blur_kernel(const unsigned char *const inputChannel,
                                     unsigned char *const outputChannel,
                                     const int numRows, const int numCols,
                                     const float *const filter, const int filterWidth){
    int j = blockIdx.x * blockDim.x + threadIdx.x;
    int i = blockIdx.y * blockDim.y + threadIdx.y;

    if (j >= numCols || i >= numRows)
        return;

    float result = 0.f;
    for (int filter_r = -filterWidth / 2; filter_r <= filterWidth / 2; ++filter_r){
        for (int filter_c = -filterWidth / 2; filter_c <= filterWidth / 2; ++filter_c){
            int image_r = min(max(i + filter_r, 0), numRows - 1);
            int image_c = min(max(j + filter_c, 0), numCols - 1);

            float image_value = static_cast<float>(inputChannel[image_r * numCols + image_c]);
            float filter_value = filter[(filter_r + filterWidth / 2) * filterWidth + filter_c + filterWidth / 2];

            result += image_value * filter_value;
        }
    }
    outputChannel[i * numCols + j] = static_cast<unsigned char>(result);
}

__global__ void separateChannels_kernel(const uchar4 *const inputImageRGBA,
                                        const int numRows, const int numCols,
                                        unsigned char *const redChannel,
                                        unsigned char *const greenChannel,
                                        unsigned char *const blueChannel){
    int j = blockIdx.x * blockDim.x + threadIdx.x;
    int i = blockIdx.y * blockDim.y + threadIdx.y;

    if (j >= numCols || i >= numRows)
        return;

    int tid = i * numCols + j;

    redChannel[tid] = inputImageRGBA[tid].x;
    greenChannel[tid] = inputImageRGBA[tid].y;
    blueChannel[tid] = inputImageRGBA[tid].z;
}

__global__ void recombineChannels_kernel(const unsigned char *const redChannel,
                                         const unsigned char *const greenChannel,
                                         const unsigned char *const blueChannel,
                                         uchar4 *const outputImageRGBA,
                                         const int numRows, const int numCols){
    int2 thread_2D_pos = make_int2(blockIdx.x * blockDim.x + threadIdx.x,
                                   blockIdx.y * blockDim.y + threadIdx.y);

    int thread_1D_pos = thread_2D_pos.y * numCols + thread_2D_pos.x;
	
    if (thread_2D_pos.x >= numCols || thread_2D_pos.y >= numRows)
        return;

    unsigned char red = redChannel[thread_1D_pos];
    unsigned char green = greenChannel[thread_1D_pos];
    unsigned char blue = blueChannel[thread_1D_pos];
	
    uchar4 outputPixel = make_uchar4(red, green, blue, 255);

    outputImageRGBA[thread_1D_pos] = outputPixel;
}

void cuda_gaussian_blur(const uchar4 *const h_inputImageRGBA, uchar4 *const d_inputImageRGBA,
                        uchar4 *const d_outputImageRGBA, const int numRows, const int numCols,
                        unsigned char *d_redBlurred, unsigned char *d_red,
                        unsigned char *d_greenBlurred, unsigned char *d_green,
                        unsigned char *d_blueBlurred, unsigned char *d_blue,
                        float *d_filter, const int filterWidth){
    int blockW = 32;
    int blockH = 32;

    dim3 blockSize(blockW, blockH);
	
    int gridW = (numCols % blockW != 0) ? (numCols / blockW + 1) : (numCols / blockW);
    int gridH = (numRows % blockH != 0) ? (numRows / blockH + 1) : (numRows / blockH);
    dim3 gridSize(gridW, gridH);

    separateChannels_kernel<<<gridSize, blockSize>>>(d_inputImageRGBA, numRows, numCols, d_red, d_green, d_blue);
    cudaDeviceSynchronize();
    checkCudaErrors(cudaGetLastError());
	
    gaussian_blur_kernel<<<gridSize, blockSize>>>(d_red, d_redBlurred, numRows, numCols, d_filter, filterWidth);
    gaussian_blur_kernel<<<gridSize, blockSize>>>(d_green, d_greenBlurred, numRows, numCols, d_filter, filterWidth);
    gaussian_blur_kernel<<<gridSize, blockSize>>>(d_blue, d_blueBlurred, numRows, numCols, d_filter, filterWidth);
    cudaDeviceSynchronize();
    checkCudaErrors(cudaGetLastError());

    recombineChannels_kernel<<<gridSize, blockSize>>>(d_redBlurred, d_greenBlurred, d_blueBlurred, d_outputImageRGBA, numRows, numCols);
    cudaDeviceSynchronize();
    checkCudaErrors(cudaGetLastError());
}

void gaussian_blur(const std::string &input_file, const std::string &output_file){
    size_t numRows, numCols;

    uchar4 *h_inputImageRGBA, *d_inputImageRGBA;
    uchar4 *h_outputImageRGBA, *d_outputImageRGBA;
    unsigned char *d_redBlurred, *d_greenBlurred, *d_blueBlurred;
    unsigned char *d_red, *d_green, *d_blue;
    float *h_filter, *d_filter;

    preProcess(&h_inputImageRGBA, &h_outputImageRGBA, &d_inputImageRGBA, &d_outputImageRGBA,
               &d_redBlurred, &d_red, &d_greenBlurred, &d_green, &d_blueBlurred, &d_blue,
               &h_filter, &d_filter, numRows, numCols, input_file);

    cuda_gaussian_blur(h_inputImageRGBA, d_inputImageRGBA, d_outputImageRGBA, numRows, numCols,
                       d_redBlurred, d_greenBlurred, d_blueBlurred, d_red, d_green, d_blue,
                       d_filter, filterWidth);
    cudaDeviceSynchronize();
    checkCudaErrors(cudaGetLastError());

    postProcess(output_file, h_outputImageRGBA, d_outputImageRGBA, numRows, numCols);

    checkCudaErrors(cudaFree(d_inputImageRGBA));
    checkCudaErrors(cudaFree(d_outputImageRGBA));
    checkCudaErrors(cudaFree(d_filter));
    checkCudaErrors(cudaFree(d_red));
    checkCudaErrors(cudaFree(d_green));
    checkCudaErrors(cudaFree(d_blue));
    checkCudaErrors(cudaFree(d_redBlurred));
    checkCudaErrors(cudaFree(d_greenBlurred));
    checkCudaErrors(cudaFree(d_blueBlurred));
    delete[] h_inputImageRGBA;
    delete[] h_outputImageRGBA;
    delete[] h_filter;
}