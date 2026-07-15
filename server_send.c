/*
 *  server_send.c
 *  XBolo
 *
 *  Builders for server->client protocol messages and send fan-out.
 */

#include "server_internal.h"


int sendsrsendmesg(int player, int to, uint16_t mask, const char *text) {
  int i;
  struct SRSendMesg srsendmesg;
  size_t len;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(text != NULL);

TRY
  srsendmesg.type = SRSENDMESG;
  srsendmesg.player = player;
  srsendmesg.to = to;
  len = strlen(text) + 1;

  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1 && (mask & (1 << i))) {
      if (writebuf(&server.players[i].sendbuf, &srsendmesg, sizeof(srsendmesg)) == -1) LOGFAIL(errno)
      if (writebuf(&server.players[i].sendbuf, text, len) == -1) LOGFAIL(errno)
//      if (sendplayerbufserver(i)) LOGFAIL(errno)
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrdamage(int player, int x, int y) {
  struct SRDamage srdamage;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srdamage.type = SRDAMAGE;
  srdamage.player = player;
  srdamage.x = x;
  srdamage.y = y;
  srdamage.terrain = server.terrain[y][x];

  if (sendtoall(&srdamage, sizeof(srdamage))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrgrabtrees(int x, int y) {
  struct SRGrabTrees srgrabtrees;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srgrabtrees.type = SRGRABTREES;
  srgrabtrees.x = x;
  srgrabtrees.y = y;

  if (sendtoall(&srgrabtrees, sizeof(srgrabtrees))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrbuild(int x, int y) {
  struct SRBuild srbuild;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srbuild.type = SRBUILD;
  srbuild.x = x;
  srbuild.y = y;

  srbuild.terrain = server.terrain[y][x];

  if (sendtoall(&srbuild, sizeof(srbuild))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrgrow(int x, int y) {
  struct SRGrow srgrow;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srgrow.type = SRGROW;
  srgrow.x = x;
  srgrow.y = y;

  if (sendtoall(&srgrow, sizeof(srgrow))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrflood(int x, int y) {
  struct SRFlood srflood;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srflood.type = SRFLOOD;
  srflood.x = x;
  srflood.y = y;

  if (sendtoall(&srflood, sizeof(srflood))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplacemine(int player, int x, int y) {
  struct SRPlaceMine srplacemine;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srplacemine.type = SRPLACEMINE;
  srplacemine.player = player;
  srplacemine.x = x;
  srplacemine.y = y;

  if (sendtoall(&srplacemine, sizeof(srplacemine))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrdropmine(int player, int x, int y) {
  struct SRDropMine srdropmine;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srdropmine.type = SRDROPMINE;
  srdropmine.player = player;
  srdropmine.x = x;
  srdropmine.y = y;

  if (sendtoall(&srdropmine, sizeof(srdropmine))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrdropboat(int x, int y) {
  struct SRDropBoat srdropboat;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srdropboat.type = SRDROPBOAT;
  srdropboat.x = x;
  srdropboat.y = y;

  if (sendtoall(&srdropboat, sizeof(srdropboat))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplayerjoin(int player) {
  struct SRPlayerJoin srplayerjoin;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srplayerjoin.type = SRPLAYERJOIN;
  srplayerjoin.player = player;
  bzero(srplayerjoin.name, sizeof(srplayerjoin.name));
  strncpy(srplayerjoin.name, server.players[player].name, sizeof(srplayerjoin.name) - 1);
  bzero(srplayerjoin.host, sizeof(srplayerjoin.host));
  strncpy(srplayerjoin.host, server.players[player].host, sizeof(srplayerjoin.host) - 1);

  if (sendtoall(&srplayerjoin, sizeof(srplayerjoin))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplayerrejoin(int player) {
  struct SRPlayerRejoin srplayerrejoin;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srplayerrejoin.type = SRPLAYERREJOIN;
  srplayerrejoin.player = player;
  bzero(srplayerrejoin.host, sizeof(srplayerrejoin.host));
  strncpy(srplayerrejoin.host, server.players[player].host, sizeof(srplayerrejoin.host) - 1);

  if (sendtoall(&srplayerrejoin, sizeof(srplayerrejoin))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplayerexit(int player) {
  struct SRPlayerExit srplayerexit;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srplayerexit.type = SRPLAYEREXIT;
  srplayerexit.player = player;

  if (sendtoone(&srplayerexit, sizeof(srplayerexit), player)) {
    if (errno != EPIPE) {
      LOGFAIL(errno)
    }
    else {
      CLEARERRLOG
    }
  }

  if (sendtoallex(&srplayerexit, sizeof(srplayerexit), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplayerdisc(int player) {
  struct SRPlayerDisc srplayerdisc;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srplayerdisc.type = SRPLAYERDISC;
  srplayerdisc.player = player;

  if (sendtoallex(&srplayerdisc, sizeof(srplayerdisc), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplayerkick(int player) {
  struct SRPlayerKick srplayerkick;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srplayerkick.type = SRPLAYERKICK;
  srplayerkick.player = player;

  if (sendtoall(&srplayerkick, sizeof(srplayerkick))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrplayerban(int player) {
  struct SRPlayerBan srplayerban;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srplayerban.type = SRPLAYERBAN;
  srplayerban.player = player;

  if (sendtoall(&srplayerban, sizeof(srplayerban))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrpause(int pause) {
  struct SRPause srpause;

  assert(pause >= 0);
  assert(pause < 256);

TRY
  srpause.type = SRPAUSE;
  srpause.pause = pause;

  if (sendtoall(&srpause, sizeof(srpause))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrrepairpill(int pill) {
  struct SRRepairPill srrepairpill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

TRY
  srrepairpill.type = SRREPAIRPILL;
  srrepairpill.pill = pill;
  srrepairpill.armour = server.pills[pill].armour;

  if (sendtoall(&srrepairpill, sizeof(srrepairpill))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrcoolpill(int pill) {
  struct SRCoolPill srcoolpill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

TRY
  srcoolpill.type = SRCOOLPILL;
  srcoolpill.pill = pill;

  if (sendtoall(&srcoolpill, sizeof(srcoolpill))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrcapturepill(int pill) {
  struct SRCapturePill srcapturepill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour == ONBOARD);

TRY
  srcapturepill.type = SRCAPTUREPILL;
  srcapturepill.pill = pill;
  srcapturepill.owner = server.pills[pill].owner;

  if (sendtoall(&srcapturepill, sizeof(srcapturepill))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrbuildpill(int pill) {
  struct SRBuildPill srbuildpill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

TRY
  srbuildpill.type = SRBUILDPILL;
  srbuildpill.pill = pill;
  srbuildpill.x = server.pills[pill].x;
  srbuildpill.y = server.pills[pill].y;
  srbuildpill.armour = server.pills[pill].armour;

  if (sendtoall(&srbuildpill, sizeof(srbuildpill))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrdroppill(int pill) {
  struct SRDropPill srdroppill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

TRY
  srdroppill.type = SRDROPPILL;
  srdroppill.pill = pill;
  srdroppill.x = server.pills[pill].x;
  srdroppill.y = server.pills[pill].y;

  if (sendtoall(&srdroppill, sizeof(srdroppill))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrreplenishbase(int base) {
  struct SRReplenishBase srreplenishbase;

  assert(base >= 0);
  assert(base < server.nbases);

TRY
  srreplenishbase.type = SRREPLENISHBASE;
  srreplenishbase.base = base;

  if (sendtoall(&srreplenishbase, sizeof(srreplenishbase))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrcapturebase(int base) {
  struct SRCaptureBase srcaputurebase;

  assert(base >= 0);
  assert(base < server.nbases);

TRY
  srcaputurebase.type = SRCAPTUREBASE;
  srcaputurebase.base = base;
  srcaputurebase.owner = server.bases[base].owner;

  if (sendtoall(&srcaputurebase, sizeof(srcaputurebase))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrrefuel(int player, int base, int armour, int shells, int mines) {
  struct SRRefuel srrefuel;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(server.players[player].cntlsock != -1);
  assert(base >= 0);
  assert(base < server.nbases);
  assert(armour >= 0);
  assert(armour < 256);
  assert(shells >= 0);
  assert(shells < 256);
  assert(mines >= 0);
  assert(mines < 256);

TRY
  srrefuel.type = SRREFUEL;
  srrefuel.base = base;
  srrefuel.armour = armour;
  srrefuel.shells = shells;
  srrefuel.mines = mines;

  if (sendtoallex(&srrefuel, sizeof(srrefuel), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrgrabboat(int player, int x, int y) {
  struct SRGrabBoat srgrabboat;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  srgrabboat.type = SRGRABBOAT;
  srgrabboat.player = player;
  srgrabboat.x = x;
  srgrabboat.y = y;

  if (sendtoall(&srgrabboat, sizeof(srgrabboat))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrmineack(int player, int success) {
  struct SRMineAck srmineack;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srmineack.type = SRMINEACK;
  srmineack.success = success != 0;

  if (sendtoone(&srmineack, sizeof(srmineack), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrbuilderack(int player, int mines, int trees, int pill) {
  struct SRBuilderAck srbuilderack;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(mines >= 0);
  assert(mines < 256);
  assert(trees >= 0);
  assert(trees < 256);
  assert(mines >= 0);
  assert(pill >= 0);
  assert(pill < server.npills || pill == 255);

TRY
  srbuilderack.type = SRBUILDERACK;
  srbuilderack.mines = mines;
  srbuilderack.trees = trees;
  srbuilderack.pill = pill;

  if (sendtoone(&srbuilderack, sizeof(srbuilderack), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrsmallboom(int player, int x, int y) {
  struct SRSmallBoom srsmallboom;

  assert(player >= 0);
  assert(player < MAXPLAYERS || player == NEUTRAL);
  assert(x >= 0);
  assert(x < 256);
  assert(y >= 0);
  assert(y < 256);

TRY
  srsmallboom.type = SRSMALLBOOM;
  srsmallboom.player = player;
  srsmallboom.x = x;
  srsmallboom.y = y;

  if (sendtoall(&srsmallboom, sizeof(srsmallboom))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrsuperboom(int player, int x, int y) {
  struct SRSuperBoom srsuperboom;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < 256);
  assert(y >= 0);
  assert(y < 256);

TRY
  srsuperboom.type = SRSUPERBOOM;
  srsuperboom.player = player;
  srsuperboom.x = x;
  srsuperboom.y = y;

  if (sendtoall(&srsuperboom, sizeof(srsuperboom))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrhittank(int player, uint32_t dir) {
  struct SRHitTank srhittank;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srhittank.type = SRHITTANK;
  srhittank.player = player;
  srhittank.dir = dir;

  if (sendtoone(&srhittank, sizeof(srhittank), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrsetalliance(int player, uint16_t alliance) {
  struct SRSetAlliance srsetalliance;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  srsetalliance.type = SRSETALLIANCE;
  srsetalliance.player = player;
  srsetalliance.alliance = htons(alliance);

  if (sendtoallex(&srsetalliance, sizeof(srsetalliance), player)) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrtimelimit(uint16_t timeremaining) {
  struct SRTimeLimit srtimelimit;

TRY
  srtimelimit.type = SRTIMELIMIT;
  srtimelimit.timeremaining = htons(timeremaining);
  
  if (sendtoall(&srtimelimit, sizeof(srtimelimit))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendsrbasecontrol(uint16_t timeleft) {
  struct SRBaseControl srbasecontrol;

TRY
  srbasecontrol.type = SRBASECONTROL;
  srbasecontrol.timeleft = htons(timeleft);
  
  if (sendtoall(&srbasecontrol, sizeof(srbasecontrol))) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendtoall(const void *data, size_t nbytes) {
  int i;

  assert(data != NULL);

TRY
  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1) {
      if (writebuf(&server.players[i].sendbuf, data, nbytes) == -1) LOGFAIL(errno)
//      if (sendplayerbufserver(i)) LOGFAIL(errno)
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendtoallex(const void *data, size_t nbytes, int player) {
  int i;

  assert(data != NULL);
  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  for (i = 0; i < MAXPLAYERS; i++) {
    if (player != i && server.players[i].cntlsock != -1) {
      if (writebuf(&server.players[i].sendbuf, data, nbytes) == -1) LOGFAIL(errno)
//      if (sendplayerbufserver(i)) LOGFAIL(errno)
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendtoone(const void *data, size_t nbytes, int player) {
  assert(data != NULL);
  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].cntlsock != -1) {
    if (writebuf(&server.players[player].sendbuf, data, nbytes) == -1) LOGFAIL(errno)
//    if (sendplayerbufserver(player)) LOGFAIL(errno)
  }

CLEANUP
ERRHANDLER(0, -1)
END
}

