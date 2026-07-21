/*
 *  client_internal.h
 *  XBolo
 *
 *  Declarations shared between the client engine translation units
 *  (client.c, client_recv.c, client_send.c, client_logic.c).  Not part
 *  of the public client API in client.h.
 */

#ifndef __CLIENT_INTERNAL__
#define __CLIENT_INTERNAL__

#include "client.h"
#include "server.h"
#include "terrain.h"
#include "tiles.h"
#include "images.h"
#include "bmap_client.h"
#include "io.h"
#include "errchk.h"
#include "timing.h"
#include "resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* builder state shared between the protocol and logic modules */
extern int buildertask;
extern int collisionowner;

int cleanupclient();
int recvsrpause();
int recvsrsendmesg();
int recvsrdamage();
int recvsrgrabtrees();
int recvsrbuild();
int recvsrgrow();
int recvsrflood();
int recvsrplacemine();
int recvsrdropmine();
int recvsrdropboat();
int recvsrplayerjoin();
int recvsrplayerrejoin();
int recvsrplayerexit();
int recvsrplayerdisc();
int recvsrplayerkick();
int recvsrplayerban();
int recvsrrepairpill();
int recvsrcoolpill();
int recvsrcapturepill();
int recvsrbuildpill();
int recvsrdroppill();
int recvsrreplenishbase();
int recvsrcapturebase();
int recvsrrefuel();
int recvsrgrabboat();
int recvsrmineack();
int recvsrbuilderack();
int recvsrsmallboom();
int recvsrsuperboom();
int recvsrhittank();
int recvsrsetalliance();
int recvsrtimelimit();
int recvsrbasecontrol();
int sendcldropboat(int x, int y);
int sendcldroppills(float x, float y, uint16_t pills);
int sendcldropmine(int x, int y);
int sendcltouch(int x, int y);
int sendclgrabtile(int x, int y);
int sendclgrabtrees(int x, int y);
int sendclbuildroad(int x, int y, int trees);
int sendclbuildwall(int x, int y, int trees);
int sendclbuildboat(int x, int y, int trees);
int sendclbuildpill(int x, int y, int trees, int pill);
int sendclrepairpill(int x, int y, int trees);
int sendclplacemine(int x, int y);
int sendcldamage(int x, int y, int boat);
int sendclsmallboom(int x, int y);
int sendclsuperboom(int x, int y);
int sendclrefuel(int base, int armour, int shells, int mines);
int sendclhittank(int player, float dir);
int sendclupdate();
float maxspeed(int x, int y);
float maxturnspeed(int x, int y);
float builderspeed(int x, int y, int player);
float buildertargetspeed(int x, int y);
int decreasevis(Recti r);
int increasevis(Recti r);
int refreshsquare(int x, int y);
int enter(Pointi new, Pointi old);
int spawn();
int tankmovelogic(int player);
int tanklocallogic(Pointi old);
int builderlogic(int player);
int pilllogic(Vec2f old);
int shelllogic(int player);
int explosionlogic(int player);
int testhiddenmine(int x, int y);
int w2t(float f);
float rounddir(float dir);
Vec2f collisiondetect(Vec2f p, float radius, int (*func)(Pointi square));
int tankcollision(Pointi square);
int buildercollision(Pointi square);
int killsquarebuilder(Pointi p);
int killpointbuilder(Vec2f p);
int killbuilder();
int tilefor(int x, int y);
int fogtilefor(int x, int y, int tile);
int killtank();
int drown();
int smallboom();
int superboom();
int clientfindpill(int x, int y);
int clientfindbase(int x, int y);
int clientprintmessage(int type, const char *body);
int shellcollisiontest(struct Shell *shell, int player);
int tanktest(int x, int y);
int tankonaboattest(int x, int y);
int testalliance(int p1, int p2);
int circlesquare(Vec2f point, float radius, Pointi square);
int getbuildertaskforcommand(int command, Pointi at);
void *clientmainthread(void *);
int dgramclient();
int joinclient();

#endif /* __CLIENT_INTERNAL__ */
