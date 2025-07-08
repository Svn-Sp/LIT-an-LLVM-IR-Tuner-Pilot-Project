#include <stdio.h>
#include <string.h>
#include <libgen.h>

void myadd(float *sum,float *addend) {
/*
c   Simple adding subroutine thrown in to allow subroutine
c   calls/returns to be factored in as part of the benchmark.
*/
      *sum = *sum + *addend;
}


__attribute__((section("OPTIMIZABLE_SECTION")))
float computation(float ztot, float yran, float ymult, float ymod, float x, float y, float z, float pi, float prod, long int low, long int ixran, long int itot, long int j, long int iprod){
      for(j=1; j<=itot; j++) {
/*
c   X and Y are two uniform random numbers between 0 and 1.
c   They are computed using two linear congruential generators.
c   A mix of integer and real arithmetic is used to simulate a
c   real program.  Magnitudes are kept small to prevent 32-bit
c   integer overflow and to allow full precision even with a 23-bit
c   mantissa.
*/

        iprod = 27611 * ixran;
        ixran = iprod - 74383*(long int)(iprod/74383);
        x = (float)ixran / 74383.0;
        prod = ymult * yran;
        yran = (prod - ymod*(long int)(prod/ymod));
        y = yran / ymod;
        z = x*x + y*y;
        myadd(&ztot,&z);
        if ( z <= 1.0 ) {
          low = low + 1;
        }
      }
      pi = 4.0 * (float)low/(float)itot;
      return pi;
}

int main(int argc, char *argv[]) {
   float ztot, yran, ymult, ymod, x, y, z, pi, prod;
   long int low, ixran, itot, j, iprod;

      ztot = 0.0;
      low = 1;
      ixran = 1907;
      yran = 5813.0;
      ymult = 1307.0;
      ymod = 5471.0;
      itot = 4000000;

      pi = computation(ztot, yran, ymult, ymod, x, y, z, pi, prod, low, ixran, itot, j, iprod);
      printf("%f\n", pi);
      
      // Get the directory of the current file
      char filepath[256];
      strcpy(filepath, __FILE__);
      char *dir = dirname(filepath);
      char output_path[512];
      snprintf(output_path, sizeof(output_path), "%s/output.txt", dir);
      
      FILE *file = fopen(output_path, "w");
      fprintf(file, "%f\n", pi);
      fclose(file);
      return 0;
}