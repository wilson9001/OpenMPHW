//Calculations completed in 203.000000 seconds with 1 thread.
//Calculations completed in 199.000000 seconds with 2 threads.
//Calculations completed in 213.000000 seconds with 4 threads.
//Calculations completed in 183.000000 seconds with 8 threads.

/*
  This program is an adaptation of the Mandelbrot program
  from the Programming Rosetta Stone, see
  http://rosettacode.org/wiki/Mandelbrot_set

  Compile the program with:

  gcc -o mandelbrot -O4 mandelbrot.c

  Usage:
 
  ./mandelbrot <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>

  Example:

  ./mandelbrot 0.27085 0.27100 0.004640 0.004810 1000 1024 pic.ppm

  The interior of Mandelbrot set is black, the levels are gray.
  If you have very many levels, the picture is likely going to be quite
  dark. You can postprocess it to fix the palette. For instance,
  with ImageMagick you can do (assuming the picture was saved to pic.ppm):

  convert -normalize pic.ppm pic.png

  The resulting pic.png is still gray, but the levels will be nicer. You
  can also add colors, for instance:

  convert -negate -normalize -fill blue -tint 100 pic.ppm pic.png

  See http://www.imagemagick.org/Usage/color_mods/ for what ImageMagick
  can do. It can do a lot.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

typedef unsigned char PIXEL[6];
const static int THREADCOUNT = 8;

int main(int argc, char *argv[])
{
  /* Parse the command line arguments. */
  if (argc != 8)
  {
    printf("Usage:   %s <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>\n", argv[0]);
    printf("Example: %s 0.27085 0.27100 0.004640 0.004810 1000 1024 pic.ppm\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* The window in the plane. */
  const double xmin = atof(argv[1]);
  const double xmax = atof(argv[2]);
  const double ymin = atof(argv[3]);
  const double ymax = atof(argv[4]);

  /* Maximum number of iterations, at most 65535. */
  const uint16_t maxiter = (unsigned short)atoi(argv[5]);

  /* Image size, width is given, height is computed. */
  const int xres = atoi(argv[6]);
  const int yres = (xres * (ymax - ymin)) / (xmax - xmin);

  /* Precompute pixel width and height. */
  double dx = (xmax - xmin) / xres;
  double dy = (ymax - ymin) / yres;

  //PIXEL **image = malloc(sizeof(PIXEL) * yres);

  PIXEL **image = malloc(yres * sizeof(PIXEL *));
	image[0] = malloc(yres * xres * sizeof(PIXEL));
	for(int a = 1; a < yres; a++)
  {
		image[a] = image[0] + a * xres;
  }

  double x, y; /* Coordinates of the current point in the complex plane. */
  //double u, v; /* Coordinates of the iterated point. */
  int i, j; /* Pixel counters */
  int k;    /* Iteration counter */

  time_t startTime = time(NULL);
  printf("Computation beginning\n");

  omp_set_num_threads(8);
  #pragma omp parallel for schedule(static)
  for (j = 0; j < yres; j++)
  {
    y = ymax - j * dy;
    for (i = 0; i < xres; i++)
    {
      double u = 0.0;
      double v = 0.0;
      double u2 = u * u;
      double v2 = v * v;
      x = xmin + i * dx;
      /* iterate the point */
      for (k = 1; k < maxiter && (u2 + v2 < 4.0); k++)
      {
        v = 2 * u * v + y;
        u = u2 - v2 + x;
        u2 = u * u;
        v2 = v * v;
      };
      /* compute  pixel color and write it to file */
      if (k >= maxiter)
      {
        /* interior */
        //const unsigned char black[] = {0, 0, 0, 0, 0, 0};
        //fwrite(black, 6, 1, fp);
        for(int z = 0; z < 6; z++)
        {
          image[j][i][z] = 0;
        }
      }
      else
      {
        /* exterior */
        //unsigned char color[6];
        /*PIXEL color;
        color[0] = k >> 8;
        color[1] = k & 255;
        color[2] = k >> 8;
        color[3] = k & 255;
        color[4] = k >> 8;
        color[5] = k & 255;*/

        image[j][i][0] = k >> 8;
        image[j][i][1] = k & 255;
        image[j][i][2] = k >> 8;
        image[j][i][3] = k & 255;
        image[j][i][4] = k >> 8;
        image[j][i][5] = k & 255;

        //fwrite(color, 6, 1, fp);
      };
    }
  }

  time_t endTime = time(NULL);
  printf("Calculations completed in %f seconds with %d thread(s).\nWriting to file...\n", difftime(endTime, startTime), THREADCOUNT);

  /* The output file name */
  const char *filename = argv[7];

  /* Open the file and write the header. */
  FILE *fp = fopen(filename, "wb");

  /*write ASCII header to the file*/
  fprintf(fp,
          "P6\n# Mandelbrot, xmin=%lf, xmax=%lf, ymin=%lf, ymax=%lf, maxiter=%d\n%d\n%d\n%d\n",
          xmin, xmax, ymin, ymax, maxiter, xres, yres, (maxiter < 256 ? 256 : maxiter));

  //copy image arrage to file.
  for (j = 0; j < yres; j++)
  {
    for (i = 0; i < xres; i++)
    {
      fwrite(image[j][i], 6, 1, fp);
    }
  }

  fclose(fp);

  /*for(i = 0; i < yres; i++)
  {
    for(j = 0; j < xres; j++)
    {
		  free(image[i][j]);
    }
	free((void *)image[j]);
  }*/

	//free((void *)image);

  return 0;
}