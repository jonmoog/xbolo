#import <Cocoa/Cocoa.h>
#include "list.h"

@class NSImage;

@class GSXBoloController;

@interface GSBoloView : NSView {
  struct ListNode rectlist;

  GSXBoloController *boloController;
  unsigned int modifiers;
}
+ (void)refresh;
+ (void)removeView:(GSBoloView *)view;

/* the controller receiving key and mouse events (connected by the nib
   historically; set explicitly now that the view is built in code) */
- (void)setBoloController:(GSXBoloController *)controller;
@end
