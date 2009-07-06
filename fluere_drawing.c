/*
 * \file fluere_drawing.c  
 * 
 * \brief The implementation for creating fluere drawings.
 * 
 * \author Jonathan Cross
 */

#include <stdlib.h>
#include <math.h>

#include "fluere_drawing.h"


/** holds an integer point */
struct point_struct
{
  int x;  /**<   x value */
  int y;  /**<   y value */
};
typedef struct point_struct point;

/** 
 * holds data for one "knot"
 *
 * Knots control the appearance of a fluere drawing.  Essentially,
 * the value (color) of each point in the drawing is some function
 * related to the distance or angle to each of the knots.
 */
struct knot_struct
{
  /*@{*/
  /** location of the point */
  double x;
  double y;
  /*@}*/
  
  /*@{*/
  /** ---- used for "flow" ---- */
  double flowsign;     /**< +/-1; is the knot a source or sink for flow? */
  /*@}*/
  
  /*@{*/
  /** ---- used for "spin" ---- */
  double spinsign;   /**< +/-1; clockwise or counterclockwise? */
  double sectors;    /**< n/(2 pi), where n is the number of "spokes" going to the point */
  double amplitude;   /*< if the spins are "twisted" then these control */
  double frequency;   /*< the size and shape of the twists.*/
  double decay;
  /*@}*/
  
  /*@{*/
  /** ---- used for "wave" ---- */
  double wavesign;
  /*@}*/
  
  /*@{*/
  /** ---- used for "leaf" ---- */
  int leafsign;
  /*@}*/
  
  /*@{*/
  /** ---- used for "rays" ---- */
  int rayssign;
  /*@}*/
  
};
typedef struct knot_struct knot;


/** this holds the parameters to make a fluere drawing */
struct fluere_drawing_struct
{
  /** we can show up to 2 styles at once */
  int style1;    
  int style2;

  /** When drawing leaves or rays, should it be continuous or discrete?
   * If these values are 1, then this will be continuous; larger values
   * give increasingly larger discrete angle sections.*/
  int leafdiscrete;  
  int raysdiscrete;

  int num_knots; /**< number of knots */
  knot *knots;   /**< the knot data */

  int width;     /**< width of the drawing */
  int height;    /**< height of the drawing */
};
typedef struct fluere_drawing_struct fluere_drawing;

/** private declarations */

double max(double a, double b);
double min(double a, double b);
double drandom();
int coinflip();

void define_knots(fluere_drawing_ptr s);

unsigned char get_value( fluere_drawing_ptr s, point where );
unsigned char get_spin_value( fluere_drawing_ptr s, point where );
unsigned char get_flow_value( fluere_drawing_ptr s, point where );
unsigned char get_wave_value( fluere_drawing_ptr s, point where );
unsigned char get_leaf_value( fluere_drawing_ptr s, point where );
unsigned char get_rays_value( fluere_drawing_ptr s, point where );


/** @name Public Interface */
/*@{*/

/**
 * Makes a new fluere drawing.
 *
 * Basic parameters for the drawing (width, height, number of knots,
 * and the drawing styles) are passed in; everything else is chosen
 * randomly within the function.
 */
fluere_drawing_ptr init_fluere_drawing(
    int width, 
    int height, 
    int num_knots,
    fluere_style style1,
    fluere_style style2 )
{
  fluere_drawing_ptr sd;

  sd = malloc(sizeof(fluere_drawing));

  sd->width = width;
  sd->height = height;
  sd->style1 = style1;
  sd->style2 = style2;
  sd->leafdiscrete = 1 + 3*(random() % 3);  /* 1,4,7 */
  sd->raysdiscrete = 1 + 3*(random() % 3);  /* 1,4,7 */

  sd->num_knots = num_knots;
  sd->knots = malloc(sizeof(knot) * num_knots);
  define_knots(sd);

  return sd;
}

/**
 * given a fluere drawing s, this fills in the image data for the 
 * drawing.  
 *
 * The caller should allocate the data prior to calling this function;
 * the data should have size width*height*1.
 * Each pixel will be assigned a number 0 to 255.  Note that in the  
 * screensaver, each image is constant; the animation is done by changing 
 * only the color table (i.e., what colors each of the numbers 0 to 255
 * stand for.
 */
void fill_pixels(fluere_drawing_ptr s,
                 unsigned char* data)
{
  int row;
  int col;
  point where;

  for (row = 0; row < s->height; ++row)
  {
    for (col = 0; col < s->width; ++col)
    {
      where.x = col;
      where.y = row;
      
      data[row * s->width + col] = get_value(s, where);
    }
  }
}

/**
 * frees the memory for a fluere drawing
 */
void delete_fluere_drawing(fluere_drawing_ptr s)
{
  free(s->knots);
  free(s);
}

/*@}*/

/** @name Private utility functions */
/*@{*/

/**
 * returns the larger of a,b
 */
double max(double a, double b)
{
  return (a > b) ? a : b;
}

/**
 * returns the smaller of a,b
 */
double min(double a, double b)
{
  return (a < b) ? a : b;
}

/**
 * returns a random number uniformly in the interval [0,1).
 */
double drandom()
{
  return (double) random() / (double) RAND_MAX;
}

/**
 * returns 0 or 1 with probability 1/2
 */
int coinflip()
{
  return (random() % 2 == 1);
}

/*@}*/

/** @name Private drawing functions */
/*@{*/

/*
 * Define the locations and characteristics of each of the knots.
 */
void define_knots(fluere_drawing_ptr s)
{
  int ii;
  double zoom = 1.1;  /* magnification factor */
  double origin_x = 0.5 * (zoom - 1.0) * s->width; 
  double origin_y = 0.5 * (zoom - 1.0) * s->height;
  
  for (ii = 0; ii < s->num_knots; ++ii)
  {
    /* find the location of each knot.  If the zoom level is 1,
     * then all the knots will lie in the screen area.  If zoom > 1, 
     * then some knots may lie outside the screen; as zoom --> 0
     * the knots will appear near the center of the screen.
     */
    s->knots[ii].x = zoom * s->width * drandom() - origin_x;
    s->knots[ii].y = zoom * s->height * drandom() - origin_y;

    /* for each of the drawing types, give a sign for the knot to
     * determine whether colors will be cycling in-or-out, or
     * clockwise-or-counterclockwise.
     */
    s->knots[ii].flowsign = coinflip() ? 1.0 : -1.0;
    s->knots[ii].spinsign = coinflip() ? 1.0 : -1.0;
    s->knots[ii].leafsign = coinflip() ? 1.0 : -1.0;
    s->knots[ii].rayssign = coinflip() ? 1.0 : -1.0;
    s->knots[ii].wavesign = coinflip() ? 1.0 : -1.0;

    /* for spin: how many "spokes" (palette rotations) will the knot have? */
    int nspokes = 1 + random() % 7;  /* 1, 2, ..., 7 */
    s->knots[ii].sectors = nspokes / (2 * M_PI);

    /* also for spin, set the characteristics of the additional 
     * waviness.  Note that amplitude has a 50% chance of being
     * 0, in which case there is no waviness.  The formula for 
     * the exponential decay factor was found to give visually pleasing
     * results. */
    s->knots[ii].frequency = 6*drandom() + 3; /* 3 to 9 */
    s->knots[ii].amplitude = coinflip() ? 0 : 
        8 * s->knots[ii].frequency / (nspokes*nspokes);
    s->knots[ii].decay = 20 + drandom()*30;  /* 20 to 50 */
  }
}

/*
 * computes the pixel value for any given pixel in the drawing
 */
unsigned char get_value( fluere_drawing_ptr s, point where )
{
  int drawingstyle;
  unsigned char value;

  /* fill the two styles in a checkerboard fashion */
  if ((int) (where.x + where.y) % 2 == 0)  
    drawingstyle = s->style1;
  else
    drawingstyle = s->style2;
  
  switch (drawingstyle)
  {
    case flow:  value = get_flow_value(s, where);  break;
    case spin:  value = get_spin_value(s, where);  break;
    case wave:  value = get_wave_value(s, where);  break;
    case leaf:  value = get_leaf_value(s, where);  break;
    case rays:  value = get_rays_value(s, where);  break;
    default:    value = 0;
  }
  
  return value;
}

/*
 * determines the value for a particular pixel for a "spin"-style drawing.
 *
 * Generally, the value is a function of the angle of each point to 
 * each knot.  To make a more wavy/spiraly effect, also add in a sin
 * component, which drops off exponentially with distance.
 */
unsigned char get_spin_value( fluere_drawing_ptr s, point where )
{
  int ii;
  double val = 0.0;
  
  for (ii = 0; ii < s->num_knots; ii++)
  {
    double dx = where.x - s->knots[ii].x;
    double dy = where.y - s->knots[ii].y;
    double r = sqrt(dx*dx + dy*dy);

    double a;    
    if (dx == 0 && dy == 0)
      a = 0.0;
    else
      a = atan2(dy,dx);
   
    /* wavy! */
    a += s->knots[ii].amplitude * s->knots[ii].sectors *
         sin(r/s->knots[ii].frequency) * exp(-r/s->knots[ii].decay);
        
    a = s->knots[ii].sectors * fmod(a, 1.0 / s->knots[ii].sectors);
    val += s->knots[ii].spinsign * a;
  }
  
  return (int) (256*val) % 256;
}

/**
 * determines the value for a particular pixel for a "flow"-style drawing.
 *
 * the value for each point is based on the distance to each of the knots.
 */
unsigned char get_flow_value( fluere_drawing_ptr s, point where )
{
  int ii;
  double val = 0.0;
  
  for (ii = 0; ii < s->num_knots; ii++)
  {
    double dx = where.x - s->knots[ii].x;
    double dy = where.y - s->knots[ii].y;
    
    val += s->knots[ii].flowsign * log(dx*dx + dy*dy);
  }
  val *= 100/s->num_knots;
  
  return (int) val % 256;
}

/**
 * determines the value for a particular pixel for a "wave"-style drawing.
 *
 * This is similar to flow, but adds a sin to make colors reflect.
 */
unsigned char get_wave_value( fluere_drawing_ptr s, point where )
{
  int ii;
  double val = 0.0;
  
  for (ii = 0; ii < s->num_knots; ii++)
  {
    double dx = where.x - s->knots[ii].x;
    double dy = where.y - s->knots[ii].y;
    
    val += s->knots[ii].wavesign * sin(1.5 * log(dx*dx + dy*dy));
  }
  val *= 100/s->num_knots;
  
  return (int) val % 256;
}

/**
 * determines the value for a particular pixel for a "leaf"-style drawing.
 */
unsigned char get_leaf_value( fluere_drawing_ptr s, point where )
{
  int ii;
  int val = 0;
  
  for (ii = 0; ii < s->num_knots; ii++)
  {
    double dx = where.x - s->knots[ii].x;
    double dy = where.y - s->knots[ii].y;
    
    double big =   max( abs(dx), abs(dy) );
    double small = min( abs(dx), abs(dy) );

    double a;  
    if (big == 0)
      a = 0.0;
    else    
      a = s->knots[ii].leafsign * 75 * (small/big) * (small/big);
    
    /* if leafdiscrete = 1, this does nothing, and the result is a
     * smooth picture.  If leafdiscrete > 1, then each knot will have
     * discrete colors at various angles.  The larger leafdiscrete is,
     * the coarser the angles.
     */
    val += ((int) a / s->leafdiscrete) * s->leafdiscrete;
  }

  return (int) val % 256;
}

/**
 * determines the value for a particular pixel for a "rays"-style drawing.
 *
 * This is almost identical to the "leaf" style; the only difference
 * is that the value varies quadratically with the ratio of dy/dx.
 */
unsigned char get_rays_value( fluere_drawing_ptr s, point where )
{
  int ii;
  int val = 0;
  
  for (ii = 0; ii < s->num_knots; ii++)
  {
    double dx = where.x - s->knots[ii].x;
    double dy = where.y - s->knots[ii].y;
    
    double big =   max( abs(dx), abs(dy) );
    double small = min( abs(dx), abs(dy) );

    double a;  
    if (big == 0)
      a = 0.0;
    else
      a = s->knots[ii].rayssign * 75 * (small/big) * (small/big); 
    
    val += ((int) a / s->raysdiscrete) * s->raysdiscrete;
  }
  
  return (int) val % 256;
}

/*@}*/
