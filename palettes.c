/**  
 * \file palettes.c  
 *
 * \brief This contains functions for reading palettes from a file,
 * and creating color tables based on these palettes.
 *  
 * \author Jonathan Cross
 **/ 

#include <stdio.h>
#include <stdlib.h>
#include "palettes.h"

#define MAX_NAME_LENGTH 20


/** holds the red/green/blue components for a color */
struct rgbcolor_struct
{
  unsigned char r;  /**< red component */
  unsigned char g;  /**< green component */
  unsigned char b;  /**< blue component */
};
typedef struct rgbcolor_struct rgbcolor;


/** holds a list of palettes */
struct palette_list_struct
{
  int  num_palettes;    /**< number of palettes */
  palette_ptr palettes; /**< the palette data */
};
typedef struct palette_list_struct palette_list;


/** holds data for a single palette */
struct palette_struct
{
  char name[MAX_NAME_LENGTH];  /**< name of the palette */
  int  num_colors;             /**< number of colors in the palette */
  rgbcolor  *colors;           /**< the color data */
};
typedef struct palette_struct palette;


/**
 * Reads a "palette file" to initialize a palette of colors.  An example
 * palette file might look like this:
 
 \verbatim
   Number_of_palettes 3
   Cold        4 0x33ccff 0x0099ff 0x0033cc 0x0033ff
   Grayscale   6 0xffffff 0x333333 0xcccccc 0x999999 0x666666 0x000000
   Hot         5 0xffff33 0xffcc00 0xff6600 0xbb0033 0xff3300 
 \endverbatim
 * 
 * Caveats: (1) This does no error checking. (2) The names of the
 * palettes should be one word, no spaces. (3) Names can be at most
 * MAX_NAME_LENGTH characters long.
 */
palette_list_ptr init_palette_list( FILE *palfile )
{
  int ii;
  
  palette_list_ptr pl = malloc(sizeof(palette_list));

  /* get the number of palettes */
  fscanf(palfile, "%*s");  /* skip over "Number_of_palettes" */
  fscanf(palfile, "%d", &pl->num_palettes);
  pl->palettes = malloc( sizeof(palette) * pl->num_palettes);

  /* initialize each palette */
  for (ii = 0; ii < pl->num_palettes; ++ii)
  {
    int n_colors;
    int jj;

    /* get the name and the number of colors */
    fscanf(palfile, "%s", &pl->palettes[ii].name);  
    fscanf(palfile, "%d", &n_colors);
    pl->palettes[ii].num_colors = n_colors;

    /* get all the colors */
    pl->palettes[ii].colors = malloc(sizeof(rgbcolor) * n_colors);
    for (jj = 0; jj < n_colors; ++jj)
    {
      unsigned long color;
      fscanf(palfile, "%x", &color);
      pl->palettes[ii].colors[jj].r = (color & 0xff0000) >> 16;
      pl->palettes[ii].colors[jj].g = (color & 0x00ff00) >> 8;
      pl->palettes[ii].colors[jj].b = (color & 0x0000ff);
    }
  }

  return pl;
}

/**
 * Deletes a list of palettes
 */
void delete_palette_list( palette_list_ptr p )
{
  int ii;
  for (ii = 0; ii < p->num_palettes; ++ii)
  {
    free(p->palettes[ii].colors);
  }

  free(p->palettes);
  free(p);
}

/**
 * Returns the number of palettes in a palette_list
 */
int get_number_of_palettes( palette_list_ptr p )
{
  return p->num_palettes;
}

/**
 * Returns a particular palette in a list of palettes
 */
palette_ptr get_palette( palette_list_ptr p, int idx )
{
  return & p->palettes[idx];
}

/**
 * Returns the name of a palette 
 */
char* get_name(palette_ptr p)
{
  return p->name;
}

/**
 * Converts a double (that should be in the range 0..255) to an unsigned
 * character value.
 */
unsigned char double2char(double d)
{
  if (d > 255) d = 255;
  if (d < 0) d = 0;
  return (unsigned char) d;
}

/**
 * Blends two colors together; the parameter t controls the percentage
 * of the blend.  When t = 0, the first color is returned; when t = 1
 * the seconds color is returned; when t = 0.5 the colors are blended 
 * equally.
 */
rgbcolor blend(rgbcolor startcolor, rgbcolor endcolor, double t)
{
  rgbcolor mix;

  mix.r = double2char(((double)startcolor.r * (1.0 - t) + (double)endcolor.r * t) + 0.5);
  mix.g = double2char(((double)startcolor.g * (1.0 - t) + (double)endcolor.g * t) + 0.5);
  mix.b = double2char(((double)startcolor.b * (1.0 - t) + (double)endcolor.b * t) + 0.5);

  return mix;
}

/**
 * Makes a color table of 256 colors given a palette_ptr.  This can
 * be used to assign colors to any 256-color image.
 *
 * The color table must be allocated by the user prior to calling this
 * function; it should have size 256*3*2.  
 * At return, ctable contains the red/green/blue components for each
 * color in sequence; this takes 256*3 bytes.  These data are then
 * _repeated_ for the second 256*3 bytes (as if there were really 512
 * colors) to make color cycling easier.  
 *
 * If randomize is false, then the colors in palette_ptr are used in
 * order to generate the color table.  If randomize is true, then 
 * a random number of colors is used, and these are picked randomly
 * from the colors in palette_ptr.
 *
 * If stripes is true, then the color table alternates the colors in
 * the palette_ptr with black.  
 */
void get_colortable(
    palette_ptr p, 
    unsigned char *ctable, 
    int randomize, 
    int stripes)
{
  /* first, decide how many bands of color to make */
  int nsteps;
  if (randomize)
  {
    if (stripes)
    {
      /* pick 3 to 5 colors, so that there will be effectively
       6 to 10 bands of color with stripes. */
      nsteps = random() % 3 + 3; 
    }
    else
    {
      /* between 5 and 10 colors */
      nsteps = random() % 6 + 5;  
    }
  }
  else
  {
    /* just use the colors in the palette_ptr directly */
    nsteps = p->num_colors;
  }
  
  if (stripes)
    nsteps *= 2;  
  
  /* now, make the list of colors we'll use */
  rgbcolor colors[nsteps];
  rgbcolor black = {0,0,0};
  int ii;
  int cindx;

  for (cindx = 0, ii = 0; cindx < nsteps; ++cindx)
  {
    /* alternate black between the original colors */
    if (stripes && cindx % 2)
      colors[cindx] = black;
    else if (randomize)
      colors[cindx] = p->colors[random() % p->num_colors];
    else {
      colors[cindx] = p->colors[ii];
      ++ii;
    }
    
  }

  /* finally, actually fill in the colors in ctable by smoothly
   * blending between the list of colors above */
  for (ii = 0; ii < nsteps; ii++)
  {
    /* now, blend the colors */
    rgbcolor startcolor = colors[ii];
    rgbcolor endcolor = colors[(ii + 1) % nsteps];
    rgbcolor mixcolor;
    int idx;

    for (idx = ii* 256/nsteps; idx < (ii+1)*256/nsteps; idx++)
    {
      double t = nsteps/256.0 * (idx - ii*256/nsteps);

      mixcolor = blend(startcolor, endcolor, t);
      ctable[3*idx + 0] = mixcolor.r;
      ctable[3*idx + 1] = mixcolor.g;
      ctable[3*idx + 2] = mixcolor.b;
      
      /* make a copy to make cycling easier */
      ctable[3*(idx+256) + 0] = mixcolor.r;
      ctable[3*(idx+256) + 1] = mixcolor.g;
      ctable[3*(idx+256) + 2] = mixcolor.b;
    }
  }
}

/*
int main()
{
  FILE *palfile;
  palette_list_ptr p;

  palfile = fopen("palettes.txt","r");
  p = init_palette_list(palfile);
  close(palfile);

  delete_palette_list(p);

  return 0;
}
*/
