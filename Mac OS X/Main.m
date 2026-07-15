#import <Cocoa/Cocoa.h>
#import "GSXBoloController.h"
#include "bolo.h"
#include "server.h"
#include "client.h"
#include "errchk.h"

int main(int argc, const char *argv[]) {
TRY
  if (initbolo(setplayerstatus, setpillstatus, setbasestatus, settankstatus, playsound, printmessage, joinprogress, clientloopupdate)) LOGFAIL(errno)

CLEANUP
  switch (ERROR) {
  case 0:
    {
      /* no main nib: create the application and its controller directly;
         the controller builds the entire UI in
         applicationWillFinishLaunching: */
      NSApplication *app = [NSApplication sharedApplication];
      static GSXBoloController *controller;
      controller = [[GSXBoloController alloc] init];
      [app setDelegate:(id)controller];
      RETURN(NSApplicationMain(argc, argv))
    }

  default:
    PCRIT(ERROR);
    printlineinfo();
    CLEARERRLOG
    exit(EXIT_FAILURE);
  }
END
}

