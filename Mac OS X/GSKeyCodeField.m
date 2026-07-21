#import "GSKeyCodeField.h"

NSMutableDictionary *nameDictionary;

@implementation GSKeyCodeFieldCell

+ (void)initialize {
  if (self == [GSKeyCodeFieldCell class]) {
    nameDictionary = [[NSMutableDictionary alloc] init];
    [nameDictionary setObject:@"A" forKey:@"0"];
    [nameDictionary setObject:@"S" forKey:@"1"];
    [nameDictionary setObject:@"D" forKey:@"2"];
    [nameDictionary setObject:@"F" forKey:@"3"];
    [nameDictionary setObject:@"H" forKey:@"4"];
    [nameDictionary setObject:@"G" forKey:@"5"];
    [nameDictionary setObject:@"Z" forKey:@"6"];
    [nameDictionary setObject:@"X" forKey:@"7"];
    [nameDictionary setObject:@"C" forKey:@"8"];
    [nameDictionary setObject:@"V" forKey:@"9"];

    [nameDictionary setObject:@"B" forKey:@"11"];
    [nameDictionary setObject:@"Q" forKey:@"12"];
    [nameDictionary setObject:@"W" forKey:@"13"];
    [nameDictionary setObject:@"E" forKey:@"14"];
    [nameDictionary setObject:@"R" forKey:@"15"];
    [nameDictionary setObject:@"Y" forKey:@"16"];
    [nameDictionary setObject:@"T" forKey:@"17"];
    [nameDictionary setObject:@"1" forKey:@"18"];
    [nameDictionary setObject:@"2" forKey:@"19"];
    [nameDictionary setObject:@"3" forKey:@"20"];
    [nameDictionary setObject:@"4" forKey:@"21"];
    [nameDictionary setObject:@"6" forKey:@"22"];
    [nameDictionary setObject:@"5" forKey:@"23"];
    [nameDictionary setObject:@"=" forKey:@"24"];
    [nameDictionary setObject:@"9" forKey:@"25"];
    [nameDictionary setObject:@"7" forKey:@"26"];
    [nameDictionary setObject:@"-" forKey:@"27"];
    [nameDictionary setObject:@"8" forKey:@"28"];
    [nameDictionary setObject:@"0" forKey:@"29"];
    [nameDictionary setObject:@"]" forKey:@"30"];
    [nameDictionary setObject:@"O" forKey:@"31"];
    [nameDictionary setObject:@"U" forKey:@"32"];
    [nameDictionary setObject:@"[" forKey:@"33"];
    [nameDictionary setObject:@"I" forKey:@"34"];
    [nameDictionary setObject:@"P" forKey:@"35"];
    [nameDictionary setObject:@"Return" forKey:@"36"];
    [nameDictionary setObject:@"L" forKey:@"37"];
    [nameDictionary setObject:@"J" forKey:@"38"];
    [nameDictionary setObject:@"'" forKey:@"39"];
    [nameDictionary setObject:@"K" forKey:@"40"];
    [nameDictionary setObject:@";" forKey:@"41"];
    [nameDictionary setObject:@"\\" forKey:@"42"];
    [nameDictionary setObject:@"," forKey:@"43"];
    [nameDictionary setObject:@"/" forKey:@"44"];
    [nameDictionary setObject:@"N" forKey:@"45"];
    [nameDictionary setObject:@"M" forKey:@"46"];
    [nameDictionary setObject:@"." forKey:@"47"];
    [nameDictionary setObject:@"Tab" forKey:@"48"];
    [nameDictionary setObject:@"Spacebar" forKey:@"49"];
    [nameDictionary setObject:@"`" forKey:@"50"];
    [nameDictionary setObject:@"Delete" forKey:@"51"];
    [nameDictionary setObject:@"Enter" forKey:@"52"];
    [nameDictionary setObject:@"Escape" forKey:@"53"];

    [nameDictionary setObject:@"Command" forKey:@"55"];
    [nameDictionary setObject:@"Shift" forKey:@"56"];
    [nameDictionary setObject:@"Caps Lock" forKey:@"57"];
    [nameDictionary setObject:@"Option" forKey:@"58"];
    [nameDictionary setObject:@"Control" forKey:@"59"];
    [nameDictionary setObject:@"Fn Shift" forKey:@"60"];


    [nameDictionary setObject:@"Function" forKey:@"63"];

    [nameDictionary setObject:@"." forKey:@"65"];

    [nameDictionary setObject:@"*" forKey:@"67"];

    [nameDictionary setObject:@"+" forKey:@"69"];

    [nameDictionary setObject:@"Clear" forKey:@"71"];



    [nameDictionary setObject:@"/" forKey:@"75"];
    [nameDictionary setObject:@"Enter" forKey:@"76"];

    [nameDictionary setObject:@"-" forKey:@"78"];


    [nameDictionary setObject:@"=" forKey:@"81"];
    [nameDictionary setObject:@"0" forKey:@"82"];
    [nameDictionary setObject:@"1" forKey:@"83"];
    [nameDictionary setObject:@"2" forKey:@"84"];
    [nameDictionary setObject:@"3" forKey:@"85"];
    [nameDictionary setObject:@"4" forKey:@"86"];
    [nameDictionary setObject:@"5" forKey:@"87"];
    [nameDictionary setObject:@"6" forKey:@"88"];
    [nameDictionary setObject:@"7" forKey:@"89"];

    [nameDictionary setObject:@"8" forKey:@"91"];
    [nameDictionary setObject:@"9" forKey:@"92"];



    [nameDictionary setObject:@"F5" forKey:@"96"];
    [nameDictionary setObject:@"F6" forKey:@"97"];
    [nameDictionary setObject:@"F7" forKey:@"98"];
    [nameDictionary setObject:@"F3" forKey:@"99"];
    [nameDictionary setObject:@"F8" forKey:@"100"];
    [nameDictionary setObject:@"F9" forKey:@"101"];
    
    [nameDictionary setObject:@"F11" forKey:@"103"];







    [nameDictionary setObject:@"F10" forKey:@"109"];
    [nameDictionary setObject:@"Fn Enter" forKey:@"110"];
    [nameDictionary setObject:@"F12" forKey:@"111"];
    
    
    
    [nameDictionary setObject:@"Home" forKey:@"115"];
    [nameDictionary setObject:@"Page Up" forKey:@"116"];
    [nameDictionary setObject:@"Fn Delete" forKey:@"117"];
    [nameDictionary setObject:@"F4" forKey:@"118"];
    [nameDictionary setObject:@"End" forKey:@"119"];
    [nameDictionary setObject:@"F2" forKey:@"120"];
    [nameDictionary setObject:@"Page Down" forKey:@"121"];
    [nameDictionary setObject:@"F1" forKey:@"122"];
    [nameDictionary setObject:@"Left Arrow" forKey:@"123"];
    [nameDictionary setObject:@"Right Arrow" forKey:@"124"];
    [nameDictionary setObject:@"Down Arrow" forKey:@"125"];
    [nameDictionary setObject:@"Up Arrow" forKey:@"126"];
    [nameDictionary setObject:@"Num Lock" forKey:@"127"];
  }
}

// ---------------------------------------------------------
//  Initialization
// ---------------------------------------------------------

- (id)init {
  return [self initWithKeyCode:(unsigned short)-1];
}

- (id)initWithKeyCode:(unsigned short)aKeyCode {
  if ((self = [super init]) != nil) {
    keyCode = aKeyCode;
  }
  return self;
}

- (id)initWithCoder:(NSCoder *)decoder {
  self = [super initWithCoder:decoder];
  if ([decoder allowsKeyedCoding]) {
    keyCode = [decoder decodeIntForKey:@"GSKeyCode"];
  }
  else {
    [decoder decodeValueOfObjCType:@encode(unsigned short) at:&keyCode];
  }
  return self;
}

- (void)encodeWithCoder:(NSCoder *)coder {
  [super encodeWithCoder:coder];
  if ([coder allowsKeyedCoding]) {
    [coder encodeInt:keyCode forKey:@"GSKeyCode"];
  }
  else {
    [coder encodeValueOfObjCType:@encode(unsigned short) at:&keyCode];
  }
}

- (id)copyWithZone:(NSZone *)zone {
  return [[GSKeyCodeFieldCell allocWithZone:zone] initWithKeyCode:[self keyCode]];
}

- (void)sendActionToTarget {
  if ([self target] && [self action]) {
    [(NSControl *)[self controlView] sendAction:[self action] to:[self target]];
  }
}

// ---------------------------------------------------------
//  Setting and getting values
// ---------------------------------------------------------

- (void)setKeyCode:(unsigned short)aKeyCode {
  if (keyCode != aKeyCode) {
    keyCode = aKeyCode;
    [(NSControl *)[self controlView] updateCell:self];
    [self sendActionToTarget];
  }
}

- (unsigned short)keyCode {
  return keyCode;
}

- (void)setObjectValue:(id)object {
  if ([object isMemberOfClass:[NSString class]]) {
    [self setStringValue:object];
  }
  else {
    [NSException raise: NSInvalidArgumentException format: @"%@ Invalid object %@", NSStringFromSelector(_cmd), object];
  }
}

- (id)objectValue {
  return [self stringValue];
}

- (void)setStringValue:(NSString *)string {
  int aKeyCode;
  NSScanner *scanner;
  scanner = [NSScanner scannerWithString:string];
  if ([scanner scanInt:&aKeyCode] && [scanner isAtEnd]) {
    [self setKeyCode:aKeyCode];
  }
  else {
    [NSException raise: NSInvalidArgumentException format: @"%@ Invalid string %@", NSStringFromSelector(_cmd), string];
  }
}

- (NSString *)stringValue {
  return [NSString stringWithFormat:@"%d", keyCode];
}

// ---------------------------------------------------------
//  Target / action methods
// ---------------------------------------------------------

- (IBAction)takeKeyValueFrom:(id)sender {
  if ([sender isMemberOfClass:[GSKeyCodeFieldCell class]]) {
    [self setKeyCode:[sender keyCode]];
  }
  else {
    [self setStringValue:[sender stringValue]];
  }
}

// ---------------------------------------------------------
//  Drawing Routines
// ---------------------------------------------------------

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
  NSString *string;
  NSRect bezelRect;
  NSBezierPath *bezel;

  if (keyCode == (unsigned short)-1) {
    string = [NSString string];
  }
  else {
    string = [nameDictionary objectForKey:[NSString stringWithFormat:@"%hu", keyCode]];
    if (string == nil) {
      string = [NSString stringWithFormat:@"%hu", keyCode];
    }
  }

  /* modern rounded field, semantic colors so light and dark mode both work;
     the recorded key is shown centered, in the accent color while the field
     has focus (recording) */
  bezelRect = NSInsetRect(cellFrame, 0.5, 0.5);
  bezel = [NSBezierPath bezierPathWithRoundedRect:bezelRect xRadius:5.0 yRadius:5.0];
  [[NSColor textBackgroundColor] set];
  [bezel fill];
  [[NSColor separatorColor] set];
  [bezel stroke];

  if ([string length] > 0) {
    /* the recorded key is shown as a keycap chip, like the shortcut
       recorders in System Settings; the chip picks up the accent color
       while the field has focus (recording) */
    NSMutableParagraphStyle *style;
    NSDictionary *attributes;
    NSSize textSize;
    NSRect chipRect, textRect;
    NSBezierPath *chip;
    BOOL recording;
    CGFloat chipWidth, chipHeight;

    recording = [self showsFirstResponder];

    style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSTextAlignmentCenter];
    [style setLineBreakMode:NSLineBreakByTruncatingTail];

    attributes = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:[self controlSize]] - 1.0], NSFontAttributeName,
      recording ? [NSColor alternateSelectedControlTextColor] : [NSColor labelColor], NSForegroundColorAttributeName,
      style, NSParagraphStyleAttributeName,
      nil];

    textSize = [string sizeWithAttributes:attributes];
    chipWidth = MIN(textSize.width + 14.0, NSWidth(cellFrame) - 6.0);
    chipHeight = MIN(textSize.height + 3.0, NSHeight(cellFrame) - 4.0);
    chipRect = NSMakeRect(NSMidX(cellFrame) - chipWidth/2.0,
                          NSMidY(cellFrame) - chipHeight/2.0,
                          chipWidth, chipHeight);

    chip = [NSBezierPath bezierPathWithRoundedRect:chipRect xRadius:4.0 yRadius:4.0];
    [recording ? [NSColor controlAccentColor] : [NSColor quaternaryLabelColor] set];
    [chip fill];

    textRect = NSMakeRect(NSMinX(chipRect) + 3.0,
                          NSMidY(chipRect) - textSize.height/2.0,
                          NSWidth(chipRect) - 6.0,
                          textSize.height);
    [string drawInRect:textRect withAttributes:attributes];
  }

  if ([self showsFirstResponder]) {
    [NSGraphicsContext saveGraphicsState];
    NSSetFocusRingStyle(NSFocusRingOnly);
    [[NSBezierPath bezierPathWithRoundedRect:bezelRect xRadius:5.0 yRadius:5.0] fill];
    [NSGraphicsContext restoreGraphicsState];
  }
}

// ---------------------------------------------------------
//  Mouse Tracking
// ---------------------------------------------------------

- (BOOL)trackMouse:(NSEvent *)theEvent inRect:(NSRect)cellFrame ofView:(NSView *)controlView untilMouseUp:(BOOL)flag {
	[(NSControl *)controlView updateCell:self];
  return YES;
}

// ---------------------------------------------------------
//  Context Menu
// ---------------------------------------------------------

- (NSMenu *)menuForEvent:(NSEvent *)theEvent inRect:(NSRect)cellFrame ofView:(NSView *)controlView {
  return nil;
}

// ---------------------------------------------------------
//  Keyboard Event Handling : Binding Methods
// ---------------------------------------------------------

- (void)performClick:(id)sender {
}

@end

@implementation GSKeyCodeField

+ (void)initialize {
  if (self == [GSKeyCodeField class]) {
    [self setCellClass:[GSKeyCodeFieldCell class]];
  }
}

+ (Class)cellClass {
  return [GSKeyCodeFieldCell class];
}

- (id)initWithFrame:(NSRect)frameRect {
  if ((self = [super initWithFrame:frameRect]) != nil) {
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)performClick:(id)sender {
  [[self cell] performClick:sender];
}

- (void)setKeyCode:(unsigned short)aKeyCode {
  [[self cell] setKeyCode:aKeyCode];
}

- (unsigned short)keyCode {
  return [[self cell] keyCode];
}

- (IBAction)takeKeyValueFrom:(id)sender {
  [[self cell] takeKeyValueFrom:sender];
}

// ---------------------------------------------------------
//  Focus ring maintenance
// ---------------------------------------------------------

- (BOOL)becomeFirstResponder {
  BOOL okToChange;
  if ((okToChange = [super becomeFirstResponder])) {
    [super setKeyboardFocusRingNeedsDisplayInRect:[self bounds]];
    modifiers = [NSEvent modifierFlags] & (NSEventModifierFlagCapsLock | NSEventModifierFlagShift | NSEventModifierFlagControl | NSEventModifierFlagOption | NSEventModifierFlagCommand);
  }
  return okToChange;
}

- (BOOL)resignFirstResponder {
  BOOL okToChange;
  okToChange = [super resignFirstResponder];
  if (okToChange) {
    [super setKeyboardFocusRingNeedsDisplayInRect:[self bounds]];
  }
  return okToChange;
}

- (void)windowKeyStateDidChange:(NSNotification *)notif {
  if ([[self window] firstResponder] == self) {
    [super setKeyboardFocusRingNeedsDisplayInRect:[self bounds]];
  }
}

- (void)viewDidMoveToWindow {
  NSNotificationCenter *notifCenter = [NSNotificationCenter defaultCenter];
  SEL callback = @selector(windowKeyStateDidChange:);

  // If we've been installed in a new window, unregister for notificaions in the old window...
  [notifCenter removeObserver:self];

  // ... then register for notifications in the new window.
  [notifCenter addObserver:self selector:callback name:NSWindowDidBecomeKeyNotification object:[self window]];
  [notifCenter addObserver:self selector:callback name:NSWindowDidResignKeyNotification object:[self window]];
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (BOOL)needsPanelToBecomeKey {
  return YES;		// Clicking and tabbing to us will makes us key
}

- (void)keyDown:(NSEvent *)theEvent {
  if (![theEvent isARepeat]) {
    [[self cell] setKeyCode:[theEvent keyCode]];
    [[self window] selectNextKeyView:self];
  }
}

- (void)keyUp:(NSEvent *)theEvent {
  if (![theEvent isARepeat]) {
    // do nothing
  }
}

- (void)flagsChanged:(NSEvent *)theEvent {
  unsigned int oldModifiers;
  oldModifiers = modifiers;
  modifiers = [theEvent modifierFlags] & (NSEventModifierFlagCapsLock | NSEventModifierFlagShift | NSEventModifierFlagControl | NSEventModifierFlagOption | NSEventModifierFlagCommand | NSEventModifierFlagNumericPad | NSEventModifierFlagHelp | NSEventModifierFlagFunction);
  if (modifiers & (oldModifiers ^ modifiers)) {
    [[self cell] setKeyCode:[theEvent keyCode]];
    [[self window] selectNextKeyView:self];
  }
  else {
    // do nothing
  }
}

@end
