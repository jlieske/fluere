//
//  FluereView.h
//
//  Created by Jonathan Cross on 7/4/09.
//  Copyright (c) 2009, Jonathan Cross. All rights reserved.
//

#import <ScreenSaver/ScreenSaver.h>
#include "fluere_drawing.h"
#include "palettes.h"

// name of the configure sheet XIB file
#define kConfigSheetXIB @"ConfigureSheet"

// default values for the number of knots
#define kDefaultsModuleName       @"Flure_Num_Knots"
#define kDefaultsNumKnotsKey      @"numKnotsDefault"
#define kDefaultsNumKnotsValue    4

// state of the view; these typically cycle through
// calcState (compute a new drawing) -->
// fadeInState (animate the drawing fading in from black) -->
// normalState (animate the drawing) -->
// fadeOutState (animate the drawing, fading to black) -->
typedef enum 
  {
    calcState,
    fadeInState,
    normalState,
    fadeOutState
  } ViewState;


@interface FluereView : ScreenSaverView 
{
  IBOutlet id configureSheet_;
  IBOutlet id knotsTextBox_;

  
  // fractal data
  int numKnots_;
  int style1_;
  int style2_;
  BOOL stripes_;
  BOOL randomizePalette_;

  int width_;
  int height_;
  CGImageRef fractalImage_;

  int animCounter_;
  int animResetValue_;  // value when we go to a new drawing

  // the palette
  palette_list_ptr paletteList_;
  unsigned char colortable_[256 * 3 * 2];  // 256 colors * {rgb} * 2 cycles

  // image data
  unsigned char *imgData_;
  fluere_drawing_ptr  fractal_;

  // color stuff
  CGDataProviderRef theProvider_;
  CGColorSpaceRef rgbspace_;

  ViewState viewstate_;
  double fadeAmount_;
  BOOL doneCalculating_;
  
  // file numbering for screenshots
  int filenum_;
}


- (IBAction) moreInfoAction: (id) sender;
- (IBAction) okSheetAction: (id) sender;
- (ScreenSaverDefaults*) defaults;

- (void) makeColorTable;
- (void) newImage;
- (void) savePNGImage;


@end
