#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define CL_TARGET_OPENCL_VERSION 200
#include <CL/opencl.h>
#include <time.h>

#define MAX_WORK_ITEMS	2000
#define MAX_STR_LEN	32
#define MAX_STR_NUM	(size_t)10000000
int main(int argc, char** argv)
{
	int err;           // error code returned from api calls
	size_t global; // global domain size for our calculation
	size_t local;   // local domain size for our calculation
	int count = 0;
	unsigned int i = 0;
	FILE * f;
	clock_t a,b;
	char *result;

	cl_device_id device_id;      // compute device id
	cl_context context;          // compute context
	cl_command_queue commands;   // compute command queue
	cl_program program;          // compute program
	cl_kernel kernel;            // compute kernel
	cl_mem input;   // device memory used for the input
	cl_mem output;  // device memory used for the output
	cl_platform_id plat_ids[2];
	cl_uint num_platforms;

	// Fill our data set with random float values


	err = clGetPlatformIDs(2, plat_ids, &num_platforms);
	if(err != CL_SUCCESS)
	{
		printf("Error: Failed to get Platform IDs!\n");
		return EXIT_FAILURE;
	}

	printf("num_platforms = %d\n", num_platforms);

	int gpu = 1;
	err = clGetDeviceIDs(plat_ids[1], gpu ?
	   CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU,
	   1, &device_id, NULL);
	if(err != CL_SUCCESS)
	{
	   printf("Error: Failed to create a device group!\n");
	   return EXIT_FAILURE;
	}

	printf("Create a compute context\n");

	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (!context)
	{
	   printf("Error: Failed to create a compute context!\n");
	   return EXIT_FAILURE;
	}

	printf("Create a command commands\n");

	commands = clCreateCommandQueue(context, device_id, 0, &err);
	if (!commands)
	{
	   printf("Error: Failed to create a command commands!\n");
	   return EXIT_FAILURE;
	}

	f = fopen("random.cl", "r");
	if (f < 0) {
	   printf("Error: Failed to open OpenCL file!\n");
	   return EXIT_FAILURE;
	}

	fseek(f, 0, SEEK_END);
        size_t programSize = ftell(f);
        rewind(f);

	char *programBuffer = (char*) malloc(programSize + 1);
	if (!programBuffer) {
		printf("Error: Failed to allocate memory to program!\n");
		return EXIT_FAILURE;
	}

	programBuffer[programSize] = '\0';
	if (fread(programBuffer, sizeof(char), programSize, f) < 0) {
	   printf("Error: Failed to read OpenCL code!\n");
	   return EXIT_FAILURE;
	}

	fclose(f);

	printf("Create the compute program from the source buffer\n");

	program = clCreateProgramWithSource(context, 1,
	   (const char **) &programBuffer, &programSize, &err);
	if (!program)
	{
	   printf("Error: Failed to create compute program!\n");
	   return EXIT_FAILURE;
	}
	// Build the program executable

	printf("Build the program\n");

	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
	   size_t len;
	   char buffer[2048];

	   printf("Error: Failed to build program executable!\n");
	   clGetProgramBuildInfo(program, device_id,
	      CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
	   printf("%s\n", buffer);
	   exit(1);
	}

	result = (char *) malloc(MAX_STR_NUM*MAX_STR_LEN);
	if (result == NULL) {
		printf("malloc() failed\n");
		exit(1);
	}

	a = clock();
	printf("Create the compute kernel in the program we wish to run\n");

	kernel = clCreateKernel(program, "miller_generator", &err);
	if (!kernel || err != CL_SUCCESS)
	{
	   printf("Error: Failed to create compute kernel!\n");
	   exit(1);
	}

	printf("Create the input and output arrays in device memory\n");

	output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
	   MAX_STR_NUM*MAX_STR_LEN, NULL, NULL);
	if (!output)
	{
	   printf("Error: Failed to allocate device memory!\n");
	   exit(1);
	}

	printf("Set the arguments to our compute kernel\n");
	err = 0;
	time_t t; time(&t); unsigned char start, end;
	err  = clSetKernelArg(kernel, 0, sizeof(unsigned int), &t);
	start = 0x21; // U+0021 EXCLAMATION MARK
	err |= clSetKernelArg(kernel, 1, sizeof(unsigned char), &start);
	end = 0x7e; // U+007E TILDE
	err |= clSetKernelArg(kernel, 2, sizeof(unsigned char), &end);
	unsigned int size = MAX_STR_NUM*MAX_STR_LEN/MAX_WORK_ITEMS;
	err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &size);
	err |= clSetKernelArg(kernel, 4, sizeof(output), &output);

	if (err != CL_SUCCESS)
	{
	    printf("Error: Failed to set kernel arguments! %d\n", err);
	    exit(1);
	}
	printf("Get the maximum work group size for executing the kernel\n");

	err = clGetKernelWorkGroupInfo(kernel, device_id,
	    CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
	if (err != CL_SUCCESS)
	{
	    printf("Error: Failed to retrieve kernel work group info! %d\n", err);
	    exit(1);
	}

	// Execute the kernel over the entire range of
	// our 1d input data set

	global = MAX_WORK_ITEMS;
	err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL,
	    &global, &local, 0, NULL, NULL);
	if (err)
	{
	    printf("Error: Failed to execute kernel!\n");
	    return EXIT_FAILURE;
	}
	// Wait for the command commands to get serviced
	// before reading back result

	clFinish(commands);

	// Read back the results from the device for verification

	err = clEnqueueReadBuffer( commands, output, CL_TRUE,
	    0, MAX_STR_NUM*MAX_STR_LEN, result, 0, NULL, NULL );
	if (err != CL_SUCCESS)
	{
	    printf("Error: Failed to read output array! %d\n", err);
	    exit(1);
	}

	b = clock();
	printf("Exec time: %d microsecs\n", (int)(b-a));
	/*char buf[MAX_STR_LEN + 1] = { 0 };
	for(i = 0; i < MAX_STR_NUM; i++)
	{
		memcpy(buf, &result[i*MAX_STR_LEN], MAX_STR_LEN);
		printf("%s\n", buf);
	} printf("\n");*/

	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);
	free(result);
	free(programBuffer);

	return 0;
}
