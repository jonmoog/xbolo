/*
 *  client.c
 *  XBolo
 *
 *  Created by Robert Chrzanowski on 4/27/09.
 *  Copyright 2009 Robert Chrzanowski. All rights reserved.
 *
 */

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
#include "client_internal.h"

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

/**********************/
/* external variables */
/**********************/

int client_running = 0;
struct Client client;

/********************/
/* static variables */
/********************/

static struct {
  /* client mutex */
  pthread_mutex_t mutex;
} iclient;


/*******************/
/* static routines */
/*******************/


/* client receive routines */


/* client send routines */


/* builder speed routines */


/* visibility routines */


/* refreshes a changed square */

/* terrain physics */

/* starts the player */

/* game logic routines */


/* convert world scale to tile */

/* converts radians to rudimentary direction */

/*  */

/* kill builder routines */

/* terrain to tile mapping functions */

/* tank killing routines */

/* pill and base finding routines */




/* client pthread routines */



int initclient(void (*setplayerstatusfunc)(int player), void (*setpillstatusfunc)(int pill), void (*setbasestatusfunc)(int base), void (*settankstatusfunc)(), void (*playsound)(int sound), void (*printmessagefunc)(int type, const char *text), void (*joinprogress)(int statuscode, float scale), void (*loopupdate)(void)) {
  int err;

TRY
  client.hostname = NULL;
  client.hiddenmines = 1;
  client.player = -1;

  if ((err = pthread_mutex_init(&iclient.mutex, NULL))) LOGFAIL(err)

  client.setplayerstatus = setplayerstatusfunc;
  client.setpillstatus = setpillstatusfunc;
  client.setbasestatus = setbasestatusfunc;
  client.settankstatus = settankstatusfunc;
  client.playsound = playsound;
  client.printmessage = printmessagefunc;
  client.joinprogress = joinprogress;
  client.loopupdate = loopupdate;

  client.cntlsock = -1;
  client.dgramsock = -1;

  initlist(&client.changedtiles);
  initlist(&client.explosions);

  if (initbuf(&client.sendbuf)) LOGFAIL(errno)
  if (initbuf(&client.recvbuf)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, errno)
END
}


int startclient(const char *hostname, uint16_t port, const char playername[], const char password[]) {
  int y, x, i;
  int flags;
  pthread_t thread;
  int err;

  assert(!client_running);

TRY
  client.timelimitreached = 0;
  client.basecontrolreached = 0;
  client.spawned = 0;
  client.kills = 0;
  client.deaths = 0;

  client.buildermines = 0;
  client.buildertrees = 0;
  client.builderpill = NOPILL;

  /* initialize the players */
  for (i = 0; i < MAXPLAYERS; i++) {
    client.players[i].used = 0;
    client.players[i].connected = 0;

    bzero(client.players[i].name, MAXNAME);
    bzero(client.players[i].host, MAXHOST);

    client.players[i].seq = 0;
    client.players[i].lastupdate = 0;
    client.players[i].dead = 1;
    client.players[i].boat = 1;
    client.players[i].tank = make2f(0.0, 0.0);
    client.players[i].dir = 0.0;
    client.players[i].speed = 0.0;
    client.players[i].turnspeed = 0.0;
    client.players[i].kickdir = 0.0;
    client.players[i].kickspeed = 0.0;
    client.players[i].builderstatus = kBuilderReady;
    client.players[i].builder = make2f(0.0, 0.0);
    client.players[i].buildertarget = makepoint(0, 0);
    client.players[i].builderwait = 0;
    client.players[i].alliance = 0;
    client.players[i].inputflags = 0;
    initlist(&client.players[i].shells);
    initlist(&client.players[i].explosions);

    if (client.setplayerstatus) {
      client.setplayerstatus(i);
    }
  }

  /* initialize the player variables */
  client.players[client.player].inputflags = 0;

  /* copy name and password */
  strncpy(client.name, playername, sizeof(client.name) - 1);

  if ((client.passreq = password != NULL)) {
    strncpy(client.pass, password, MAXPASS - 1);
  }
  else {
    client.pass[0] = '\0';
  }

  /* make the screen blank */
  for (y = 0; y < WIDTH; y++) {
    for (x = 0; x < WIDTH; x++) {
      client.images[y][x] = SEAA00IMAGE;
      client.fog[y][x] = 0;
    }
  }

  /* copy hostname */
  if ((client.hostname = (char *)malloc(strlen(hostname) + 1)) == NULL) LOGFAIL(errno)
  strcpy(client.hostname, hostname);

  /* set port number */
  client.srvaddr.sin_port = htons(port);

  /* create the client.cntlsock */
  if ((client.cntlsock = socket(AF_INET, SOCK_STREAM, 0)) == -1) LOGFAIL(errno)
  if ((flags = fcntl(client.cntlsock, F_GETFL, 0)) == -1) LOGFAIL(errno)
  if (fcntl(client.cntlsock, F_SETFL, flags | O_NONBLOCK)) LOGFAIL(errno);

  /* create the client.dgramsock */
  if ((client.dgramsock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) LOGFAIL(errno)
  if ((flags = fcntl(client.dgramsock, F_GETFL, 0)) == -1) LOGFAIL(errno)
  if (fcntl(client.dgramsock, F_SETFL, flags | O_NONBLOCK)) LOGFAIL(errno);

  /* clear buffers */
  if (readbuf(&client.sendbuf, NULL, client.sendbuf.nbytes) == -1) LOGFAIL(errno)
  if (readbuf(&client.recvbuf, NULL, client.recvbuf.nbytes) == -1) LOGFAIL(errno)

  /* initialize control pipes */
  if (pipe(client.mainpipe)) LOGFAIL(errno)
  if (pipe(client.threadpipe)) LOGFAIL(errno)

  /* create the mainthread */
  if ((err = pthread_create(&thread, NULL, clientmainthread, NULL))) LOGFAIL(err)
  if ((err = pthread_detach(thread))) LOGFAIL(err)

  client_running = 1;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (client.cntlsock != -1) {
      while (close(client.cntlsock) && errno == EINTR);
      client.cntlsock = -1;
    }

    if (client.dgramsock != -1) {
      while (close(client.dgramsock) && errno == EINTR);
      client.dgramsock = -1;
    }

    RETERR(-1)
  }
END
}


int stopclient() {
  char *buf[10];
  ssize_t r;

  assert(client_running);

TRY
  /* close mainpipe */
  if (closesock(client.mainpipe + 1)) LOGFAIL(errno)

  /* wait for threadpipe to be closed */
  for (;;) {
    if ((r = read(client.threadpipe[0], buf, sizeof(buf))) == -1) {
      if (errno != EINTR) LOGFAIL(errno)
    }
    else if (r == 0) {
      break;
    }
  }

  if (closesock(client.threadpipe)) LOGFAIL(errno)

  client_running = 0;

CLEANUP
ERRHANDLER(0, -1)
END
}


int cleanupclient() {
  int i;

TRY
  free(client.hostname);
  client.hostname = NULL;

  client.player = -1;

  if (closesock(&client.cntlsock)) LOGFAIL(errno)
  if (closesock(&client.dgramsock)) LOGFAIL(errno)
  clearlist(&client.changedtiles, free);

  for (i = 0; i < MAXPLAYERS; i++) {
    client.players[i].builderstatus = kBuilderReady;
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


/* reads a float stored in a packed network struct without an unaligned cast */

int lockclient() {
  int err;

TRY
  if ((err = pthread_mutex_lock(&iclient.mutex))) LOGFAIL(err)

CLEANUP
ERRHANDLER(0, -1)
END
}


int unlockclient() {
  int err;

TRY
  if ((err = pthread_mutex_unlock(&iclient.mutex))) LOGFAIL(err)

CLEANUP
ERRHANDLER(0, -1)
END
}


int runclient() {
  int i;
  Vec2f old;

TRY
  if (client.timelimitreached || client.basecontrolreached || client.pause) {
    SUCCESS;
  }

  client.players[client.player].seq++;
  old = client.players[client.player].tank;

  /* update status of lagged players */
  if (client.setplayerstatus) {
    for (i = 0; i < MAXPLAYERS; i++) {
      if (client.players[client.player].seq - client.players[i].lastupdate == 3*TICKSPERSEC) {
        client.setplayerstatus(i);
      }
      else if (client.players[client.player].seq - client.players[i].lastupdate == TICKSPERSEC) {
        client.setplayerstatus(i);
      }
    }
  }

  /* move tanks */
  for (i = 0; i < MAXPLAYERS; i++) {
    if (client.players[i].connected && client.players[i].seq != 0) {
      Pointi old, new;

      old = makepoint(client.players[i].tank.x, client.players[i].tank.y);
      if (tankmovelogic(i)) LOGFAIL(errno)
      new = makepoint(client.players[i].tank.x, client.players[i].tank.y);

      if (!isequalpoint(new, old) && testalliance(client.player, i)) {
        if (increasevis(makerect(new.x - 14, new.y - 14, 29, 29))) LOGFAIL(errno)
        if (decreasevis(makerect(old.x - 14, old.y - 14, 29, 29))) LOGFAIL(errno)
      }
    }
  }

  /* local tank logic */
  if (tanklocallogic(makepoint(old.x, old.y))) LOGFAIL(errno)

  /* builder logic */
  for (i = 0; i < MAXPLAYERS; i++) {
    if (builderlogic(i)) LOGFAIL(errno)
  }

  /* pill logic */
  if (pilllogic(old)) LOGFAIL(errno)

  /* perform shell logic */
  for (i = 0; i < MAXPLAYERS; i++) {
    if (shelllogic(i)) LOGFAIL(errno)
  }

  /* perform explosion logic */
  for (i = -1; i < MAXPLAYERS; i++) {
    if (explosionlogic(i)) LOGFAIL(errno)
  }

  /* send updated location */
  if (client.players[client.player].seq%5 == 0) {
    if (sendclupdate()) LOGFAIL(errno)
  }
    
  /* loop update callback */
  client.loopupdate();

CLEANUP
ERRHANDLER(0, -1)
END
}


/* clamps an untrusted coordinate to the map so (int)coord is always a
   valid array index; the leading !(>=0) test also rejects NaN, which raw
   network bytes decoded as a float can produce */
static float clampcoord(float c) {
  if (!(c >= 0.0f)) {
    return 0.0f;
  }

  if (c > FWIDTH - 1.0f) {
    return FWIDTH - 1.0f;
  }

  return c;
}


int joinclient() {
  int i;
  struct JOIN_Preamble joinpreamble;
  struct BOLO_Preamble bolopreamble;
  uint8_t msg;
  int ret;
  int lookup = -1;
  int gotlock = 0;

TRY
  /* see if already a dot quad */
  client.srvaddr.sin_addr.s_addr = inet_addr(client.hostname);

  /* not a dot quad, resolve */
  if (client.srvaddr.sin_addr.s_addr == INADDR_NONE) {
    if (client.joinprogress) {
      client.joinprogress(kJoinRESOLVING, 0.0);
    }

    /* lookup address */
    if ((lookup = nslookup(client.hostname)) == -1) LOGFAIL(errno)

    /* wait for exit or dns reply */
    if ((ret = selectreadread(client.mainpipe[0], lookup)) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      if (closesock(&lookup)) LOGFAIL(errno)
      SUCCESS
    }

    /* get nslookup result */
    if (nslookupresult(lookup, &client.srvaddr.sin_addr)) {
      LOGFAIL(errno)
    }

    if (closesock(&lookup)) LOGFAIL(errno)
  }

  /* initialize name to addr and port */
  client.srvaddr.sin_family = AF_INET;
  bzero(client.srvaddr.sin_zero, 8);

  /* connect to server */
  if (client.joinprogress) {
    client.joinprogress(kJoinCONNECTING, 0.0);
  }
  
  if ((connect(client.cntlsock, (struct sockaddr *)&client.srvaddr, INET_ADDRSTRLEN))) {
    if (errno != EINPROGRESS) LOGFAIL(errno)
  }
  
  /* wait for connection to complete */
  for (;;) {
    int nfds;
    fd_set readfds;
    fd_set writefds;
    
    /* select until we have success */
    for (;;) {
      nfds = 0;
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);

      FD_SET(client.mainpipe[0], &readfds);
      nfds = MAX(nfds, client.mainpipe[0]);

      FD_SET(client.cntlsock, &writefds);
      nfds = MAX(nfds, client.cntlsock);
      
      if ((nfds = select(nfds + 1, &readfds, &writefds, NULL, NULL)) == -1) {
        if (errno != EINTR) LOGFAIL(errno)
      }
      else {
        break;
      }
    }
    
    if (FD_ISSET(client.mainpipe[0], &readfds)) {
      ret = 1;
      SUCCESS
    }
    else if (FD_ISSET(client.cntlsock, &writefds)) {
      if (connect(client.cntlsock, (struct sockaddr *)&client.srvaddr, INET_ADDRSTRLEN) && errno != EISCONN) LOGFAIL(errno)
      break;
    }
  }

  /* get local address of cntlsock */
  struct sockaddr_in saddr;
  socklen_t addrlen;

  addrlen = INET_ADDRSTRLEN;
  if (getsockname(client.cntlsock, (struct sockaddr *)&saddr, &addrlen)) LOGFAIL(errno)

  /* bind dgramsock to local address */
  while (bind(client.dgramsock, (struct sockaddr *)&saddr, addrlen)) {
    if (errno != EAGAIN) {
      LOGFAIL(errno)
    }

    usleep(10000);
  }

  /* set remote end for dgramsock */
  if ((connect(client.dgramsock, (struct sockaddr *)&client.srvaddr, INET_ADDRSTRLEN))) LOGFAIL(errno)

  bcopy(NET_GAME_IDENT, joinpreamble.ident, sizeof(NET_GAME_IDENT));
  joinpreamble.version = NET_GAME_VERSION;
  bzero(joinpreamble.name, MAXNAME);
  strncpy(joinpreamble.name, client.name, sizeof(joinpreamble.name) - 1);
  bzero(joinpreamble.pass, MAXPASS);
  strncpy(joinpreamble.pass, client.pass, sizeof(joinpreamble.pass) - 1);

  if (client.joinprogress) {
    client.joinprogress(kJoinSENDJOIN, 0.0);
  }

  /* send join preamble */
  if (writebuf(&client.sendbuf, &joinpreamble, sizeof(joinpreamble)) == -1) LOGFAIL(errno)

  if ((ret = cntlsend(client.mainpipe[0], client.cntlsock, &client.sendbuf)) == -1) {
    LOGFAIL(errno)
  }
  else if (ret == 1) {
    SUCCESS
  }

  if (client.joinprogress) {
    client.joinprogress(kJoinRECVPREAMBLE, 0.0);
  }

  /* recv msg */
  if ((ret = cntlrecv(client.mainpipe[0], client.cntlsock, &client.recvbuf, sizeof(msg))) == -1) {
    LOGFAIL(errno)
  }
  else if (ret == 1) {
    SUCCESS
  }

  if (readbuf(&client.recvbuf, &msg, sizeof(msg)) == -1) LOGFAIL(errno)

  if (msg == kBadVersionJOIN) FAIL(EBADVERSION)
  if (msg == kDisallowJOIN) FAIL(EDISSALLOW)
  if (msg == kBadPasswordJOIN) FAIL(EBADPASS)
  if (msg == kServerFullJOIN) FAIL(ESERVERFULL)
  if (msg == kServerTimeLimitReachedJOIN) FAIL(ETIMELIMIT)
  if (msg == kBannedPlayerJOIN) FAIL(EBANNEDPLAYER)
  if (msg != kSendingPreambleJOIN) LOGFAIL(ESERVERERROR)

  /* recv preamble */
  if ((ret = cntlrecv(client.mainpipe[0], client.cntlsock, &client.recvbuf, sizeof(bolopreamble))) == -1) {
    LOGFAIL(errno)
  }
  else if (ret == 1) {
    SUCCESS
  }

  if (readbuf(&client.recvbuf, &bolopreamble, sizeof(bolopreamble)) == -1) LOGFAIL(errno)

  /* convert byte order */
  bolopreamble.maplen = ntohl(bolopreamble.maplen);

  /* recv map data */
  ssize_t r;
  if ((r = cntlrecv(client.mainpipe[0], client.cntlsock, &client.recvbuf, bolopreamble.maplen)) == -1) {
    LOGFAIL(errno)
  }
  else if (r == 1) {
    SUCCESS
  }
  if (client.joinprogress) {
    client.joinprogress(kJoinRECVMAP, 100.0);
  }
    
  /*
  while (client.recvbuf.nbytes < bolopreamble.maplen) {
    ssize_t r;

    if ((r = cntlrecv(client.mainpipe[0], client.cntlsock, &client.recvbuf, MIN(bolopreamble.maplen - client.recvbuf.nbytes, RECVMAPBLOCKSIZE))) == -1) {
      LOGFAIL(errno)
    }
    else if (r == 1) {
      SUCCESS
    }

    if (client.joinprogress) {
      client.joinprogress(kJoinRECVMAP, (client.recvbuf.nbytes*100.0)/bolopreamble.maplen);
    }
  }
  */

  /* the server-assigned player id becomes our persistent index into
     client.players[]; reject an out-of-range value from an untrusted
     server before it is used everywhere as an array index */
  if (bolopreamble.player >= MAXPLAYERS) LOGFAIL(EBADVERSION)

  /* modify client */
  if (lockclient()) LOGFAIL(errno)
  gotlock = 1;

  client.player = bolopreamble.player;
  client.hiddenmines = bolopreamble.hiddenmines;

  if (bolopreamble.pause == 255) {
    client.pause = -1;
  }
  else {
    client.pause = bolopreamble.pause;
  }

  client.gametype = bolopreamble.gametype;

  if (bolopreamble.gametype == kDominationGameType) {
    /* the number of seconds to have control of all bases to win */
    switch (bolopreamble.game.domination.type) {
    case kOpenGame:
    case kTournamentGame:
    case kStrictGame:
      client.game.domination.type = bolopreamble.game.domination.type;
      break;

    default:
      assert(0);
    }

    client.game.domination.basecontrol = bolopreamble.game.domination.basecontrol;
  }
  else {
    assert(0);
  }

  /* initialize the players */
  for (i = 0; i < MAXPLAYERS; i++) {
    client.players[i].used = bolopreamble.players[i].used;
    client.players[i].connected = bolopreamble.players[i].connected;
    client.players[i].seq = ntohl(bolopreamble.players[i].seq);

    strncpy(client.players[i].name, (char *)bolopreamble.players[i].name, MAXNAME - 1);
    strncpy(client.players[i].host, (char *)bolopreamble.players[i].host, MAXHOST - 1);

    client.players[i].builderstatus = kBuilderReady;
    client.players[i].alliance = ntohs(bolopreamble.players[i].alliance);

    if (client.setplayerstatus) {
      client.setplayerstatus(i);
    }
  }

  /* load map data */
  if (clientloadmap(client.recvbuf.ptr, bolopreamble.maplen)) LOGFAIL(errno)
  if (readbuf(&client.recvbuf, NULL, bolopreamble.maplen) == -1) LOGFAIL(errno)

  spawn();
  if (increasevis(makerect(((int)client.players[bolopreamble.player].tank.x) - 14, ((int)client.players[bolopreamble.player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
//  if (increasevis(makerect(0, 0, WIDTH, WIDTH)) == -1) LOGFAIL(errno) /* for cheaters */

  if (unlockclient()) LOGFAIL(errno)
  gotlock = 0;

  /* update pill status */
  if (client.setpillstatus) {
    for (i = 0; i < client.npills; i++) {
      client.setpillstatus(i);
    }
  }

  /* update base status */
  if (client.setbasestatus) {
    for (i = 0; i < client.nbases; i++) {
      client.setbasestatus(i);
    }
  }

  /* join complete */
  if (client.joinprogress) {
    client.joinprogress(kJoinSUCCESS, 100.0);
  }

  ret = 0;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(ret)

  default:
    if (lookup != -1) {
      closesock(&lookup);
    }

    if (gotlock) {
      unlockclient();
    }

    RETERR(-1)
  }
END
}


int recvclient() {
TRY
  /* receive loop */
  for (;;) {
    if (client.recvbuf.nbytes < sizeof(uint8_t)) FAIL(EAGAIN)

    switch (*(uint8_t *)client.recvbuf.ptr) {
    case SRPAUSE:
      if (recvsrpause()) FAIL(errno)
      break;

    case SRSENDMESG:
      if (recvsrsendmesg()) FAIL(errno)
      break;

    case SRDAMAGE:
      if (recvsrdamage()) FAIL(errno)
      break;

    case SRGRABTREES:
      if (recvsrgrabtrees()) FAIL(errno)
      break;

    case SRBUILD:
      if (recvsrbuild()) FAIL(errno)
      break;

    case SRGROW:
      if (recvsrgrow()) FAIL(errno)
      break;

    case SRFLOOD:
      if (recvsrflood()) FAIL(errno)
      break;

    case SRPLACEMINE:
      if (recvsrplacemine()) FAIL(errno)
      break;

    case SRDROPMINE:
      if (recvsrdropmine()) FAIL(errno)
      break;

    case SRDROPBOAT:
      if (recvsrdropboat()) FAIL(errno)
      break;

    case SRPLAYERJOIN:
      if (recvsrplayerjoin()) FAIL(errno)
      break;

    case SRPLAYERREJOIN:
      if (recvsrplayerrejoin()) FAIL(errno)
      break;

    case SRPLAYEREXIT:
      if (recvsrplayerexit()) FAIL(errno)
      if (!client.players[client.player].connected) SUCCESS
      break;

    case SRPLAYERDISC:
      if (recvsrplayerdisc()) FAIL(errno)
      if (!client.players[client.player].connected) SUCCESS
      break;

    case SRPLAYERKICK:
      if (recvsrplayerkick()) FAIL(errno)
      if (!client.players[client.player].connected) SUCCESS
      break;

    case SRPLAYERBAN:
      if (recvsrplayerban()) FAIL(errno)
      if (!client.players[client.player].connected) SUCCESS
      break;

    case SRREPAIRPILL:
      if (recvsrrepairpill()) FAIL(errno)
      break;

    case SRCOOLPILL:
      if (recvsrcoolpill()) FAIL(errno)
      break;

    case SRCAPTUREPILL:
      if (recvsrcapturepill()) FAIL(errno)
      break;

    case SRBUILDPILL:
      if (recvsrbuildpill()) FAIL(errno)
      break;

    case SRDROPPILL:
      if (recvsrdroppill()) FAIL(errno)
      break;

    case SRREPLENISHBASE:
      if (recvsrreplenishbase()) FAIL(errno)
      break;

    case SRCAPTUREBASE:
      if (recvsrcapturebase()) FAIL(errno)
      break;

    case SRREFUEL:
      if (recvsrrefuel()) FAIL(errno)
      break;

    case SRGRABBOAT:
      if (recvsrgrabboat()) FAIL(errno)
      break;

    case SRMINEACK:
      if (recvsrmineack()) FAIL(errno)
      break;

    case SRBUILDERACK:
      if (recvsrbuilderack()) FAIL(errno)
      break;

    case SRSMALLBOOM:
      if (recvsrsmallboom()) FAIL(errno)
      break;

    case SRSUPERBOOM:
      if (recvsrsuperboom()) FAIL(errno)
      break;

    case SRHITTANK:
      if (recvsrhittank()) FAIL(errno)
      break;

    case SRSETALLIANCE:
      if (recvsrsetalliance()) FAIL(errno)
      break;

    case SRTIMELIMIT:
      if (recvsrtimelimit()) FAIL(errno)
      break;

    case SRBASECONTROL:
      if (recvsrbasecontrol()) FAIL(errno)
      break;

    default:
      LOGFAIL(EINVAL)
      break;
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int selectclient(fd_set *readfds, fd_set *writefds, struct timeval *timeout) {
  int nfds;

TRY
  nfds = 0;
  FD_ZERO(readfds);
  FD_ZERO(writefds);
  
  /* read on mainpipe */
  FD_SET(client.mainpipe[0], readfds);
  nfds = MAX(nfds, client.mainpipe[0]);
  
  /* read on dgram sock */
  FD_SET(client.dgramsock, readfds);
  nfds = MAX(nfds, client.dgramsock);
  
  /* read/write cntlsock */
  FD_SET(client.cntlsock, readfds);
  nfds = MAX(nfds, client.cntlsock);
      
  if (client.sendbuf.nbytes > 0) {
    FD_SET(client.cntlsock, writefds);
  }

  if ((nfds = select(nfds + 1, readfds, writefds, NULL, timeout)) == -1) {
    if (errno != EINTR) {
      LOGFAIL(errno)
    }

    nfds = 0;
  }
  
CLEANUP
ERRHANDLER(nfds, -1)
END
}


void *clientmainthread(void *arg) {
  int i;
  uint64_t currenttime;
  uint64_t nexttick;
  int gotlock = 0;

TRY
  if ((i = joinclient()) == -1) {
    LOGFAIL(errno)
  }
  else if (i == 1) {
    SUCCESS
  }

  /* set TCP_NODELAY on socket */
  i = 1;
  if (setsockopt(client.cntlsock, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i))) LOGFAIL(errno)

  /* initialize time */
  nexttick = getcurrenttime();

  /* main loop */
  for (;;) {
    if ((currenttime = getcurrenttime()) < nexttick) {
      int nfds;
      fd_set readfds;
      fd_set writefds;
      struct timeval timeout;

      timeout.tv_sec = 0;
      timeout.tv_usec = (suseconds_t)((nexttick - currenttime)/1000);

      if ((nfds = selectclient(&readfds, &writefds, &timeout)) == -1) LOGFAIL(errno)

      /* time to quit */
      if (FD_ISSET(client.mainpipe[0], &readfds)) {
        break;
      }

      /* we have i/o */
      if (nfds > 0) {
        if (FD_ISSET(client.dgramsock, &readfds)) {
          /* process dgram packets */
          if (lockclient()) LOGFAIL(errno)
          gotlock = 1;

          if (dgramclient()) LOGFAIL(errno)

          if (unlockclient()) LOGFAIL(errno)
          gotlock = 0;
        }

        if (FD_ISSET(client.cntlsock, &readfds)) {
          ssize_t r;

          if ((r = recvbuf(&client.recvbuf, client.cntlsock)) == -1) {
            if (errno == ECONNRESET) {
              CLEARERRLOG
              /* process remaining buf and disconnect */
              if (lockclient()) LOGFAIL(errno)
              gotlock = 1;

              if (recvclient()) {
                if (errno != EAGAIN) {
                  LOGFAIL(errno)
                }
                else {
                  CLEARERRLOG
                }
              }
              if (clientprintmessage(MSGGAME, "disconnected")) LOGFAIL(errno)

              break;
            }
            else {
              LOGFAIL(errno)
            }
          }
          else if (r == 0) {  /* socket closed. */
            /* process remaining buf and disconnect */
            if (lockclient()) LOGFAIL(errno)
            gotlock = 1;

            if (recvclient()) {
              if (errno != EAGAIN) {
                LOGFAIL(errno)
              }
              else {
                CLEARERRLOG
              }
            }
            if (clientprintmessage(MSGGAME, "disconnected")) LOGFAIL(errno)

            break;
          }
          else {
            /* process buf */
            if (lockclient()) LOGFAIL(errno)
            gotlock = 1;

            if (recvclient()) {  /* returns errno of EAGAIN until exit, disconnected, kick or ban received for this player */
              if (errno != EAGAIN) {
                LOGFAIL(errno)
              }
              else {
                CLEARERRLOG
              }
            }
            else {  /* player has exited, disconnected, kicked or banned */
              if (clientprintmessage(MSGGAME, "disconnected")) LOGFAIL(errno)
              break;
            }

            if (unlockclient()) LOGFAIL(errno)
            gotlock = 0;
          }
        }

        if (FD_ISSET(client.cntlsock, &writefds)) {
          ssize_t r;
          if ((r = sendbuf(&client.sendbuf, client.cntlsock)) == -1) LOGFAIL(errno)
        }
      }
    }
    else {
      if (lockclient()) LOGFAIL(errno)
      gotlock = 1;

      if (runclient()) LOGFAIL(errno)

      if (unlockclient()) LOGFAIL(errno)
      gotlock = 0;

      currenttime = getcurrenttime();
      nexttick = nexttick + (1000000000ull/TICKSPERSEC);

      if (currenttime >= nexttick) {
        nexttick = currenttime + (1000000000ull/TICKSPERSEC);
      }
    }
  }

CLEANUP
  if (!gotlock) {
    lockclient();
  }

  cleanupclient();
  unlockclient();

  switch (ERROR) {
  case 0:
    break;

  case EHOSTNOTFOUND:
    if (client.joinprogress) {
      client.joinprogress(kJoinEHOSTNOTFOUND, 0.0);
    }

    break;

  case EHOSTNORECOVERY:
    if (client.joinprogress) {
      client.joinprogress(kJoinEHOSTNORECOVERY, 0.0);
    }

    break;

  case EHOSTNODATA:
    if (client.joinprogress) {
      client.joinprogress(kJoinEHOSTNODATA, 0.0);
    }

    break;

  case ETIMEDOUT:
    if (client.joinprogress) {
      client.joinprogress(kJoinETIMEOUT, 0.0);
    }

    break;

  case ECONNREFUSED:
    if (client.joinprogress) {
      client.joinprogress(kJoinECONNREFUSED, 0.0);
    }

    break;

  case ENETUNREACH:
    if (client.joinprogress) {
      client.joinprogress(kJoinENETUNREACH, 0.0);
    }

    break;

  case EHOSTUNREACH:
    if (client.joinprogress) {
      client.joinprogress(kJoinEHOSTUNREACH, 0.0);
    }

    break;

  case EBADVERSION:
    if (client.joinprogress) {
      client.joinprogress(kJoinEBADVERSION, 0.0);
    }

    break;

  case EDISSALLOW:
    if (client.joinprogress) {
      client.joinprogress(kJoinEDISALLOW, 0.0);
    }

    break;

  case EBADPASS:
    if (client.joinprogress) {
      client.joinprogress(kJoinEBADPASS, 0.0);
    }

    break;

  case ESERVERFULL:
    if (client.joinprogress) {
      client.joinprogress(kJoinESERVERFULL, 0.0);
    }

    break;

  case ETIMELIMIT:
    if (client.joinprogress) {
      client.joinprogress(kJoinETIMELIMIT, 0.0);
    }

    break;

  case EBANNEDPLAYER:
    if (client.joinprogress) {
      client.joinprogress(kJoinEBANNEDPLAYER, 0.0);
    }

    break;

  case ESERVERERROR:
    if (client.joinprogress) {
      client.joinprogress(kJoinESERVERERROR, 0.0);
    }

    break;

  case ECONNRESET:
    if (client.joinprogress) {
      client.joinprogress(kJoinECONNRESET, 0.0);
    }

    break;

  default:
    PCRIT(ERROR)
    printlineinfo();
    CLEARERRLOG
    exit(EXIT_FAILURE);
    break;
  }

  /* close thread pipe */
  closesock(client.mainpipe);
  closesock(client.threadpipe + 1);
  CLEARERRLOG
  pthread_exit(NULL);
END
}


int clientprintmessage(int type, const char *body) {
  char *text = NULL;

TRY
  if (client.printmessage) {
    if (asprintf(&text, "%s", body) == -1) LOGFAIL(errno)
    client.printmessage(type, text);
    free(text);
    text = NULL;
  }

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int dgramclient() {
  struct CLUpdate clupdate;
  struct CLUpdateShell *clupdateshells;
  struct CLUpdateExplosion *clupdateexplosions;
  ssize_t r;
  int i;
  int oldseq;
  Pointi oldsqr, newsqr;

TRY
  for (;;) {
    if ((r = recv(client.dgramsock, &clupdate, sizeof(clupdate), O_NONBLOCK)) == -1) {
      if (errno == EAGAIN) {
        break;
      }
      else if (errno == EINTR) {
        continue;
      }
      else {
        LOGFAIL(errno)
      }
    }

    if (  /* sanity check psize */
      r < sizeof(clupdate.hdr) ||
      r != sizeof(clupdate.hdr) + clupdate.hdr.nshells*sizeof(struct CLUpdateShell) + clupdate.hdr.nexplosions*sizeof(struct CLUpdateExplosion) ||
      clupdate.hdr.player >= MAXPLAYERS ||
      clupdate.hdr.player == client.player
    ) {
      continue;
    }

    /* network to host byte order */
    for (i = 0; i < MAXPLAYERS; i++) {
      clupdate.hdr.seq[i] = ntohl(clupdate.hdr.seq[i]);
    }

    clupdate.hdr.tankx = ntohl(clupdate.hdr.tankx);
    clupdate.hdr.tanky = ntohl(clupdate.hdr.tanky);
    clupdate.hdr.tankspeed = ntohl(clupdate.hdr.tankspeed);
    clupdate.hdr.tankturnspeed = ntohl(clupdate.hdr.tankturnspeed);
    clupdate.hdr.tankkickdir = ntohl(clupdate.hdr.tankkickdir);
    clupdate.hdr.tankkickspeed = ntohl(clupdate.hdr.tankkickspeed);
    clupdate.hdr.builderx = ntohl(clupdate.hdr.builderx);
    clupdate.hdr.buildery = ntohl(clupdate.hdr.buildery);
    clupdate.hdr.inputflags = ntohl(clupdate.hdr.inputflags);

    clupdateshells = (void *)clupdate.buf;
    clupdateexplosions = (void *)(clupdateshells + clupdate.hdr.nshells);

    /* verify this is a valid player */
    if (client.players[clupdate.hdr.player].connected) {
      /* make sure this is not an old update */
      if ((clupdate.hdr.seq[clupdate.hdr.player] - client.players[clupdate.hdr.player].seq) > 0) {
        struct ListNode *node;

//        if (client.srvaddr.sin_addr.s_addr != addr.sin_port) {
//          client.srvaddr.sin_port = addr.sin_port;
//        }

        if (client.setplayerstatus && client.players[client.player].seq - client.players[clupdate.hdr.player].lastupdate >= TICKSPERSEC) {
          client.setplayerstatus(clupdate.hdr.player);
        }

        oldseq = client.players[clupdate.hdr.player].seq;
        oldsqr = makepoint(client.players[clupdate.hdr.player].tank.x, client.players[clupdate.hdr.player].tank.y);

        client.players[clupdate.hdr.player].lastupdate = client.players[client.player].seq;
        client.players[clupdate.hdr.player].seq = clupdate.hdr.seq[clupdate.hdr.player];
        client.players[clupdate.hdr.player].dead = clupdate.hdr.tankstatus == kTankDead;
        client.players[clupdate.hdr.player].boat = clupdate.hdr.tankstatus == kTankOnBoat;
        client.players[clupdate.hdr.player].dir = clupdate.hdr.tankdir*((k2Pif)/FWIDTH);
        client.players[clupdate.hdr.player].tank.x = clampcoord(unpackfloat(&clupdate.hdr.tankx));
        client.players[clupdate.hdr.player].tank.y = clampcoord(unpackfloat(&clupdate.hdr.tanky));
        client.players[clupdate.hdr.player].speed = unpackfloat(&clupdate.hdr.tankspeed);
        client.players[clupdate.hdr.player].turnspeed = unpackfloat(&clupdate.hdr.tankturnspeed);
        client.players[clupdate.hdr.player].kickdir = unpackfloat(&clupdate.hdr.tankkickdir);
        client.players[clupdate.hdr.player].kickspeed = unpackfloat(&clupdate.hdr.tankkickspeed);
        client.players[clupdate.hdr.player].builderstatus = clupdate.hdr.builderstatus;
        client.players[clupdate.hdr.player].builder.x = clampcoord(unpackfloat(&clupdate.hdr.builderx));
        client.players[clupdate.hdr.player].builder.y = clampcoord(unpackfloat(&clupdate.hdr.buildery));
        client.players[clupdate.hdr.player].buildertarget.x = clupdate.hdr.buildertargetx;
        client.players[clupdate.hdr.player].buildertarget.y = clupdate.hdr.buildertargety;
        client.players[clupdate.hdr.player].builderwait = clupdate.hdr.builderwait;
        client.players[clupdate.hdr.player].inputflags = clupdate.hdr.inputflags;

        /* sounds that have played since last update */
        if (client.playsound) {
          if (clupdate.hdr.tankshotsound) {
            if (client.fog[(int)client.players[clupdate.hdr.player].tank.y][(int)client.players[clupdate.hdr.player].tank.x] > 0) {
              client.playsound(kTankShotSound);
            }
            else {
              client.playsound(kFarShotSound);
            }
          }

          if (clupdate.hdr.pillshotsound) {
            client.playsound(kFarShotSound);
          }

          if (clupdate.hdr.sinksound) {
            if (client.fog[(int)client.players[clupdate.hdr.player].tank.y][(int)client.players[clupdate.hdr.player].tank.x] > 0) {
              client.playsound(kSinkSound);
            }
            else {
              client.playsound(kFarSinkSound);
            }
          }

          if (clupdate.hdr.builderdeathsound) {
            if (client.fog[(int)client.players[clupdate.hdr.player].builder.y][(int)client.players[clupdate.hdr.player].builder.x] > 0) {
              client.playsound(kBuilderDeathSound);
            }
            else {
              client.playsound(kFarBuilderDeathSound);
            }
          }
        }
        if (client.printmessage && clupdate.hdr.builderdeathsound) {
            char *text;
            if (asprintf(&text, "%s just lost his builder", client.players[clupdate.hdr.player].name) == -1) LOGFAIL(errno)
                client.printmessage(MSGGAME, text);
            free(text);
        }


        /* build new shell list */
        clearlist(&client.players[clupdate.hdr.player].shells, free);
        node = &client.players[clupdate.hdr.player].shells;

        for (i = 0; i < clupdate.hdr.nshells; i++) {
          struct Shell *shell;

          if ((shell = (struct Shell *)malloc(sizeof(struct Shell))) == NULL) LOGFAIL(errno)
          shell->owner = clupdateshells[i].owner;
          shell->point.x = ntohs(clupdateshells[i].shellx)/FWIDTH;
          shell->point.y = ntohs(clupdateshells[i].shelly)/FWIDTH;
          shell->boat = clupdateshells[i].boat;
          shell->pill = clupdateshells[i].pill;
          shell->dir = clupdateshells[i].shelldir*((k2Pif)/FWIDTH);
          shell->range = ntohs(clupdateshells[i].range)/FWIDTH;
          if (addlist(node, shell)) LOGFAIL(errno)
          node = nextlist(node);
        }

        /* build new explosion list */
        clearlist(&client.players[clupdate.hdr.player].explosions, free);
        node = &client.players[clupdate.hdr.player].explosions;

        for (i = 0; i < clupdate.hdr.nexplosions; i++) {
          struct Explosion *explosion;

          if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
          explosion->point.x = ntohs(clupdateexplosions[i].explosionx)/FWIDTH;
          explosion->point.y = ntohs(clupdateexplosions[i].explosiony)/FWIDTH;
          explosion->counter = clupdateexplosions[i].counter;
          if (addlist(node, explosion)) LOGFAIL(errno)

          if (explosion->counter < 5) {
            killpointbuilder(explosion->point);
          }

          node = nextlist(node);
        }

        /* extrapolate the future position of the tank and builder to compensate for latency */
        if (clupdate.hdr.seq[client.player] != 0) {  /* make sure they have one update from us */
          int extrapolate = 0;

          /* seqs are unsigned; only extrapolate when we are ahead, and cap
             the count so a malicious/garbage seq cannot spin this loop
             (legitimate latency compensation is only a few ticks) */
          if (client.players[client.player].seq > clupdate.hdr.seq[client.player]) {
            extrapolate = (client.players[client.player].seq - clupdate.hdr.seq[client.player])/2;
          }

          if (extrapolate > TICKSPERSEC) {
            extrapolate = TICKSPERSEC;
          }

          for (i = 0; i < extrapolate; i++) {
            if (tankmovelogic(clupdate.hdr.player)) LOGFAIL(errno)
            if (builderlogic(clupdate.hdr.player)) LOGFAIL(errno)
            if (shelllogic(clupdate.hdr.player)) LOGFAIL(errno)
            if (explosionlogic(clupdate.hdr.player)) LOGFAIL(errno)
          }
        }

        newsqr = makepoint(client.players[clupdate.hdr.player].tank.x, client.players[clupdate.hdr.player].tank.y);

        if (!isequalpoint(newsqr, oldsqr) && testalliance(client.player, clupdate.hdr.player)) {
          if (increasevis(makerect(newsqr.x - 14, newsqr.y - 14, 29, 29))) LOGFAIL(errno)

          if (oldseq != 0) {
            if (decreasevis(makerect(oldsqr.x - 14, oldsqr.y - 14, 29, 29))) LOGFAIL(errno)
          }
        }
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}

