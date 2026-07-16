/*
 *  GSXBoloController+CodeUI.m
 *  XBolo
 *
 *  See GSXBoloController+CodeUI.h.  Geometry and wiring reproduce the
 *  MainMenu.nib originals (captured with GSUIDump).
 */

#import "GSXBoloController+CodeUI.h"
#import "GSStatusView.h"
#import "GSBuilderStatusView.h"
#import "GSStatusBar.h"
#import "GSKeyCodeField.h"
#import "GSBoloView.h"

#include <float.h>

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


/* nib-era frames were sized for 2009 font metrics; modern fonts render
   wider.  Refit the control to its content, growing leftward so its right
   edge stays aligned against the adjacent field column. */
static void GSFitKeepingRightEdge(NSControl *control) {
  NSRect old = [control frame];
  [control sizeToFit];
  NSRect fitted = [control frame];
  fitted.origin.x = NSMaxX(old) - NSWidth(fitted);
  fitted.origin.y = old.origin.y + (NSHeight(old) - NSHeight(fitted)) / 2.0;
  [control setFrame:fitted];
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

- (void)buildStatusPanel {
  NSPanel *panel =
    [[NSPanel alloc] initWithContentRect:NSMakeRect(1304.0, 649.0, 198.0, 274.0)
                               styleMask:403  /* from the nib: titled|closable|utility|... */
                                 backing:NSBackingStoreBuffered
                                   defer:YES];
  [panel setTitle:@"Status"];
  [panel setReleasedWhenClosed:NO];
  [panel setDelegate:(id)self];
  NSView *content = [panel contentView];

  GSStatusView *v2 = [[GSStatusView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 198.0, 274.0)];
  playerDeathsTextField = GSLabel(@"", NSMakeRect(144.0, 161.0, 32.0, 14.0));
  [playerDeathsTextField setFont:[NSFont systemFontOfSize:11.0]];
  [v2 addSubview:playerDeathsTextField];
  playerKillsTextField = GSLabel(@"", NSMakeRect(144.0, 176.0, 32.0, 14.0));
  [playerKillsTextField setFont:[NSFont systemFontOfSize:11.0]];
  [v2 addSubview:playerKillsTextField];
  pill4StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 172.0, 12.0, 12.0)];
  [v2 addSubview:pill4StatusImageView];
  base8StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 79.0, 12.0, 12.0)];
  [v2 addSubview:base8StatusImageView];
  pillEStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 126.0, 12.0, 12.0)];
  [v2 addSubview:pillEStatusImageView];
  builderStatusView = [[GSBuilderStatusView alloc] initWithFrame:NSMakeRect(134.0, 214.0, 33.0, 33.0)];
  [v2 addSubview:builderStatusView];
  player8StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 219.0, 12.0, 12.0)];
  [v2 addSubview:player8StatusImageView];
  pill6StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 149.0, 12.0, 12.0)];
  [v2 addSubview:pill6StatusImageView];
  player4StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 242.0, 12.0, 12.0)];
  [v2 addSubview:player4StatusImageView];
  playerTreesStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(167.0, 34.0, 3.0, 80.0)];
  [v2 addSubview:playerTreesStatusBar];
  base2StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(52.0, 102.0, 12.0, 12.0)];
  [v2 addSubview:base2StatusImageView];
  player6StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 219.0, 12.0, 12.0)];
  [v2 addSubview:player6StatusImageView];
  base1StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 102.0, 12.0, 12.0)];
  [v2 addSubview:base1StatusImageView];
  base0StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 102.0, 12.0, 12.0)];
  [v2 addSubview:base0StatusImageView];
  player0StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 242.0, 12.0, 12.0)];
  [v2 addSubview:player0StatusImageView];
  pillBStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 126.0, 12.0, 12.0)];
  [v2 addSubview:pillBStatusImageView];
  pillDStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(66.0, 126.0, 12.0, 12.0)];
  [v2 addSubview:pillDStatusImageView];
  baseMinesStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(36.0, 32.0, 70.0, 3.0)];
  [v2 addSubview:baseMinesStatusBar];
  playerAStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 196.0, 12.0, 12.0)];
  [v2 addSubview:playerAStatusImageView];
  base9StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 79.0, 12.0, 12.0)];
  [v2 addSubview:base9StatusImageView];
  playerBStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 196.0, 12.0, 12.0)];
  [v2 addSubview:playerBStatusImageView];
  player7StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 219.0, 12.0, 12.0)];
  [v2 addSubview:player7StatusImageView];
  playerMinesStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(143.0, 34.0, 3.0, 80.0)];
  [v2 addSubview:playerMinesStatusBar];
  pillAStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 126.0, 12.0, 12.0)];
  [v2 addSubview:pillAStatusImageView];
  baseBStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 56.0, 12.0, 12.0)];
  [v2 addSubview:baseBStatusImageView];
  baseCStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(52.0, 56.0, 12.0, 12.0)];
  [v2 addSubview:baseCStatusImageView];
  playerArmourStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(155.0, 34.0, 3.0, 80.0)];
  [v2 addSubview:playerArmourStatusBar];
  baseFStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 56.0, 12.0, 12.0)];
  [v2 addSubview:baseFStatusImageView];
  player9StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 219.0, 12.0, 12.0)];
  [v2 addSubview:player9StatusImageView];
  base4StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 102.0, 12.0, 12.0)];
  [v2 addSubview:base4StatusImageView];
  pillCStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(52.0, 126.0, 12.0, 12.0)];
  [v2 addSubview:pillCStatusImageView];
  pill0StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 172.0, 12.0, 12.0)];
  [v2 addSubview:pill0StatusImageView];
  base5StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 102.0, 12.0, 12.0)];
  [v2 addSubview:base5StatusImageView];
  base6StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 79.0, 12.0, 12.0)];
  [v2 addSubview:base6StatusImageView];
  pillFStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 126.0, 12.0, 12.0)];
  [v2 addSubview:pillFStatusImageView];
  player1StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 242.0, 12.0, 12.0)];
  [v2 addSubview:player1StatusImageView];
  pill9StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 149.0, 12.0, 12.0)];
  [v2 addSubview:pill9StatusImageView];
  baseShellsStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(36.0, 42.0, 70.0, 3.0)];
  [v2 addSubview:baseShellsStatusBar];
  baseEStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 56.0, 12.0, 12.0)];
  [v2 addSubview:baseEStatusImageView];
  pill2StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(52.0, 172.0, 12.0, 12.0)];
  [v2 addSubview:pill2StatusImageView];
  base3StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(66.0, 102.0, 12.0, 12.0)];
  [v2 addSubview:base3StatusImageView];
  pill3StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(66.0, 172.0, 12.0, 12.0)];
  [v2 addSubview:pill3StatusImageView];
  pill5StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 172.0, 12.0, 12.0)];
  [v2 addSubview:pill5StatusImageView];
  playerShellsStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(131.0, 34.0, 3.0, 80.0)];
  [v2 addSubview:playerShellsStatusBar];
  baseArmourStatusBar = [[GSStatusBar alloc] initWithFrame:NSMakeRect(36.0, 22.0, 70.0, 3.0)];
  [v2 addSubview:baseArmourStatusBar];
  player5StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 242.0, 12.0, 12.0)];
  [v2 addSubview:player5StatusImageView];
  pill1StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 172.0, 12.0, 12.0)];
  [v2 addSubview:pill1StatusImageView];
  playerFStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(94.0, 196.0, 12.0, 12.0)];
  [v2 addSubview:playerFStatusImageView];
  playerDStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(66.0, 196.0, 12.0, 12.0)];
  [v2 addSubview:playerDStatusImageView];
  playerCStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(52.0, 196.0, 12.0, 12.0)];
  [v2 addSubview:playerCStatusImageView];
  playerEStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 196.0, 12.0, 12.0)];
  [v2 addSubview:playerEStatusImageView];
  baseDStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(66.0, 56.0, 12.0, 12.0)];
  [v2 addSubview:baseDStatusImageView];
  pill8StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(80.0, 149.0, 12.0, 12.0)];
  [v2 addSubview:pill8StatusImageView];
  player3StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(66.0, 242.0, 12.0, 12.0)];
  [v2 addSubview:player3StatusImageView];
  base7StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 79.0, 12.0, 12.0)];
  [v2 addSubview:base7StatusImageView];
  baseAStatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(24.0, 56.0, 12.0, 12.0)];
  [v2 addSubview:baseAStatusImageView];
  pill7StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(38.0, 149.0, 12.0, 12.0)];
  [v2 addSubview:pill7StatusImageView];
  player2StatusImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(52.0, 242.0, 12.0, 12.0)];
  [v2 addSubview:player2StatusImageView];
  [content addSubview:v2];

  [panel setFrameAutosaveName:@"GSStatusPanel"];

  statusPanel = panel;
}

- (void)buildAllegiancePanel {
  NSPanel *panel =
    [[NSPanel alloc] initWithContentRect:NSMakeRect(1246.0, 431.0, 256.0, 192.0)
                               styleMask:411  /* from the nib: titled|closable|utility|... */
                                 backing:NSBackingStoreBuffered
                                   defer:YES];
  [panel setTitle:@"Allegiance"];
  [panel setReleasedWhenClosed:NO];
  [panel setDelegate:(id)self];
  NSView *content = [panel contentView];

  NSScrollView *v2 = [[NSScrollView alloc] initWithFrame:NSMakeRect(0.0, 34.0, 256.0, 158.0)];
  [v2 setBorderType:2];
  [v2 setHasVerticalScroller:YES];
  playerInfoTableView = [[NSTableView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 254.0, 128.0)];
  NSTableColumn *col3 = [[NSTableColumn alloc] initWithIdentifier:@"Player"];
  [col3 setTitle:@"Player"];
  [col3 setWidth:90.0];
  [playerInfoTableView addTableColumn:col3];
  [playerInfoTableView setDelegate:self];
  [playerInfoTableView setDataSource:self];
  [v2 setDocumentView:playerInfoTableView];
  [content addSubview:v2];
  [v2 setAutoresizingMask:18];

  requestAllianceButton = [[NSButton alloc] initWithFrame:NSMakeRect(9.0, 10.0, 96.0, 16.0)];
  [requestAllianceButton setTitle:@"Request Alliance"];
  [requestAllianceButton setBezelStyle:1];
  [requestAllianceButton setControlSize:NSControlSizeMini];
  [requestAllianceButton setFont:[NSFont systemFontOfSize:9.0]];
  [requestAllianceButton setTarget:self];
  [requestAllianceButton setAction:@selector(requestAlliance:)];
  [content addSubview:requestAllianceButton];
  [requestAllianceButton setAutoresizingMask:36];

  leaveAllianceButton = [[NSButton alloc] initWithFrame:NSMakeRect(111.0, 10.0, 86.0, 16.0)];
  [leaveAllianceButton setTitle:@"Leave Alliance"];
  [leaveAllianceButton setBezelStyle:1];
  [leaveAllianceButton setControlSize:NSControlSizeMini];
  [leaveAllianceButton setFont:[NSFont systemFontOfSize:9.0]];
  [leaveAllianceButton setTarget:self];
  [leaveAllianceButton setAction:@selector(leaveAlliance:)];
  [content addSubview:leaveAllianceButton];
  [leaveAllianceButton setAutoresizingMask:36];

  [panel setFrameAutosaveName:@"GSAllegiancePanel"];

  allegiancePanel = panel;
}

- (void)buildMessagesPanel {
  NSPanel *panel =
    [[NSPanel alloc] initWithContentRect:NSMakeRect(1246.0, 149.0, 256.0, 256.0)
                               styleMask:283  /* from the nib: titled|closable|utility|... */
                                 backing:NSBackingStoreBuffered
                                   defer:YES];
  [panel setTitle:@"Messages"];
  [panel setReleasedWhenClosed:NO];
  [panel setDelegate:(id)self];
  NSView *content = [panel contentView];

  messageTextField = [[NSTextField alloc] initWithFrame:NSMakeRect(10.0, 9.0, 169.0, 63.0)];
  [messageTextField setEditable:YES];
  [messageTextField setBezeled:YES];
  [messageTextField setFont:[NSFont systemFontOfSize:9.0]];
  [messageTextField setTarget:self];
  [messageTextField setAction:@selector(sendMessage:)];
  [content addSubview:messageTextField];
  [messageTextField setAutoresizingMask:34];

  NSScrollView *v2 = [[NSScrollView alloc] initWithFrame:NSMakeRect(0.0, 80.0, 256.0, 176.0)];
  [v2 setBorderType:2];
  [v2 setHasVerticalScroller:YES];
  messagesTextView = [[NSTextView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 254.0, 174.0)];
  [messagesTextView setEditable:NO];
  [messagesTextView setRichText:YES];
  [messagesTextView setVerticallyResizable:YES];
  [messagesTextView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [[messagesTextView textContainer] setWidthTracksTextView:YES];
  [messagesTextView setMinSize:NSMakeSize(0.0, 174.0)];
  [messagesTextView setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
  [v2 setDocumentView:messagesTextView];
  [content addSubview:v2];
  [v2 setAutoresizingMask:18];

  NSButtonCell *proto = [[NSButtonCell alloc] init];
  [proto setButtonType:NSButtonTypeRadio];
  [proto setControlSize:NSControlSizeMini];
  [proto setFont:[NSFont systemFontOfSize:9.0]];
  messageTargetMatrix = [[NSMatrix alloc] initWithFrame:NSMakeRect(184.0, 33.0, 62.0, 40.0) mode:0 prototype:proto numberOfRows:3 numberOfColumns:1];
  [messageTargetMatrix setCellSize:NSMakeSize(62.0, 12.0)];
  [[messageTargetMatrix cellAtRow:0 column:0] setTitle:@"Everyone"];
  [[messageTargetMatrix cellAtRow:0 column:0] setTag:0];
  [[messageTargetMatrix cellAtRow:1 column:0] setTitle:@"Allies"];
  [[messageTargetMatrix cellAtRow:1 column:0] setTag:1];
  [[messageTargetMatrix cellAtRow:2 column:0] setTitle:@"Nearby"];
  [[messageTargetMatrix cellAtRow:2 column:0] setTag:2];
  [messageTargetMatrix selectCellAtRow:0 column:0];
  [messageTargetMatrix setTarget:self];
  [messageTargetMatrix setAction:@selector(messageTarget:)];
  [content addSubview:messageTargetMatrix];
  [messageTargetMatrix setAutoresizingMask:33];

  NSButton *v3 = [[NSButton alloc] initWithFrame:NSMakeRect(186.0, 10.0, 60.0, 16.0)];
  [v3 setTitle:@"Send"];
  [v3 setBezelStyle:1];
  [v3 setControlSize:NSControlSizeMini];
  [v3 setFont:[NSFont systemFontOfSize:9.0]];
  [v3 setTarget:self];
  [v3 setAction:@selector(sendMessage:)];
  [v3 setKeyEquivalent:@"\r"];
  [content addSubview:v3];
  [v3 setAutoresizingMask:33];

  [panel setInitialFirstResponder:messageTextField];
  [panel setFrameAutosaveName:@"GSMessagesPanel"];

  messagesPanel = panel;
}

/* small helper: item with target/action/key/modifiers */
static NSMenuItem *GSItem(NSString *title, SEL action, id target, NSString *key, NSUInteger mods, NSInteger tag) {
  NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:title action:action keyEquivalent:key ? key : @""];
  item.target = target;
  if (mods) {
    item.keyEquivalentModifierMask = mods;
  }
  item.tag = tag;
  return item;
}

- (void)buildMainMenu {
  NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];

  /* application menu */
  NSMenu *appMenu = [[NSMenu alloc] initWithTitle:@"XBolo"];
  [appMenu addItem:GSItem(@"About XBolo", @selector(orderFrontStandardAboutPanel:), NSApp, nil, 0, 0)];
  [appMenu addItem:GSItem(@"Settings…", @selector(showPrefs:), self, @",", NSEventModifierFlagCommand, 0)];
  [appMenu addItem:[NSMenuItem separatorItem]];
  NSMenuItem *servicesItem = [appMenu addItemWithTitle:@"Services" action:NULL keyEquivalent:@""];
  NSMenu *servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
  servicesItem.submenu = servicesMenu;
  [NSApp setServicesMenu:servicesMenu];
  [appMenu addItem:[NSMenuItem separatorItem]];
  [appMenu addItem:GSItem(@"Hide XBolo", @selector(hide:), NSApp, @"h", NSEventModifierFlagCommand, 0)];
  [appMenu addItem:GSItem(@"Hide Others", @selector(hideOtherApplications:), NSApp, nil, 0, 0)];
  [appMenu addItem:GSItem(@"Show All", @selector(unhideAllApplications:), NSApp, nil, 0, 0)];
  [appMenu addItem:[NSMenuItem separatorItem]];
  [appMenu addItem:GSItem(@"Quit XBolo", @selector(terminate:), NSApp, @"q", NSEventModifierFlagCommand, 0)];
  [mainMenu addItemWithTitle:@"XBolo" action:NULL keyEquivalent:@""].submenu = appMenu;

  /* file menu */
  NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
  [fileMenu addItem:GSItem(@"New Game…", @selector(newGame:), self, @"n", NSEventModifierFlagCommand, 0)];
  [fileMenu addItem:[NSMenuItem separatorItem]];
  [fileMenu addItem:GSItem(@"Close Window", @selector(performClose:), nil, @"w", NSEventModifierFlagCommand, 0)];
  [fileMenu addItem:GSItem(@"Close All", @selector(closeAll:), NSApp, @"w", NSEventModifierFlagCommand | NSEventModifierFlagOption, 0)];
  [fileMenu addItem:GSItem(@"Close Game", @selector(closeGame:), self, @"W", NSEventModifierFlagCommand, 0)];
  [fileMenu addItem:[NSMenuItem separatorItem]];
  [fileMenu addItem:GSItem(@"Page Setup…", @selector(runPageLayout:), nil, @"P", NSEventModifierFlagCommand, 0)];
  [fileMenu addItem:GSItem(@"Print…", @selector(print:), nil, @"p", NSEventModifierFlagCommand, 0)];
  [mainMenu addItemWithTitle:@"File" action:NULL keyEquivalent:@""].submenu = fileMenu;

  /* edit menu (AppKit injects Writing Tools, AutoFill, Dictation, Emoji) */
  NSMenu *editMenu = [[NSMenu alloc] initWithTitle:@"Edit"];
  [editMenu addItem:GSItem(@"Undo", @selector(undo:), nil, @"z", NSEventModifierFlagCommand, 0)];
  [editMenu addItem:GSItem(@"Redo", @selector(redo:), nil, @"Z", NSEventModifierFlagCommand, 0)];
  [editMenu addItem:[NSMenuItem separatorItem]];
  [editMenu addItem:GSItem(@"Cut", @selector(cut:), nil, @"x", NSEventModifierFlagCommand, 0)];
  [editMenu addItem:GSItem(@"Copy", @selector(copy:), nil, @"c", NSEventModifierFlagCommand, 0)];
  [editMenu addItem:GSItem(@"Paste", @selector(paste:), nil, @"v", NSEventModifierFlagCommand, 0)];
  [editMenu addItem:GSItem(@"Clear", @selector(clear:), nil, nil, 0, 0)];
  [editMenu addItem:GSItem(@"Select All", @selector(selectAll:), nil, @"a", NSEventModifierFlagCommand, 0)];
  [editMenu addItem:[NSMenuItem separatorItem]];
  NSMenu *spellingMenu = [[NSMenu alloc] initWithTitle:@"Spelling"];
  [spellingMenu addItem:GSItem(@"Spelling...", @selector(showGuessPanel:), nil, @":", NSEventModifierFlagCommand, 0)];
  [spellingMenu addItem:GSItem(@"Check Spelling", @selector(checkSpelling:), nil, @";", NSEventModifierFlagCommand, 0)];
  [spellingMenu addItem:GSItem(@"Check Spelling As You Type", @selector(toggleContinuousSpellChecking:), nil, nil, 0, 0)];
  [editMenu addItemWithTitle:@"Spelling" action:NULL keyEquivalent:@""].submenu = spellingMenu;
  [mainMenu addItemWithTitle:@"Edit" action:NULL keyEquivalent:@""].submenu = editMenu;

  /* builder menu */
  NSMenu *builderMenu = [[NSMenu alloc] initWithTitle:@"Builder"];
  NSArray *tools = @[@"Tree", @"Road", @"Wall", @"Pill", @"Mine"];
  for (NSUInteger i = 0; i < tools.count; i++) {
    [builderMenu addItem:GSItem(tools[i], @selector(builderToolMenu:), self,
                                [NSString stringWithFormat:@"%lu", (unsigned long)(i + 1)],
                                NSEventModifierFlagCommand, (NSInteger)i)];
  }
  [mainMenu addItemWithTitle:@"Builder" action:NULL keyEquivalent:@""].submenu = builderMenu;

  /* game menu */
  NSMenu *gameMenu = [[NSMenu alloc] initWithTitle:@"Game"];
  gamePauseResumeMenuItem = GSItem(@"Pause", @selector(gamePauseResumeMenu:), self, @".", NSEventModifierFlagCommand, 0);
  [gameMenu addItem:gamePauseResumeMenuItem];
  [gameMenu addItem:GSItem(@"Allow Join", @selector(toggleJoin:), self, @"d", NSEventModifierFlagCommand, 0)];
  [gameMenu addItem:GSItem(@"Mute", @selector(toggleMute:), self, @"M", NSEventModifierFlagCommand, 0)];
  [gameMenu addItem:[NSMenuItem separatorItem]];

  NSMenu *kickMenu = [[NSMenu alloc] initWithTitle:@"Kick Player"];
  NSMenu *banMenu = [[NSMenu alloc] initWithTitle:@"Ban Player"];
  for (NSInteger i = 0; i < 16; i++) {
    NSString *title = [NSString stringWithFormat:@"Player %ld", (long)(i + 1)];
    [kickMenu addItem:GSItem(title, @selector(kickPlayer:), self, nil, 0, i)];
    [banMenu addItem:GSItem(title, @selector(banPlayer:), self, nil, 0, i)];
  }
  NSMenuItem *kickItem = GSItem(@"Kick Player", NULL, nil, nil, 0, -1);
  kickItem.submenu = kickMenu;
  [gameMenu addItem:kickItem];
  NSMenuItem *banItem = GSItem(@"Ban Player", NULL, nil, nil, 0, -1);
  banItem.submenu = banMenu;
  [gameMenu addItem:banItem];
  NSMenuItem *unbanItem = GSItem(@"Unban Player", NULL, nil, nil, 0, -1);
  NSMenu *unbanMenu = [[NSMenu alloc] initWithTitle:@"Unban Player"];
  unbanMenu.delegate = (id)self;  /* populated in menuNeedsUpdate: */
  unbanItem.submenu = unbanMenu;
  [gameMenu addItem:unbanItem];
  [mainMenu addItemWithTitle:@"Game" action:NULL keyEquivalent:@""].submenu = gameMenu;

  /* view menu */
  NSMenu *viewMenu = [[NSMenu alloc] initWithTitle:@"View"];
  [viewMenu addItem:GSItem(@"Show/Hide Toolbar", @selector(toggleToolbarShown:), nil, nil, 0, 0)];
  [viewMenu addItem:GSItem(@"Customize Toolbar…", @selector(runToolbarCustomizationPalette:), nil, nil, 0, 0)];
  [mainMenu addItemWithTitle:@"View" action:NULL keyEquivalent:@""].submenu = viewMenu;

  /* window menu */
  NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
  [windowMenu addItem:GSItem(@"Minimize", @selector(performMiniaturize:), nil, @"m", NSEventModifierFlagCommand, 0)];
  [windowMenu addItem:GSItem(@"Zoom", @selector(performZoom:), nil, nil, 0, 0)];
  [windowMenu addItem:[NSMenuItem separatorItem]];
  [windowMenu addItem:GSItem(@"Status", @selector(statusPanel:), self, @"s", NSEventModifierFlagCommand, 0)];
  [windowMenu addItem:GSItem(@"Allegiance", @selector(allegiancePanel:), self, @"l", NSEventModifierFlagCommand, 0)];
  [windowMenu addItem:GSItem(@"Messages", @selector(messagesPanel:), self, nil, 0, 0)];
  [windowMenu addItem:[NSMenuItem separatorItem]];
  [windowMenu addItem:GSItem(@"Bring All to Front", @selector(arrangeInFront:), nil, nil, 0, 0)];
  [mainMenu addItemWithTitle:@"Window" action:NULL keyEquivalent:@""].submenu = windowMenu;
  [NSApp setWindowsMenu:windowMenu];

  /* help menu */
  NSMenu *helpMenu = [[NSMenu alloc] initWithTitle:@"Help"];
  [helpMenu addItem:GSItem(@"XBolo Help", @selector(showHelp:), nil, @"?", NSEventModifierFlagCommand, 0)];
  [mainMenu addItemWithTitle:@"Help" action:NULL keyEquivalent:@""].submenu = helpMenu;
  [NSApp setHelpMenu:helpMenu];

  [NSApp setMainMenu:mainMenu];
}


- (void)buildPreferencesWindow {
  NSWindow *window =
    [[NSWindow alloc] initWithContentRect:NSMakeRect(506, 397, 443, 373)
                                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                  backing:NSBackingStoreBuffered
                                    defer:YES];
  [window setTitle:@"XBolo Preferences"];
  [window setReleasedWhenClosed:NO];
  NSView *content = [window contentView];

  prefTab = [[NSTabView alloc] initWithFrame:NSMakeRect(0, 44, 443, 329)];
  [prefTab setTabViewType:6];
  NSTabViewItem *item1 = [[NSTabViewItem alloc] initWithIdentifier:@"GSToolbarPlayerInfoItemIdentifier"];
  [item1 setLabel:@"Player Info"];
  NSView *pane2 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 443, 329)];
  NSBox *box3 = [[NSBox alloc] initWithFrame:NSMakeRect(17, 154, 409, 169)];
  [box3 setTitle:@"Attributes"];
  [box3 setContentViewMargins:NSMakeSize(0.0, 0.0)];
  NSTextField *t4 = GSLabel(@"Player Name:", NSMakeRect(42, 112, 86, 17));
  [t4 setAlignment:2];
  [[box3 contentView] addSubview:t4];

  prefPlayerNameField = [[NSTextField alloc] initWithFrame:NSMakeRect(133, 110, 180, 22)];
  [prefPlayerNameField setEditable:YES];
  [prefPlayerNameField setBezeled:YES];
  [prefPlayerNameField setStringValue:@"Newbie"];
  [[box3 contentView] addSubview:prefPlayerNameField];

  [pane2 addSubview:box3];

  [item1 setView:pane2];
  [prefTab addTabViewItem:item1];

  NSTabViewItem *item5 = [[NSTabViewItem alloc] initWithIdentifier:@"GSToolbarKeyConfigItemIdentifier"];
  [item5 setLabel:@"Key Config"];
  NSView *pane6 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 443, 329)];
  NSBox *box7 = [[NSBox alloc] initWithFrame:NSMakeRect(17, 16, 222, 307)];
  [box7 setTitle:@"Tank"];
  [box7 setContentViewMargins:NSMakeSize(0.0, 0.0)];
  NSTextField *t8 = GSLabel(@"Accelerate:", NSMakeRect(43, 256, 74, 17));
  [t8 setAlignment:2];
  [[box7 contentView] addSubview:t8];

  NSTextField *t9 = GSLabel(@"Brake:", NSMakeRect(74, 226, 43, 17));
  [t9 setAlignment:2];
  [[box7 contentView] addSubview:t9];

  NSTextField *t10 = GSLabel(@"Turn Left:", NSMakeRect(51, 196, 66, 17));
  [t10 setAlignment:2];
  [[box7 contentView] addSubview:t10];

  NSTextField *t11 = GSLabel(@"Turn Right:", NSMakeRect(42, 166, 75, 17));
  [t11 setAlignment:2];
  [[box7 contentView] addSubview:t11];

  NSTextField *t12 = GSLabel(@"Lay Mine:", NSMakeRect(53, 136, 64, 17));
  [t12 setAlignment:2];
  [[box7 contentView] addSubview:t12];

  NSTextField *t13 = GSLabel(@"Shoot:", NSMakeRect(72, 106, 45, 17));
  [t13 setAlignment:2];
  [[box7 contentView] addSubview:t13];

  NSTextField *t14 = GSLabel(@"Increase Aim:", NSMakeRect(27, 76, 90, 17));
  [t14 setAlignment:2];
  [[box7 contentView] addSubview:t14];

  NSTextField *t15 = GSLabel(@"Decrease Aim:", NSMakeRect(22, 46, 95, 17));
  [t15 setAlignment:2];
  [[box7 contentView] addSubview:t15];

  prefAutoSlowdownSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(120, 17, 22, 18)];
  [prefAutoSlowdownSwitch setButtonType:NSButtonTypeSwitch];
  [prefAutoSlowdownSwitch setTitle:@""];
  [prefAutoSlowdownSwitch setBezelStyle:2];
  [prefAutoSlowdownSwitch setState:NSControlStateValueOn];
  [[box7 contentView] addSubview:prefAutoSlowdownSwitch];

  NSTextField *t16 = GSLabel(@"Auto Slowdown:", NSMakeRect(11, 18, 106, 17));
  [t16 setAlignment:2];
  [[box7 contentView] addSubview:t16];

  prefAccelerateField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 253, 78, 22)];
  [[box7 contentView] addSubview:prefAccelerateField];

  prefBrakeField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 223, 78, 22)];
  [[box7 contentView] addSubview:prefBrakeField];

  prefTurnLeftField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 193, 78, 22)];
  [[box7 contentView] addSubview:prefTurnLeftField];

  prefTurnRightField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 163, 78, 22)];
  [[box7 contentView] addSubview:prefTurnRightField];

  prefLayMineField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 133, 78, 22)];
  [[box7 contentView] addSubview:prefLayMineField];

  prefShootField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 103, 78, 22)];
  [[box7 contentView] addSubview:prefShootField];

  prefIncreaseAimField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 73, 78, 22)];
  [[box7 contentView] addSubview:prefIncreaseAimField];

  prefDecreaseAimField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(122, 43, 78, 22)];
  [[box7 contentView] addSubview:prefDecreaseAimField];

  [pane6 addSubview:box7];

  NSBox *box17 = [[NSBox alloc] initWithFrame:NSMakeRect(241, 106, 185, 217)];
  [box17 setTitle:@"View"];
  [box17 setContentViewMargins:NSMakeSize(0.0, 0.0)];
  NSTextField *t18 = GSLabel(@"Right:", NSMakeRect(42, 76, 42, 17));
  [t18 setAlignment:2];
  [[box17 contentView] addSubview:t18];

  NSTextField *t19 = GSLabel(@"Left:", NSMakeRect(52, 106, 32, 17));
  [t19 setAlignment:2];
  [[box17 contentView] addSubview:t19];

  NSTextField *t20 = GSLabel(@"Up:", NSMakeRect(58, 166, 26, 17));
  [t20 setAlignment:2];
  [[box17 contentView] addSubview:t20];

  NSTextField *t21 = GSLabel(@"Down:", NSMakeRect(40, 136, 44, 17));
  [t21 setAlignment:2];
  [[box17 contentView] addSubview:t21];

  NSTextField *t22 = GSLabel(@"Tank View:", NSMakeRect(11, 46, 73, 17));
  [t22 setAlignment:2];
  [[box17 contentView] addSubview:t22];

  NSTextField *t23 = GSLabel(@"Pill View:", NSMakeRect(23, 16, 61, 17));
  [t23 setAlignment:2];
  [[box17 contentView] addSubview:t23];

  prefUpField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(89, 163, 78, 22)];
  [[box17 contentView] addSubview:prefUpField];

  prefDownField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(89, 133, 78, 22)];
  [[box17 contentView] addSubview:prefDownField];

  prefLeftField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(89, 103, 78, 22)];
  [[box17 contentView] addSubview:prefLeftField];

  prefRightField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(89, 73, 78, 22)];
  [[box17 contentView] addSubview:prefRightField];

  prefTankViewField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(89, 43, 78, 22)];
  [[box17 contentView] addSubview:prefTankViewField];

  prefPillViewField = [[GSKeyCodeField alloc] initWithFrame:NSMakeRect(89, 13, 78, 22)];
  [[box17 contentView] addSubview:prefPillViewField];

  [pane6 addSubview:box17];

  [item5 setView:pane6];
  [prefTab addTabViewItem:item5];

  [content addSubview:prefTab];

  [content addSubview:GSPushButton(@"Cancel", NSMakeRect(255.0, 8.0, 86.0, 32.0), self, @selector(prefCancel:), @"\033")];
  [content addSubview:GSPushButton(@"OK", NSMakeRect(345.0, 8.0, 86.0, 32.0), self, @selector(prefOK:), @"\r")];

  /* The nib chained the key fields into an explicit key-view loop, but
     AppKit drops nextKeyView links on views in unselected (detached) tab
     panes, so we rely on the automatic geometric loop instead; in this
     layout it produces the same top-down order. */
  [window setInitialFirstResponder:prefTab];

  [window setFrameAutosaveName:@"GSPreferencesWindow"];

  preferencesWindow = window;
}


- (void)buildNewGameWindow {
  NSWindow *window =
    [[NSWindow alloc] initWithContentRect:NSMakeRect(436, 255, 640, 480)
                                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                  backing:NSBackingStoreBuffered
                                    defer:YES];
  [window setTitle:@"New Game"];
  [window setReleasedWhenClosed:NO];
  NSView *content = [window contentView];

  newGameTabView = [[NSTabView alloc] initWithFrame:NSMakeRect(13, 10, 614, 464)];
  NSTabViewItem *item1 = [[NSTabViewItem alloc] initWithIdentifier:@"0"];
  [item1 setLabel:@"Host"];
  NSView *pane2 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 594, 418)];
  hostPortField = [[NSTextField alloc] initWithFrame:NSMakeRect(237, 360, 240, 22)];
  [hostPortField setEditable:YES];
  [hostPortField setBezeled:YES];
  [hostPortField setStringValue:@"50000"];
  [hostPortField setTarget:self];
  [hostPortField setAction:@selector(hostPort:)];
  [pane2 addSubview:hostPortField];

  NSTextField *t3 = GSLabel(@"Port:", NSMakeRect(197, 362, 34, 17));
  [t3 setAlignment:2];
  GSFitKeepingRightEdge(t3);
  [pane2 addSubview:t3];

  hostGameTypeMenu = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(234, 211, 214, 26) pullsDown:NO];
  [hostGameTypeMenu addItemsWithTitles:@[@"Domination", @"Capture The Flag", @"King of the Hill", @"Kill the Man with the Ball", @"Body Count"]];
  [hostGameTypeMenu setTarget:self];
  [hostGameTypeMenu setAction:@selector(hostGameType:)];
  [pane2 addSubview:hostGameTypeMenu];

  /* a single self-describing button, per the HIG, instead of the old
     "Choose" button with a separate "Map:" label; its right edge sits on
     the label column so it reads with the field beside it */
  NSButton *b4 = [[NSButton alloc] initWithFrame:NSMakeRect(110, 385, 121, 32)];
  [b4 setTitle:@"Choose Map…"];
  [b4 setBezelStyle:1];
  [b4 setTarget:self];
  [b4 setAction:@selector(hostChoose:)];
  GSFitKeepingRightEdge(b4);
  [pane2 addSubview:b4];

  hostMapField = [[NSTextField alloc] initWithFrame:NSMakeRect(237, 390, 240, 22)];
  [hostMapField setEditable:NO];
  [hostMapField setBezeled:YES];
  [hostMapField setSelectable:YES];
  [hostMapField setStringValue:@"Cognitive Dissonance"];
  [pane2 addSubview:hostMapField];

  NSButton *b6 = [[NSButton alloc] initWithFrame:NSMakeRect(490, 12, 90, 32)];
  [b6 setTitle:@"OK"];
  [b6 setBezelStyle:1];
  [b6 setKeyEquivalent:@"\r"];
  [b6 setTarget:self];
  [b6 setAction:@selector(hostOK:)];
  [pane2 addSubview:b6];

  hostGameTypeTab = [[NSTabView alloc] initWithFrame:NSMakeRect(112, 56, 369, 153)];
  [hostGameTypeTab setTabViewType:4];
  NSTabViewItem *item7 = [[NSTabViewItem alloc] initWithIdentifier:@"1"];
  [item7 setLabel:@"Domination"];
  NSView *pane8 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 349, 133)];
  NSTextField *t9 = GSLabel(@"Base Control:", NSMakeRect(22, 25, 88, 17));
  [pane8 addSubview:t9];

  hostDominationBaseControlSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(113, 23, 138, 26)];
  [hostDominationBaseControlSlider setMinValue:10];
  [hostDominationBaseControlSlider setMaxValue:60];
  [hostDominationBaseControlSlider setDoubleValue:30];
  [hostDominationBaseControlSlider setTarget:self];
  [hostDominationBaseControlSlider setAction:@selector(hostDominationBaseControl:)];
  [pane8 addSubview:hostDominationBaseControlSlider];

  hostDominationBaseControlField = [[NSTextField alloc] initWithFrame:NSMakeRect(257, 23, 72, 22)];
  [hostDominationBaseControlField setEditable:YES];
  [hostDominationBaseControlField setBezeled:YES];
  [hostDominationBaseControlField setStringValue:@"00:00:30"];
  [hostDominationBaseControlField setTarget:self];
  [hostDominationBaseControlField setAction:@selector(hostDominationBaseControl:)];
  [pane8 addSubview:hostDominationBaseControlField];

  NSButtonCell *proto10 = [[NSButtonCell alloc] init];
  [proto10 setButtonType:NSButtonTypeRadio];
  hostDominationTypeMatrix = [[NSMatrix alloc] initWithFrame:NSMakeRect(113, 55, 176, 58) mode:0 prototype:proto10 numberOfRows:3 numberOfColumns:1];
  [hostDominationTypeMatrix setCellSize:NSMakeSize(176, 18)];
  [[hostDominationTypeMatrix cellAtRow:0 column:0] setTitle:@"Open Game"];
  [[hostDominationTypeMatrix cellAtRow:0 column:0] setTag:0];
  [[hostDominationTypeMatrix cellAtRow:1 column:0] setTitle:@"Tournament Game"];
  [[hostDominationTypeMatrix cellAtRow:1 column:0] setTag:1];
  [[hostDominationTypeMatrix cellAtRow:2 column:0] setTitle:@"Strict Tournament Game"];
  [[hostDominationTypeMatrix cellAtRow:2 column:0] setTag:2];
  [hostDominationTypeMatrix selectCellAtRow:0 column:0];
  [hostDominationTypeMatrix setTarget:self];
  [hostDominationTypeMatrix setAction:@selector(hostDominationType:)];
  [pane8 addSubview:hostDominationTypeMatrix];

  NSTextField *t11 = GSLabel(@"Type:", NSMakeRect(71, 96, 39, 17));
  [t11 setAlignment:4];
  [pane8 addSubview:t11];

  [item7 setView:pane8];
  [hostGameTypeTab addTabViewItem:item7];

  NSTabViewItem *item12 = [[NSTabViewItem alloc] initWithIdentifier:@"2"];
  [item12 setLabel:@"Capture The Flag"];
  NSView *pane13 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 349, 133)];
  [item12 setView:pane13];
  [hostGameTypeTab addTabViewItem:item12];

  NSTabViewItem *item14 = [[NSTabViewItem alloc] initWithIdentifier:@"3"];
  [item14 setLabel:@"King of the Hill"];
  NSView *pane15 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 349, 133)];
  [item14 setView:pane15];
  [hostGameTypeTab addTabViewItem:item14];

  NSTabViewItem *item16 = [[NSTabViewItem alloc] initWithIdentifier:@"4"];
  [item16 setLabel:@"Kill the Man with the Ball"];
  NSView *pane17 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 349, 133)];
  [item16 setView:pane17];
  [hostGameTypeTab addTabViewItem:item16];

  NSTabViewItem *item18 = [[NSTabViewItem alloc] initWithIdentifier:@"5"];
  [item18 setLabel:@"Body Count"];
  NSView *pane19 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 349, 133)];
  [item18 setView:pane19];
  [hostGameTypeTab addTabViewItem:item18];

  [pane2 addSubview:hostGameTypeTab];

  NSTextField *t20 = GSLabel(@"Game Type:", NSMakeRect(152, 217, 79, 17));
  [t20 setAlignment:2];
  GSFitKeepingRightEdge(t20);
  [pane2 addSubview:t20];

  hostPasswordField = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(237, 330, 240, 22)];
  [hostPasswordField setEditable:YES];
  [hostPasswordField setBezeled:YES];
  [hostPasswordField setTarget:self];
  [hostPasswordField setAction:@selector(hostPassword:)];
  [pane2 addSubview:hostPasswordField];

  hostPasswordSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(144, 331, 86, 18)];
  [hostPasswordSwitch setButtonType:NSButtonTypeSwitch];
  [hostPasswordSwitch setTitle:@"Password:"];
  [hostPasswordSwitch setBezelStyle:2];
  GSFitKeepingRightEdge(hostPasswordSwitch);
  [hostPasswordSwitch setTarget:self];
  [hostPasswordSwitch setAction:@selector(hostPasswordSwitch:)];
  [pane2 addSubview:hostPasswordSwitch];

  hostTimeLimitSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(136, 297, 94, 18)];
  [hostTimeLimitSwitch setButtonType:NSButtonTypeSwitch];
  [hostTimeLimitSwitch setTitle:@"Time Limit:"];
  [hostTimeLimitSwitch setBezelStyle:2];
  GSFitKeepingRightEdge(hostTimeLimitSwitch);
  [hostTimeLimitSwitch setTarget:self];
  [hostTimeLimitSwitch setAction:@selector(hostTimeLimitSwitch:)];
  [pane2 addSubview:hostTimeLimitSwitch];

  hostTimeLimitSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(235, 296, 164, 26)];
  [hostTimeLimitSlider setMinValue:600];
  [hostTimeLimitSlider setMaxValue:3600];
  [hostTimeLimitSlider setDoubleValue:1800];
  [hostTimeLimitSlider setTarget:self];
  [hostTimeLimitSlider setAction:@selector(hostTimeLimit:)];
  [pane2 addSubview:hostTimeLimitSlider];

  hostTimeLimitField = [[NSTextField alloc] initWithFrame:NSMakeRect(405, 296, 72, 22)];
  [hostTimeLimitField setEditable:YES];
  [hostTimeLimitField setBezeled:YES];
  [hostTimeLimitField setStringValue:@"00:30:00"];
  [hostTimeLimitField setTarget:self];
  [hostTimeLimitField setAction:@selector(hostTimeLimit:)];
  [pane2 addSubview:hostTimeLimitField];

  NSButton *b21 = [[NSButton alloc] initWithFrame:NSMakeRect(400, 12, 90, 32)];
  [b21 setTitle:@"Cancel"];
  [b21 setBezelStyle:1];
  [b21 setKeyEquivalent:@"\033"];
  [b21 setTarget:window];
  [b21 setAction:@selector(orderOut:)];
  [pane2 addSubview:b21];

  hostTrackerSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(156, 244, 74, 18)];
  [hostTrackerSwitch setButtonType:NSButtonTypeSwitch];
  [hostTrackerSwitch setTitle:@"Tracker:"];
  [hostTrackerSwitch setBezelStyle:2];
  GSFitKeepingRightEdge(hostTrackerSwitch);
  [hostTrackerSwitch setTarget:self];
  [hostTrackerSwitch setAction:@selector(hostTrackerSwitch:)];
  [pane2 addSubview:hostTrackerSwitch];

  hostTrackerField = [[NSTextField alloc] initWithFrame:NSMakeRect(237, 243, 240, 22)];
  [hostTrackerField setEditable:YES];
  [hostTrackerField setBezeled:YES];
  [hostTrackerField setStringValue:@"tracker.xbolo.org"];
  [hostTrackerField setAlignment:4];
  [hostTrackerField setTarget:self];
  [hostTrackerField setAction:@selector(tracker:)];
  [pane2 addSubview:hostTrackerField];

  hostHiddenMinesSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(117, 272, 113, 18)];
  [hostHiddenMinesSwitch setButtonType:NSButtonTypeSwitch];
  [hostHiddenMinesSwitch setTitle:@"Hidden Mines:"];
  [hostHiddenMinesSwitch setBezelStyle:2];
  GSFitKeepingRightEdge(hostHiddenMinesSwitch);
  [hostHiddenMinesSwitch setTarget:self];
  [hostHiddenMinesSwitch setAction:@selector(hostHiddenMinesSwitch:)];
  [pane2 addSubview:hostHiddenMinesSwitch];

  hostHiddenMinesTextField = GSLabel(@"Mines Will Always Be Visible", NSMakeRect(234, 273, 246, 17));
  [hostHiddenMinesTextField setAlignment:4];
  [pane2 addSubview:hostHiddenMinesTextField];

  hostUPnPSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(139, 361, 54, 18)];
  [hostUPnPSwitch setButtonType:NSButtonTypeSwitch];
  [hostUPnPSwitch setTitle:@"UPnP"];
  [hostUPnPSwitch setBezelStyle:2];
  GSFitKeepingRightEdge(hostUPnPSwitch);
  [hostUPnPSwitch setTarget:self];
  [hostUPnPSwitch setAction:@selector(hostUPnPSwitch:)];
  [pane2 addSubview:hostUPnPSwitch];

  [item1 setView:pane2];
  [newGameTabView addTabViewItem:item1];

  NSTabViewItem *item22 = [[NSTabViewItem alloc] initWithIdentifier:@"1"];
  [item22 setLabel:@"Join"];
  NSView *pane23 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 594, 418)];
  NSTextField *t24 = GSLabel(@"Address:", NSMakeRect(157, 392, 60, 17));
  [t24 setAlignment:2];
  GSFitKeepingRightEdge(t24);
  [pane23 addSubview:t24];

  joinAddressField = [[NSTextField alloc] initWithFrame:NSMakeRect(222, 390, 240, 22)];
  [joinAddressField setEditable:YES];
  [joinAddressField setBezeled:YES];
  [joinAddressField setStringValue:@"localhost"];
  [joinAddressField setTarget:self];
  [joinAddressField setAction:@selector(joinAddress:)];
  [pane23 addSubview:joinAddressField];

  NSButton *b25 = [[NSButton alloc] initWithFrame:NSMakeRect(490, 12, 90, 32)];
  [b25 setTitle:@"OK"];
  [b25 setBezelStyle:1];
  [b25 setKeyEquivalent:@"\r"];
  [b25 setTarget:self];
  [b25 setAction:@selector(joinOK:)];
  [pane23 addSubview:b25];

  NSTextField *t26 = GSLabel(@"Port:", NSMakeRect(183, 362, 34, 17));
  [t26 setAlignment:2];
  GSFitKeepingRightEdge(t26);
  [pane23 addSubview:t26];

  joinPortField = [[NSTextField alloc] initWithFrame:NSMakeRect(222, 360, 240, 22)];
  [joinPortField setEditable:YES];
  [joinPortField setBezeled:YES];
  [joinPortField setStringValue:@"50000"];
  [joinPortField setTarget:self];
  [joinPortField setAction:@selector(joinPort:)];
  [pane23 addSubview:joinPortField];

  joinPasswordSwitch = [[NSButton alloc] initWithFrame:NSMakeRect(130, 331, 86, 18)];
  [joinPasswordSwitch setButtonType:NSButtonTypeSwitch];
  [joinPasswordSwitch setTitle:@"Password:"];
  [joinPasswordSwitch setBezelStyle:2];
  GSFitKeepingRightEdge(joinPasswordSwitch);
  [joinPasswordSwitch setTarget:self];
  [joinPasswordSwitch setAction:@selector(joinPasswordSwitch:)];
  [pane23 addSubview:joinPasswordSwitch];

  joinPasswordField = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(222, 330, 240, 22)];
  [joinPasswordField setEditable:YES];
  [joinPasswordField setBezeled:YES];
  [joinPasswordField setTarget:self];
  [joinPasswordField setAction:@selector(joinPassword:)];
  [pane23 addSubview:joinPasswordField];

  NSButton *b27 = [[NSButton alloc] initWithFrame:NSMakeRect(400, 12, 90, 32)];
  [b27 setTitle:@"Cancel"];
  [b27 setBezelStyle:1];
  [b27 setKeyEquivalent:@"\033"];
  [b27 setTarget:window];
  [b27 setAction:@selector(orderOut:)];
  [pane23 addSubview:b27];

  joinTrackerField = [[NSTextField alloc] initWithFrame:NSMakeRect(222, 298, 240, 22)];
  [joinTrackerField setEditable:YES];
  [joinTrackerField setBezeled:YES];
  [joinTrackerField setStringValue:@"tracker.xbolo.org"];
  [joinTrackerField setAlignment:4];
  [joinTrackerField setTarget:self];
  [joinTrackerField setAction:@selector(tracker:)];
  [pane23 addSubview:joinTrackerField];

  NSScrollView *scroll28 = [[NSScrollView alloc] initWithFrame:NSMakeRect(20, 60, 554, 230)];
  [scroll28 setBorderType:2];
  [scroll28 setHasVerticalScroller:YES];
  [scroll28 setHasHorizontalScroller:YES];
  joinTrackerTableView = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 552, 200)];
  NSTableColumn *col29 = [[NSTableColumn alloc] initWithIdentifier:@"GSHostPlayerColumn"];
  [col29 setTitle:@"Host Player"];
  [col29 setWidth:70];
  [joinTrackerTableView addTableColumn:col29];
  NSTableColumn *col30 = [[NSTableColumn alloc] initWithIdentifier:@"GSMapNameColumn"];
  [col30 setTitle:@"Map Name"];
  [col30 setWidth:104];
  [joinTrackerTableView addTableColumn:col30];
  NSTableColumn *col31 = [[NSTableColumn alloc] initWithIdentifier:@"GSPasswordColumn"];
  [col31 setTitle:@"Password"];
  [col31 setWidth:52];
  [joinTrackerTableView addTableColumn:col31];
  NSTableColumn *col32 = [[NSTableColumn alloc] initWithIdentifier:@"GSPlayersColumn"];
  [col32 setTitle:@"Players"];
  [col32 setWidth:44.4521484375];
  [joinTrackerTableView addTableColumn:col32];
  NSTableColumn *col33 = [[NSTableColumn alloc] initWithIdentifier:@"GSPausedColumn"];
  [col33 setTitle:@"Paused"];
  [col33 setWidth:36.4521484375];
  [joinTrackerTableView addTableColumn:col33];
  NSTableColumn *col34 = [[NSTableColumn alloc] initWithIdentifier:@"GSAllowJoinColumn"];
  [col34 setTitle:@"Allow Join"];
  [col34 setWidth:58.4521484375];
  [joinTrackerTableView addTableColumn:col34];
  NSTableColumn *col35 = [[NSTableColumn alloc] initWithIdentifier:@"GSHostnameColumn"];
  [col35 setTitle:@"Hostname"];
  [col35 setWidth:100];
  [joinTrackerTableView addTableColumn:col35];
  NSTableColumn *col36 = [[NSTableColumn alloc] initWithIdentifier:@"GSPortColumn"];
  [col36 setTitle:@"Port"];
  [col36 setWidth:53.4521484375];
  [joinTrackerTableView addTableColumn:col36];
  [joinTrackerTableView setDelegate:self];
  [joinTrackerTableView setDataSource:self];
  [scroll28 setDocumentView:joinTrackerTableView];
  [pane23 addSubview:scroll28];

  NSButton *b37 = [[NSButton alloc] initWithFrame:NSMakeRect(14, 12, 88, 32)];
  [b37 setTitle:@"Refresh"];
  [b37 setBezelStyle:1];
  [b37 setTarget:self];
  [b37 setAction:@selector(joinTrackerRefresh:)];
  [pane23 addSubview:b37];

  NSTextField *t38 = GSLabel(@"Tracker:", NSMakeRect(161, 300, 56, 17));
  [t38 setAlignment:4];
  [pane23 addSubview:t38];

  [item22 setView:pane23];
  [newGameTabView addTabViewItem:item22];

  [content addSubview:newGameTabView];

  [window setInitialFirstResponder:newGameTabView];
  [window setFrameAutosaveName:@"GSNewGameWindow"];
  [window setContentSize:NSMakeSize(640, 480)];

  newGameWindow = window;
}


- (void)buildBoloWindow {
  NSWindow *window =
    [[NSWindow alloc] initWithContentRect:NSMakeRect(9.0, 115.0, 1186.0, 744.0)
                                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskResizable)
                                  backing:NSBackingStoreBuffered
                                    defer:YES];
  [window setTitle:@"XBolo"];
  [window setReleasedWhenClosed:NO];
  [window setDelegate:(id)self];

  NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 1186.0, 744.0)];
  [scroll setHasVerticalScroller:YES];
  [scroll setHasHorizontalScroller:YES];
  [scroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];

  boloView = [[GSBoloView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 4096.0, 4096.0)];
  [boloView setBoloController:self];
  [scroll setDocumentView:boloView];

  [[window contentView] addSubview:scroll];

  /* the pause notice floats above the scroll view, pinned to the window
     center, so panning the map never moves it */
  pauseOverlayLabel = [[NSTextField alloc] initWithFrame:NSZeroRect];
  [pauseOverlayLabel setEditable:NO];
  [pauseOverlayLabel setSelectable:NO];
  [pauseOverlayLabel setBordered:NO];
  [pauseOverlayLabel setBezeled:NO];
  [pauseOverlayLabel setDrawsBackground:NO];
  [pauseOverlayLabel setFont:[NSFont systemFontOfSize:72.0 weight:NSFontWeightBold]];
  [pauseOverlayLabel setTextColor:[NSColor whiteColor]];
  [pauseOverlayLabel setHidden:YES];

  NSShadow *shadow = [[NSShadow alloc] init];
  [shadow setShadowColor:[[NSColor blackColor] colorWithAlphaComponent:0.8]];
  [shadow setShadowBlurRadius:4.0];
  [shadow setShadowOffset:NSMakeSize(0.0, -2.0)];
  [pauseOverlayLabel setShadow:shadow];

  [[window contentView] addSubview:pauseOverlayLabel];

  [window setFrameAutosaveName:@"GSXBoloWindow"];

  boloWindow = window;
}


- (void)buildBuilderToolView {
  /* the builder tool selector shown in the game window toolbar; the nib
     kept this as a top-level view outside any window */
  NSButtonCell *proto = [[NSButtonCell alloc] init];
  [proto setButtonType:NSButtonTypeOnOff];
  [proto setBezelStyle:NSBezelStyleShadowlessSquare];
  [proto setBordered:YES];
  [proto setImagePosition:NSImageOnly];

  builderToolMatrix = [[NSMatrix alloc] initWithFrame:NSMakeRect(0.0, 0.0, 180.0, 36.0)
                                                 mode:NSRadioModeMatrix
                                            prototype:proto
                                         numberOfRows:1
                                      numberOfColumns:5];
  [builderToolMatrix setCellSize:NSMakeSize(36.0, 36.0)];

  NSArray *images = @[@"TreeRadio", @"RoadRadio", @"WallRadio", @"PillRadio", @"MineRadio"];
  for (NSUInteger i = 0; i < images.count; i++) {
    NSButtonCell *cell = [builderToolMatrix cellAtRow:0 column:(NSInteger)i];
    [cell setImage:[NSImage imageNamed:images[(NSUInteger)i]]];
    [cell setTitle:@""];
    [cell setTag:(NSInteger)i];
  }

  [builderToolMatrix setTarget:self];
  [builderToolMatrix setAction:@selector(builderTool:)];
  [builderToolMatrix selectCellWithTag:0];

  builderToolView = builderToolMatrix;
}

@end
