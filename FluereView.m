///
///  FluereView.m
///  Fluere
///
///  Created by Jonathan Cross on 7/4/09.
///  Copyright (c) 2009, Jonthan Cross. All rights reserved.
///

#import "FluereView.h"


@implementation FluereView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{
  self = [super initWithFrame:frame isPreview:isPreview];
  if (self)
  {

    // init the random number generator
    srandom([[NSDate date] timeIntervalSince1970]);

    width_ = frame.size.width;
    height_ = frame.size.height;

    // read the number of knots from defaults
    ScreenSaverDefaults* screenSaverDefaults = [self defaults];
    //  create a dictionary to contain our global default values
    NSDictionary* defaultDict = [NSDictionary
      dictionaryWithObject: [NSNumber numberWithInt:kDefaultsNumKnotsValue]
      forKey: kDefaultsNumKnotsKey];
    //  register those global defaults
    [screenSaverDefaults registerDefaults: defaultDict];
    //  now we can read in the current default value knowing
    //  that we'll always get the user defaults or our global defaults
    //  if no user defaults have been set
    numKnots_ = [screenSaverDefaults integerForKey: kDefaultsNumKnotsKey];

    // read the palette file
    FILE *palfile;  

    NSBundle  *saverBundle = [NSBundle bundleForClass: [self class]];
    NSString  *nspath  = [saverBundle pathForResource:@"palettes" ofType:@"txt"];
    const char *cpath = [nspath cStringUsingEncoding:NSASCIIStringEncoding];
    palfile = fopen(cpath,"r");
    if (palfile == NULL)
    {
      NSLog(@"FluereView: Couldn't load palette file: %s", cpath);
      exit(1);
    }
    paletteList_ = init_palette_list(palfile);
    fclose(palfile);

    fadeAmount_ = 0.0;  // completely faded
    viewstate_ = calcState;
    doneCalculating_ = FALSE;
    animCounter_ = 0;
    animResetValue_ = 12 *30*2;  //12 seconds (30 frames/sec* 2 tics/frame)

    filenum_ = 1;

    [self setAnimationTimeInterval:1/30.0];
  }
  return self;
}

- (void)startAnimation
{
  [super startAnimation];
}

- (void)stopAnimation
{
  [super stopAnimation];
}

- (void)drawRect:(NSRect)rect
{
  CGRect  cg = CGRectMake(NSMinX(rect), NSMinY(rect),
      NSWidth(rect), NSHeight(rect));
  NSGraphicsContext*  nscontext = [NSGraphicsContext currentContext];
  CGContextRef cgcontext = (CGContextRef) [nscontext graphicsPort];

  switch (viewstate_)
  {
    case normalState:   // draw the image
      CGContextDrawImage(cgcontext, cg, fractalImage_);
      break;

    case fadeInState:
    case fadeOutState:  // draw the image with fading
      CGContextSetRGBFillColor( cgcontext, 0.0, 0.0, 0.0, 1.0 );
      CGContextFillRect(cgcontext, cg);
      CGContextSetAlpha(cgcontext, fadeAmount_);
      CGContextDrawImage(cgcontext, cg, fractalImage_);      
      break;

    case calcState:
    default:            // draw a black screen
      CGContextSetRGBFillColor( cgcontext, 0.0, 0.0, 0.0, 1.0 );
      CGContextFillRect(cgcontext, cg);
      break;      
  }

}

- (void)animateOneFrame
{

  if (viewstate_ == calcState)
  {
    if (!doneCalculating_)
    {
      doneCalculating_ = TRUE;
      [self newImage];
    }

    [self setNeedsDisplay:YES];
    return;
  }

  // update the state
  switch (viewstate_)
  {
    case fadeInState:
      fadeAmount_ += 0.05;
      if (fadeAmount_ >= 1)
      {
        fadeAmount_ = 1.0;
        viewstate_ = normalState;
      }
      break;

    case fadeOutState:
      fadeAmount_ -= 0.05;
      if (fadeAmount_ <= 0)
      {
        fadeAmount_ = 0;
        viewstate_ = calcState;
        doneCalculating_ = TRUE;
        [self newImage];
      }
      break;

    case normalState:
      if (animCounter_ > animResetValue_)
      {
        viewstate_ = fadeOutState;
      }
      break;

    default:
      NSLog(@"How did I get here?");
  }

  animCounter_ += 2;



  // update the picture
  CGColorSpaceRef theColorspace;
  theColorspace = CGColorSpaceCreateIndexed(rgbspace_, 255, colortable_+3*(animCounter_ % 256));

  if (fractalImage_) CGImageRelease(fractalImage_);
  fractalImage_ = CGImageCreate (width_,
      height_,
      8,       // bitsPerComponent,
      8,      // bitsPerPixel,
      width_,  // bytesPerRow,
      theColorspace,
      kCGImageAlphaNoneSkipLast,  // ignore the alpha channel
      theProvider_,
      nil,     // const float decode[], -- color mapping array
      FALSE,   // shouldInterpolate,
      kCGRenderingIntentDefault);

  CGColorSpaceRelease( theColorspace );

  [self setNeedsDisplay:YES];

  return;
}

// this keeps Cocoa from unneccessarily redrawing our superview
- (BOOL) isOpaque
{
  return YES;
}

- (BOOL) hasConfigureSheet
{
  return YES;
}

- (NSWindow*) configureSheet
{
  if ( configureSheet_ == nil )
  {
    [NSBundle loadNibNamed: kConfigSheetXIB owner:self];
  }
  [knotsTextBox_ setIntValue:numKnots_];
  return configureSheet_;
}

- (IBAction) moreInfoAction: (id) sender
{
  NSTask *theProcess;
  theProcess = [[NSTask alloc] init];

  [theProcess setLaunchPath:@"/usr/bin/open"];
  // Path of the shell command we'll execute
  NSBundle  *saverBundle = [NSBundle bundleForClass: [self class]];
  NSString  *nspath  = [saverBundle pathForResource:@"README" ofType:@"html"];

  [theProcess setArguments: [NSArray arrayWithObject: nspath]];

  // Run the command
  [theProcess launch];

  [theProcess release];
}

- (IBAction) okSheetAction: (id) sender
{
  //  record the settings in the configuration sheet
  numKnots_ = [knotsTextBox_ intValue];
  if (numKnots_ < 1) numKnots_ = 1;    // sanity check the user!
  if (numKnots_ > 50) numKnots_ = 50;

  //  write out the current default so other instances of the saver pick up the change, too.
  [[self defaults] setInteger:numKnots_ forKey:kDefaultsNumKnotsKey];
  //  update the disk so that the screen saver engine will pick up the correct values
  [[self defaults] synchronize];

  viewstate_ = calcState;
  doneCalculating_ = FALSE;
  fadeAmount_ = 0;

  [NSApp endSheet: configureSheet_];
}

- (void) savePNGImage
{
  // get the path to the desktop
  NSArray * paths = NSSearchPathForDirectoriesInDomains (NSDesktopDirectory, NSUserDomainMask, YES);
  NSString * desktop_path = [paths objectAtIndex:0];

  // find the first file name that doesn't overwrite a previous file
  BOOL fileExists;
  NSString *filename;
  NSFileManager *fm = [NSFileManager defaultManager];
  do
  {
    filename = [NSString stringWithFormat:@"%@/Fluere %d.png",
             desktop_path, filenum_];
    fileExists = [fm fileExistsAtPath:filename];
    filenum_++;
  } while (fileExists);

  // write the image
  NSURL *outURL = [[NSURL alloc] initFileURLWithPath:filename]; 
  CGImageDestinationRef dr = CGImageDestinationCreateWithURL (
     (CFURLRef)outURL, (CFStringRef)@"public.png" , 1, NULL);
  CGImageDestinationAddImage(dr, fractalImage_, NULL);
  CGImageDestinationFinalize(dr);

}

- (void) keyDown: (NSEvent*) theEvent
{
  NSString* chars = [theEvent characters];
  unichar pressedChar = [chars characterAtIndex:0];

  if (pressedChar == NSTabCharacter)
  {
    if ((viewstate_ == normalState) || (viewstate_ = fadeInState))
    {
      viewstate_ = calcState;
      doneCalculating_ = FALSE;
      fadeAmount_ = 0;
      return;	  
    }
  }
  if (pressedChar == '3')
  {
    [self savePNGImage];
  }
  else
  {
    [super keyDown:theEvent];
  }
}

// make a new palette if the user presses control
- (void) flagsChanged: (NSEvent*) theEvent
{

  if (([theEvent modifierFlags] & NSAlternateKeyMask) && (viewstate_ == normalState))
  {
    [self makeColorTable];
    [self setNeedsDisplay:YES];
  }
}


- (void) newImage
{
  style1_ = random() % 5;
  style2_ = random() % 5;
  numKnots_ = [[self defaults] integerForKey: kDefaultsNumKnotsKey];


  if (fractal_)
    delete_fluere_drawing(fractal_);

  fractal_ = init_fluere_drawing(width_, height_, numKnots_, style1_, style2_);
  if (!imgData_)
    imgData_ = malloc(width_*height_);
  fill_pixels(fractal_, imgData_);

  [self makeColorTable];

  viewstate_ = fadeInState;
}

- (void) makeColorTable
{
  randomizePalette_ = random() % 2;
  stripes_ = random() % 2; 
  int palIndex = random() % get_number_of_palettes(paletteList_);  

  get_colortable(get_palette(paletteList_, palIndex),
      colortable_,
      randomizePalette_,
      stripes_);

  if (rgbspace_) CGColorSpaceRelease(rgbspace_);
  rgbspace_ = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

  if (theProvider_) CGDataProviderRelease(theProvider_);
  theProvider_ = CGDataProviderCreateWithData( NULL, 
      imgData_, 
      width_*height_, 
      NULL );

  animCounter_ = 0;
}

- (ScreenSaverDefaults*) defaults
{
  // there's no need to cache this value so we'll make a handy accessor
  return [ScreenSaverDefaults defaultsForModuleWithName: kDefaultsModuleName];
}


@end
