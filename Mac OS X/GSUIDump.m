/*
 *  GSUIDump.m
 *  XBolo
 *
 *  Development tool: dumps the nib-loaded UI (windows, view hierarchies,
 *  outlet names, target/action wiring, and the menu bar) as JSON so the
 *  frozen MainMenu.nib can be regenerated as code.
 *
 *  Only active when launched with:  -GSDumpUI YES [-GSDumpUIPath <file>]
 *  Writes the JSON and terminates the app.
 */

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

#import "GSXBoloController.h"

static NSMutableDictionary *outletNames;  /* pointer string -> ivar name */

static id safe(id obj) {
  return obj ? obj : [NSNull null];
}

static NSString *pointerKey(id obj) {
  return [NSString stringWithFormat:@"%p", obj];
}

static NSString *targetName(id target) {
  if (target == nil) {
    return nil;
  }
  if ([target isKindOfClass:[GSXBoloController class]]) {
    return @"controller";
  }
  return NSStringFromClass([target class]);
}

/* builds pointer -> outlet-name map from the controller's ivars */
static void buildOutletMap(id controller) {
  unsigned int count = 0;
  Ivar *ivars = class_copyIvarList([controller class], &count);

  outletNames = [NSMutableDictionary dictionary];

  for (unsigned int i = 0; i < count; i++) {
    const char *type = ivar_getTypeEncoding(ivars[i]);

    if (type[0] != '@') {
      continue;  /* not an object ivar */
    }

    id value = object_getIvar(controller, ivars[i]);

    if (value != nil) {
      outletNames[pointerKey(value)] = [NSString stringWithUTF8String:ivar_getName(ivars[i])];
    }
  }

  free(ivars);
}

static id dumpView(NSView *view);

static NSArray *dumpSubviews(NSArray *subviews) {
  NSMutableArray *arr = [NSMutableArray array];
  for (NSView *sub in subviews) {
    [arr addObject:dumpView(sub)];
  }
  return arr;
}

static id dumpCell(NSCell *cell) {
  NSMutableDictionary *d = [NSMutableDictionary dictionary];
  d[@"class"] = NSStringFromClass([cell class]);
  d[@"title"] = safe([cell isKindOfClass:[NSCell class]] && [cell type] == NSTextCellType ? [cell stringValue] : ([cell respondsToSelector:@selector(title)] ? [(id)cell title] : nil));
  d[@"tag"] = @([cell tag]);
  if ([cell isKindOfClass:[NSButtonCell class]]) {
    d[@"buttonTitle"] = safe([(NSButtonCell *)cell title]);
  }
  if ([cell action]) {
    d[@"action"] = NSStringFromSelector([cell action]);
    d[@"target"] = safe(targetName([cell target]));
  }
  return d;
}

static id dumpView(NSView *view) {
  NSMutableDictionary *d = [NSMutableDictionary dictionary];

  d[@"class"] = NSStringFromClass([view class]);
  d[@"ptr"] = pointerKey(view);
  d[@"window"] = safe(view.window ? pointerKey(view.window) : nil);
  d[@"frame"] = NSStringFromRect(view.frame);
  d[@"autoresizingMask"] = @(view.autoresizingMask);
  d[@"hidden"] = @(view.isHidden);
  d[@"tag"] = @(view.tag);

  NSString *outlet = outletNames[pointerKey(view)];
  if (outlet) {
    d[@"outlet"] = outlet;
  }

  if ([view respondsToSelector:@selector(nextKeyView)] && view.nextKeyView) {
    NSString *nk = outletNames[pointerKey(view.nextKeyView)];
    d[@"nextKeyView"] = nk ? nk : NSStringFromClass([view.nextKeyView class]);
  }

  /* controls: wiring and text */
  if ([view isKindOfClass:[NSControl class]]) {
    NSControl *c = (NSControl *)view;
    if (c.action) {
      d[@"action"] = NSStringFromSelector(c.action);
      d[@"target"] = safe(targetName(c.target));
    }
    d[@"enabled"] = @(c.enabled);
    if (c.font) {
      d[@"font"] = [NSString stringWithFormat:@"%@ %.1f", c.font.fontName, c.font.pointSize];
    }
  }

  if ([view isKindOfClass:[NSButton class]]) {
    NSButton *b = (NSButton *)view;
    d[@"title"] = safe(b.title);
    d[@"bezelStyle"] = @(b.bezelStyle);
    d[@"buttonType"] = safe([b.cell isKindOfClass:[NSButtonCell class]] ? @(((NSButtonCell *)b.cell).showsStateBy) : nil);
    d[@"state"] = @(b.state);
    d[@"keyEquivalent"] = safe(b.keyEquivalent.length ? b.keyEquivalent : nil);
  }
  else if ([view isKindOfClass:[NSTextField class]]) {
    NSTextField *t = (NSTextField *)view;
    d[@"stringValue"] = safe(t.stringValue);
    d[@"editable"] = @(t.editable);
    d[@"bordered"] = @(t.bordered);
    d[@"bezeled"] = @(t.bezeled);
    d[@"drawsBackground"] = @(t.drawsBackground);
    d[@"alignment"] = @(t.alignment);
  }
  else if ([view isKindOfClass:[NSImageView class]]) {
    NSImageView *iv = (NSImageView *)view;
    d[@"imageName"] = safe(iv.image.name);
  }
  else if ([view isKindOfClass:[NSTabView class]]) {
    NSTabView *tv = (NSTabView *)view;
    NSMutableArray *tabs = [NSMutableArray array];
    for (NSTabViewItem *item in tv.tabViewItems) {
      [tabs addObject:@{ @"label": safe(item.label),
                         @"identifier": safe([item.identifier description]),
                         @"view": dumpView(item.view) }];
    }
    d[@"tabs"] = tabs;
    d[@"tabViewType"] = @(tv.tabViewType);
    d[@"subviews"] = @[];  /* children live under tabs */
    return d;
  }
  else if ([view isKindOfClass:[NSMatrix class]]) {
    NSMatrix *m = (NSMatrix *)view;
    d[@"mode"] = @(m.mode);
    d[@"rows"] = @(m.numberOfRows);
    d[@"columns"] = @(m.numberOfColumns);
    d[@"cellSize"] = NSStringFromSize(m.cellSize);
    NSMutableArray *cells = [NSMutableArray array];
    for (NSCell *cell in m.cells) {
      [cells addObject:dumpCell(cell)];
    }
    d[@"cells"] = cells;
  }
  else if ([view isKindOfClass:[NSTableView class]]) {
    NSTableView *t = (NSTableView *)view;
    NSMutableArray *cols = [NSMutableArray array];
    for (NSTableColumn *col in t.tableColumns) {
      [cols addObject:@{ @"identifier": safe([col.identifier description]),
                         @"title": safe(col.title),
                         @"width": @(col.width) }];
    }
    d[@"columns"] = cols;
    d[@"delegate"] = safe(targetName(t.delegate));
    d[@"dataSource"] = safe(targetName(t.dataSource));
  }
  else if ([view isKindOfClass:[NSSlider class]]) {
    NSSlider *s = (NSSlider *)view;
    d[@"minValue"] = @(s.minValue);
    d[@"maxValue"] = @(s.maxValue);
    d[@"doubleValue"] = @(s.doubleValue);
  }
  else if ([view isKindOfClass:[NSProgressIndicator class]]) {
    NSProgressIndicator *p = (NSProgressIndicator *)view;
    d[@"indeterminate"] = @(p.indeterminate);
    d[@"style"] = @(p.style);
  }
  else if ([view isKindOfClass:[NSPopUpButton class]]) {
    d[@"items"] = [(NSPopUpButton *)view itemTitles];
  }
  else if ([view isKindOfClass:[NSBox class]]) {
    NSBox *b = (NSBox *)view;
    d[@"title"] = safe(b.title);
    d[@"boxType"] = @(b.boxType);
    d[@"titlePosition"] = @(b.titlePosition);
  }
  else if ([view isKindOfClass:[NSScrollView class]]) {
    NSScrollView *sv = (NSScrollView *)view;
    d[@"hasHorizontalScroller"] = @(sv.hasHorizontalScroller);
    d[@"hasVerticalScroller"] = @(sv.hasVerticalScroller);
    d[@"borderType"] = @(sv.borderType);
  }
  else if ([view isKindOfClass:[NSTextView class]]) {
    NSTextView *t = (NSTextView *)view;
    d[@"editable"] = @(t.editable);
    d[@"richText"] = @(t.richText);
  }

  d[@"subviews"] = dumpSubviews(view.subviews);
  return d;
}

static id dumpMenu(NSMenu *menu) {
  NSMutableArray *items = [NSMutableArray array];

  for (NSMenuItem *item in menu.itemArray) {
    if (item.separatorItem) {
      [items addObject:@{ @"separator": @YES }];
      continue;
    }

    NSMutableDictionary *d = [NSMutableDictionary dictionary];
    d[@"title"] = safe(item.title);
    d[@"keyEquivalent"] = safe(item.keyEquivalent.length ? item.keyEquivalent : nil);
    d[@"modifierMask"] = @(item.keyEquivalentModifierMask);
    d[@"tag"] = @(item.tag);

    if (item.action) {
      d[@"action"] = NSStringFromSelector(item.action);
      d[@"target"] = safe(targetName(item.target));
    }

    if (item.submenu) {
      d[@"submenu"] = dumpMenu(item.submenu);
    }

    [items addObject:d];
  }

  return @{ @"title": safe(menu.title), @"items": items };
}

static id dumpWindow(NSWindow *window) {
  NSMutableDictionary *d = [NSMutableDictionary dictionary];

  d[@"class"] = NSStringFromClass([window class]);
  d[@"ptr"] = pointerKey(window);
  d[@"title"] = safe(window.title);
  d[@"frame"] = NSStringFromRect(window.frame);
  d[@"contentSize"] = NSStringFromSize(((NSView *)window.contentView).frame.size);
  d[@"styleMask"] = @(window.styleMask);
  d[@"visible"] = @(window.visible);
  d[@"releasedWhenClosed"] = @(window.releasedWhenClosed);
  d[@"frameAutosaveName"] = safe(window.frameAutosaveName.length ? window.frameAutosaveName : nil);
  d[@"delegate"] = safe(targetName(window.delegate));

  NSString *outlet = outletNames[pointerKey(window)];
  if (outlet) {
    d[@"outlet"] = outlet;
  }

  if (window.initialFirstResponder) {
    NSString *ifr = outletNames[pointerKey(window.initialFirstResponder)];
    d[@"initialFirstResponder"] = ifr ? ifr : NSStringFromClass([window.initialFirstResponder class]);
  }

  if (window.toolbar) {
    NSMutableArray *items = [NSMutableArray array];
    for (NSToolbarItem *item in window.toolbar.items) {
      [items addObject:@{ @"identifier": safe(item.itemIdentifier),
                          @"label": safe(item.label) }];
    }
    d[@"toolbar"] = @{ @"identifier": safe(window.toolbar.identifier), @"items": items };
  }

  d[@"contentView"] = dumpView(window.contentView);
  return d;
}

@interface GSUIDump : NSObject
@end

@implementation GSUIDump

+ (void)load {
  [[NSNotificationCenter defaultCenter] addObserverForName:NSApplicationDidFinishLaunchingNotification
                                                    object:nil
                                                     queue:[NSOperationQueue mainQueue]
                                                usingBlock:^(NSNotification *note) {
    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"GSDumpUI"]) {
      return;
    }

    /* let the controller finish awakeFromNib work */
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
      id controller = nil;

      if ([[NSApp delegate] isKindOfClass:[GSXBoloController class]]) {
        controller = [NSApp delegate];
      }

      /* the controller is the target of most menu items; find it */
      for (NSWindow *w in controller ? @[] : [NSApp windows]) {
        if ([w.delegate isKindOfClass:[GSXBoloController class]]) {
          controller = w.delegate;
          break;
        }
      }
      if (controller == nil) {
        /* fall back: scan menu targets */
        for (NSMenuItem *item in [NSApp mainMenu].itemArray) {
          for (NSMenuItem *sub in item.submenu.itemArray) {
            if ([sub.target isKindOfClass:[GSXBoloController class]]) {
              controller = sub.target;
              break;
            }
          }
          if (controller) break;
        }
      }

      if (controller) {
        buildOutletMap(controller);
      }
      else {
        outletNames = [NSMutableDictionary dictionary];
        fprintf(stderr, "GSUIDump: warning: controller not found, no outlet names\n");
      }

      NSMutableArray *windows = [NSMutableArray array];
      for (NSWindow *w in [NSApp windows]) {
        [windows addObject:dumpWindow(w)];
      }

      NSDictionary *dump = @{
        @"windows": windows,
        @"mainMenu": dumpMenu([NSApp mainMenu]),
        @"outletCount": @(outletNames.count),
      };

      NSString *path = [[NSUserDefaults standardUserDefaults] stringForKey:@"GSDumpUIPath"];
      if (path == nil) {
        path = [NSHomeDirectory() stringByAppendingPathComponent:@"Desktop/xbolo-ui-dump.json"];
      }

      NSError *error = nil;
      NSData *json = [NSJSONSerialization dataWithJSONObject:dump
                                                     options:NSJSONWritingPrettyPrinted | NSJSONWritingSortedKeys
                                                       error:&error];
      if (json == nil) {
        fprintf(stderr, "GSUIDump: JSON serialization failed: %s\n", error.localizedDescription.UTF8String);
        exit(1);
      }

      if (![json writeToFile:path atomically:YES]) {
        fprintf(stderr, "GSUIDump: could not write %s\n", path.UTF8String);
        exit(1);
      }

      fprintf(stderr, "GSUIDump: wrote %s (%lu windows, %lu outlets mapped)\n",
              path.UTF8String, (unsigned long)windows.count, (unsigned long)outletNames.count);
      [NSApp terminate:nil];
    });
  }];
}

@end
