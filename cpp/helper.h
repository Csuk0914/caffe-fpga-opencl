//
// Created by 张鑫语 on 8/20/16.
//

#ifndef C_VERSION_HELPER_H
#define C_VERSION_HELPER_H

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <map>
#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>
#include <cassert>
#include <fstream>
#include <glob.h>
#include <opencv2/highgui/highgui.hpp>
#include "./json/json.h"

#ifdef __APPLE__
#include <OpenCL/cl.h>
//#include <OpenCL/cl.hpp>
#else
#include <CL/cl.h>
//#include <CL/cl.hpp> // Xilinx Opencl doesn't support Opencl C++ Bindings.
#endif

#define XILINX_FPGA "Xilinx"
#define APPLE_MAC  "Apple"
#define INTEL_OPENCL  "Intel(R) OpenCL"
#define NVIDIA_CUDA  "NVIDIA CUDA"

#define ALPHA_DATA_KU3_DDR1 "xilinx:adm-pcie-ku3:1ddr:2.1"
#define ALPHA_DATA_KU3_DDR2 "xilinx:adm-pcie-ku3:2ddr:2.1"
#define ALPHA_DATA_7V3_DDR1  "xilinx:adm-pcie-7v3:1ddr:3.0"
#define DEVICE_GPU    CL_DEVICE_TYPE_GPU
#define DEVICE_CPU    CL_DEVICE_TYPE_CPU
#define DEVICE_FPGA   CL_DEVICE_TYPE_ACCELERATOR
#define CHOOSEN_DEVICE  ALPHA_DATA_KU3_DDR1

/////////////////////////////////////////////// Customized Settings ///////////////////////////////////////////////
#define PROJECT_NAME net
typedef float dType;

#define OPENCL_COMPILE_OPTION "-I/Users/xinyuzhang/Dropbox/Centos_WorkSpace/OpenCL/C++_Version/kernels/net"

#define OPENCL_VERSION OCL12
#define PLATFORM_FILTER APPLE_MAC
#define DISABLE_DEVICE_FILTER  true


// #define PLATFORM_FILTER NVIDIA_CUDA
// #define DISABLE_DEVICE_FILTER  true

//#define PLATFORM_FILTER XILINX_FPGA
//#define DISABLE_DEVICE_FILTER  false

//////////////////////////////////////////////////////////////////////////////// //////////////////////////////////

// Macro for adding quotes
#define STRINGIFY(X) STRINGIFY2(X)
#define STRINGIFY2(X) #X

// Macros for concatenating tokens
#define CAT(X,Y) CAT2(X,Y)
#define CAT2(X,Y) X##Y
#define CAT_2 CAT
#define CAT_3(X,Y,Z) CAT(X,CAT(Y,Z))
#define CAT_4(A,X,Y,Z) CAT(A,CAT_3(X,Y,Z))
#define INCLUDE_PROJECT_HEADER STRINGIFY(./RunOpenCL/PROJECT_NAME/PROJECT_NAME.h)

#define TO_STRING(x) #x
#define REPORT_ERR(x)  if(err != CL_SUCCESS) {std::cout << clErrorCode(err) << "\n"; return x;};
#define REPORT_ERRM(x,y)  if(err != CL_SUCCESS) {std::cout <<"ERROR: From " <<y <<" -> " << clErrorCode(err) << "\n"; return x;} else{cout<<"DEBUG: "<<y<<" -> Success"<<endl; }
#define REPORT_ERR_NR(y)  if(err != CL_SUCCESS) {std::cout <<"ERROR: From " <<y <<" -> " << clErrorCode(err) << "\n"; }else{cout<<"DEBUG: "<<y<<" -> Success"<<endl;};

#define DEBUG_LOG  cout<<"DEBUG: "
#define INFO_LOG  cout<<"INFO: "
#define ERROR_LOG  cout<<"ERROR: "

using namespace cv;
using namespace std;
//using json = nlohmann::json;


typedef  struct oclHardware {
    cl_platform_id mPlatform;
    cl_context mContext;
    cl_device_id mDevice;
    cl_command_queue mQueue;
    cl_device_type deviceType;
    short mMajorVersion;
    short mMinorVersion;
} oclHardware;

typedef  struct oclSoftware {
    cl_program mProgram;
    char mFileName[1024];
    char mCompileOptions[1024];
    map<std::string, cl_kernel> *kernelMap;
} oclSoftware;

//cl_kernel mKernel;
//char mKernelName[128];
typedef std::map<std::string, cl_kernel>::iterator itKernelMap;

typedef struct cmdArg{
    cl_device_type deviceType;
    char mFileName[1024];
    char mKernelName[128];
    char dataDir[1024];
    char network[1024];
} cmdArg;


oclSoftware getOclSoftware(const oclHardware &hardware, const char* kernelNames, const char* kernelFileName);

oclHardware getOclHardware(cl_device_type type, const char *target_device);


cl_int compileProgram(const oclHardware &hardware, oclSoftware &software);
void getDeviceVersion(oclHardware& hardware);
void printHelp();
cmdArg parseCmdArg(int argc, char** argv);
void release(oclSoftware& software);
void release(oclHardware& hardware);
int loadFile2Memory(const char *filename, char **result);
const char *clErrorCode(cl_int code);

typedef double (*RunOpenCL)(cmdArg arg, oclHardware hardware, oclSoftware software);
double runProgram(int argc, char** argv, RunOpenCL F);


Mat readImage(string fpath);
size_t imageSize(Mat m);
typedef vector<std::string>  FileList;
FileList ls(const std::string& pattern);
std::string getFileName(const std::string& s);

void split(const string &s, char delim, vector<string> &elems);
bool endsWith(const string& s, const string& suffix);
string trim(string& str);

/**
 * Network related declaration
 */
typedef struct {
    dType scale;
    int stride;
    int kernelSize;
    int pad;
    int dilation;
    int inputChannel;
    int inputHeight;
    int inputWidth;
    int outputChannel;
    int outputHeight;
    int outputWidth;
    int inputTotalDataNum;
    int outputTotalDataNum;
} NetParam;


typedef struct{
    dType *weight;
    dType *bias;
    int *weightShape; // (outputChannel, inputChannel, kernelSize, kernelSize)
    int *biasShape; // (outputChannel)
    long weight_data_num;
    long bias_data_num;
    int weight_dim_num;
    int bias_dim_num;
} WeightData;

enum LayerType {Convolution, Relu, Data, Split, Pooling, Accuracy, SoftmaxWithLoss, Output, Padding};
enum OpenCLVersion {OCL20, OCL12};

class Layer {
public:
    ~Layer();
    Layer(Json::Value);
    bool forward(oclHardware hardware, oclSoftware software, OpenCLVersion mode);
    bool freeCLMemory();
    cl_mem weightCL;
    cl_mem biasCL;
    cl_mem outputBufferCL;
    cl_mem inputBufferCL;
    cl_mem paramCL;
    size_t globalSize[3], localSize[3], offset[3];
    LayerType type;
    string kernelKey;
    WeightData learnedParam;
    NetParam param;
    Layer* next;
    bool phase;
    Layer* prev;
    dType *inputBuffer;
    dType *outputBuffer;
    std::string info;
};


class Net{
public:
    Layer **layers;
    int num_layers;
    string name;
    OpenCLVersion mode;
    bool freeCLMemory();
    Net(Json::Value, cmdArg arg,OpenCLVersion version);
    ~Net();
    bool forward(oclHardware hardware, oclSoftware software, dType *data);
    Layer* outputLayer();
};


template<typename T, typename U> U maxLabel(T* arr, U size){
    if(size == 0)
        return -1;
    if(size == 1)
        return 0;
    T max = arr[0];
    U maxIdx = 0;
    for(U i = 1; i<size;i++){
        if(max < arr[i]){
            max = arr[i];
            maxIdx = i;
        }
    }
    return maxIdx;
};
void print2D(dType *fm, int height, int width);



#endif //C_VERSION_HELPER_H
