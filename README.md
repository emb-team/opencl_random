# opencl_random
Random string generation test on CPU and GPU

See test.log to check the performance.

The OpenCL GPU test uses 2000 Work Items to generate random strings in parallel.
Memory copy from VRAM to DRAM is very expensive operation and affects performance negatively.

The most efficient way to initialize VRAM memory and let OpenCL kernels to do the computation job.
Result returning to the host shall be in a short format.
