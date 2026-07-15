/*
 *  GSXBoloController+CodeUI.m
 *  XBolo
 *
 *  See GSXBoloController+CodeUI.h.  Geometry and wiring reproduce the
 *  MainMenu.nib originals (captured with GSUIDump).
 */

#import "GSXBoloController+CodeUI.h"

/* non-editable transparent label, nib-style geometry */
static NSTextField *GSLabel(NSString *text, NSRect frame) {
  NSTextField *label = [[NSTextField alloc] initWithFrame:frame];
  label.stringValue = text;
  label.editable = NO;
  label.selectable = NO;
  label.bordered = NO;
  label.bezeled = NO;
  label.drawsBackground = NO;
  label.font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
  return label;
}

static NSButton *GSPushButton(NSString *title, NSRect frame, id target, SEL action, NSString *keyEquivalent) {
  NSButton *button = [[NSButton alloc] initWithFrame:frame];
  button.title = title;
  button.bezelStyle = NSBezelStyleRounded;
  button.font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
  button.target = target;
  button.action = action;
  if (keyEquivalent) {
    button.keyEquivalent = keyEquivalent;
  }
  return button;
}

@implementation GSXBoloController (CodeUI)

- (void)buildJoinProgressWindow {
  NSWindow *window =
    [[NSWindow alloc] initWithContentRect:NSMakeRect(613.0, 441.0, 286.0, 97.0)
                                styleMask:NSWindowStyleMaskTitled
                                  backing:NSBackingStoreBuffered
                                    defer:YES];
  window.title = @"Join Progress";
  window.releasedWhenClosed = NO;

  NSView *content = window.contentView;

  [content addSubview:GSLabel(@"Status:", NSMakeRect(17.0, 60.0, 47.0, 17.0))];

  NSTextField *statusField = GSLabel(@"", NSMakeRect(76.0, 60.0, 193.0, 17.0));
  [content addSubview:statusField];

  NSProgressIndicator *indicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(18.0, 18.0, 172.0, 20.0)];
  indicator.style = NSProgressIndicatorStyleBar;
  indicator.indeterminate = YES;
  [content addSubview:indicator];

  [content addSubview:GSPushButton(@"Cancel", NSMakeRect(190.0, 12.0, 82.0, 32.0), self, @selector(joinCancel:), @"\033")];

  joinProgressWindow = window;
  joinProgressStatusTextField = statusField;
  joinProgressIndicator = indicator;
}

@end
