/**  
 * \file palettes.h  
 *
 * \brief This contains the public interface for reading palettes from a file,
 * and creating color tables based on these palettes.
 *  
 * \author Jonathan Cross
 **/ 

#ifndef PALETTES_H
#define PALETTES_H

#include <stdio.h>

typedef struct palette_list_struct *palette_list_ptr;
typedef struct palette_struct *palette_ptr;


palette_list_ptr init_palette_list( FILE *palfile );
void delete_palette_list( palette_list_ptr p );

int get_number_of_palettes( palette_list_ptr p );
palette_ptr get_palette( palette_list_ptr p, int idx );
char* get_name(palette_ptr p);

void get_colortable( palette_ptr p, 
                     unsigned char *ctable, 
                     int randomize, 
                     int stripes);

#endif
