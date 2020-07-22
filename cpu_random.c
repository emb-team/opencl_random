#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

unsigned int seed = 0;

//Random string generator based on W. Miller algorithm
void miller_random(unsigned char *string, unsigned int start, unsigned int end, int size)
{
    int a = 16807; // 7**5
    int m = 2147483647; // 2**31-1

    for (int i = 0; i < size; i+=4) {
        seed = (a * seed) % m;
        for (int j = 0; j < 4; j++)
            string[i+j] = ((seed & (0xff << j*8)) >> j*8) % (end - start + 1) + start;
    }
}
#define MAX_STR_LEN 32
void test_random(size_t reps)
{
    clock_t a, b;
    unsigned char *string = malloc(reps * MAX_STR_LEN);
    if (!string) {
	printf("malloc() failed. Exiting.\n");
	exit(1);
    }

    printf("Test for %ld reps\n", reps);
    a = clock();
    //according to UTF-8 encoding table there are symbols starting from U+0020 till U+007E
    //exclude space 0x20 and control symbols
    miller_random(string, 0x21, 0x7e, reps * MAX_STR_LEN);

    b = clock();

    /*char buf[MAX_STR_LEN + 1] = { 0 };
    for(unsigned int i = 0; i < reps; i++)
    {
        memcpy(buf, &string[i*MAX_STR_LEN], MAX_STR_LEN);
        printf("%s\n", buf);
    } printf("\n");
    */
    free(string);
    printf("Exec time: %d ms\n", (int) (b-a));
}

int main()
{
    seed = (int) time(NULL);

    //test_random(1);
    //test_random(200000);
    //test_random(1000000);
    //test_random(2000000);
    //test_random(5000000);
    test_random(10000000);

    return 0;
}
