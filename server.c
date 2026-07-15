/*
 *  server.c
 *  XBolo
 *
 *  Created by Robert Chrzanowski on 4/27/09.
 *  Copyright 2009 Robert Chrzanowski. All rights reserved.
 *
 */

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
#include "server_internal.h"

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

struct Server server;

//static int sendplayerbufserver(int player);

/* pthread routines */


/* server receive routines */










int initserver() {
  int err;
  int i;

TRY
  bzero(&server, sizeof(server));

  server.setup = 0;
  server.running = 0;

  /* initialize tracker data */
  server.tracker.hostname = NULL;
  server.tracker.sock = -1;
  server.tracker.callback = NULL;
  if (initbuf(&server.tracker.recvbuf)) LOGFAIL(errno)
  if (initbuf(&server.tracker.sendbuf)) LOGFAIL(errno)

  server.listensock = -1;
  server.dgramsock = -1;
  server.mainpipe[0] = -1;
  server.mainpipe[1] = -1;
  server.threadpipe[0] = -1;
  server.threadpipe[1] = -1;

  /* initialize server mutex */
  if ((err = pthread_mutex_init(&server.mutex, NULL))) LOGFAIL(err)

  /* alloc and initialize the chain detonation */
  for (i = 0; i < (CHAINTICKS + 1); i++) {
    if (initlist(server.chains + i)) LOGFAIL(errno)
  }

  /* alloc and initialize the flood fill */
  for (i = 0; i < (FLOODTICKS + 1); i++) {
    if (initlist(server.floods + i)) LOGFAIL(errno)
  }

  /* alloc and initialize joining player buffers */
  if (initbuf(&server.joiningplayer.recvbuf)) LOGFAIL(errno)
  if (initbuf(&server.joiningplayer.sendbuf)) LOGFAIL(errno)

  /* alloc and initialize player buffers */
  for (i = 0; i < MAXPLAYERS; i++) {
    server.players[i].used = 0;
    server.players[i].cntlsock = -1;
    server.players[i].seq = 0;
    if (initbuf(&server.players[i].recvbuf)) LOGFAIL(errno)
    if (initbuf(&server.players[i].sendbuf)) LOGFAIL(errno)
  }

  /* alloc and initialize banned players list */
  if (initlist(&server.bannedplayers)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int setupserver(int initiallypaused, const void *buf, size_t nbytes, uint16_t port, const char password[], int timelimit, int hiddenmines, int pauseonplayerexit, int gametype, const void *gamedata) {
  int i, j;
  struct sockaddr_in addr;
  socklen_t addrlen;
  int flags;

  server.timelimit = timelimit;
  server.hiddenmines = hiddenmines;
  server.pauseonplayerexit = pauseonplayerexit;
  server.basecontrol = 0;
  server.gametype = gametype;
  server.game.domination.type = ((struct Domination *)gamedata)->type;
  server.game.domination.basecontrol = ((struct Domination *)gamedata)->basecontrol;
  server.allowjoin = 1;
  server.pause = initiallypaused ? -1 : 0;
  server.listensock = -1;
  server.dgramsock = -1;
  server.tracker.sock = -1;
  server.joiningplayer.cntlsock = -1;
  server.joiningplayer.disconnect = 0;

TRY
  /* initialized ticks */
  server.ticks = 0;

  /* load the map into server.terrain */
  if (serverloadmap(buf, nbytes)) LOGFAIL(errno)

  for (i = Y_MIN_MINE; i <= Y_MAX_MINE; i++) {
    for (j = X_MIN_MINE; j <= X_MAX_MINE; j++) {
      switch (server.terrain[i][j]) {
      case kCraterTerrain:
      case kMinedCraterTerrain:
        if (floodtest(j - 1, i)) LOGFAIL(errno)
        if (floodtest(j + 1, i)) LOGFAIL(errno)
        if (floodtest(j, i - 1)) LOGFAIL(errno)
        if (floodtest(j, i + 1)) LOGFAIL(errno)

      default:
        break;
      }
    }
  }

  /* initialize tree growing algorithm */
  server.growx = random()%WIDTH;
  server.growy = random()%WIDTH;
  server.growbestof = 0;

  if ((server.passreq = (password != NULL))) {
    strncpy(server.pass, password, MAXPASS - 1);
  }
  else {
    server.pass[0] = '\0';
  }

  /* initialize listensock */
  int one = 1;
  if ((server.listensock = socket(AF_INET, SOCK_STREAM, 0)) == -1) LOGFAIL(errno)
  if ((flags = fcntl(server.listensock, F_GETFL, 0)) == -1) LOGFAIL(errno)
  if (fcntl(server.listensock, F_SETFL, flags | O_NONBLOCK) == -1) LOGFAIL(errno)
  setsockopt(server.listensock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  /* initialize dgramsock */
  if ((server.dgramsock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) LOGFAIL(errno)
  if ((flags = fcntl(server.dgramsock, F_GETFL, 0)) == -1) LOGFAIL(errno)
  if (fcntl(server.dgramsock, F_SETFL, flags | O_NONBLOCK) == -1) LOGFAIL(errno)

  /* initialize name to INADDR_ANY port */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(addr.sin_zero, 8);

  /* bind listensock to name */
  while (bind(server.listensock, (void *)&addr, INET_ADDRSTRLEN)) {
    if (errno != EAGAIN) {
      LOGFAIL(errno)
    }

    usleep(10000);
  }

  addrlen = INET_ADDRSTRLEN;
  if (getsockname(server.listensock, (void *)&addr, &addrlen)) LOGFAIL(errno)

  /* bind dgramsock to name */
  while (bind(server.dgramsock, (void *)&addr, INET_ADDRSTRLEN)) {
    if (errno != EAGAIN) {
      LOGFAIL(errno)
    }

    usleep(10000);
  }

  /* begin listening */
  if (listen(server.listensock, 3)) LOGFAIL(errno)

  if (pipe(server.mainpipe)) LOGFAIL(errno)
  if (pipe(server.threadpipe)) LOGFAIL(errno)

  server.setup = 1;

CLEANUP
ERRHANDLER(0, -1)
END
}


int startserverthread() {
  pthread_t thread;
  int err;

TRY
  if (server.tracker.hostname) {
    free(server.tracker.hostname);
    server.tracker.hostname = NULL;
  }

  if ((err = pthread_create(&thread, NULL, servermainthread, NULL))) LOGFAIL(err)
  if ((err = pthread_detach(thread))) LOGFAIL(err)

  server.running = 1;

CLEANUP
ERRHANDLER(0, -1)
END
}

int startserverthreadwithtracker(
    const char trackerhostname[], uint16_t port, const char hostplayername[],
    const char mapname[], void (*callback)(int status)
  ) {
  pthread_t thread;
  int err;

TRY
  if ((server.tracker.hostname = (char *)malloc(strlen(trackerhostname) + 1)) == NULL) LOGFAIL(errno)
  strcpy(server.tracker.hostname, trackerhostname);
  server.tracker.port = port;
  strncpy(server.tracker.hostplayername, hostplayername, MAXNAME - 1);
  strncpy(server.tracker.mapname, mapname, TRKMAPNAMELEN - 1);
  server.tracker.callback = callback;

  if ((err = pthread_create(&thread, NULL, servermainthread, NULL))) LOGFAIL(err)
  if ((err = pthread_detach(thread))) LOGFAIL(err)

  server.running = 1;

CLEANUP
ERRHANDLER(0, -1)
END
}


int stopserver() {
  char buf[10];
  ssize_t r;

  assert(server.setup);

TRY
  /* close mainpipe */
  if (closesock(server.mainpipe + 1)) LOGFAIL(errno)

  if (!server.running) {
    if (startserverthread()) LOGFAIL(errno)
  }

  /* wait for threadpipe to be closed */
  for (;;) {
    if ((r = read(server.threadpipe[0], buf, sizeof(buf))) == -1) {
      if (errno != EINTR) LOGFAIL(errno)
    }
    else if (r == 0) {
      break;
    }
  }

  server.running = 0;

  if (closesock(server.threadpipe)) LOGFAIL(errno)

  server.setup = 0;

CLEANUP
ERRHANDLER(0, -1)
END
}


int getpauseserver() {
  return server.pause == -1;
}


void pauseserver() {
  if (!getpauseserver()) {
    server.pause = -1;
    sendsrpause(255);
  }
}


void resumeserver() {
  if (getpauseserver()) {
    server.pause = TICKSPERSEC*5;
    sendsrpause(server.pause/TICKSPERSEC);
  }
}


int pauseresumeserver() {
  int gotlock = 0;

TRY
  if (lockserver()) LOGFAIL(errno)
  gotlock = 1;

  if (getpauseserver()) {
    resumeserver();
  }
  else {
    pauseserver();
  }

  if (unlockserver()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (gotlock) {
      unlockserver();
    }

    RETERR(-1)
  }
END
}


int getallowjoinserver() {
  return server.allowjoin;
}


void setallowjoinserver(int allowjoin) {
  server.allowjoin = allowjoin;
}


int togglejoinserver() {
  int gotlock = 0;

TRY
  if (lockserver()) LOGFAIL(errno)
  gotlock = 1;

  server.allowjoin = !server.allowjoin;

  if (unlockserver()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (gotlock) {
      unlockserver();
    }

    RETERR(-1)
  }
END
}




int lockserver() {
  int err;

TRY
  if ((err = pthread_mutex_lock(&server.mutex))) LOGFAIL(err)

CLEANUP
ERRHANDLER(0, -1)
END
}


int unlockserver() {
  int err;

TRY
  if ((err = pthread_mutex_unlock(&server.mutex))) LOGFAIL(err)

CLEANUP
ERRHANDLER(0, -1)
END
}


int kickplayer(int player) {
  int gotlock = 0;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (lockserver()) LOGFAIL(errno)
  gotlock = 1;

  if (sendsrplayerkick(player)) LOGFAIL(errno)
  if (removeplayer(player)) LOGFAIL(errno)

  if (unlockserver()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (gotlock) {
      unlockserver();
    }

    RETERR(-1)
  }
END
}


int banplayer(int player) {
  int gotlock = 0;
  struct BannedPlayer *bannedplayer = NULL;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (lockserver()) LOGFAIL(errno)
  gotlock = 1;

  if (server.players[player].cntlsock != -1) {
    if ((bannedplayer = (struct BannedPlayer *)malloc(sizeof(struct BannedPlayer))) == NULL) LOGFAIL(errno)
    strncpy(bannedplayer->name, server.players[player].name, MAXNAME - 1);
    bannedplayer->sin_addr = server.players[player].addr.sin_addr;
    if (addlist(&server.bannedplayers, bannedplayer)) LOGFAIL(errno)
    bannedplayer = NULL;
    if (sendsrplayerban(player)) LOGFAIL(errno)
    if (removeplayer(player)) LOGFAIL(errno)
  }

  if (unlockserver()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (gotlock) {
      unlockserver();
    }

    if (bannedplayer) {
      free(bannedplayer);
    }

    RETERR(-1)
  }
END
}


int unbanplayer(int index) {
  struct ListNode *node;
  int i;

  int gotlock = 0;

TRY
  if (lockserver()) LOGFAIL(errno)
  gotlock = 1;

  for (node = nextlist(&server.bannedplayers), i = 0; node && (i < index); node = nextlist(node), i++);

  if (node != NULL) {
    removelist(node, free);
  }

  if (unlockserver()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (gotlock) {
      unlockserver();
    }

    RETERR(-1)
  }
END
}


int removeplayer(int player) {
  int i;
  uint16_t pills;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(server.players[player].cntlsock != -1);

TRY
  if (closesock(&server.players[player].cntlsock)) LOGFAIL(errno)
  server.players[player].seq = 0;
  if (readbuf(&server.players[player].recvbuf, NULL, server.players[player].recvbuf.nbytes) == -1) LOGFAIL(errno)
  if (readbuf(&server.players[player].sendbuf, NULL, server.players[player].sendbuf.nbytes) == -1) LOGFAIL(errno)

  pills = 0;

  for (i = 0; i < server.npills; i++) {
    if (server.pills[i].owner == player && server.pills[i].armour == ONBOARD) {
      pills |= 1 << i;
    }
  }

  /* drop pills */
  droppills(player, server.players[player].tank.x, server.players[player].tank.y, pills);

CLEANUP
ERRHANDLER(0, -1)
END
}


int dgramserver() {
  ssize_t r;
  struct CLUpdate clupdate;
  struct sockaddr_in addr;
  socklen_t addrlen;
  uint32_t seq;
  int i;

TRY
  for (;;) {
    addrlen = INET_ADDRSTRLEN;

    if ((r = recvfrom(server.dgramsock, &clupdate, sizeof(clupdate), O_NONBLOCK, (struct sockaddr *)&addr, &addrlen)) == -1) {
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

    /* test packet from tracker */
    if (
        r == sizeof(clupdate.hdr) &&
        clupdate.hdr.player == 255
      ) {

      /* send response */
      if (sendto(server.dgramsock, &clupdate, sizeof(clupdate.hdr), 0, (void *)&addr, addrlen) == -1) {
        if (errno != EAGAIN) LOGFAIL(errno)
      }

      continue;
    }
    else if (  /* sanity check the size */
        r < sizeof(clupdate.hdr) ||
        r != sizeof(clupdate.hdr) + clupdate.hdr.nshells*sizeof(struct CLUpdateShell) + clupdate.hdr.nexplosions*sizeof(struct CLUpdateExplosion) ||
        clupdate.hdr.player >= MAXPLAYERS
      ) {
      continue;
    }

    /* network to host byte order */
    seq = ntohl(clupdate.hdr.seq[clupdate.hdr.player]);

    /* verify this is a valid player */
    if (
        server.players[clupdate.hdr.player].used &&
        server.players[clupdate.hdr.player].cntlsock != -1 &&
        server.players[clupdate.hdr.player].dgramaddr.sin_family == addr.sin_family &&
        server.players[clupdate.hdr.player].dgramaddr.sin_addr.s_addr == addr.sin_addr.s_addr
      ) {
      /* make sure this is not an old update */
      if ((int32_t)(seq - server.players[clupdate.hdr.player].seq) > 0) {
        server.players[clupdate.hdr.player].seq = seq;
        server.players[clupdate.hdr.player].lastupdate = server.ticks;
        *(uint32_t *)&server.players[clupdate.hdr.player].tank.x = ntohl(clupdate.hdr.tankx);
        *(uint32_t *)&server.players[clupdate.hdr.player].tank.y = ntohl(clupdate.hdr.tanky);

        if (server.players[clupdate.hdr.player].dgramaddr.sin_port != addr.sin_port) {
          server.players[clupdate.hdr.player].dgramaddr.sin_port = addr.sin_port;
        }

        /* send update to all other players */
        for (i = 0; i < MAXPLAYERS; i++) {
          if (i != clupdate.hdr.player && server.players[i].cntlsock != -1) {
            if (sendto(server.dgramsock, &clupdate, r, 0, (void *)&server.players[i].dgramaddr, INET_ADDRSTRLEN) == -1) {
              if (errno != EAGAIN) LOGFAIL(errno)
            }
          }
        }
      }
    }
  }

  if (errno != EAGAIN) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int discjoiningplayerserver() {
TRY
  if (sendbuf(&server.joiningplayer.sendbuf, server.joiningplayer.cntlsock) == -1) LOGFAIL(errno)

  if (server.joiningplayer.sendbuf.nbytes == 0) {
    if (closesock(&server.joiningplayer.cntlsock)) LOGFAIL(errno)
    server.joiningplayer.disconnect = 0;
    if (readbuf(&server.joiningplayer.recvbuf, NULL, server.joiningplayer.recvbuf.nbytes) == -1) LOGFAIL(errno)
  }  

CLEANUP
ERRHANDLER(0, -1)
END
}


int joinplayerserver() {
  int i;
  int rejoin;
  int player;
  void *data = NULL;
  ssize_t len;
  uint8_t type;
  struct JOIN_Preamble *joinpreamble;
  struct BOLO_Preamble bolopreamble;

TRY
  if (server.joiningplayer.disconnect) {
    if (discjoiningplayerserver()) LOGFAIL(errno)
    SUCCESS
  }

  if (server.joiningplayer.recvbuf.nbytes < sizeof(struct JOIN_Preamble)) SUCCESS
  joinpreamble = (struct JOIN_Preamble *)server.joiningplayer.recvbuf.ptr;

  /* check client version */
  if (joinpreamble->version != NET_GAME_VERSION) {
    type = kBadVersionJOIN;
    if (writebuf(&server.joiningplayer.sendbuf, &type, sizeof(type)) == -1) LOGFAIL(errno)
    if (discjoiningplayerserver()) LOGFAIL(errno)
    SUCCESS
  }

  /* check password */
  if (server.passreq && strcmp(joinpreamble->pass, server.pass)) {
    type = kBadPasswordJOIN;
    if (writebuf(&server.joiningplayer.sendbuf, &type, sizeof(type)) == -1) LOGFAIL(errno)
    if (discjoiningplayerserver()) LOGFAIL(errno)
    SUCCESS
  }

  /* not allowing new players */
  if (!server.allowjoin) {
    type = kDisallowJOIN;
    if (writebuf(&server.joiningplayer.sendbuf, &type, sizeof(type)) == -1) LOGFAIL(errno)
    if (discjoiningplayerserver()) LOGFAIL(errno)
    SUCCESS
  }

  {  /* check if banned */
    struct ListNode *node;
    struct BannedPlayer *bannedplayer;

    node = nextlist(&server.bannedplayers);

    while (node != NULL) {
      bannedplayer = ptrlist(node);

      if (strncmp(bannedplayer->name, joinpreamble->name, MAXNAME) == 0 && bannedplayer->sin_addr.s_addr == server.joiningplayer.addr.sin_addr.s_addr) {
        type = kBannedPlayerJOIN;
        if (writebuf(&server.joiningplayer.sendbuf, &type, sizeof(type)) == -1) LOGFAIL(errno)
        if (discjoiningplayerserver()) LOGFAIL(errno)
        SUCCESS
      }

      node = nextlist(node);
    }
  }

  /* see if this ip has already joined once */
  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].used && server.players[i].cntlsock == -1 && !strncmp(server.players[i].name, joinpreamble->name, MAXNAME)) {
      break;
    }
  }

  /* not a rejoin */
  if (!(i < MAXPLAYERS)) {
    /* find an open slot that has never been used */
    for (i = 0; i < MAXPLAYERS; i++) {
      if (!server.players[i].used && server.players[i].cntlsock == -1) {
        break;
      }
    }

    /* all slots have been used once or are in use, see if we can find a slot that was used but player is not connected */
    if (!(i < MAXPLAYERS)) {
      int p, age;

      /* find first disconnected player */
      for (p = 0; p < MAXPLAYERS; p++) {
        if (server.players[p].cntlsock == -1) {
          i = p;
          age = server.ticks - server.players[p].lastupdate;

          /* looking for the oldest disconnected player */
          for (p++; p < MAXPLAYERS; p++) {
            if (server.players[p].cntlsock == -1) {
              if (age < server.ticks - server.players[p].lastupdate) {
                i = p;
                age = server.ticks - server.players[p].lastupdate;
              }
            }
          }

          break;
        }
      }
    }

    /* server is full */
    if (!(i < MAXPLAYERS)) {
      type = kServerFullJOIN;
      if (writebuf(&server.joiningplayer.sendbuf, &type, sizeof(type)) == -1) LOGFAIL(errno)
      if (discjoiningplayerserver()) LOGFAIL(errno)
      SUCCESS
    }

    server.players[i].alliance = 1 << i;
    strncpy(server.players[i].name, joinpreamble->name, sizeof(server.players[i].name) - 1);
    rejoin = 0;
  }
  else {
    rejoin = 1;
  }

  if (readbuf(&server.joiningplayer.recvbuf, NULL, sizeof(struct JOIN_Preamble)) == -1) LOGFAIL(errno)

  /* initialize player */
  player = i;

  freebuf(&server.players[player].recvbuf);

  server.players[player].used = 1;
  server.players[player].cntlsock = server.joiningplayer.cntlsock;
  server.players[player].addr = server.joiningplayer.addr;
  server.players[player].dgramaddr = server.joiningplayer.addr;
//  server.players[player].seq = 0;
  server.players[player].lastupdate = server.ticks;
  server.players[player].recvbuf = server.joiningplayer.recvbuf;

  server.joiningplayer.cntlsock = -1;
  if (initbuf(&server.joiningplayer.recvbuf)) LOGFAIL(errno)

  /* sending preamble */
  type = kSendingPreambleJOIN;
  if (writebuf(&server.players[player].sendbuf, &type, sizeof(type)) == -1) LOGFAIL(errno)

  /* init bolo preamble */
  bcopy(NET_GAME_IDENT, bolopreamble.ident, sizeof(NET_GAME_IDENT));
  bolopreamble.version = NET_GAME_VERSION;
  bolopreamble.player = player;
  bolopreamble.hiddenmines = server.hiddenmines;

  if (server.pause == -1) {
    bolopreamble.pause = 255;
  }
  else {
    bolopreamble.pause = server.pause/TICKSPERSEC;
  }

  bolopreamble.gametype = server.gametype;
  bolopreamble.game.domination.type = server.game.domination.type;
  bolopreamble.game.domination.basecontrol = server.game.domination.basecontrol;

  for (i = 0; i < MAXPLAYERS; i++) {
    bolopreamble.players[i].used = server.players[i].used;
    bolopreamble.players[i].connected = server.players[i].cntlsock != -1;
    bolopreamble.players[i].seq = htonl(server.players[i].seq);
    bzero(bolopreamble.players[i].name, MAXNAME);
    strncpy((char *)bolopreamble.players[i].name, server.players[i].name, MAXNAME - 1);
    bzero(bolopreamble.players[i].host, MAXHOST);
    strncpy((char *)bolopreamble.players[i].host, server.players[i].host, MAXHOST - 1);
    bolopreamble.players[i].alliance = htons(server.players[i].alliance);
  }

  /* encode map */
  if ((len = serversavemap(&data)) == -1) LOGFAIL(errno)
  bolopreamble.maplen = htonl(len);

  /* send game data */
  if (writebuf(&server.players[player].sendbuf, &bolopreamble, sizeof(bolopreamble)) == -1) LOGFAIL(errno)
  if (writebuf(&server.players[player].sendbuf, data, len) == -1) LOGFAIL(errno)
//  if (sendplayerbufserver(player)) LOGFAIL(errno)

  /* free map */
  free(data);
  data = NULL;

  /* set TCP_NODELAY on socket */
  i = 1;
  if (setsockopt(server.players[player].cntlsock, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i))) LOGFAIL(errno)

  /* player join */
  if (rejoin) {
    if (sendsrplayerrejoin(player)) LOGFAIL(errno)
  }
  else {
    if (sendsrplayerjoin(player)) LOGFAIL(errno)
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int selectserver(fd_set *readfds, fd_set *writefds, struct timeval *timeout) {
  int i;
  int nfds;
  int selecterrno;

TRY
  nfds = 0;
  FD_ZERO(readfds);
  FD_ZERO(writefds);
  
  /* read on main pipe */
  FD_SET(server.mainpipe[0], readfds);
  nfds = MAX(nfds, server.mainpipe[0]);
  
  /* read on dgram sock */
  FD_SET(server.dgramsock, readfds);
  nfds = MAX(nfds, server.dgramsock);
  
  /* read/write player control socks */
  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1) {
      FD_SET(server.players[i].cntlsock, readfds);
      nfds = MAX(nfds, server.players[i].cntlsock);
      
      if (server.players[i].sendbuf.nbytes > 0) {
        FD_SET(server.players[i].cntlsock, writefds);
      }
    }
  }
  
  /* read/write on joining player sock */
  if (server.joiningplayer.cntlsock != -1) {
    FD_SET(server.joiningplayer.cntlsock, readfds);
    nfds = MAX(nfds, server.joiningplayer.cntlsock);
    
    if (server.joiningplayer.sendbuf.nbytes > 0) {
      FD_SET(server.joiningplayer.cntlsock, writefds);
    }
  }
  
  /* accept on listen sock if we don't already have a joining player */
  if (server.joiningplayer.cntlsock == -1) {
    FD_SET(server.listensock, readfds);
    nfds = MAX(nfds, server.listensock);
  }
  
  /* write on tracker sock */
  if (server.tracker.sendbuf.nbytes > 0) {
    FD_SET(server.tracker.sock, writefds);
    nfds = MAX(nfds, server.tracker.sock);
  }

  /* drop the server lock while blocked in select() so other threads are not
     starved waiting for it; the watched fds are only closed by this thread */
  if (unlockserver()) LOGFAIL(errno)

  nfds = select(nfds + 1, readfds, writefds, NULL, timeout);
  selecterrno = errno;

  if (lockserver()) LOGFAIL(errno)

  if (nfds == -1) {
    if (selecterrno != EINTR) {
      LOGFAIL(selecterrno)
    }

    nfds = 0;
  }

CLEANUP
  switch(ERROR) {
  case 0:
    RETURN(nfds)
  default:
    RETERR(-1)
  }
END
}


int recvplayerserver(int player) {
  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  for (;;) {
    if (server.players[player].recvbuf.nbytes < sizeof(uint8_t)) FAIL(EAGAIN)

    switch (*(uint8_t *)server.players[player].recvbuf.ptr) {
    case CLSENDMESG:
      if (recvclsendmesg(player)) LOGFAIL(errno)
      break;

    case CLDROPBOAT:
      if (recvcldropboat(player)) LOGFAIL(errno)
      break;

    case CLDROPPILLS:
      if (recvcldroppills(player)) LOGFAIL(errno)
      break;

    case CLDROPMINE:
      if (recvcldropmine(player)) LOGFAIL(errno)
      break;

    case CLTOUCH:
      if (recvcltouch(player)) LOGFAIL(errno)
      break;

    case CLGRABTILE:
      if (recvclgrabtile(player)) LOGFAIL(errno)
      break;

    case CLGRABTREES:
      if (recvclgrabtrees(player)) LOGFAIL(errno)
      break;

    case CLBUILDROAD:
      if (recvclbuildroad(player)) LOGFAIL(errno)
      break;

    case CLBUILDWALL:
      if (recvclbuildwall(player)) LOGFAIL(errno)
      break;

    case CLBUILDBOAT:
      if (recvclbuildboat(player)) LOGFAIL(errno)
      break;

    case CLBUILDPILL:
      if (recvclbuildpill(player)) LOGFAIL(errno)
      break;

    case CLREPAIRPILL:
      if (recvclrepairpill(player)) LOGFAIL(errno)
      break;

    case CLPLACEMINE:
      if (recvclplacemine(player)) LOGFAIL(errno)
      break;

    case CLDAMAGE:
      if (recvcldamage(player)) LOGFAIL(errno)
      break;

    case CLSMALLBOOM:
      if (recvclsmallboom(player)) LOGFAIL(errno)
      break;

    case CLSUPERBOOM:
      if (recvclsuperboom(player)) LOGFAIL(errno)
      break;

    case CLREFUEL:
      if (recvclrefuel(player)) LOGFAIL(errno)
      break;

    case CLHITTANK:
      if (recvclhittank(player)) LOGFAIL(errno)
      break;

    case CLSETALLIANCE:
      if (recvclsetalliance(player)) LOGFAIL(errno)
      break;

    case kHangupClientMessage:
      SUCCESS
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


int runserver() {
  int i;
  int nplayers;

TRY
  /* pause logic */
  if (server.pause) {
    if (server.pause > 0) {
      server.pause--;

      if (server.pause%TICKSPERSEC == 0) {
        sendsrpause(server.pause/TICKSPERSEC);
      }
    }

    SUCCESS
  }

  /* time limited games */
  if (server.timelimit > 0) {
    if (server.ticks == TICKSPERSEC*(server.timelimit - 300)) {  /* five minute warning */
      sendsrtimelimit(300);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 60)) {  /* one minute warning */
      sendsrtimelimit(60);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 10)) {  /* ten second warning */
      sendsrtimelimit(10);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 5)) {  /* five second warning */
      sendsrtimelimit(5);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 4)) {  /* four second warning */
      sendsrtimelimit(4);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 3)) {  /* three second warning */
      sendsrtimelimit(3);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 2)) {  /* two second warning */
      sendsrtimelimit(2);
    }
    else if (server.ticks == TICKSPERSEC*(server.timelimit - 1)) {  /* one second warning */
      sendsrtimelimit(1);
    }
    else if (server.ticks == TICKSPERSEC*server.timelimit) {  /* time limit reached */
      sendsrtimelimit(0);
      server.ticks++;
      SUCCESS
    }
    else if (server.ticks > TICKSPERSEC*server.timelimit) {  /* time limit reached */
      SUCCESS
    }
  }

  /* game type logic */
  switch (server.gametype) {
  case kDominationGameType:
    if (server.nbases > 0 && server.bases[0].armour >= MINBASEARMOUR && server.bases[0].owner != NEUTRAL) {
      for (i = 1; i < server.nbases && server.bases[i].armour >= MINBASEARMOUR && ((server.players[server.bases[i].owner].alliance & (1 << server.bases[0].owner)) && (server.players[server.bases[0].owner].alliance & (1 << server.bases[i].owner))); i++) ;

      if (i == server.nbases) {
        server.basecontrol++;

        if (server.basecontrol == TICKSPERSEC*(server.game.domination.basecontrol - 10)) {  /* ten second warning */
          sendsrbasecontrol(10);
        }
        else if (server.basecontrol == TICKSPERSEC*(server.game.domination.basecontrol - 5)) {  /* five second warning */
          sendsrbasecontrol(5);
        }
        else if (server.basecontrol == TICKSPERSEC*(server.game.domination.basecontrol - 4)) {  /* four second warning */
          sendsrbasecontrol(4);
        }
        else if (server.basecontrol == TICKSPERSEC*(server.game.domination.basecontrol - 3)) {  /* three second warning */
          sendsrbasecontrol(3);
        }
        else if (server.basecontrol == TICKSPERSEC*(server.game.domination.basecontrol - 2)) {  /* two second warning */
          sendsrbasecontrol(2);
        }
        else if (server.basecontrol == TICKSPERSEC*(server.game.domination.basecontrol - 1)) {  /* one second warning */
          sendsrbasecontrol(1);
        }
        else if (server.basecontrol == TICKSPERSEC*server.game.domination.basecontrol) {  /* time limit reached */
          sendsrbasecontrol(0);
          server.ticks++;
          SUCCESS
        }
        else if (server.basecontrol > TICKSPERSEC*server.game.domination.basecontrol) {  /* time limit reached */
          SUCCESS
        }
      }
      else{
        server.basecontrol = 0;
      }
    }

    break;

  default:
    break;
  }

  server.ticks++;

  nplayers = 0;

  /* disconnect lagged players */
  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1) {
      if (server.ticks - server.players[i].lastupdate >= 9*TICKSPERSEC) {
        if (removeplayer(i)) LOGFAIL(errno)
        if (sendsrplayerdisc(i)) LOGFAIL(errno)

        if (server.pauseonplayerexit) {
          server.pause = -1;
          sendsrpause(255);
        }
      }
      else {
        nplayers++;
      }
    }
  }

  /* cool pills */
  for (i = 0; i < server.npills; i++) {
    if (server.pills[i].armour != ONBOARD) {
      server.pills[i].counter++;

      if (server.pills[i].counter >= COOLPILLTICKS) {
        if (server.pills[i].speed < MAXTICKSPERSHOT) {
          server.pills[i].speed++;
          sendsrcoolpill(i);
        }

        server.pills[i].counter = 0;
      }
    }
  }

  /* replenish bases */
  for (i = 0; i < server.nbases; i++) {
    server.bases[i].counter += nplayers;

    if (server.bases[i].counter >= REPLENISHBASETICKS) {
      if (++server.bases[i].armour > MAXBASEARMOUR) {
        server.bases[i].armour = MAXBASEARMOUR;
      }

      if (++server.bases[i].mines > MAXBASEMINES) {
        server.bases[i].mines = MAXBASEMINES;
      }

      if (++server.bases[i].shells > MAXBASESHELLS) {
        server.bases[i].shells = MAXBASESHELLS;
      }

      sendsrreplenishbase(i);

      server.bases[i].counter = 0;
    }
  }

  /* grow trees */
  growtrees(nplayers);

  /* chain detonation */
  if (chain()) LOGFAIL(errno)

  /* flood fill */
  if (flood()) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int registerserver() {
  int lookup = -1;
  int ret = 1;

TRY
  if (server.tracker.hostname) {
    struct TRACKER_Preamble preamble;
    struct TrackerHost trackerhost;
    int flags;
    int ret;
    uint8_t msg;

    if (server.tracker.callback) {
      server.tracker.callback(kRegisterRESOLVING);
    }

    /* lookup address */
    if ((lookup = nslookup(server.tracker.hostname)) == -1) LOGFAIL(errno)

    /* wait for exit or dns reply */
    if ((ret = selectreadread(server.mainpipe[0], lookup)) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      if (closesock(&lookup)) LOGFAIL(errno)
      SUCCESS
    }

    /* get nslookup result */
    if (nslookupresult(lookup, &server.tracker.addr.sin_addr)) LOGFAIL(errno)
    if (closesock(&lookup)) LOGFAIL(errno)

    server.tracker.addr.sin_family = AF_INET;
    server.tracker.addr.sin_port = htons(TRACKERPORT);
    bzero(server.tracker.addr.sin_zero, 8);

    if (server.tracker.callback) {
      server.tracker.callback(kRegisterCONNECTING);
    }

    /* initialize trackersock */
    if ((server.tracker.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) LOGFAIL(errno)
    if ((flags = fcntl(server.tracker.sock, F_GETFL, 0)) == -1) LOGFAIL(errno)
    if (fcntl(server.tracker.sock, F_SETFL, flags | O_NONBLOCK) == -1) LOGFAIL(errno)

    if ((connect(server.tracker.sock, (struct sockaddr *)&server.tracker.addr, INET_ADDRSTRLEN))) {
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

        FD_SET(server.mainpipe[0], &readfds);
        nfds = MAX(nfds, server.mainpipe[0]);

        FD_SET(server.tracker.sock, &writefds);
        nfds = MAX(nfds, server.tracker.sock);

        if ((nfds = select(nfds + 1, &readfds, &writefds, NULL, NULL)) == -1) {
          if (errno != EINTR) LOGFAIL(errno)
        }
        else {
          break;
        }
      }

      if (FD_ISSET(server.mainpipe[0], &readfds)) {
        SUCCESS
      }
      else if (FD_ISSET(server.tracker.sock, &writefds)) {
        if (connect(server.tracker.sock, (struct sockaddr *)&server.tracker.addr, INET_ADDRSTRLEN) && errno != EISCONN) LOGFAIL(errno)
        break;
      }
    }

    if ((ret = cntlsend(server.mainpipe[0], server.tracker.sock, &server.tracker.sendbuf)) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      SUCCESS
    }

    if (server.tracker.callback) {
      server.tracker.callback(kRegisterSENDINGDATA);
    }

    /* send preamble */
    bcopy(TRACKERIDENT, preamble.ident, sizeof(preamble.ident));
    preamble.version = TRACKERVERSION;
    if (writebuf(&server.tracker.sendbuf, &preamble, sizeof(preamble)) == -1) LOGFAIL(errno)

    if ((ret = cntlsend(server.mainpipe[0], server.tracker.sock, &server.tracker.sendbuf)) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      SUCCESS
    }

    if ((ret = cntlrecv(server.mainpipe[0], server.tracker.sock, &server.tracker.recvbuf, sizeof(uint8_t))) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      SUCCESS
    }

    if (readbuf(&server.tracker.recvbuf, &msg, sizeof(uint8_t)) == -1) LOGFAIL(errno)

    /* receive ok */
    if (msg != kTrackerVersionOK) LOGFAIL(EBADVERSION)

    /* send game data */
    msg = kTrackerHost;
    strncpy((char *)trackerhost.playername, server.tracker.hostplayername, TRKPLYRNAMELEN - 1);
    strncpy((char *)trackerhost.mapname, server.tracker.mapname, TRKMAPNAMELEN - 1);
    trackerhost.port = htons(server.tracker.port);
    trackerhost.timelimit = htonl(server.timelimit);
    trackerhost.passreq = server.passreq;
    trackerhost.nplayers = nplayers();
    trackerhost.allowjoin = server.allowjoin;
    trackerhost.pause = server.pause == -1;
    trackerhost.gametype = server.gametype;
    if (writebuf(&server.tracker.sendbuf, &msg, sizeof(msg)) == -1) LOGFAIL(errno)
    if (writebuf(&server.tracker.sendbuf, &trackerhost, sizeof(trackerhost)) == -1) LOGFAIL(errno)

    if ((ret = cntlsend(server.mainpipe[0], server.tracker.sock, &server.tracker.sendbuf)) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      SUCCESS
    }

    if (server.tracker.callback) {
      server.tracker.callback(kRegisterTESTINGTCP);
    }

    /* receive tcp ok */
    if ((ret = cntlrecv(server.mainpipe[0], server.tracker.sock, &server.tracker.recvbuf, sizeof(msg))) == -1) {
      LOGFAIL(errno)
    }
    else if (ret == 1) {
      SUCCESS
    }

    if (readbuf(&server.tracker.recvbuf, &msg, sizeof(uint8_t)) == -1) LOGFAIL(errno)
    if (msg != kTrackerTCPPortOK) LOGFAIL(ETCPCLOSED)

    if (server.tracker.callback) {
      server.tracker.callback(kRegisterTESTINGUDP);
    }

    /* receive udp ok */
    for (;;) {
      int nfds;
      fd_set readfds;

      /* select until we have success */
      for (;;) {
        nfds = 0;
        FD_ZERO(&readfds);
        FD_SET(server.mainpipe[0], &readfds);
        nfds = MAX(nfds, server.mainpipe[0]);
        FD_SET(server.dgramsock, &readfds);
        nfds = MAX(nfds, server.dgramsock);
        FD_SET(server.tracker.sock, &readfds);
        nfds = MAX(nfds, server.tracker.sock);

        if ((nfds = select(nfds + 1, &readfds, NULL, NULL, NULL)) == -1) {
          if (errno != EINTR) LOGFAIL(errno)
        }
        else {
          break;
        }
      }

      if (FD_ISSET(server.mainpipe[0], &readfds)) {
        SUCCESS
      }
      else if (FD_ISSET(server.dgramsock, &readfds)) {
        struct sockaddr_in addr;
        socklen_t addrlen;
        struct CLUpdate clupdate;
        ssize_t r;

        for (;;) {
          addrlen = INET_ADDRSTRLEN;

          if ((r = recvfrom(server.dgramsock, &clupdate, sizeof(clupdate), O_NONBLOCK, (void *)&addr, &addrlen)) == -1) {
            if (errno == EAGAIN) {
              break;
            }
            else if (errno != EINTR) {
              LOGFAIL(errno)
            }
            else {
              continue;
            }
          }

          /* test packet from tracker */
          if (
              r == sizeof(clupdate.hdr) &&
              clupdate.hdr.player == 255
            ) {

            bzero(&clupdate, sizeof(clupdate));
            clupdate.hdr.player = 255;

            /* send response */
            if (sendto(server.dgramsock, &clupdate, sizeof(clupdate.hdr), 0, (void *)&addr, addrlen) == -1) {
              if (errno != EAGAIN) LOGFAIL(errno)
            }
          }
        }
      }
      else if (FD_ISSET(server.tracker.sock, &readfds)) {
        if (recvbuf(&server.tracker.recvbuf, server.tracker.sock) == -1) LOGFAIL(errno)

        if (server.tracker.recvbuf.nbytes >= sizeof(msg)) {
          break;
        }
      }
    }

    if (readbuf(&server.tracker.recvbuf, &msg, sizeof(uint8_t)) == -1) LOGFAIL(errno)
    if (msg != kTrackerUDPPortOK) LOGFAIL(EUDPCLOSED)

    /* successfully registered */
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterSUCCESS);
    }
  }

  ret = 0;

CLEANUP
  if (lookup != -1) {
    closesock(&lookup);
  }

ERRHANDLER(ret, -1)
END
}


int cleanupserver() {
  int i;

TRY
  if (closesock(&server.listensock)) LOGFAIL(errno)

  if (server.tracker.hostname) {
    if (server.tracker.sock != -1) {
      if (closesock(&server.tracker.sock)) LOGFAIL(errno)
    }

    free(server.tracker.hostname);
    server.tracker.hostname = NULL;
  }

  if (closesock(&server.dgramsock)) LOGFAIL(errno)

  /* close joining player sock and clear buffers */
  if (server.joiningplayer.cntlsock != -1) {
    if (closesock(&server.joiningplayer.cntlsock)) LOGFAIL(errno)
  }

  server.joiningplayer.disconnect = 0;
  if (readbuf(&server.joiningplayer.recvbuf, NULL, server.joiningplayer.recvbuf.nbytes) == -1) LOGFAIL(errno)
  if (readbuf(&server.joiningplayer.sendbuf, NULL, server.joiningplayer.sendbuf.nbytes) == -1) LOGFAIL(errno)

  /* close player socks and clear buffers */
  for (i = 0; i < MAXPLAYERS; i++) {
    server.players[i].used = 0;
    server.players[i].seq = 0;

    bzero(server.players[i].name, MAXNAME);

    if (server.players[i].cntlsock != -1) {
      if (closesock(&server.players[i].cntlsock)) LOGFAIL(errno)
    }

    if (readbuf(&server.players[i].recvbuf, NULL, server.players[i].recvbuf.nbytes) == -1) LOGFAIL(errno)
    if (readbuf(&server.players[i].sendbuf, NULL, server.players[i].sendbuf.nbytes) == -1) LOGFAIL(errno)
  }

  /* cleanup chain detonation */
  for (i = 0; i < (CHAINTICKS + 1); i++) {
    clearlist(server.chains + i, free);
  }

  /* flood fill detonation */
  for (i = 0; i < (FLOODTICKS + 1); i++) {
    clearlist(server.floods + i, free);
  }

  clearlist(&server.bannedplayers, free);

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendtrackerupdate() {
  struct TrackerHost trackerhost;

TRY
  strncpy((char *)trackerhost.playername, server.tracker.hostplayername, TRKPLYRNAMELEN - 1);
  strncpy((char *)trackerhost.mapname, server.tracker.mapname, TRKMAPNAMELEN - 1);
  trackerhost.port = htons(server.tracker.port);
  trackerhost.gametype = server.gametype;
  trackerhost.timelimit = server.timelimit;
  trackerhost.passreq = server.passreq;
  trackerhost.nplayers = nplayers();
  trackerhost.allowjoin = getallowjoinserver();
  trackerhost.pause = getpauseserver();
  if (writebuf(&server.tracker.sendbuf, &trackerhost, sizeof(trackerhost)) == -1) LOGFAIL(errno)
  if (sendbuf(&server.tracker.sendbuf, server.tracker.sock) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


void *servermainthread(void *arg) {
  int i;
  uint64_t currenttime;
  uint64_t nexttick;
  uint64_t nexttrackerupdate;
  int gotlock = 0;
  
TRY

  if (lockserver()) LOGFAIL(errno)
  gotlock = 1;

  /* register with tracker */
  if ((i = registerserver()) == -1) {
    LOGFAIL(errno)
  }
  else if (i == 1) {  /* closed by main thread */
    SUCCESS
  }

  if (unlockserver()) LOGFAIL(errno)
  gotlock = 0;

  /* initialize times */
  nexttick = getcurrenttime();
  nexttrackerupdate = nexttick + TRACKERUPDATESECONDS*1000000000ull;

  /* main loop */
  for (;;) {
    currenttime = getcurrenttime();

    /* send update to tracker */
    if (server.tracker.hostname && server.tracker.sendbuf.nbytes == 0 && currenttime >= nexttrackerupdate) {
      if (sendtrackerupdate() == -1) LOGFAIL(errno)

      nexttrackerupdate = nexttrackerupdate + (TRACKERUPDATESECONDS*1000000000ull);

      if ((currenttime = getcurrenttime()) >= nexttrackerupdate) {
        nexttrackerupdate = currenttime + (TRACKERUPDATESECONDS*1000000000ull);
      }
    }

    if ((currenttime = getcurrenttime()) < nexttick) {
      int nfds;
      fd_set readfds;
      fd_set writefds;
      struct timeval timeout;

      timeout.tv_sec = 0;
      timeout.tv_usec = (nexttick - currenttime)/1000;

      if (lockserver()) LOGFAIL(errno)
      gotlock = 1;

      if ((nfds = selectserver(&readfds, &writefds, &timeout)) == -1) LOGFAIL(errno)

      /* time to quit */
      if (FD_ISSET(server.mainpipe[0], &readfds)) {
        for (i = 0; i < MAXPLAYERS; i++) {
          if (server.players[i].cntlsock != -1) {
            if (removeplayer(i)) LOGFAIL(errno)
          }
        }

        break;
      }

      /* we have i/o */
      if (nfds > 0) {
        /* read dgram sock */
        if (FD_ISSET(server.dgramsock, &readfds)) {
          if (dgramserver()) LOGFAIL(errno)
        }

        /* read player socks */
        for (i = 0; i < MAXPLAYERS; i++) {
          if (server.players[i].cntlsock != -1) {
            if (FD_ISSET(server.players[i].cntlsock, &readfds)) {
              ssize_t r;

              if ((r = recvbuf(&server.players[i].recvbuf, server.players[i].cntlsock)) == -1) {
                if (errno == ECONNRESET) {
                  CLEARERRLOG

                  /* parse received data */
                  if (recvplayerserver(i) == -1) {
                    /* player exited abnomrally */
                    if (removeplayer(i)) LOGFAIL(errno)
                    if (sendsrplayerdisc(i)) LOGFAIL(errno)
                  }
                  else {
                    /* player exited normally */
                    if (removeplayer(i)) LOGFAIL(errno)
                    if (sendsrplayerexit(i)) LOGFAIL(errno)
                  }

                  if (server.pauseonplayerexit) {
                    server.pause = -1;
                    sendsrpause(255);
                  }
                }
                else {
                  LOGFAIL(errno)
                }
              }
              else if (r == 0) {  /* socket closed, disconnect player */
                /* parse received data */
                if (recvplayerserver(i) == -1) {
                  /* player exited abnomrally */
                  if (removeplayer(i)) LOGFAIL(errno)
                  if (sendsrplayerdisc(i)) LOGFAIL(errno)
                }
                else {
                  /* player exited normally */
                  if (removeplayer(i)) LOGFAIL(errno)
                  if (sendsrplayerexit(i)) LOGFAIL(errno)
                }

                if (server.pauseonplayerexit) {
                  server.pause = -1;
                  sendsrpause(255);
                }
              }
              else {
                /* parse data */
                if (recvplayerserver(i) == -1) {
                  /* error occured */
                  if (errno != EAGAIN) {  /* a unrecoverable error occured */
                    if (removeplayer(i)) LOGFAIL(errno)
                    if (sendsrplayerdisc(i)) LOGFAIL(errno)

                    if (server.pauseonplayerexit) {
                      server.pause = -1;
                      sendsrpause(255);
                    }

                    CLEARERRLOG
                  }
                }
                else {
                  /* player exited normally */
                  if (removeplayer(i)) LOGFAIL(errno)
                  if (sendsrplayerexit(i)) LOGFAIL(errno)

                  if (server.pauseonplayerexit) {
                    server.pause = -1;
                    sendsrpause(255);
                  }
                }
              }
            }
          }
        }

        /* write player socks */
        for (i = 0; i < MAXPLAYERS; i++) {
          if (server.players[i].cntlsock != -1 && server.players[i].sendbuf.nbytes > 0) {
            if (sendbuf(&server.players[i].sendbuf, server.players[i].cntlsock) == -1) {
              if (errno != EPIPE) {  /* receving data code will disconnect player next time through the loop */
                LOGFAIL(errno)
              }
            }
          }
        }

        /* read joining player sock */
        if (server.joiningplayer.cntlsock != -1 && FD_ISSET(server.joiningplayer.cntlsock, &readfds)) {
          ssize_t r;

          if ((r = recvbuf(&server.joiningplayer.recvbuf, server.joiningplayer.cntlsock)) == -1) {
            if (errno == ECONNRESET) {
              CLEARERRLOG
              if (closesock(&server.joiningplayer.cntlsock)) LOGFAIL(errno)
            }
            else {
              LOGFAIL(errno)
            }
          }
          else if (r == 0) {
            if (closesock(&server.joiningplayer.cntlsock)) LOGFAIL(errno)
          }
          else {
            if (joinplayerserver()) LOGFAIL(errno)
          }
        }

        /* write joining player sock */
        if (server.joiningplayer.cntlsock != -1 && FD_ISSET(server.joiningplayer.cntlsock, &writefds)) {
          if (sendbuf(&server.joiningplayer.sendbuf, server.joiningplayer.cntlsock) == -1) LOGFAIL(errno)
          if (joinplayerserver()) LOGFAIL(errno)
        }

        /* accept new connections */
        if (FD_ISSET(server.listensock, &readfds)) {
          socklen_t addrlen;
          addrlen = INET_ADDRSTRLEN;
          if ((server.joiningplayer.cntlsock = accept(server.listensock, (void *)&server.joiningplayer.addr, &addrlen)) == -1) LOGFAIL(errno)
        }

        /* send buf to tracker sock */
        if (server.tracker.sock != -1 && FD_ISSET(server.tracker.sock, &writefds)) {
          if (sendbuf(&server.tracker.sendbuf, server.tracker.sock) == -1) LOGFAIL(errno)
        }
      }

      if (unlockserver()) LOGFAIL(errno)
      gotlock = 0;
    }
    else {
      if (lockserver()) LOGFAIL(errno)
      gotlock = 1;

      if (runserver()) LOGFAIL(errno)

      if (unlockserver()) LOGFAIL(errno)
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
    lockserver();
  }

  cleanupserver();
  unlockserver();

  switch (ERROR) {
  case 0:
    break;

  case EHOSTNOTFOUND:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEHOSTNOTFOUND);
    }

    break;

  case EHOSTNORECOVERY:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEHOSTNORECOVERY);
    }

    break;

  case EHOSTNODATA:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEHOSTNODATA);
    }

    break;

  case ETIMEDOUT:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterETIMEOUT);
    }

    break;

  case EBADVERSION:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEBADVERSION);
    }

    break;

  case ECONNREFUSED:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterECONNREFUSED);
    }

    break;

  case EHOSTDOWN:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEHOSTDOWN);
    }

    break;

  case EHOSTUNREACH:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEHOSTUNREACH);
    }

    break;

  case ETCPCLOSED:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterETCPCLOSED);
    }

    break;

  case EUDPCLOSED:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterEUDPCLOSED);
    }

    break;

  case ECONNRESET:
    if (server.tracker.callback) {
      server.tracker.callback(kRegisterECONNRESET);
    }

    break;

  default:
    PCRIT(ERROR)
    printlineinfo();
    break;
  }

  /* close thread pipe */
  closesock(server.mainpipe);
  closesock(server.threadpipe + 1);
  CLEARERRLOG
  pthread_exit(NULL);
END
}


int getservertcpport() {
  struct sockaddr_in addr;
  socklen_t addrlen;

  addrlen = INET_ADDRSTRLEN;
  getsockname(server.listensock, (void *)&addr, &addrlen);
  return ntohs(addr.sin_port);
}


int getserverudpport() {
  struct sockaddr_in addr;
  socklen_t addrlen;

  addrlen = INET_ADDRSTRLEN;
  getsockname(server.dgramsock, (void *)&addr, &addrlen);
  return ntohs(addr.sin_port);
}

