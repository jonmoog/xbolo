/*
 *  GSXBoloController+CodeUI.h
 *  XBolo
 *
 *  Windows built in code, migrating the UI out of the frozen IB3
 *  MainMenu.nib one window at a time.  Each build method creates its
 *  window and repoints the controller's outlets at it, replacing the
 *  nib-loaded version.
 */

#import "GSXBoloController.h"

@interface GSXBoloController (CodeUI)

- (void)buildJoinProgressWindow;
- (void)buildStatusPanel;
- (void)buildAllegiancePanel;
- (void)buildMessagesPanel;
- (void)buildMainMenu;
- (void)buildPreferencesWindow;
- (void)buildNewGameWindow;

@end
