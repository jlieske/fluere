/**  
 * \file fluere_drawing.h  
 *
 * \brief The public interface for making a "fluere" drawing.
 *  
 * \author Jonathan Cross
 **/ 

#ifndef FLUERE_DRAWING_H
#define FLUERE_DRAWING_H


/** different styles we can draw */
typedef enum 
{
  flow,
  wave,
  spin,
  leaf,
  rays
} fluere_style;

typedef struct fluere_drawing_struct *fluere_drawing_ptr;


/** 
 * Allocates a new fluere drawing 
 */
fluere_drawing_ptr init_fluere_drawing(
    int width, 
    int height, 
    int num_knots,
    fluere_style style1,
    fluere_style style2 );

/** 
 * Fills the array "data" of image data for a fluere drawing.
 * data must already be allocated by the user of size width x height
 * before calling this function.  
 *
 * The data is in row major order; each unsigned character
 * corresponds to a pixel.
 */
void fill_pixels(fluere_drawing_ptr s,      /* in */
                 unsigned char* data);      /* out */ 

/**
 * Deletes a fluere drawing
 */
void delete_fluere_drawing(fluere_drawing_ptr s);


#endif
