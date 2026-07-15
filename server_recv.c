/*
 *  server_recv.c
 *  XBolo
 *
 *  Handlers for client->server protocol messages.
 */

#include "server_internal.h"


int recvclsendmesg(int player) {
  int i;
  struct CLSendMesg *clsendmesg;
  char *text;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  /* check for whole struct */
  if (server.players[player].recvbuf.nbytes < sizeof(clsendmesg)) FAIL(EAGAIN)

  /* check for complete string following struct */
  for (i = sizeof(struct CLSendMesg) ; i < server.players[player].recvbuf.nbytes; i++) {
    if (((char *)server.players[player].recvbuf.ptr)[i] == '\0') {
      break;
    }
  }

  /* buf does not have the whole string */
  if (!(i < server.players[player].recvbuf.nbytes)) FAIL(EAGAIN)

  /* get pointers to data */
  clsendmesg = (struct CLSendMesg *)server.players[player].recvbuf.ptr;
  /* get pointer to text */
  text = (char *)(clsendmesg + 1);

  /* convert byte order */
  clsendmesg->mask = ntohs(clsendmesg->mask);

  /* send message to all players */
  if (sendsrsendmesg(player, clsendmesg->to, clsendmesg->mask, text)) LOGFAIL(errno)

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, i + 1) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvcldropboat(int player) {
  struct CLDropBoat *cldropboat;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  /* receive message */
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLDropBoat)) FAIL(EAGAIN)

  cldropboat = (struct CLDropBoat *)server.players[player].recvbuf.ptr;

  if (ispointinrect(kSeaRect, makepoint(cldropboat->x, cldropboat->y))) {
    if (server.terrain[cldropboat->y][cldropboat->x] == kRiverTerrain) {
      server.terrain[cldropboat->y][cldropboat->x] = kBoatTerrain;
      sendsrdropboat(cldropboat->x, cldropboat->y);
    }

    /* clear buffer of read data */
    if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLDropBoat)) == -1) LOGFAIL(errno)
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvcldroppills(int player) {
  struct CLDropPills *cldroppills;
  int i;
  float x, y;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLDropPills)) FAIL(EAGAIN)

  cldroppills = (struct CLDropPills *)server.players[player].recvbuf.ptr;

  cldroppills->pills = ntohs(cldroppills->pills);
  cldroppills->x = ntohl(cldroppills->x);
  cldroppills->y = ntohl(cldroppills->y);
  x = unpackfloat(&cldroppills->x);
  y = unpackfloat(&cldroppills->y);

  for (i = 0; i < server.npills; i++) {
    if ((cldroppills->pills & (1 << i)) && (server.pills[i].owner != player || server.pills[i].armour != ONBOARD)) {
      break;
    }
  }

  if (!(i < server.npills) && x > 0.0 && x < FWIDTH && y > 0.0 && y < FWIDTH) {
    droppills(player, x, y, cldroppills->pills);
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLDropPills)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvcldropmine(int player) {
  struct CLDropMine *cldropmine;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLDropMine)) FAIL(EAGAIN)

  cldropmine = (struct CLDropMine *)server.players[player].recvbuf.ptr;

  if (ispointinrect(kSeaRect, makepoint(cldropmine->x, cldropmine->y))) {
    switch (server.terrain[cldropmine->y][cldropmine->x]) {
    case kSwampTerrain0:
    case kSwampTerrain1:
    case kSwampTerrain2:
    case kSwampTerrain3:
      server.terrain[cldropmine->y][cldropmine->x] = kMinedSwampTerrain;
      sendsrdropmine(player, cldropmine->x, cldropmine->y);
      sendsrmineack(player, 1);
      break;

    case kCraterTerrain:
      server.terrain[cldropmine->y][cldropmine->x] = kMinedCraterTerrain;
      sendsrdropmine(player, cldropmine->x, cldropmine->y);
      sendsrmineack(player, 1);
      break;

    case kRoadTerrain:
      server.terrain[cldropmine->y][cldropmine->x] = kMinedRoadTerrain;
      sendsrdropmine(player, cldropmine->x, cldropmine->y);
      sendsrmineack(player, 1);
      break;

    case kForestTerrain:
      server.terrain[cldropmine->y][cldropmine->x] = kMinedForestTerrain;
      sendsrdropmine(player, cldropmine->x, cldropmine->y);
      sendsrmineack(player, 1);
      break;

    case kRubbleTerrain0:
    case kRubbleTerrain1:
    case kRubbleTerrain2:
    case kRubbleTerrain3:
      server.terrain[cldropmine->y][cldropmine->x] = kMinedRubbleTerrain;
      sendsrdropmine(player, cldropmine->x, cldropmine->y);
      sendsrmineack(player, 1);
      break;

    case kGrassTerrain0:
    case kGrassTerrain1:
    case kGrassTerrain2:
    case kGrassTerrain3:
      server.terrain[cldropmine->y][cldropmine->x] = kMinedGrassTerrain;
      sendsrdropmine(player, cldropmine->x, cldropmine->y);
      sendsrmineack(player, 1);
      break;

    default:
      sendsrmineack(player, 0);
      break;
    }

    /* clear buffer of read data */
    if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLDropMine)) == -1) LOGFAIL(errno)
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvcltouch(int player) {
  struct CLTouch *cltouch;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLTouch)) FAIL(EAGAIN)

  cltouch = (struct CLTouch *)server.players[player].recvbuf.ptr;

  /* interact with terrain */
  switch (server.terrain[cltouch->y][cltouch->x]) {
  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(player, cltouch->x, cltouch->y)) LOGFAIL(errno)
    break;

  default:
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLTouch)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclgrabtile(int player) {
  struct CLGrabTile *clgrabtile;
  int pill, base;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLGrabTile)) FAIL(EAGAIN)
  clgrabtile = (struct CLGrabTile *)server.players[player].recvbuf.ptr;

  /* grab pills */
  if ((pill = findpill(clgrabtile->x, clgrabtile->y)) != -1) {
    server.pills[pill].owner = player;
    server.pills[pill].armour = ONBOARD;
    server.pills[pill].speed = MAXTICKSPERSHOT;
    sendsrcapturepill(pill);
  }

  /* grab bases */
  if ((base = findbase(clgrabtile->x, clgrabtile->y)) != -1) {
    if (server.bases[base].owner == NEUTRAL) {
      server.bases[base].owner = player;
      server.bases[base].armour = MAXBASEARMOUR;
      server.bases[base].shells = MAXBASESHELLS;
      server.bases[base].mines = MAXBASEMINES;
      sendsrcapturebase(base);
    }
    else if (
      server.bases[base].owner != NEUTRAL &&
      (
        (server.players[server.bases[base].owner].alliance & (1 << player)) &&
        (server.players[player].alliance & (1 << server.bases[base].owner))
      )
    ) {
      server.bases[base].owner = player;
      sendsrcapturebase(base);
    }
    else {
      server.bases[base].owner = player;
      server.bases[base].armour = 0;
      server.bases[base].shells = 0;
      server.bases[base].mines = 0;
      sendsrcapturebase(base);
    }
  }

  /* interact with terrain */
  switch (server.terrain[clgrabtile->y][clgrabtile->x]) {
  case kBoatTerrain:
    server.terrain[clgrabtile->y][clgrabtile->x] = kRiverTerrain;
    sendsrgrabboat(player, clgrabtile->x, clgrabtile->y);
    break;

  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(player, clgrabtile->x, clgrabtile->y)) LOGFAIL(errno)
    break;

  default:
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLGrabTile)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclgrabtrees(int player) {
  struct CLGrabTrees *clgrabtrees;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLGrabTrees)) FAIL(EAGAIN)
  clgrabtrees = (struct CLGrabTrees *)server.players[player].recvbuf.ptr;

  switch (server.terrain[clgrabtrees->y][clgrabtrees->x]) {
  case kForestTerrain:
    server.terrain[clgrabtrees->y][clgrabtrees->x] = kGrassTerrain3;
    sendsrgrabtrees(clgrabtrees->x, clgrabtrees->y);
    sendsrbuilderack(player, 0, FORRESTTREES, NOPILL);
    break;

  case kMinedForestTerrain:
    server.terrain[clgrabtrees->y][clgrabtrees->x] = kMinedGrassTerrain;
    sendsrgrabtrees(clgrabtrees->x, clgrabtrees->y);
    sendsrbuilderack(player, 0, FORRESTTREES, NOPILL);
    break;

  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(player, clgrabtrees->x, clgrabtrees->y)) LOGFAIL(errno)
    sendsrbuilderack(player, 0, 0, NOPILL);
    break;

  default:
    sendsrbuilderack(player, 0, 0, NOPILL);
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLGrabTrees)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclbuildroad(int player) {
  struct CLBuildRoad *clbuildroad;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLBuildRoad)) FAIL(EAGAIN)
  clbuildroad = (struct CLBuildRoad *)server.players[player].recvbuf.ptr;

  switch (server.terrain[clbuildroad->y][clbuildroad->x]) {
  case kRiverTerrain:
  case kSwampTerrain0:
  case kSwampTerrain1:
  case kSwampTerrain2:
  case kSwampTerrain3:
  case kCraterTerrain:
  case kRubbleTerrain0:
  case kRubbleTerrain1:
  case kRubbleTerrain2:
  case kRubbleTerrain3:
  case kGrassTerrain0:
  case kGrassTerrain1:
  case kGrassTerrain2:
  case kGrassTerrain3:
    if (clbuildroad->trees >= ROADTREES) {
      server.terrain[clbuildroad->y][clbuildroad->x] = kRoadTerrain;
      sendsrbuild(clbuildroad->x, clbuildroad->y);
      sendsrbuilderack(player, 0, clbuildroad->trees - ROADTREES, NOPILL);
    }
    else {
      sendsrbuilderack(player, 0, clbuildroad->trees, NOPILL);
    }

    break;

  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
  if (explosionat(player, clbuildroad->x, clbuildroad->y)) LOGFAIL(errno)
    sendsrbuilderack(player, 0, clbuildroad->trees, NOPILL);
    break;

  default:
    sendsrbuilderack(player, 0, clbuildroad->trees, NOPILL);
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLBuildRoad)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclbuildwall(int player) {
  struct CLBuildWall *clbuildwall;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLBuildWall)) FAIL(EAGAIN)
  clbuildwall = (struct CLBuildWall *)server.players[player].recvbuf.ptr;

  switch (server.terrain[clbuildwall->y][clbuildwall->x]) {
  case kSwampTerrain0:
  case kSwampTerrain1:
  case kSwampTerrain2:
  case kSwampTerrain3:
  case kCraterTerrain:
  case kRoadTerrain:
  case kRubbleTerrain0:
  case kRubbleTerrain1:
  case kRubbleTerrain2:
  case kRubbleTerrain3:
  case kGrassTerrain0:
  case kGrassTerrain1:
  case kGrassTerrain2:
  case kGrassTerrain3:
  case kDamagedWallTerrain0:
  case kDamagedWallTerrain1:
  case kDamagedWallTerrain2:
  case kDamagedWallTerrain3:
    if (clbuildwall->trees >= WALLTREES) {
      server.terrain[clbuildwall->y][clbuildwall->x] = kWallTerrain;
      sendsrbuild(clbuildwall->x, clbuildwall->y);
      sendsrbuilderack(player, 0, clbuildwall->trees - WALLTREES, NOPILL);
    }
    else {
      sendsrbuilderack(player, 0, clbuildwall->trees, NOPILL);
    }

    break;

  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(player, clbuildwall->x, clbuildwall->y)) LOGFAIL(errno)
    sendsrbuilderack(player, 0, clbuildwall->trees, NOPILL);
    break;

  default:
    sendsrbuilderack(player, 0, clbuildwall->trees, NOPILL);
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLBuildWall)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclbuildboat(int player) {
  struct CLBuildBoat *clbuildboat;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLBuildBoat)) FAIL(EAGAIN)
  clbuildboat = (struct CLBuildBoat *)server.players[player].recvbuf.ptr;

  switch (server.terrain[clbuildboat->y][clbuildboat->x]) {
  case kRiverTerrain:
    server.terrain[clbuildboat->y][clbuildboat->x] = kBoatTerrain;
    sendsrbuild(clbuildboat->x, clbuildboat->y);
    sendsrbuilderack(player, 0, clbuildboat->trees - BOATTREES, NOPILL);
    break;

  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(player, clbuildboat->x, clbuildboat->y)) LOGFAIL(errno)
    sendsrbuilderack(player, 0, clbuildboat->trees, NOPILL);
    break;

  default:
    sendsrbuilderack(player, 0, clbuildboat->trees, NOPILL);
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLBuildBoat)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclbuildpill(int player) {
  struct CLBuildPill *clbuildpill;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLBuildPill)) FAIL(EAGAIN)
  clbuildpill = (struct CLBuildPill *)server.players[player].recvbuf.ptr;

  if (findpill(clbuildpill->x, clbuildpill->y) == -1 && findbase(clbuildpill->x, clbuildpill->y) == -1) {
    switch (server.terrain[clbuildpill->y][clbuildpill->x]) {
    case kSwampTerrain0:
    case kSwampTerrain1:
    case kSwampTerrain2:
    case kSwampTerrain3:
    case kCraterTerrain:
    case kRoadTerrain:
    case kRubbleTerrain0:
    case kRubbleTerrain1:
    case kRubbleTerrain2:
    case kRubbleTerrain3:
    case kGrassTerrain0:
    case kGrassTerrain1:
    case kGrassTerrain2:
    case kGrassTerrain3:
    case kDamagedWallTerrain0:
    case kDamagedWallTerrain1:
    case kDamagedWallTerrain2:
    case kDamagedWallTerrain3:
      server.pills[clbuildpill->pill].x = clbuildpill->x;
      server.pills[clbuildpill->pill].y = clbuildpill->y;
      server.pills[clbuildpill->pill].owner = player;
      server.pills[clbuildpill->pill].armour = clbuildpill->trees*4;

      if (server.pills[clbuildpill->pill].armour > MAXPILLARMOUR) {
        clbuildpill->trees = (server.pills[clbuildpill->pill].armour - MAXPILLARMOUR)/4;
        server.pills[clbuildpill->pill].armour = MAXPILLARMOUR;
      }
      else {
        clbuildpill->trees = 0;
      }

      sendsrbuildpill(clbuildpill->pill);
      sendsrbuilderack(player, 0, clbuildpill->trees, NOPILL);
      break;

    case kMinedSeaTerrain:
    case kMinedSwampTerrain:
    case kMinedCraterTerrain:
    case kMinedRoadTerrain:
    case kMinedForestTerrain:
    case kMinedRubbleTerrain:
    case kMinedGrassTerrain:
      if (explosionat(player, clbuildpill->x, clbuildpill->y)) LOGFAIL(errno)
      sendsrbuilderack(player, 0, clbuildpill->trees, NOPILL);
      break;

    default:
      sendsrbuilderack(player, 0, clbuildpill->trees, NOPILL);
      break;
    }
  }
  else {
    sendsrbuilderack(player, 0, clbuildpill->trees, NOPILL);
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLBuildPill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclrepairpill(int player) {
  struct CLRepairPill *clrepairpill;
  int pill;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLRepairPill)) FAIL(EAGAIN)
  clrepairpill = (struct CLRepairPill *)server.players[player].recvbuf.ptr;

  if ((pill = findpill(clrepairpill->x, clrepairpill->y)) != -1 && findbase(clrepairpill->x, clrepairpill->y) == -1) {
    switch (server.terrain[clrepairpill->y][clrepairpill->x]) {
    case kSwampTerrain0:
    case kSwampTerrain1:
    case kSwampTerrain2:
    case kSwampTerrain3:
    case kCraterTerrain:
    case kRoadTerrain:
    case kRubbleTerrain0:
    case kRubbleTerrain1:
    case kRubbleTerrain2:
    case kRubbleTerrain3:
    case kGrassTerrain0:
    case kGrassTerrain1:
    case kGrassTerrain2:
    case kGrassTerrain3:
    case kDamagedWallTerrain0:
    case kDamagedWallTerrain1:
    case kDamagedWallTerrain2:
    case kDamagedWallTerrain3:
      server.pills[pill].armour += clrepairpill->trees*4;

      if (server.pills[pill].armour > MAXPILLARMOUR) {
        clrepairpill->trees = (server.pills[pill].armour - MAXPILLARMOUR)/4;
        server.pills[pill].armour = MAXPILLARMOUR;
      }
      else {
        clrepairpill->trees = 0;
      }

      sendsrrepairpill(pill);
      sendsrbuilderack(player, 0, clrepairpill->trees, NOPILL);
      break;

    case kMinedSeaTerrain:
    case kMinedSwampTerrain:
    case kMinedCraterTerrain:
    case kMinedRoadTerrain:
    case kMinedForestTerrain:
    case kMinedRubbleTerrain:
    case kMinedGrassTerrain:
      if (explosionat(player, clrepairpill->x, clrepairpill->y)) LOGFAIL(errno)
      sendsrbuilderack(player, 0, clrepairpill->trees, NOPILL);
      break;

    default:
      sendsrbuilderack(player, 0, clrepairpill->trees, NOPILL);
      break;
    }
  }
  else {
    sendsrbuilderack(player, 0, clrepairpill->trees, NOPILL);
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLRepairPill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclplacemine(int player) {
  struct CLPlaceMine *clplacemine;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLPlaceMine)) FAIL(EAGAIN)
  clplacemine = (struct CLPlaceMine *)server.players[player].recvbuf.ptr;

  switch (server.terrain[clplacemine->y][clplacemine->x]) {
  case kSwampTerrain0:
  case kSwampTerrain1:
  case kSwampTerrain2:
  case kSwampTerrain3:
  case kCraterTerrain:
  case kRoadTerrain:
  case kForestTerrain:
  case kRubbleTerrain0:
  case kRubbleTerrain1:
  case kRubbleTerrain2:
  case kRubbleTerrain3:
  case kGrassTerrain0:
  case kGrassTerrain1:
  case kGrassTerrain2:
  case kGrassTerrain3:
    switch (server.terrain[clplacemine->y][clplacemine->x]) {
    case kSwampTerrain0:
    case kSwampTerrain1:
    case kSwampTerrain2:
    case kSwampTerrain3:
      server.terrain[clplacemine->y][clplacemine->x] = kMinedSwampTerrain;

      break;

    case kCraterTerrain:
      server.terrain[clplacemine->y][clplacemine->x] = kMinedCraterTerrain;

      break;

    case kRoadTerrain:
      server.terrain[clplacemine->y][clplacemine->x] = kMinedRoadTerrain;

      break;

    case kForestTerrain:
      server.terrain[clplacemine->y][clplacemine->x] = kMinedForestTerrain;

      break;

    case kRubbleTerrain0:
    case kRubbleTerrain1:
    case kRubbleTerrain2:
    case kRubbleTerrain3:
      server.terrain[clplacemine->y][clplacemine->x] = kMinedRubbleTerrain;

      break;

    case kGrassTerrain0:
    case kGrassTerrain1:
    case kGrassTerrain2:
    case kGrassTerrain3:
      server.terrain[clplacemine->y][clplacemine->x] = kMinedGrassTerrain;

      break;

    default:
      break;
    }

    sendsrplacemine(player, clplacemine->x, clplacemine->y);
    sendsrbuilderack(player, 0, 0, NOPILL);
    break;

  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(player, clplacemine->x, clplacemine->y)) LOGFAIL(errno)
    sendsrbuilderack(player, 0, 0, NOPILL);
    break;

  default:
    sendsrbuilderack(player, 0, 0, NOPILL);
    break;
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLPlaceMine)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvcldamage(int player) {
  struct CLDamage *cldamage;
  int pill, base, i;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLDamage)) FAIL(EAGAIN)
  cldamage = (struct CLDamage *)server.players[player].recvbuf.ptr;

  if ((pill = findpill(cldamage->x, cldamage->y)) != -1) {
    if (server.pills[pill].armour > 0) {
      /* heat pill */
      server.pills[pill].armour--;
      server.pills[pill].speed /= 2;
      server.pills[pill].speed = MAX(server.pills[pill].speed, MINTICKSPERSHOT);
      server.pills[pill].counter = 0;
    }

    sendsrdamage(player, cldamage->x, cldamage->y);
  }
  else {
    if ((base = findbase(cldamage->x, cldamage->y)) != -1) {
      if (server.bases[base].armour >= MINBASEARMOUR) {
        server.bases[base].armour -= MINBASEARMOUR;
        server.bases[base].counter = 0;

        /* heat pills */
        for (i = 0; i < server.npills; i++) {
          if (
            mag2f(sub2f(make2f(server.pills[i].x + 0.5, server.pills[i].y + 0.5), make2f(server.bases[base].x + 0.5, server.bases[base].y + 0.5))) <= 8.0 &&
            server.pills[i].owner != NEUTRAL && server.bases[base].owner != NEUTRAL &&
            (
              (server.players[server.pills[i].owner].alliance & (1 << server.bases[base].owner)) &&
              (server.players[server.bases[base].owner].alliance & (1 << server.pills[i].owner))
            )
          ) {
            server.pills[i].speed /= 2;
            server.pills[pill].speed = MAX(server.pills[pill].speed, MINTICKSPERSHOT);
            server.pills[i].counter = 0;
          }
        }
      }

      sendsrdamage(player, cldamage->x, cldamage->y);
    }
    else {
      if (cldamage->boat) {
        switch (server.terrain[cldamage->y][cldamage->x]) {
        case kBoatTerrain:
          server.terrain[cldamage->y][cldamage->x] = kRiverTerrain;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kWallTerrain:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kSwampTerrain0:
          server.terrain[cldamage->y][cldamage->x] = kRiverTerrain;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kSwampTerrain1:
          server.terrain[cldamage->y][cldamage->x] = kSwampTerrain0;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kSwampTerrain2:
          server.terrain[cldamage->y][cldamage->x] = kSwampTerrain1;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kSwampTerrain3:
          server.terrain[cldamage->y][cldamage->x] = kSwampTerrain2;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kRoadTerrain:
          if (
            (isWaterLikeTerrain(server.terrain[cldamage->y][cldamage->x - 1]) && isWaterLikeTerrain(server.terrain[cldamage->y][cldamage->x + 1])) ||
            (isWaterLikeTerrain(server.terrain[cldamage->y - 1][cldamage->x]) && isWaterLikeTerrain(server.terrain[cldamage->y + 1][cldamage->x]))
          ) {
            server.terrain[cldamage->y][cldamage->x] = kRiverTerrain;
          }
          sendsrdamage(player, cldamage->x, cldamage->y);

          break;

        case kForestTerrain:
          server.terrain[cldamage->y][cldamage->x] = kGrassTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kRubbleTerrain0:
          server.terrain[cldamage->y][cldamage->x] = kRiverTerrain;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kRubbleTerrain1:
          server.terrain[cldamage->y][cldamage->x] = kRubbleTerrain0;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kRubbleTerrain2:
          server.terrain[cldamage->y][cldamage->x] = kRubbleTerrain1;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kRubbleTerrain3:
          server.terrain[cldamage->y][cldamage->x] = kRubbleTerrain2;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kGrassTerrain0:
          server.terrain[cldamage->y][cldamage->x] = kSwampTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kGrassTerrain1:
          server.terrain[cldamage->y][cldamage->x] = kGrassTerrain0;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kGrassTerrain2:
          server.terrain[cldamage->y][cldamage->x] = kGrassTerrain1;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kGrassTerrain3:
          server.terrain[cldamage->y][cldamage->x] = kGrassTerrain2;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain0:
          server.terrain[cldamage->y][cldamage->x] = kRubbleTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain1:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain0;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain2:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain1;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain3:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain2;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kMinedSeaTerrain:
        case kMinedSwampTerrain:
        case kMinedCraterTerrain:
        case kMinedRoadTerrain:
        case kMinedForestTerrain:
        case kMinedRubbleTerrain:
        case kMinedGrassTerrain:
          if (explosionat(player, cldamage->x, cldamage->y)) LOGFAIL(errno)
          break;

        default:
          break;
        }
      }
      else {
        switch (server.terrain[cldamage->y][cldamage->x]) {
        case kBoatTerrain:
          server.terrain[cldamage->y][cldamage->x] = kRiverTerrain;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kWallTerrain:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kForestTerrain:
          server.terrain[cldamage->y][cldamage->x] = kGrassTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain0:
          server.terrain[cldamage->y][cldamage->x] = kRubbleTerrain3;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain1:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain0;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain2:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain1;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kDamagedWallTerrain3:
          server.terrain[cldamage->y][cldamage->x] = kDamagedWallTerrain2;
          sendsrdamage(player, cldamage->x, cldamage->y);
          break;

        case kMinedSeaTerrain:
        case kMinedSwampTerrain:
        case kMinedCraterTerrain:
        case kMinedRoadTerrain:
        case kMinedForestTerrain:
        case kMinedRubbleTerrain:
        case kMinedGrassTerrain:
          if (explosionat(player, cldamage->x, cldamage->y)) LOGFAIL(errno)
          break;

        default:
          break;
        }
      }
    }
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLDamage)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclsmallboom(int player) {
  struct CLSmallBoom *clsmallboom;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLSmallBoom)) FAIL(EAGAIN)
  clsmallboom = (struct CLSmallBoom *)server.players[player].recvbuf.ptr;

  if (explosionat(player, clsmallboom->x, clsmallboom->y)) LOGFAIL(errno)

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLSmallBoom)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclsuperboom(int player) {
  struct CLSuperBoom *clsuperboom;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLSuperBoom)) FAIL(EAGAIN)
  clsuperboom = (struct CLSuperBoom *)server.players[player].recvbuf.ptr;

  if (superboomat(player, clsuperboom->x, clsuperboom->y)) LOGFAIL(errno)

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLSuperBoom)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclrefuel(int player) {
  struct CLRefuel *clrefuel;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLRefuel)) FAIL(EAGAIN)
  clrefuel = (struct CLRefuel *)server.players[player].recvbuf.ptr;

  if (clrefuel->base < server.nbases) {
    server.bases[clrefuel->base].armour -= clrefuel->armour;
    server.bases[clrefuel->base].shells -= clrefuel->shells;
    server.bases[clrefuel->base].mines -= clrefuel->mines;
    sendsrrefuel(player, clrefuel->base, clrefuel->armour, clrefuel->shells, clrefuel->mines);
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLRefuel)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclhittank(int player) {
  struct CLHitTank *clhittank;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLHitTank)) FAIL(EAGAIN)
  clhittank = (struct CLHitTank *)server.players[player].recvbuf.ptr;

  if (clhittank->player < MAXPLAYERS) {
    sendsrhittank(clhittank->player, clhittank->dir);
  }

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLHitTank)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvclsetalliance(int player) {
  struct CLSetAlliance *clsettalliance;

  assert(player >= 0);
  assert(player < MAXPLAYERS);

TRY
  if (server.players[player].recvbuf.nbytes < sizeof(struct CLSetAlliance)) FAIL(EAGAIN)
  clsettalliance = (struct CLSetAlliance *)server.players[player].recvbuf.ptr;

  /* convert byte order*/
  clsettalliance->alliance = ntohs(clsettalliance->alliance);

  server.players[player].alliance = clsettalliance->alliance;
  sendsrsetalliance(player, clsettalliance->alliance);

  /* clear buffer of read data */
  if (readbuf(&server.players[player].recvbuf, NULL, sizeof(struct CLSetAlliance)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}

