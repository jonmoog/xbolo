/*
 *  server_logic.c
 *  XBolo
 *
 *  Server-side world logic: terrain, tree growth, chain reactions, floods, explosions, scoring and lookups.
 */

#include "server_internal.h"


int clearterrain(int x, int y) {
  if (x < X_MIN_MINE || x > X_MAX_MINE || y < Y_MIN_MINE || y > Y_MAX_MINE) {
    return 0;
  }

  switch (server.terrain[y][x]) {
  case kSeaTerrain:
  case kBoatTerrain:
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
  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    return findpill(x, y) == -1 && findbase(x, y) == -1;

  case kWallTerrain:
  case kDamagedWallTerrain0:
  case kDamagedWallTerrain1:
  case kDamagedWallTerrain2:
  case kDamagedWallTerrain3:
    return 0;

  default:
    assert(0);
    return 0;
  }
}


int dr(int x, int y, int i, uint16_t pills) {
  if (clearterrain(x, y) && findpill(x, y) == -1) {
    while (i < server.npills) {
      if ((1 << i) & pills) {
        server.pills[i].armour = 0;
        server.pills[i].x = x;
        server.pills[i].y = y;
        sendsrdroppill(i);
        i++;
        break;
      }

      i++;
    }
  }

  return i;
}


void droppills(int player, float x, float y, uint16_t pills) {
  int i, j;
  float lx, hx, ly, hy;
  int minx, miny, maxx, maxy;

  assert(player >= 0 && player < MAXPLAYERS);

  if (x < 0.0) {
    x = 0.0;
  }
  else if (x >= FWIDTH) {
    x = FWIDTH - 0.00001;
  }
  else if (isnan(x)) {
    x = FWIDTH/2.0;
  }

  if (y < 0.0) {
    y = 0.0;
  }
  else if (y >= FWIDTH) {
    y = FWIDTH - 0.00001;
  }
  else if (isnan(x)) {
    y = FWIDTH/2.0;
  }

  minx = (int)x;
  maxx = minx + 1;
  miny = (int)y;
  maxy = miny + 1;

  i = dr(minx, miny, 0, pills);

  while (i < server.npills) {
    /* find closest edge */
    lx = x - minx;
    hx = maxx - x;
    ly = y - miny;
    hy = maxy - y;

    if (lx <= hx && lx <= ly && lx <= hy) {
      minx--;

      for (j = 0; miny + j < maxy; j++) {
        i = dr(minx, miny + j, i, pills);
      }
    }
    else if (hx <= lx && hx <= ly && hx <= hy) {
      for (j = 0; miny + j < maxy; j++) {
        i = dr(maxx, miny + j, i, pills);
      }

      maxx++;
    }
    else if (ly <= lx && ly <= hx && ly <= hy) {
      miny--;

      for (j = 0; minx + j < maxx; j++) {
        i = dr(minx + j, miny, i, pills);
      }
    }
    else if (hy <= lx && hy <= hx && hy <= ly) {
      for (j = 0; minx + j < maxx; j++) {
        i = dr(minx + j, maxy, i, pills);
      }

      maxy++;
    }
    else {
      assert(0);
    }
  }
}


int basescore(int x, int y) {
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  if (findpill(x, y) != -1 || findbase(x, y) != -1) {
    return 0;
  }

  switch (server.terrain[y][x]) {
  case kGrassTerrain0:
  case kGrassTerrain1:
  case kGrassTerrain2:
  case kGrassTerrain3:
  case kMinedGrassTerrain:
    return 5;

  case kSwampTerrain0:
  case kSwampTerrain1:
  case kSwampTerrain2:
  case kSwampTerrain3:
  case kMinedSwampTerrain:
    return 4;

  case kCraterTerrain:
  case kMinedCraterTerrain:
    return 3;

  case kRubbleTerrain0:
  case kRubbleTerrain1:
  case kRubbleTerrain2:
  case kRubbleTerrain3:
  case kMinedRubbleTerrain:
    return 2;

  case kRoadTerrain:
  case kMinedRoadTerrain:
    return 1;

  default:
    return 0;
  }
}


int adjacentscore(int x, int y) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= WIDTH) {
    return 0;
  }
  else {
    switch (server.terrain[y][x]) {
    case kForestTerrain:
    case kMinedForestTerrain:
      return 1;

    default:
      return 0;
    }
  }
}


int treescore(int x, int y) {
  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  return
      basescore(x, y)*(
        2*(adjacentscore(x + 1, y) + adjacentscore(x - 1, y) + adjacentscore(x, y + 1) + adjacentscore(x, y - 1)) +
        adjacentscore(x - 1, y - 1) + adjacentscore(x + 1, y - 1) + adjacentscore(x - 1, y + 1) + adjacentscore(x + 1, y + 1)
      );
}


void growtrees(int nplayers) {
  int i;
  int x, y;
  
  for (i = 0; i < nplayers*(TREESBESTOF/(TREESPLANTRATE*TICKSPERSEC)); i++) {
    x = random()%(WIDTH*WIDTH);
    y = x/WIDTH;
    x %= WIDTH;

    if (treescore(server.growx, server.growy) < treescore(x, y)) {
      server.growx = x;
      server.growy = y;
    }

    server.growbestof++;

    if (server.growbestof >= TREESBESTOF) {
      server.growbestof = 0;

      /* if clear grow trees */
      if (findpill(x, y) == -1 && findbase(x, y) == -1) {
        switch (server.terrain[server.growy][server.growx]) {
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
          if (findpill(server.growx, server.growy) == -1 && findbase(server.growx, server.growy) == -1) {
            server.terrain[server.growy][server.growx] = kForestTerrain;
            sendsrgrow(server.growx, server.growy);
          }

          break;

        case kMinedGrassTerrain:
        case kMinedRubbleTerrain:
        case kMinedCraterTerrain:
        case kMinedSwampTerrain:
        case kMinedRoadTerrain:
          if (findpill(server.growx, server.growy) == -1 && findbase(server.growx, server.growy) == -1) {
            server.terrain[server.growy][server.growx] = kMinedForestTerrain;
            sendsrgrow(server.growx, server.growy);
          }

          break;

        default:
          break;
        }
      }

      /* begin new search */
      server.growx = random()%(WIDTH*WIDTH);
      server.growy = server.growx/WIDTH;
      server.growx %= WIDTH;
    }
  }
}


int chainat(int x, int y) {
  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

TRY
  switch (server.terrain[y][x]) {
  case kMinedSeaTerrain:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(NEUTRAL, x, y)) LOGFAIL(errno)
    break;

  default:
    break;
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int floodat(int x, int y) {
  Pointi *flood = NULL;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

TRY
  switch (server.terrain[y][x]) {
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    if (explosionat(NEUTRAL, x, y)) LOGFAIL(errno)
    break;

  case kCraterTerrain:
    server.terrain[y][x] = kRiverTerrain;
    sendsrflood(x, y);
    if ((flood = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
    flood->x = x;
    flood->y = y;
    if (addlist(server.floods + (server.ticks - 1)%(FLOODTICKS + 1), flood)) LOGFAIL(errno)
    flood = NULL;
    break;

  default:
    break;
  }

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (flood != NULL) {
      free(flood);
    }

    RETERR(-1)
  }
END
}


int floodtest(int x, int y) {
  Pointi *flood;

  flood = NULL;
  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

TRY
  switch (server.terrain[y][x]) {
  case kRiverTerrain:
  case kSeaTerrain:
  case kMinedSeaTerrain:
  case kBoatTerrain:
    if ((flood = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
    flood->x = x;
    flood->y = y;
    if (addlist(server.floods + (server.ticks - 1)%(FLOODTICKS + 1), flood)) LOGFAIL(errno)
    flood = NULL;
    break;

  default:
    break;
  }

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (flood != NULL) {
      free(flood);
    }

    RETERR(-1);
  }
END
}


int explosionat(int player, int x, int y) {
  Pointi *chain = NULL;

  assert(((player >= 0 && player < MAXPLAYERS) || player == NEUTRAL) && x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

TRY
  switch (server.terrain[y][x]) {
  case kBoatTerrain:
  case kWallTerrain:
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
  case kDamagedWallTerrain0:
  case kDamagedWallTerrain1:
  case kDamagedWallTerrain2:
  case kDamagedWallTerrain3:
  case kMinedSwampTerrain:
  case kMinedCraterTerrain:
  case kMinedRoadTerrain:
  case kMinedForestTerrain:
  case kMinedRubbleTerrain:
  case kMinedGrassTerrain:
    server.terrain[y][x] = kCraterTerrain;
    if (floodtest(x, y - 1)) LOGFAIL(errno)
    if (floodtest(x - 1, y)) LOGFAIL(errno)
    if (floodtest(x + 1, y)) LOGFAIL(errno)
    if (floodtest(x, y + 1)) LOGFAIL(errno)
    if ((chain = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
    chain->x = x;
    chain->y = y;
    if (addlist(server.chains + (server.ticks - 1)%(CHAINTICKS + 1), chain)) LOGFAIL(errno)
    chain = NULL;
    sendsrsmallboom(NEUTRAL, x, y);
    break;

  case kMinedSeaTerrain:
    sendsrsmallboom(NEUTRAL, x, y);
    break;

  default:
    break;
  }

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (chain != NULL) {
      free(chain);
    }

    RETERR(-1)
  }
END
}


int superboomat(int player, int x, int y) {
  Pointi *chain = NULL;

  assert(player >= 0 && player < MAXPLAYERS && x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

TRY
  /* turn terrain to crater */
  if (server.terrain[y][x] != kSeaTerrain && server.terrain[y][x] != kMinedSeaTerrain) {
    server.terrain[y][x] = kCraterTerrain;
  }
  if (server.terrain[y][x + 1] != kSeaTerrain && server.terrain[y][x + 1] != kMinedSeaTerrain) {
    server.terrain[y][x + 1] = kCraterTerrain;
  }
  if (server.terrain[y + 1][x] != kSeaTerrain && server.terrain[y + 1][x] != kMinedSeaTerrain) {
    server.terrain[y + 1][x] = kCraterTerrain;
  }
  if (server.terrain[y + 1][x + 1] != kSeaTerrain && server.terrain[y + 1][x + 1] != kMinedSeaTerrain) {
    server.terrain[y + 1][x + 1] = kCraterTerrain;
  }

  /* begin flood test */
  if (floodtest(x, y - 1)) LOGFAIL(errno)
  if (floodtest(x + 1, y - 1)) LOGFAIL(errno)
  if (floodtest(x - 1, y)) LOGFAIL(errno)
  if (floodtest(x - 1, y + 1)) LOGFAIL(errno)
  if (floodtest(x + 2, y)) LOGFAIL(errno)
  if (floodtest(x + 2, y + 1)) LOGFAIL(errno)
  if (floodtest(x, y + 2)) LOGFAIL(errno)
  if (floodtest(x + 1, y + 2)) LOGFAIL(errno)

  /* begin chain explosions */
  if ((chain = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
  chain->x = x;
  chain->y = y;
  if (addlist(server.chains + (server.ticks - 1)%(CHAINTICKS + 1), chain)) LOGFAIL(errno)
  if ((chain = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
  chain->x = x + 1;
  chain->y = y;
  if (addlist(server.chains + (server.ticks - 1)%(CHAINTICKS + 1), chain)) LOGFAIL(errno)
  if ((chain = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
  chain->x = x;
  chain->y = y + 1;
  if (addlist(server.chains + (server.ticks - 1)%(CHAINTICKS + 1), chain)) LOGFAIL(errno)
  if ((chain = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
  chain->x = x + 1;
  chain->y = y + 1;
  if (addlist(server.chains + (server.ticks - 1)%(CHAINTICKS + 1), chain)) LOGFAIL(errno)
  chain = NULL;

  /* send superboom to clients */
  sendsrsuperboom(player, x, y);

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(0)

  default:
    if (chain) {
      free(chain);
    }

    RETERR(-1)
  }
END
}


int chain() {
  struct ListNode *node;

TRY
  for (node = nextlist(server.chains + server.ticks%(CHAINTICKS + 1)); node != NULL; node = nextlist(node)) {
    Pointi *chain;

    chain = ptrlist(node);
    if (chainat(chain->x, chain->y - 1)) LOGFAIL(errno)
    if (chainat(chain->x - 1, chain->y)) LOGFAIL(errno)
    if (chainat(chain->x + 1, chain->y)) LOGFAIL(errno)
    if (chainat(chain->x, chain->y + 1)) LOGFAIL(errno)
  }

  clearlist(server.chains + server.ticks%(CHAINTICKS + 1), free);

CLEANUP
ERRHANDLER(0, -1)
END
}


int flood() {
  struct ListNode *node;

TRY
  for (node = nextlist(server.floods + server.ticks%(FLOODTICKS + 1)); node != NULL; node = nextlist(node)) {
    Pointi *flood;

    flood = ptrlist(node);
    if (floodat(flood->x, flood->y - 1)) LOGFAIL(errno)
    if (floodat(flood->x - 1, flood->y)) LOGFAIL(errno)
    if (floodat(flood->x + 1, flood->y)) LOGFAIL(errno)
    if (floodat(flood->x, flood->y + 1)) LOGFAIL(errno)
  }

  clearlist(server.floods + server.ticks%(FLOODTICKS + 1), free);

CLEANUP
ERRHANDLER(0, -1)
END
}


int tiletoterrain(int tile) {
  switch (tile) {
  case kWallTile:  /* wall */
    return kWallTerrain;

  case kRiverTile:  /* river */
    return kRiverTerrain;

  case kSwampTile:  /* swamp */
    return kSwampTerrain3;

  case kCraterTile:  /* crater */
    return kCraterTerrain;

  case kRoadTile:  /* road */
    return kRoadTerrain;

  case kForestTile:  /* forest */
    return kForestTerrain;

  case kRubbleTile:  /* rubble */
    return kRubbleTerrain3;

  case kGrassTile:  /* grass */
    return kGrassTerrain3;

  case kDamagedWallTile:  /* damaged wall */
    return kDamagedWallTerrain3;

  case kBoatTile:  /* river w/boat */
    return kBoatTerrain;

  case kMinedSwampTile:  /* mined swamp */
    return kMinedSwampTerrain;

  case kMinedCraterTile:  /* mined crater */
    return kMinedCraterTerrain;

  case kMinedRoadTile:  /* mined road */
    return kMinedRoadTerrain;

  case kMinedForestTile:  /* mined forest */
    return kMinedForestTerrain;

  case kMinedRubbleTile:  /* mined rubble */
    return kMinedRubbleTerrain;

  case kMinedGrassTile:  /* mined grass */
    return kMinedGrassTerrain;

  case kSeaTile:  /* sea */
    return kSeaTerrain;

  case kMinedSeaTile:  /* mined sea */
    return kMinedSeaTerrain;

  case kUnknownTile:  /* unknown */
    return kMinedSeaTerrain;

  default:
    assert(0);
    return -1;
  }
}


int findpill(int x, int y) {
  int i;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

  for (i = 0; i < server.npills; i++) {
    if (server.pills[i].armour != ONBOARD && server.pills[i].x == x && server.pills[i].y == y) {
      return i;
    }
  }

  return -1;
}


int findbase(int x, int y) {
  int i;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

  for (i = 0; i < server.nbases; i++) {
    if (server.bases[i].x == x && server.bases[i].y == y) {
      return i;
    }
  }

  return -1;
}


int nplayers() {
  int i, nplayers;

  for (i = 0, nplayers = 0; i < MAXPLAYERS; i++) {
    if (server.players[i].cntlsock != -1) {
      nplayers++;
    }
  }

  return nplayers;
}

