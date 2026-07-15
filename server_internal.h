/*
 *  server_internal.h
 *  XBolo
 *
 *  Declarations shared between the server engine translation units
 *  (server.c, server_recv.c, server_send.c, server_logic.c).  Not part
 *  of the public server API in server.h.
 */

#ifndef __SERVER_INTERNAL__
#define __SERVER_INTERNAL__

#include "server.h"
#include "client.h"
#include "terrain.h"
#include "tiles.h"
#include "bmap_server.h"
#include "tracker.h"
#include "errchk.h"
#include "io.h"
#include "timing.h"
#include "resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <math.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <fcntl.h>

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

void *servermainthread(void *);
int recvclsendmesg(int player);
int recvcldropboat(int player);
int recvcldroppills(int player);
int recvcldropmine(int player);
int recvcltouch(int player);
int recvclgrabtile(int player);
int recvclgrabtrees(int player);
int recvclbuildroad(int player);
int recvclbuildwall(int player);
int recvclbuildboat(int player);
int recvclbuildpill(int player);
int recvclrepairpill(int player);
int recvclplacemine(int player);
int recvcldamage(int player);
int recvclsmallboom(int player);
int recvclsuperboom(int player);
int recvclrefuel(int player);
int recvclhittank(int player);
int recvclsetalliance(int player);
int sendsrsendmesg(int player, int to, uint16_t mask, const char *text);
int sendsrplayerjoin(int player);
int sendsrplayerrejoin(int player);
int sendsrplayerexit(int player);
int sendsrplayerdisc(int player);
int sendsrplayerkick(int player);
int sendsrplayerban(int player);
int sendsrdamage(int player, int x, int y);
int sendsrgrabtrees(int x, int y);
int sendsrbuild(int x, int y);
int sendsrgrow(int x, int y);
int sendsrflood(int x, int y);
int sendsrplacemine(int player, int x, int y);
int sendsrdropmine(int player, int x, int y);
int sendsrdropboat(int x, int y);
int sendsrrepairpill(int pill);
int sendsrcoolpill(int pill);
int sendsrcapturepill(int pill);
int sendsrbuildpill(int pill);
int sendsrdroppill(int pill);
int sendsrreplenishbase(int base);
int sendsrcapturebase(int base);
int sendsrrefuel(int player, int base, int armour, int shells, int mines);
int sendsrgrabboat(int player, int x, int y);
int sendsrmineack(int player, int success);
int sendsrbuilderack(int player, int mines, int trees, int pill);
int sendsrsmallboom(int player, int x, int y);
int sendsrsuperboom(int player, int x, int y);
int sendsrhittank(int player, uint32_t dir);
int sendsrsetalliance(int player, uint16_t alliance);
int sendsrtimelimit(uint16_t timeremaining);
int sendsrbasecontrol(uint16_t timeleft);
int sendsrpause(int counter);
int sendtoall(const void *data, size_t size);
int sendtoallex(const void *data, size_t size, int player);
int sendtoone(const void *data, size_t size, int player);
void growtrees(int nplayers);
int chain();
int chainat(int x, int y);
int explosionat(int player, int x, int y);
int superboomat(int player, int x, int y);
int floodtest(int x, int y);
int flood();
int floodat(int x, int y);
int findpill(int x, int y);
int findbase(int x, int y);
void droppills(int player, float x, float y, uint16_t pills);
int removeplayer(int player);
int nplayers();
int cleanupserver();

#endif /* __SERVER_INTERNAL__ */
