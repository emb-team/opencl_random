__kernel void miller_generator(uint seed, uchar start, uchar end, uint len,
    __global uint *res_g)
{
  unsigned int a = 16807;
  unsigned int m = 2147483647;
  int gid = get_global_id(0);

  seed = seed * (gid + 1);

  unsigned int final, val;
  res_g = res_g + gid*len/4;
  for (int i = 0; i < len/4; i++) {
    final = 0;
    seed = (a * seed) % m;
    for (int j = 0; j < 4; j++) {
        val = 0;
        val = ((seed & (0xff << j*8)) >> j*8) % (end - start + 1) + start;
        final = final | (val << j*8);
    }
    res_g[i] = final;
  }
}
