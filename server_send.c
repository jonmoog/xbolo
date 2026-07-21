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

  srsendmesg.type = SRSENDMESG;
  srsendmesg.player = player;
  srsendmesg.to = to;
  len = strlen(text) + 1;

  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1 && (mask & (1 << i))) {
      if (writebuf(&server.players[i].sendbuf, &srsendmesg, sizeof(srsendmesg)) == -1) return ERRLOG(errno);
      if (writebuf(&server.players[i].sendbuf, text, len) == -1) return ERRLOG(errno);
//      if (sendplayerbufserver(i)) return ERRLOG(errno);
    }
  }


  return 0;
}


int sendsrdamage(int player, int x, int y) {
  struct SRDamage srdamage;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srdamage.type = SRDAMAGE;
  srdamage.player = player;
  srdamage.x = x;
  srdamage.y = y;
  srdamage.terrain = server.terrain[y][x];

  if (sendtoall(&srdamage, sizeof(srdamage))) return ERRLOG(errno);


  return 0;
}


int sendsrgrabtrees(int x, int y) {
  struct SRGrabTrees srgrabtrees;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srgrabtrees.type = SRGRABTREES;
  srgrabtrees.x = x;
  srgrabtrees.y = y;

  if (sendtoall(&srgrabtrees, sizeof(srgrabtrees))) return ERRLOG(errno);


  return 0;
}


int sendsrbuild(int x, int y) {
  struct SRBuild srbuild;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srbuild.type = SRBUILD;
  srbuild.x = x;
  srbuild.y = y;

  srbuild.terrain = server.terrain[y][x];

  if (sendtoall(&srbuild, sizeof(srbuild))) return ERRLOG(errno);


  return 0;
}


int sendsrgrow(int x, int y) {
  struct SRGrow srgrow;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srgrow.type = SRGROW;
  srgrow.x = x;
  srgrow.y = y;

  if (sendtoall(&srgrow, sizeof(srgrow))) return ERRLOG(errno);


  return 0;
}


int sendsrflood(int x, int y) {
  struct SRFlood srflood;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srflood.type = SRFLOOD;
  srflood.x = x;
  srflood.y = y;

  if (sendtoall(&srflood, sizeof(srflood))) return ERRLOG(errno);


  return 0;
}


int sendsrplacemine(int player, int x, int y) {
  struct SRPlaceMine srplacemine;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srplacemine.type = SRPLACEMINE;
  srplacemine.player = player;
  srplacemine.x = x;
  srplacemine.y = y;

  if (sendtoall(&srplacemine, sizeof(srplacemine))) return ERRLOG(errno);


  return 0;
}


int sendsrdropmine(int player, int x, int y) {
  struct SRDropMine srdropmine;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srdropmine.type = SRDROPMINE;
  srdropmine.player = player;
  srdropmine.x = x;
  srdropmine.y = y;

  if (sendtoall(&srdropmine, sizeof(srdropmine))) return ERRLOG(errno);


  return 0;
}


int sendsrdropboat(int x, int y) {
  struct SRDropBoat srdropboat;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srdropboat.type = SRDROPBOAT;
  srdropboat.x = x;
  srdropboat.y = y;

  if (sendtoall(&srdropboat, sizeof(srdropboat))) return ERRLOG(errno);


  return 0;
}


int sendsrplayerjoin(int player) {
  struct SRPlayerJoin srplayerjoin;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srplayerjoin.type = SRPLAYERJOIN;
  srplayerjoin.player = player;
  bzero(srplayerjoin.name, sizeof(srplayerjoin.name));
  strncpy(srplayerjoin.name, server.players[player].name, sizeof(srplayerjoin.name) - 1);
  bzero(srplayerjoin.host, sizeof(srplayerjoin.host));
  strncpy(srplayerjoin.host, server.players[player].host, sizeof(srplayerjoin.host) - 1);

  if (sendtoall(&srplayerjoin, sizeof(srplayerjoin))) return ERRLOG(errno);


  return 0;
}


int sendsrplayerrejoin(int player) {
  struct SRPlayerRejoin srplayerrejoin;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srplayerrejoin.type = SRPLAYERREJOIN;
  srplayerrejoin.player = player;
  bzero(srplayerrejoin.host, sizeof(srplayerrejoin.host));
  strncpy(srplayerrejoin.host, server.players[player].host, sizeof(srplayerrejoin.host) - 1);

  if (sendtoall(&srplayerrejoin, sizeof(srplayerrejoin))) return ERRLOG(errno);


  return 0;
}


int sendsrplayerexit(int player) {
  struct SRPlayerExit srplayerexit;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srplayerexit.type = SRPLAYEREXIT;
  srplayerexit.player = player;

  if (sendtoone(&srplayerexit, sizeof(srplayerexit), player)) {
    if (errno != EPIPE) {
      return ERRLOG(errno);
    }
    else {
      CLEARERRLOG
    }
  }

  if (sendtoallex(&srplayerexit, sizeof(srplayerexit), player)) return ERRLOG(errno);


  return 0;
}


int sendsrplayerdisc(int player) {
  struct SRPlayerDisc srplayerdisc;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srplayerdisc.type = SRPLAYERDISC;
  srplayerdisc.player = player;

  if (sendtoallex(&srplayerdisc, sizeof(srplayerdisc), player)) return ERRLOG(errno);


  return 0;
}


int sendsrplayerkick(int player) {
  struct SRPlayerKick srplayerkick;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srplayerkick.type = SRPLAYERKICK;
  srplayerkick.player = player;

  if (sendtoall(&srplayerkick, sizeof(srplayerkick))) return ERRLOG(errno);


  return 0;
}


int sendsrplayerban(int player) {
  struct SRPlayerBan srplayerban;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srplayerban.type = SRPLAYERBAN;
  srplayerban.player = player;

  if (sendtoall(&srplayerban, sizeof(srplayerban))) return ERRLOG(errno);


  return 0;
}


int sendsrpause(int pause) {
  struct SRPause srpause;

  assert(pause >= 0);
  assert(pause < 256);

  srpause.type = SRPAUSE;
  srpause.pause = pause;

  if (sendtoall(&srpause, sizeof(srpause))) return ERRLOG(errno);


  return 0;
}


int sendsrrepairpill(int pill) {
  struct SRRepairPill srrepairpill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

  srrepairpill.type = SRREPAIRPILL;
  srrepairpill.pill = pill;
  srrepairpill.armour = server.pills[pill].armour;

  if (sendtoall(&srrepairpill, sizeof(srrepairpill))) return ERRLOG(errno);


  return 0;
}


int sendsrcoolpill(int pill) {
  struct SRCoolPill srcoolpill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

  srcoolpill.type = SRCOOLPILL;
  srcoolpill.pill = pill;

  if (sendtoall(&srcoolpill, sizeof(srcoolpill))) return ERRLOG(errno);


  return 0;
}


int sendsrcapturepill(int pill) {
  struct SRCapturePill srcapturepill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour == ONBOARD);

  srcapturepill.type = SRCAPTUREPILL;
  srcapturepill.pill = pill;
  srcapturepill.owner = server.pills[pill].owner;

  if (sendtoall(&srcapturepill, sizeof(srcapturepill))) return ERRLOG(errno);


  return 0;
}


int sendsrbuildpill(int pill) {
  struct SRBuildPill srbuildpill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

  srbuildpill.type = SRBUILDPILL;
  srbuildpill.pill = pill;
  srbuildpill.x = server.pills[pill].x;
  srbuildpill.y = server.pills[pill].y;
  srbuildpill.armour = server.pills[pill].armour;

  if (sendtoall(&srbuildpill, sizeof(srbuildpill))) return ERRLOG(errno);


  return 0;
}


int sendsrdroppill(int pill) {
  struct SRDropPill srdroppill;

  assert(pill >= 0);
  assert(pill < server.npills);
  assert(server.pills[pill].armour != ONBOARD);

  srdroppill.type = SRDROPPILL;
  srdroppill.pill = pill;
  srdroppill.x = server.pills[pill].x;
  srdroppill.y = server.pills[pill].y;

  if (sendtoall(&srdroppill, sizeof(srdroppill))) return ERRLOG(errno);


  return 0;
}


int sendsrreplenishbase(int base) {
  struct SRReplenishBase srreplenishbase;

  assert(base >= 0);
  assert(base < server.nbases);

  srreplenishbase.type = SRREPLENISHBASE;
  srreplenishbase.base = base;

  if (sendtoall(&srreplenishbase, sizeof(srreplenishbase))) return ERRLOG(errno);


  return 0;
}


int sendsrcapturebase(int base) {
  struct SRCaptureBase srcaputurebase;

  assert(base >= 0);
  assert(base < server.nbases);

  srcaputurebase.type = SRCAPTUREBASE;
  srcaputurebase.base = base;
  srcaputurebase.owner = server.bases[base].owner;

  if (sendtoall(&srcaputurebase, sizeof(srcaputurebase))) return ERRLOG(errno);


  return 0;
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

  srrefuel.type = SRREFUEL;
  srrefuel.base = base;
  srrefuel.armour = armour;
  srrefuel.shells = shells;
  srrefuel.mines = mines;

  if (sendtoallex(&srrefuel, sizeof(srrefuel), player)) return ERRLOG(errno);


  return 0;
}


int sendsrgrabboat(int player, int x, int y) {
  struct SRGrabBoat srgrabboat;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  srgrabboat.type = SRGRABBOAT;
  srgrabboat.player = player;
  srgrabboat.x = x;
  srgrabboat.y = y;

  if (sendtoall(&srgrabboat, sizeof(srgrabboat))) return ERRLOG(errno);


  return 0;
}


int sendsrmineack(int player, int success) {
  struct SRMineAck srmineack;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srmineack.type = SRMINEACK;
  srmineack.success = success != 0;

  if (sendtoone(&srmineack, sizeof(srmineack), player)) return ERRLOG(errno);


  return 0;
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

  srbuilderack.type = SRBUILDERACK;
  srbuilderack.mines = mines;
  srbuilderack.trees = trees;
  srbuilderack.pill = pill;

  if (sendtoone(&srbuilderack, sizeof(srbuilderack), player)) return ERRLOG(errno);


  return 0;
}


int sendsrsmallboom(int player, int x, int y) {
  struct SRSmallBoom srsmallboom;

  assert(player >= 0);
  assert(player < MAXPLAYERS || player == NEUTRAL);
  assert(x >= 0);
  assert(x < 256);
  assert(y >= 0);
  assert(y < 256);

  srsmallboom.type = SRSMALLBOOM;
  srsmallboom.player = player;
  srsmallboom.x = x;
  srsmallboom.y = y;

  if (sendtoall(&srsmallboom, sizeof(srsmallboom))) return ERRLOG(errno);


  return 0;
}


int sendsrsuperboom(int player, int x, int y) {
  struct SRSuperBoom srsuperboom;

  assert(player >= 0);
  assert(player < MAXPLAYERS);
  assert(x >= 0);
  assert(x < 256);
  assert(y >= 0);
  assert(y < 256);

  srsuperboom.type = SRSUPERBOOM;
  srsuperboom.player = player;
  srsuperboom.x = x;
  srsuperboom.y = y;

  if (sendtoall(&srsuperboom, sizeof(srsuperboom))) return ERRLOG(errno);


  return 0;
}


int sendsrhittank(int player, uint32_t dir) {
  struct SRHitTank srhittank;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srhittank.type = SRHITTANK;
  srhittank.player = player;
  srhittank.dir = dir;

  if (sendtoone(&srhittank, sizeof(srhittank), player)) return ERRLOG(errno);


  return 0;
}


int sendsrsetalliance(int player, uint16_t alliance) {
  struct SRSetAlliance srsetalliance;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

  srsetalliance.type = SRSETALLIANCE;
  srsetalliance.player = player;
  srsetalliance.alliance = htons(alliance);

  if (sendtoallex(&srsetalliance, sizeof(srsetalliance), player)) return ERRLOG(errno);


  return 0;
}


int sendsrtimelimit(uint16_t timeremaining) {
  struct SRTimeLimit srtimelimit;

  srtimelimit.type = SRTIMELIMIT;
  srtimelimit.timeremaining = htons(timeremaining);
  
  if (sendtoall(&srtimelimit, sizeof(srtimelimit))) return ERRLOG(errno);


  return 0;
}


int sendsrbasecontrol(uint16_t timeleft) {
  struct SRBaseControl srbasecontrol;

  srbasecontrol.type = SRBASECONTROL;
  srbasecontrol.timeleft = htons(timeleft);
  
  if (sendtoall(&srbasecontrol, sizeof(srbasecontrol))) return ERRLOG(errno);


  return 0;
}


int sendtoall(const void *data, size_t nbytes) {
  int i;

  assert(data != NULL);

  for (i = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1) {
      if (writebuf(&server.players[i].sendbuf, data, nbytes) == -1) return ERRLOG(errno);
//      if (sendplayerbufserver(i)) return ERRLOG(errno);
    }
  }


  return 0;
}


int sendtoallex(const void *data, size_t nbytes, int player) {
  int i;

  assert(data != NULL);
  assert(player >= 0);
  assert(player < MAXPLAYERS);

  for (i = 0; i < MAXPLAYERS; i++) {
    if (player != i && server.players[i].cntlsock != -1) {
      if (writebuf(&server.players[i].sendbuf, data, nbytes) == -1) return ERRLOG(errno);
//      if (sendplayerbufserver(i)) return ERRLOG(errno);
    }
  }


  return 0;
}


int sendtoone(const void *data, size_t nbytes, int player) {
  assert(data != NULL);
  assert(player >= 0);
  assert(player < MAXPLAYERS);

  if (server.players[player].cntlsock != -1) {
    if (writebuf(&server.players[player].sendbuf, data, nbytes) == -1) return ERRLOG(errno);
//    if (sendplayerbufserver(player)) return ERRLOG(errno);
  }


  return 0;
}

