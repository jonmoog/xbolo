/*
 *  client_recv.c
 *  XBolo
 *
 *  Handlers for server->client protocol messages.
 */

#include "client_internal.h"


int recvsrpause() {
  struct SRPause *srpause;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPause)) FAIL(EAGAIN)
  srpause = (struct SRPause *)client.recvbuf.ptr;

  if (srpause->pause == 255) {
    client.pause = -1;
  }
  else {
    client.pause = srpause->pause;
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPause)) == -1) FAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrsendmesg() {
  struct SRSendMesg *srsendmesg;
  char *messagetext = NULL;
  int i;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRSendMesg)) FAIL(EAGAIN)

  /* check for complete string following struct */
  for (i = sizeof(struct SRSendMesg) ; i < client.recvbuf.nbytes; i++) {
    if (((char *)client.recvbuf.ptr)[i] == '\0') {
      break;
    }
  }

  /* buf does not have the whole string */
  if (!(i < client.recvbuf.nbytes)) FAIL(EAGAIN)

  srsendmesg = (struct SRSendMesg *)client.recvbuf.ptr;

  /* print the message on the client side */
  if (client.printmessage) {
    if (asprintf(&messagetext, "%s: %s", client.players[srsendmesg->player].name, (char *)(srsendmesg + 1)) == -1) LOGFAIL(errno)
    client.printmessage(srsendmesg->to, messagetext);
    free(messagetext);
    messagetext = NULL;
  }

  /* clear buffer of read data */
  if (readbuf(&client.recvbuf, NULL, i + 1) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrdamage() {
  struct SRDamage *srdamage;
  int pill, base;
  struct Explosion *explosion;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRDamage)) FAIL(EAGAIN)
  srdamage = (struct SRDamage *)client.recvbuf.ptr;

  if ((pill = clientfindpill(srdamage->x, srdamage->y)) != -1) {
    /* damage pill */
    if (client.pills[pill].armour > 0) {
      client.pills[pill].armour--;

      /* fog of war */
      if (client.pills[pill].armour == 0) {
        if (testalliance(client.pills[pill].owner, client.player)) {
          if (refresh(srdamage->x, srdamage->y)) LOGFAIL(errno)
          if (decreasevis(makerect(client.pills[pill].x - 7, client.pills[pill].y - 7, 15, 15))) LOGFAIL(errno)
        }

        if (client.setpillstatus) {
          client.setpillstatus(pill);
        }
      }

      /* heat pill */
      client.pills[pill].speed /= 2;
      client.pills[pill].speed = MAX(client.pills[pill].speed, MINTICKSPERSHOT);
    }
  }
  else if ((base = clientfindbase(srdamage->x, srdamage->y)) != -1) {
    /* damage base */
    if (client.bases[base].armour >= MINBASEARMOUR) {
      client.bases[base].armour -= MINBASEARMOUR;

      /* heat pills */
      for (pill = 0; pill < client.npills; pill++) {
        if (
          mag2f(sub2f(make2f(client.pills[pill].x + 0.5, client.pills[pill].y + 0.5), make2f(client.bases[base].x + 0.5, client.bases[base].y + 0.5))) <= 8.0 &&
          client.pills[pill].owner != NEUTRAL && client.bases[base].owner != NEUTRAL &&
          testalliance(client.pills[pill].owner, client.bases[base].owner)
        ) {
          client.pills[pill].speed /= 2;
          client.pills[pill].speed = MAX(client.pills[pill].speed, MINTICKSPERSHOT);
        }
      }

      if (client.setbasestatus) {
        client.setbasestatus(base);
      }
    }
  }

  /* play sound */
  if (client.playsound) {
    if (client.terrain[srdamage->y][srdamage->x] == kForestTerrain) {
      if (client.fog[srdamage->y][srdamage->x] > 0) {
        client.playsound(kHitTreeSound);
      }
      else {
        client.playsound(kFarHitTreeSound);
      }
    }
    else {
      if (client.fog[srdamage->y][srdamage->x] > 0) {
        client.playsound(kHitTerrainSound);
      }
      else {
        client.playsound(kFarHitTerrainSound);
      }
    }
  }

  client.terrain[srdamage->y][srdamage->x] = srdamage->terrain;

  /* create explosion */
  if (srdamage->player != client.player) {
    explosion = (void *)malloc(sizeof(struct Explosion));
    if (explosion == NULL) LOGFAIL(errno)
    explosion->point.x = srdamage->x + 0.5;
    explosion->point.y = srdamage->y + 0.5;
    explosion->counter = 0;
    addlist(&client.explosions, explosion);
    if (killsquarebuilder(makepoint(srdamage->x, srdamage->y))) LOGFAIL(errno)
  }

  /* refresh seen tiles */
  if (refresh(srdamage->x, srdamage->y)) LOGFAIL(errno)

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRDamage)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrgrabtrees() {
  struct SRGrabTrees *srgrabtrees;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRGrabTrees)) FAIL(EAGAIN)
  srgrabtrees = (struct SRGrabTrees *)client.recvbuf.ptr;

  if (client.terrain[srgrabtrees->y][srgrabtrees->x] == kMinedForestTerrain) {
    client.terrain[srgrabtrees->y][srgrabtrees->x] = kMinedGrassTerrain;
  }
  else {
    client.terrain[srgrabtrees->y][srgrabtrees->x] = kGrassTerrain0;
  }

  if (refresh(srgrabtrees->x, srgrabtrees->y) == -1) LOGFAIL(errno)

  if (client.playsound) {
    if (client.fog[srgrabtrees->y][srgrabtrees->x] > 0) {
      client.playsound(kTreeSound);
    }
    else {
      client.playsound(kFarTreeSound);
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRGrabTrees)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrbuild() {
  struct SRBuild *srbuild;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRBuild)) FAIL(EAGAIN)
  srbuild = (struct SRBuild *)client.recvbuf.ptr;

  client.terrain[srbuild->y][srbuild->x] = srbuild->terrain;
  if (refresh(srbuild->x, srbuild->y) == -1) LOGFAIL(errno)

  if (client.playsound) {
    if (client.fog[srbuild->y][srbuild->x] > 0) {
      client.playsound(kBuildSound);
    }
    else {
      client.playsound(kFarBuildSound);
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRBuild)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrgrow() {
  struct SRGrow *srgrow;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRGrow)) FAIL(EAGAIN)
  srgrow = (struct SRGrow *)client.recvbuf.ptr;

  switch (client.terrain[srgrow->y][srgrow->x]) {
    case kGrassTerrain0:
    case kGrassTerrain1:
    case kGrassTerrain2:
    case kGrassTerrain3:
    case kRubbleTerrain0:
    case kRubbleTerrain1:
    case kRubbleTerrain2:
    case kRubbleTerrain3:
    case kCraterTerrain:
    case kSwampTerrain0:
    case kSwampTerrain1:
    case kSwampTerrain2:
    case kSwampTerrain3:
    case kRoadTerrain:
      client.terrain[srgrow->y][srgrow->x] = kForestTerrain;
      if (refresh(srgrow->x, srgrow->y)) LOGFAIL(errno)
      break;

    case kMinedGrassTerrain:
    case kMinedRubbleTerrain:
    case kMinedCraterTerrain:
    case kMinedSwampTerrain:
    case kMinedRoadTerrain:
      client.terrain[srgrow->y][srgrow->x] = kMinedForestTerrain;
      if (refresh(srgrow->x, srgrow->y)) LOGFAIL(errno)
      break;

    default:
      break;
    }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRGrow)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrflood() {
  struct SRFlood *srflood;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRFlood)) FAIL(EAGAIN)
  srflood = (struct SRFlood *)client.recvbuf.ptr;

  client.terrain[srflood->y][srflood->x] = kRiverTerrain;
  if (refresh(srflood->x, srflood->y) == -1) LOGFAIL(errno)

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRFlood)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrplacemine() {
  struct SRPlaceMine *srplacemine;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlaceMine)) FAIL(EAGAIN)
  srplacemine = (struct SRPlaceMine *)client.recvbuf.ptr;

  switch (client.terrain[srplacemine->y][srplacemine->x]) {
	case kSwampTerrain0:
	case kSwampTerrain1:
	case kSwampTerrain2:
	case kSwampTerrain3:
    client.terrain[srplacemine->y][srplacemine->x] = kMinedSwampTerrain;
    break;

	case kCraterTerrain:
    client.terrain[srplacemine->y][srplacemine->x] = kMinedCraterTerrain;
    break;

	case kRoadTerrain:
    client.terrain[srplacemine->y][srplacemine->x] = kMinedRoadTerrain;
    break;

	case kForestTerrain:
    client.terrain[srplacemine->y][srplacemine->x] = kMinedForestTerrain;
    break;

	case kRubbleTerrain0:
	case kRubbleTerrain1:
	case kRubbleTerrain2:
	case kRubbleTerrain3:
    client.terrain[srplacemine->y][srplacemine->x] = kMinedRubbleTerrain;
    break;

	case kGrassTerrain0:
	case kGrassTerrain1:
	case kGrassTerrain2:
	case kGrassTerrain3:
    client.terrain[srplacemine->y][srplacemine->x] = kMinedGrassTerrain;
    break;

	case kSeaTerrain:
	case kBoatTerrain:
	case kWallTerrain:
	case kRiverTerrain:
	case kDamagedWallTerrain0:
	case kDamagedWallTerrain1:
	case kDamagedWallTerrain2:
	case kDamagedWallTerrain3:
	case kMinedSeaTerrain:
	case kMinedSwampTerrain:
	case kMinedCraterTerrain:
	case kMinedRoadTerrain:
	case kMinedForestTerrain:
	case kMinedRubbleTerrain:
	case kMinedGrassTerrain:
    break;

  default:
    break;
  }

  if (client.playsound && client.fog[srplacemine->y][srplacemine->x] > 0) {
    client.playsound(kMineSound);
  }

  if (!client.hiddenmines || testalliance(client.player, srplacemine->player)) {
    if (refresh(srplacemine->x, srplacemine->y) == -1) LOGFAIL(errno)
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlaceMine)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrdropmine() {
  struct SRDropMine *srdropmine;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRDropMine)) FAIL(EAGAIN)
  srdropmine = (struct SRDropMine *)client.recvbuf.ptr;

  switch (client.terrain[srdropmine->y][srdropmine->x]) {
	case kSwampTerrain0:
	case kSwampTerrain1:
	case kSwampTerrain2:
	case kSwampTerrain3:
    client.terrain[srdropmine->y][srdropmine->x] = kMinedSwampTerrain;
    break;

	case kCraterTerrain:
    client.terrain[srdropmine->y][srdropmine->x] = kMinedCraterTerrain;
    break;

	case kRoadTerrain:
    client.terrain[srdropmine->y][srdropmine->x] = kMinedRoadTerrain;
    break;

	case kForestTerrain:
    client.terrain[srdropmine->y][srdropmine->x] = kMinedForestTerrain;
    break;

	case kRubbleTerrain0:
	case kRubbleTerrain1:
	case kRubbleTerrain2:
	case kRubbleTerrain3:
    client.terrain[srdropmine->y][srdropmine->x] = kMinedRubbleTerrain;
    break;

	case kGrassTerrain0:
	case kGrassTerrain1:
	case kGrassTerrain2:
	case kGrassTerrain3:
    client.terrain[srdropmine->y][srdropmine->x] = kMinedGrassTerrain;
    break;

	case kSeaTerrain:
	case kBoatTerrain:
	case kWallTerrain:
	case kRiverTerrain:
	case kDamagedWallTerrain0:
	case kDamagedWallTerrain1:
	case kDamagedWallTerrain2:
	case kDamagedWallTerrain3:
	case kMinedSeaTerrain:
	case kMinedSwampTerrain:
	case kMinedCraterTerrain:
	case kMinedRoadTerrain:
	case kMinedForestTerrain:
	case kMinedRubbleTerrain:
	case kMinedGrassTerrain:
    break;

  default:
    break;
  }

  if (client.playsound && client.fog[srdropmine->y][srdropmine->x] > 0) {
    client.playsound(kMineSound);
  }

  if (refresh(srdropmine->x, srdropmine->y) == -1) LOGFAIL(errno)

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRDropMine)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrdropboat() {
  struct SRDropBoat *srdropboat;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRDropBoat)) FAIL(EAGAIN)
  srdropboat = (struct SRDropBoat *)client.recvbuf.ptr;

  switch (client.terrain[srdropboat->y][srdropboat->x]) {
	case kRiverTerrain:
    client.terrain[srdropboat->y][srdropboat->x] = kBoatTerrain;
    break;

	case kSeaTerrain:
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
	case kBoatTerrain:
	case kWallTerrain:
	case kDamagedWallTerrain0:
	case kDamagedWallTerrain1:
	case kDamagedWallTerrain2:
	case kDamagedWallTerrain3:
	case kMinedSeaTerrain:
	case kMinedSwampTerrain:
	case kMinedCraterTerrain:
	case kMinedRoadTerrain:
	case kMinedForestTerrain:
	case kMinedRubbleTerrain:
	case kMinedGrassTerrain:
    break;

  default:
    break;
  }

  if (refresh(srdropboat->x, srdropboat->y) == -1) LOGFAIL(errno)

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRDropBoat)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrplayerjoin() {
  struct SRPlayerJoin *srplayerjoin;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlayerJoin)) FAIL(EAGAIN)
  srplayerjoin = (struct SRPlayerJoin *)client.recvbuf.ptr;

  client.players[srplayerjoin->player].used = 1;
  client.players[srplayerjoin->player].connected = 1;
  client.players[srplayerjoin->player].seq = 0;
  client.players[srplayerjoin->player].lastupdate = client.players[client.player].seq;
  strncpy(client.players[srplayerjoin->player].name, srplayerjoin->name, sizeof(client.players[srplayerjoin->player].name) - 1);
  strncpy(client.players[srplayerjoin->player].host, srplayerjoin->host, sizeof(client.players[srplayerjoin->player].host) - 1);

  client.players[srplayerjoin->player].alliance = 1 << srplayerjoin->player;

  if (client.printmessage) {
    if (asprintf(&text, "%s joined", srplayerjoin->name) == -1) LOGFAIL(errno)
    client.printmessage(MSGGAME, text);
    free(text);
    text = NULL;
  }

  if (client.setplayerstatus) {
    client.setplayerstatus(srplayerjoin->player);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlayerJoin)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrplayerrejoin() {
  struct SRPlayerRejoin *srplayerrejoin;
  int i;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlayerRejoin)) FAIL(EAGAIN)
  srplayerrejoin = (struct SRPlayerRejoin *)client.recvbuf.ptr;

  client.players[srplayerrejoin->player].connected = 1;
  client.players[srplayerrejoin->player].seq = 0;
  client.players[srplayerrejoin->player].lastupdate = client.players[client.player].seq;

  if (client.player == srplayerrejoin->player) {
    for (i = 0; i < client.npills; i++) {
      if (testalliance(client.player, client.pills[i].owner)) {
        if (client.pills[i].armour != ONBOARD && client.pills[i].armour > 0) {
          if (increasevis(makerect(client.pills[i].x - 7, client.pills[i].y - 7, 15, 15))) LOGFAIL(errno)
        }

        if (client.setpillstatus) {
          client.setpillstatus(i);
        }
      }
    }
  }

  if (client.printmessage) {
    if (asprintf(&text, "%s rejoined", client.players[srplayerrejoin->player].name) == -1) LOGFAIL(errno)
    client.printmessage(MSGGAME, text);
    free(text);
    text = NULL;
  }

  if (client.setplayerstatus) {
    client.setplayerstatus(srplayerrejoin->player);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlayerRejoin)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrplayerexit() {
  struct SRPlayerExit *srplayerexit;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlayerExit)) FAIL(EAGAIN)
  srplayerexit = (struct SRPlayerExit *)client.recvbuf.ptr;

  if (client.printmessage) {
    if (asprintf(&text, "%s left", client.players[srplayerexit->player].name) == -1) LOGFAIL(errno)
    client.printmessage(MSGGAME, text);
    free(text);
    text = NULL;
  }

  if (client.players[srplayerexit->player].seq != 0 && testalliance(client.player, srplayerexit->player)) {
    if (decreasevis(makerect(((int)client.players[srplayerexit->player].tank.x) - 14, ((int)client.players[srplayerexit->player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
  }

  client.players[srplayerexit->player].connected = 0;
  client.players[srplayerexit->player].seq = 0;

  if (client.setplayerstatus) {
    client.setplayerstatus(srplayerexit->player);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlayerExit)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrplayerdisc() {
  struct SRPlayerDisc *srplayerdisc;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlayerDisc)) FAIL(EAGAIN)
  srplayerdisc = (struct SRPlayerDisc *)client.recvbuf.ptr;

  if (client.printmessage) {
    if (asprintf(&text, "%s disconnected", client.players[srplayerdisc->player].name) == -1) LOGFAIL(errno)
    client.printmessage(MSGGAME, text);
    free(text);
    text = NULL;
  }

  if (srplayerdisc->player != client.player && client.players[srplayerdisc->player].seq != 0 && testalliance(client.player, srplayerdisc->player)) {
    if (decreasevis(makerect(((int)client.players[srplayerdisc->player].tank.x) - 14, ((int)client.players[srplayerdisc->player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
  }

  client.players[srplayerdisc->player].connected = 0;
  client.players[srplayerdisc->player].seq = 0;

  if (client.setplayerstatus) {
    client.setplayerstatus(srplayerdisc->player);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlayerDisc)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrplayerkick() {
  struct SRPlayerKick *srplayerkick;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlayerKick)) FAIL(EAGAIN)
  srplayerkick = (struct SRPlayerKick *)client.recvbuf.ptr;

  if (client.printmessage) {
    if (asprintf(&text, "%s kicked", client.players[srplayerkick->player].name) == -1) LOGFAIL(errno)
    client.printmessage(MSGGAME, text);
    free(text);
    text = NULL;
  }

  if (srplayerkick->player != client.player && client.players[srplayerkick->player].seq != 0 && testalliance(client.player, srplayerkick->player)) {
    if (decreasevis(makerect(((int)client.players[srplayerkick->player].tank.x) - 14, ((int)client.players[srplayerkick->player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
  }

  client.players[srplayerkick->player].connected = 0;
  client.players[srplayerkick->player].seq = 0;

  if (client.setplayerstatus) {
    client.setplayerstatus(srplayerkick->player);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlayerKick)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrplayerban() {
  struct SRPlayerBan *srplayerban;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRPlayerBan)) FAIL(EAGAIN)
  srplayerban = (struct SRPlayerBan *)client.recvbuf.ptr;

  if (client.printmessage) {
    if (asprintf(&text, "%s banned", client.players[srplayerban->player].name) == -1) LOGFAIL(errno)
    client.printmessage(MSGGAME, text);
    free(text);
    text = NULL;
  }

  if (srplayerban->player != client.player && client.players[srplayerban->player].seq != 0 && testalliance(client.player, srplayerban->player)) {
    if (decreasevis(makerect(((int)client.players[srplayerban->player].tank.x) - 14, ((int)client.players[srplayerban->player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
  }

  client.players[srplayerban->player].connected = 0;
  client.players[srplayerban->player].seq = 0;

  if (client.setplayerstatus) {
    client.setplayerstatus(srplayerban->player);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRPlayerBan)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrrepairpill() {
  struct SRRepairPill *srrepairpill;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRRepairPill)) FAIL(EAGAIN)
  srrepairpill = (struct SRRepairPill *)client.recvbuf.ptr;

  /* update the fog of war and status */
  if (
    testalliance(client.pills[srrepairpill->pill].owner, client.player) &&
    client.pills[srrepairpill->pill].armour == 0 && srrepairpill->armour != 0 && srrepairpill->armour != ONBOARD
  ) {
    if (increasevis(makerect(client.pills[srrepairpill->pill].x - 7, client.pills[srrepairpill->pill].y - 7, 15, 15))) LOGFAIL(errno)
  }

  client.pills[srrepairpill->pill].armour = srrepairpill->armour;

  /* refresh seen tiles */
  if (refresh(client.pills[srrepairpill->pill].x, client.pills[srrepairpill->pill].y) == -1) LOGFAIL(errno)

  if (client.playsound) {
    if (client.fog[client.pills[srrepairpill->pill].y][client.pills[srrepairpill->pill].x] > 0) {
      client.playsound(kBuildSound);
    }
    else {
      client.playsound(kFarBuildSound);
    }
  }

  /* update pill status */
  if (client.setpillstatus) {
    client.setpillstatus(srrepairpill->pill);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRRepairPill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrcoolpill() {
  struct SRCoolPill *srcoolpill;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRCoolPill)) FAIL(EAGAIN)
  srcoolpill = (struct SRCoolPill *)client.recvbuf.ptr;

  /* cool pill */
  client.pills[srcoolpill->pill].speed++;

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRCoolPill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrcapturepill() {
  struct SRCapturePill *srcapturepill;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRCapturePill)) FAIL(EAGAIN)
  srcapturepill = (struct SRCapturePill *)client.recvbuf.ptr;

  if (client.printmessage && client.pills[srcapturepill->pill].owner != srcapturepill->owner) {
    if (client.pills[srcapturepill->pill].owner == NEUTRAL) {
      if (asprintf(&text, "%s captured neutral pill %d", client.players[srcapturepill->owner].name, srcapturepill->pill) == -1) LOGFAIL(errno)
      client.printmessage(MSGGAME, text);
      free(text);
      text = NULL;
    }
    else {
      if (asprintf(&text, "%s captured pill %d from %s", client.players[srcapturepill->owner].name, srcapturepill->pill, client.players[client.pills[srcapturepill->pill].owner].name) == -1) LOGFAIL(errno)
      client.printmessage(MSGGAME, text);
      free(text);
      text = NULL;
    }
  }

  client.pills[srcapturepill->pill].owner = srcapturepill->owner;
  client.pills[srcapturepill->pill].armour = ONBOARD;
  client.pills[srcapturepill->pill].speed = MAXTICKSPERSHOT;

  if (client.pills[srcapturepill->pill].x == (int)client.players[client.player].tank.x && client.pills[srcapturepill->pill].y == (int)client.players[client.player].tank.y) {
    switch (client.terrain[(int)client.players[client.player].tank.y][(int)client.players[client.player].tank.x]) {
    case kSeaTerrain:  /* drown */
      if (!client.players[client.player].boat) {
        if (drown()) LOGFAIL(errno)
      }

      break;

    case kBoatTerrain:  /* grab boat */
      if (sendclgrabtile(client.pills[srcapturepill->pill].x, client.pills[srcapturepill->pill].y)) LOGFAIL(errno)
      break;

    case kWallTerrain:
    case kDamagedWallTerrain0:
    case kDamagedWallTerrain1:
    case kDamagedWallTerrain2:
    case kDamagedWallTerrain3:
      if (superboom()) LOGFAIL(errno)
      break;

    case kRiverTerrain:
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
      /* do nothing */
      break;

    case kMinedSeaTerrain:
      if (drown()) LOGFAIL(errno)
      if (sendclgrabtile(client.pills[srcapturepill->pill].x, client.pills[srcapturepill->pill].y)) LOGFAIL(errno)
      break;

    case kMinedSwampTerrain:
    case kMinedCraterTerrain:
    case kMinedRoadTerrain:
    case kMinedForestTerrain:
    case kMinedRubbleTerrain:
    case kMinedGrassTerrain:
      /* explode mine */
      if (sendclgrabtile(client.pills[srcapturepill->pill].x, client.pills[srcapturepill->pill].y)) LOGFAIL(errno)
      break;

    default:
      assert(0);
    }
  }

  /* refresh tiles */
  if (refresh(client.pills[srcapturepill->pill].x, client.pills[srcapturepill->pill].y) == -1) LOGFAIL(errno)

  if (client.setpillstatus) {
    client.setpillstatus(srcapturepill->pill);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRCapturePill)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrbuildpill() {
  struct SRBuildPill *srbuildpill;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRBuildPill)) FAIL(EAGAIN)
  srbuildpill = (struct SRBuildPill *)client.recvbuf.ptr;

  client.pills[srbuildpill->pill].x = srbuildpill->x;
  client.pills[srbuildpill->pill].y = srbuildpill->y;
  client.pills[srbuildpill->pill].armour = srbuildpill->armour;
  client.pills[srbuildpill->pill].speed = MAXTICKSPERSHOT;

  /* refresh seen tiles */
  if (refresh(srbuildpill->x, srbuildpill->y) == -1) LOGFAIL(errno)

  /* play sound */
  if (client.playsound) {
    if (client.fog[srbuildpill->y][srbuildpill->x] > 0) {
      client.playsound(kBuildSound);
    }
    else {
      client.playsound(kFarBuildSound);
    }
  }

  /* fog of war */
  if (testalliance(client.pills[srbuildpill->pill].owner, client.player) && srbuildpill->armour != 0 && srbuildpill->armour != ONBOARD) {
    if (increasevis(makerect(client.pills[srbuildpill->pill].x - 7, client.pills[srbuildpill->pill].y - 7, 15, 15))) LOGFAIL(errno)
  }

  /* update and status */
  if (client.setpillstatus) {
    client.setpillstatus(srbuildpill->pill);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRBuildPill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrdroppill() {
  struct SRDropPill *srdroppill;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRDropPill)) FAIL(EAGAIN)
  srdroppill = (struct SRDropPill *)client.recvbuf.ptr;

  client.pills[srdroppill->pill].x = srdroppill->x;
  client.pills[srdroppill->pill].y = srdroppill->y;
  client.pills[srdroppill->pill].armour = 0;
  client.pills[srdroppill->pill].speed = MAXTICKSPERSHOT;

  /* refresh seen tiles */
  if (refresh(srdroppill->x, srdroppill->y) == -1) LOGFAIL(errno)

  /* update pill status */
  if (client.setpillstatus) {
    client.setpillstatus(srdroppill->pill);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRDropPill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrreplenishbase() {
  struct SRReplenishBase *srreplenishbase;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRReplenishBase)) FAIL(EAGAIN)
  srreplenishbase = (struct SRReplenishBase *)client.recvbuf.ptr;

  if (++client.bases[srreplenishbase->base].armour > MAXBASEARMOUR) {
    client.bases[srreplenishbase->base].armour = MAXBASEARMOUR;
  }

  if (++client.bases[srreplenishbase->base].shells > MAXBASESHELLS) {
    client.bases[srreplenishbase->base].shells = MAXBASESHELLS;
  }

  if (++client.bases[srreplenishbase->base].mines > MAXBASEMINES) {
    client.bases[srreplenishbase->base].mines = MAXBASEMINES;
  }

  if (client.setbasestatus) {
    client.setbasestatus(srreplenishbase->base);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRReplenishBase)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrcapturebase() {
  struct SRCaptureBase *srcapturebase;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRCaptureBase)) FAIL(EAGAIN)
  srcapturebase = (struct SRCaptureBase *)client.recvbuf.ptr;

  if (client.bases[srcapturebase->base].owner == NEUTRAL) {
    if (client.printmessage) {
      if (asprintf(&text, "%s captured neutral base %d", client.players[srcapturebase->owner].name, srcapturebase->base) == -1) LOGFAIL(errno)
      client.printmessage(MSGGAME, text);
      free(text);
      text = NULL;
    }

    client.bases[srcapturebase->base].shells = MAXBASESHELLS;
    client.bases[srcapturebase->base].armour = MAXBASEARMOUR;
    client.bases[srcapturebase->base].mines = MAXBASEMINES;
  }
  else {
    if (client.printmessage) {
      if (asprintf(&text, "%s captured base %d from %s", client.players[srcapturebase->owner].name, srcapturebase->base, client.players[client.bases[srcapturebase->base].owner].name) == -1) LOGFAIL(errno)
      client.printmessage(MSGGAME, text);
      free(text);
      text = NULL;
    }

    client.bases[srcapturebase->base].shells = 0;
    client.bases[srcapturebase->base].armour = 0;
    client.bases[srcapturebase->base].mines = 0;
  }

  client.bases[srcapturebase->base].owner = srcapturebase->owner;

  if (refresh(client.bases[srcapturebase->base].x, client.bases[srcapturebase->base].y) == -1) LOGFAIL(errno)

  if (client.setbasestatus) {
    client.setbasestatus(srcapturebase->base);
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRCaptureBase)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrrefuel() {
  struct SRRefuel *srrefuel;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRRefuel)) FAIL(EAGAIN)
  srrefuel = (struct SRRefuel *)client.recvbuf.ptr;

  assert(srrefuel->base < client.nbases);

  client.bases[srrefuel->base].armour -= srrefuel->armour;
  client.bases[srrefuel->base].shells -= srrefuel->shells;
  client.bases[srrefuel->base].mines -= srrefuel->mines;

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRRefuel)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrgrabboat() {
  struct SRGrabBoat *srgrabboat;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRGrabBoat)) FAIL(EAGAIN)
  srgrabboat = (struct SRGrabBoat *)client.recvbuf.ptr;

  if (srgrabboat->player == client.player) {
    client.players[client.player].boat = 1;
  }

  if (client.terrain[srgrabboat->y][srgrabboat->x] == kBoatTerrain) {
    client.terrain[srgrabboat->y][srgrabboat->x] = kRiverTerrain;
    if (refresh(srgrabboat->x, srgrabboat->y)) LOGFAIL(errno)
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRGrabBoat)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrmineack() {
  struct SRMineAck *srmineack;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRMineAck)) FAIL(EAGAIN)
  srmineack = (struct SRMineAck *)client.recvbuf.ptr;

  if (!srmineack->success) {
    client.mines++;
  }

  if (client.settankstatus) {
    client.settankstatus();
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRMineAck)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrbuilderack() {
  struct SRBuilderAck *srbuilderack;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRBuilderAck)) FAIL(EAGAIN)
  srbuilderack = (struct SRBuilderAck *)client.recvbuf.ptr;

  switch (client.players[client.player].builderstatus) {
  case kBuilderWork:
    switch (client.buildertask) {
    case kBuilderGetTree:
      client.buildertrees = srbuilderack->trees;
      client.players[client.player].builderstatus = kBuilderWait;
      client.players[client.player].builderwait = 0;
      break;

    case kBuilderBuildRoad:
    case kBuilderBuildWall:
    case kBuilderBuildBoat:
    case kBuilderRepairPill:
      client.buildertrees = srbuilderack->trees;
      client.players[client.player].builderstatus = kBuilderWait;
      client.players[client.player].builderwait = 0;
      break;
      
    case kBuilderBuildPill:
      client.buildertrees = srbuilderack->trees;
      client.builderpill = srbuilderack->pill;
      client.players[client.player].builderstatus = kBuilderWait;
      client.players[client.player].builderwait = 0;
      break;

    case kBuilderPlaceMine:
      client.buildermines = srbuilderack->mines;
      client.players[client.player].builderstatus = kBuilderWait;
      client.players[client.player].builderwait = 0;
      break;

    case kBuilderDoNothing:
    default:
      break;
    }

    break;

  case kBuilderReady:
  case kBuilderGoto:
  case kBuilderReturn:
  case kBuilderParachute:
  default:
    break;
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRBuilderAck)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrsmallboom() {
  struct SRSmallBoom *srsmallboom;
  struct Explosion *explosion = NULL;
  Vec2f point;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRSmallBoom)) FAIL(EAGAIN)
  srsmallboom = (struct SRSmallBoom *)client.recvbuf.ptr;

  /* turn terrain to crater */
  if (client.terrain[srsmallboom->y][srsmallboom->x] != kSeaTerrain && client.terrain[srsmallboom->y][srsmallboom->x] != kMinedSeaTerrain) {
    client.terrain[srsmallboom->y][srsmallboom->x] = kCraterTerrain;
    if (refresh(srsmallboom->x, srsmallboom->y)) LOGFAIL(errno)
  }

  point.x = srsmallboom->x + 0.5;
  point.y = srsmallboom->y + 0.5;

  /* create explosions */
  if (srsmallboom->player != client.player) {
    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(srsmallboom->x, srsmallboom->y))) LOGFAIL(errno)
  }

  /* check for damage to tank */
  if (!client.players[client.player].dead && mag2f(sub2f(client.players[client.player].tank, point)) <= 1.0) {
    client.armour -= 10;
    client.players[client.player].boat = 0;

    if (client.armour < 0) {
      client.armour = 0;
      if (client.mines > 32) {
        superboom();
      }
      else if (client.mines > 0 || client.shells > 0) {
        smallboom();
      }
      else {
        killtank();
      }
    }

    if (client.settankstatus) {
      client.settankstatus();
    }

    /* play sound */
    if (client.playsound) {
      client.playsound(kHitTankSound);
    }
  }

  /* play sound */
  if (client.playsound) {
    if (client.fog[srsmallboom->y][srsmallboom->x] > 0) {
      client.playsound(kExplosionSound);
    }
    else {
      client.playsound(kFarExplosionSound);
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRSmallBoom)) == -1) LOGFAIL(errno)

CLEANUP
  if (explosion) {
    free(explosion);
  }

ERRHANDLER(0, -1)
END
}


int recvsrsuperboom() {
  struct SRSuperBoom *srsuperboom;
  struct Explosion *explosion = NULL;
  Vec2f point;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRSuperBoom)) FAIL(EAGAIN)
  srsuperboom = (struct SRSuperBoom *)client.recvbuf.ptr;

  /* turn terrain to crater */
  if (client.terrain[srsuperboom->y][srsuperboom->x] != kSeaTerrain && client.terrain[srsuperboom->y][srsuperboom->x] != kMinedSeaTerrain) {
    client.terrain[srsuperboom->y][srsuperboom->x] = kCraterTerrain;
    if (refresh(srsuperboom->x, srsuperboom->y)) LOGFAIL(errno)
  }
  if (client.terrain[srsuperboom->y][srsuperboom->x + 1] != kSeaTerrain && client.terrain[srsuperboom->y][srsuperboom->x + 1] != kMinedSeaTerrain) {
    client.terrain[srsuperboom->y][srsuperboom->x + 1] = kCraterTerrain;
    if (refresh(srsuperboom->x + 1, srsuperboom->y)) LOGFAIL(errno)
  }
  if (client.terrain[srsuperboom->y + 1][srsuperboom->x] != kSeaTerrain && client.terrain[srsuperboom->y + 1][srsuperboom->x] != kMinedSeaTerrain) {
    client.terrain[srsuperboom->y + 1][srsuperboom->x] = kCraterTerrain;
    if (refresh(srsuperboom->x, srsuperboom->y + 1)) LOGFAIL(errno)
  }
  if (client.terrain[srsuperboom->y + 1][srsuperboom->x + 1] != kSeaTerrain && client.terrain[srsuperboom->y + 1][srsuperboom->x + 1] != kMinedSeaTerrain) {
    client.terrain[srsuperboom->y + 1][srsuperboom->x + 1] = kCraterTerrain;
    if (refresh(srsuperboom->x + 1, srsuperboom->y + 1)) LOGFAIL(errno)
  }

  /* create explosions */
  if (srsuperboom->player != client.player) {
    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = srsuperboom->x + 0.5;
    explosion->point.y = srsuperboom->y + 0.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(srsuperboom->x, srsuperboom->y))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = srsuperboom->x + 1.5;
    explosion->point.y = srsuperboom->y + 0.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(srsuperboom->x + 1, srsuperboom->y))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = srsuperboom->x + 0.5;
    explosion->point.y = srsuperboom->y + 1.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(srsuperboom->x, srsuperboom->y + 1))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = srsuperboom->x + 1.5;
    explosion->point.y = srsuperboom->y + 1.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(srsuperboom->x + 1, srsuperboom->y + 1))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = srsuperboom->x + 0.25;
    point.y = srsuperboom->y + 1.0;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = srsuperboom->x + 1.0;
    point.y = srsuperboom->y + 0.25;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = srsuperboom->x + 1.75;
    point.y = srsuperboom->y + 1.0;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = srsuperboom->x + 1.0;
    point.y = srsuperboom->y + 1.75;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = srsuperboom->x + 1.0;
    point.y = srsuperboom->y + 1.0;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    /* check for damage to tank */
    if (!client.players[client.player].dead && mag2f(sub2f(client.players[client.player].tank, make2f(srsuperboom->x + 1, srsuperboom->y + 1))) <= 1.5) {
      client.armour -= 20;
      client.players[client.player].boat = 0;

      if (client.armour < 0) {
        client.armour = 0;
        if (client.mines > 32) {
          superboom();
        }
        else if (client.mines > 0 || client.shells > 0) {
          smallboom();
        }
        else {
          killtank();
        }
      }

      if (client.settankstatus) {
        client.settankstatus();
      }

      if (client.playsound) {
        client.playsound(kHitTankSound);
      }
    }

    /* play sound */
    if (client.playsound) {
      if (client.fog[srsuperboom->y][srsuperboom->x] > 0) {
        client.playsound(kSuperBoomSound);
      }
      else {
        client.playsound(kFarSuperBoomSound);
      }
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRSuperBoom)) == -1) LOGFAIL(errno)

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (explosion) {
      free(explosion);
    }

    RETERR(-1)
  }
END
}


int recvsrhittank() {
  struct SRHitTank *srhittank;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRHitTank)) FAIL(EAGAIN)
  srhittank = (struct SRHitTank *)client.recvbuf.ptr;

  srhittank->dir = ntohl(srhittank->dir);

  client.players[client.player].boat = 0;
  client.players[client.player].kickdir = unpackfloat(&srhittank->dir);
  client.players[client.player].kickspeed = KICKFORCE;

  client.armour -= 5;

  if (client.armour < 0) {
    client.armour = 0;
    killtank();
  }

  if (client.playsound) {
    client.playsound(kHitTankSound);
  }

  if (client.settankstatus) {
    client.settankstatus();
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRHitTank)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int recvsrsetalliance() {
  struct SRSetAlliance *srsetalliance;
  int i;
  uint16_t xor;
  char *text = NULL;

TRY
  if (client.recvbuf.nbytes < sizeof(struct SRSetAlliance)) FAIL(EAGAIN)
  srsetalliance = (struct SRSetAlliance *)client.recvbuf.ptr;

  /* convert byte order */
  srsetalliance->alliance = ntohs(srsetalliance->alliance);

  xor = client.players[srsetalliance->player].alliance ^ srsetalliance->alliance;
  client.players[srsetalliance->player].alliance = srsetalliance->alliance;

  /* their alliance bit has changed */
  if (xor & (1 << client.player)) {
    /* my alliance bit is set */
    if (client.players[client.player].alliance & (1 << srsetalliance->player)) {
      /* their alliance bit is set */
      if (srsetalliance->alliance & (1 << client.player)) {
        if (client.printmessage) {
          if (asprintf(&text, "%s accepted the alliance", client.players[srsetalliance->player].name) == -1) LOGFAIL(errno)
          client.printmessage(MSGGAME, text);
          free(text);
          text = NULL;
        }

        if (client.setplayerstatus) {
          client.setplayerstatus(srsetalliance->player);
        }

        for (i = 0; i < client.nbases; i++) {
          if (client.bases[i].owner == srsetalliance->player) {
            refresh(client.bases[i].x, client.bases[i].y);

            if (client.setbasestatus) {
              client.setbasestatus(i);
            }
          }
        }

        for (i = 0; i < client.npills; i++) {
          if (client.pills[i].owner == srsetalliance->player) {
            refresh(client.pills[i].x, client.pills[i].y);

            /* fog of war */
            if (client.pills[i].armour != ONBOARD && client.pills[i].armour > 0) {
              if (increasevis(makerect(client.pills[i].x - 7, client.pills[i].y - 7, 15, 15))) LOGFAIL(errno)
            }

            if (client.setpillstatus) {
              client.setpillstatus(i);
            }
          }
        }

        if (increasevis(makerect(((int)client.players[srsetalliance->player].tank.x) - 14, ((int)client.players[srsetalliance->player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
      }
      /* their alliance bit is unset */
      else {
        if (client.printmessage) {
          if (asprintf(&text, "%s left the alliance", client.players[srsetalliance->player].name) == -1) LOGFAIL(errno)
          client.printmessage(MSGGAME, text);
          free(text);
          text = NULL;
        }

        if (client.setplayerstatus) {
          client.setplayerstatus(srsetalliance->player);
        }

        for (i = 0; i < client.nbases; i++) {
          if (client.bases[i].owner == srsetalliance->player) {
            if (client.setbasestatus) {
              client.setbasestatus(i);
            }

            refresh(client.bases[i].x, client.bases[i].y);
          }
        }

        for (i = 0; i < client.npills; i++) {
          if (client.pills[i].owner == srsetalliance->player) {
            /* fog of war */
            if (client.pills[i].armour != ONBOARD && client.pills[i].armour > 0) {
              if (refresh(client.pills[i].x, client.pills[i].y)) LOGFAIL(errno)
              if (decreasevis(makerect(client.pills[i].x - 7, client.pills[i].y - 7, 15, 15))) LOGFAIL(errno)
            }

            if (client.setpillstatus) {
              client.setpillstatus(i);
            }
          }
        }

        if (decreasevis(makerect(((int)client.players[srsetalliance->player].tank.x) - 14, ((int)client.players[srsetalliance->player].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)

        if (leavealliance(1 << srsetalliance->player)) LOGFAIL(errno)
      }
    }
    /* my alliance bit is unset */
    else {
      /* their alliance bit is set */
      if (client.printmessage && srsetalliance->alliance & (1 << client.player)) {
        if (asprintf(&text, "%s requests an alliance", client.players[srsetalliance->player].name) == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRSetAlliance)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrtimelimit() {
  struct SRTimeLimit *srtimelimit;
  int minutes, seconds;
  char *text = NULL;
  
TRY
  if (client.recvbuf.nbytes < sizeof(struct SRTimeLimit)) FAIL(EAGAIN)
  srtimelimit = (struct SRTimeLimit *)client.recvbuf.ptr;

  /* convert byte order */
  srtimelimit->timeremaining = ntohs(srtimelimit->timeremaining);

  minutes = srtimelimit->timeremaining/60;
  seconds = srtimelimit->timeremaining%60;

  if (client.printmessage) {
    if (minutes != 0) {
      if (seconds != 0) {
        if (asprintf(&text, "%d Minute%s and %d Second%s Remaining!", minutes, minutes > 1 ? "s" : "", seconds, seconds > 1 ? "s" : "") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
      else {
        if (asprintf(&text, "%d Minute%s Remaining!", minutes, minutes > 1 ? "s" : "") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
    }
    else {
      if (seconds != 0) {
        if (asprintf(&text, "%d Second%s Remaining!", seconds, seconds > 1 ? "s" : "") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
      else {
        if (asprintf(&text, "Time Limit Reached!") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
        client.timelimitreached = 1;
      }
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRTimeLimit)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}


int recvsrbasecontrol() {
  struct SRBaseControl *srbasecontrol;
  int minutes, seconds;
  char *text = NULL;
  
TRY
  if (client.recvbuf.nbytes < sizeof(struct SRBaseControl)) FAIL(EAGAIN)
  srbasecontrol = (struct SRBaseControl *)client.recvbuf.ptr;

  /* convert byte order */
  srbasecontrol->timeleft = ntohs(srbasecontrol->timeleft);

  minutes = srbasecontrol->timeleft/60;
  seconds = srbasecontrol->timeleft%60;

  if (client.printmessage) {
    if (minutes != 0) {
      if (seconds != 0) {
        if (asprintf(&text, "%d Minute%s and %d Second%s Remaining!", minutes, minutes > 1 ? "s" : "", seconds, seconds > 1 ? "s" : "") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
      else {
        if (asprintf(&text, "%d Minute%s Remaining!", minutes, minutes > 1 ? "s" : "") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
    }
    else {
      if (seconds != 0) {
        if (asprintf(&text, "%d Second%s Remaining!", seconds, seconds > 1 ? "s" : "") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
      }
      else {
        if (asprintf(&text, "Base Control Reached!") == -1) LOGFAIL(errno)
        client.printmessage(MSGGAME, text);
        free(text);
        text = NULL;
        client.basecontrolreached = 1;
      }
    }
  }

  if (readbuf(&client.recvbuf, NULL, sizeof(struct SRBaseControl)) == -1) LOGFAIL(errno)

CLEANUP
  if (text) {
    free(text);
  }

ERRHANDLER(0, -1)
END
}

