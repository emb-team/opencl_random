all: gpu_random cpu_random

gpu_random: gpu_random.c
	gcc -O3 -o gpu_random gpu_random.c -lOpenCL

cpu_random: cpu_random.c
	gcc -O3 -o cpu_random cpu_random.c

clean:
	rm -f cpu_random gpu_random
