/*
 *  client_send.c
 *  XBolo
 *
 *  Builders for client->server protocol messages.
 */

#include "client_internal.h"


int sendclsmallboom(int x, int y) {
  struct CLSmallBoom clsmallboom;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clsmallboom.type = CLSMALLBOOM;
  clsmallboom.x = x;
  clsmallboom.y = y;

  if (writebuf(&client.sendbuf, &clsmallboom, sizeof(clsmallboom)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclsuperboom(int x, int y) {
  struct CLSuperBoom clsuperboom;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clsuperboom.type = CLSUPERBOOM;
  clsuperboom.x = x;
  clsuperboom.y = y;

  if (writebuf(&client.sendbuf, &clsuperboom, sizeof(clsuperboom)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendcldropmine(int x, int y) {
  struct CLDropMine cldropmine;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  cldropmine.type = CLDROPMINE;
  cldropmine.x = x;
  cldropmine.y = y;

  if (writebuf(&client.sendbuf, &cldropmine, sizeof(cldropmine)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendcltouch(int x, int y) {
  struct CLTouch cltouch;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  cltouch.type = CLTOUCH;
  cltouch.x = x;
  cltouch.y = y;

  if (writebuf(&client.sendbuf, &cltouch, sizeof(cltouch)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclgrabtile(int x, int y) {
  struct CLGrabTile clgrabtile;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clgrabtile.type = CLGRABTILE;
  clgrabtile.x = x;
  clgrabtile.y = y;

  if (writebuf(&client.sendbuf, &clgrabtile, sizeof(clgrabtile)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclgrabtrees(int x, int y) {
  struct CLGrabTrees clgrabtrees;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clgrabtrees.type = CLGRABTREES;
  clgrabtrees.x = x;
  clgrabtrees.y = y;

  if (writebuf(&client.sendbuf, &clgrabtrees, sizeof(clgrabtrees)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclbuildroad(int x, int y, int trees) {
  struct CLBuildRoad clbuildroad;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clbuildroad.type = CLBUILDROAD;
  clbuildroad.x = x;
  clbuildroad.y = y;
  clbuildroad.trees = trees;

  if (writebuf(&client.sendbuf, &clbuildroad, sizeof(clbuildroad)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclbuildwall(int x, int y, int trees) {
  struct CLBuildWall clbuildwall;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clbuildwall.type = CLBUILDWALL;
  clbuildwall.x = x;
  clbuildwall.y = y;
  clbuildwall.trees = trees;

  if (writebuf(&client.sendbuf, &clbuildwall, sizeof(clbuildwall)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclbuildboat(int x, int y, int trees) {
  struct CLBuildBoat clbuildboat;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clbuildboat.type = CLBUILDBOAT;
  clbuildboat.x = x;
  clbuildboat.y = y;
  clbuildboat.trees = trees;

  if (writebuf(&client.sendbuf, &clbuildboat, sizeof(clbuildboat)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclbuildpill(int x, int y, int trees, int pill) {
  struct CLBuildPill clbuildpill;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clbuildpill.type = CLBUILDPILL;
  clbuildpill.x = x;
  clbuildpill.y = y;
  clbuildpill.trees = trees;
  clbuildpill.pill = pill;

  if (writebuf(&client.sendbuf, &clbuildpill, sizeof(clbuildpill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclrepairpill(int x, int y, int trees) {
  struct CLRepairPill clrepairpill;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clrepairpill.type = CLREPAIRPILL;
  clrepairpill.x = x;
  clrepairpill.y = y;
  clrepairpill.trees = trees;

  if (writebuf(&client.sendbuf, &clrepairpill, sizeof(clrepairpill)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclplacemine(int x, int y) {
  struct CLPlaceMine clplacemine;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  clplacemine.type = CLPLACEMINE;
  clplacemine.x = x;
  clplacemine.y = y;

  if (writebuf(&client.sendbuf, &clplacemine, sizeof(clplacemine)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendcldamage(int x, int y, int boat) {
  struct CLDamage cldamage;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  cldamage.type = CLDAMAGE;
  cldamage.x = x;
  cldamage.y = y;
  cldamage.boat = boat;

  if (writebuf(&client.sendbuf, &cldamage, sizeof(cldamage)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendcldropboat(int x, int y) {
  struct CLDropBoat cldropboat;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

TRY
  cldropboat.type = CLDROPBOAT;
  cldropboat.x = x;
  cldropboat.y = y;

  if (writebuf(&client.sendbuf, &cldropboat, sizeof(cldropboat)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendcldroppills(float x, float y, uint16_t pills) {
  int i;
  struct CLDropPills cldroppills;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  for (i = 0; i < client.npills; i++) {
    if (pills & (1 << i)) {
      assert(client.pills[i].owner == client.player);
    }
  }

  assert((pills >> i) == 0);

TRY
  cldroppills.type = CLDROPPILLS;
  cldroppills.x = htonl(*((uint32_t *)&x));
  cldroppills.y = htonl(*((uint32_t *)&y));
  cldroppills.pills = htons(pills);

  if (writebuf(&client.sendbuf, &cldroppills, sizeof(cldroppills)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclrefuel(int base, int armour, int shells, int mines) {
  struct CLRefuel clrefuel;

  assert(base >= 0);
  assert(base < client.nbases);
  assert(armour >= 0);
  assert(armour < 256);
  assert(shells >= 0);
  assert(shells < 256);
  assert(mines >= 0);
  assert(mines < 256);

TRY
  clrefuel.type = CLREFUEL;
  clrefuel.base = base;
  clrefuel.armour = armour;
  clrefuel.shells = shells;
  clrefuel.mines = mines;

  if (writebuf(&client.sendbuf, &clrefuel, sizeof(clrefuel)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclhittank(int player, float dir) {
  struct CLHitTank clhittank;

  assert(player >= 0);
  assert(client.players[player].connected);
  assert(dir >= 0.0);
  assert(dir < k2Pif);

TRY
  clhittank.type = CLHITTANK;
  clhittank.player = player;
  clhittank.dir = htonl(*((uint32_t *)&dir));

  if (writebuf(&client.sendbuf, &clhittank, sizeof(clhittank)) == -1) LOGFAIL(errno)

CLEANUP
ERRHANDLER(0, -1)
END
}


int sendclupdate() {
  struct CLUpdate clupdate;
  struct CLUpdateShell *updateshells;
  struct CLUpdateExplosion *updateexplosions;
  struct ListNode *node;
  int i;

TRY
  client.players[client.player].lastupdate = client.players[client.player].seq;

  clupdate.hdr.player = client.player;

  for (i = 0; i < MAXPLAYERS; i++) {
    clupdate.hdr.seq[i] = htonl(client.players[i].seq);
  }

  clupdate.hdr.tankstatus = client.players[client.player].dead ? kTankDead : (client.players[client.player].boat ? kTankOnBoat : kTankNormal);
  clupdate.hdr.tankx = htonl(*((uint32_t *)&client.players[client.player].tank.x));
  clupdate.hdr.tanky = htonl(*((uint32_t *)&client.players[client.player].tank.y));
  clupdate.hdr.tankspeed = htonl(*((uint32_t *)&client.players[client.player].speed));
  clupdate.hdr.tankturnspeed = htonl(*((uint32_t *)&client.players[client.player].turnspeed));
  clupdate.hdr.tankkickdir = htonl(*((uint32_t *)&client.players[client.player].kickdir));
  clupdate.hdr.tankkickspeed = htonl(*((uint32_t *)&client.players[client.player].kickspeed));
  clupdate.hdr.tankdir = (uint8_t)(client.players[client.player].dir*(FWIDTH/k2Pif));
  clupdate.hdr.builderstatus = client.players[client.player].builderstatus;
  clupdate.hdr.builderx = htonl(*((uint32_t *)&client.players[client.player].builder.x));
  clupdate.hdr.buildery = htonl(*((uint32_t *)&client.players[client.player].builder.y));
  clupdate.hdr.buildertargetx = client.players[client.player].buildertarget.x;
  clupdate.hdr.buildertargety = client.players[client.player].buildertarget.y;
  clupdate.hdr.builderwait = client.players[client.player].builderwait;
  clupdate.hdr.inputflags = htonl(client.players[client.player].inputflags);
  clupdate.hdr.tankshotsound = client.tankshotsound;
  clupdate.hdr.pillshotsound = client.pillshotsound;
  clupdate.hdr.sinksound = client.sinksound;
  clupdate.hdr.builderdeathsound = client.builderdeathsound;
  clupdate.hdr.nshells = 0;
  clupdate.hdr.nexplosions = 0;

  client.tankshotsound = 0;
  client.pillshotsound = 0;
  client.sinksound = 0;
  client.builderdeathsound = 0;

  updateshells = (void *)clupdate.buf;
  node = nextlist(&client.players[client.player].shells);

  while (node != NULL && clupdate.hdr.nshells < CLUPDATEMAXSHELLS) {
    struct Shell *shell;

    shell = ptrlist(node);

    updateshells[clupdate.hdr.nshells].owner = shell->owner;
    updateshells[clupdate.hdr.nshells].shellx = htons((uint16_t)(shell->point.x*FWIDTH));
    updateshells[clupdate.hdr.nshells].shelly = htons((uint16_t)(shell->point.y*FWIDTH));
    updateshells[clupdate.hdr.nshells].boat = shell->boat;
    updateshells[clupdate.hdr.nshells].pill = shell->pill;
    updateshells[clupdate.hdr.nshells].shelldir = (uint8_t)(shell->dir*(FWIDTH/k2Pif));
    updateshells[clupdate.hdr.nshells].range = htons((uint16_t)(shell->range*FWIDTH));
    node = nextlist(node);
    clupdate.hdr.nshells++;
  }

  updateexplosions = (void *)(updateshells + clupdate.hdr.nshells);
  node = nextlist(&client.players[client.player].explosions);

  while (node != NULL && clupdate.hdr.nexplosions < CLUPDATEMAXEXPLOSIONS) {
    struct Explosion *explosion;

    explosion = ptrlist(node);
    updateexplosions[clupdate.hdr.nexplosions].explosionx = htons((uint16_t)(explosion->point.x*FWIDTH));
    updateexplosions[clupdate.hdr.nexplosions].explosiony = htons((uint16_t)(explosion->point.y*FWIDTH));
    updateexplosions[clupdate.hdr.nexplosions].counter = explosion->counter;
    node = nextlist(node);
    clupdate.hdr.nexplosions++;
  }

  if (send(client.dgramsock, &clupdate, sizeof(clupdate.hdr) + clupdate.hdr.nshells*sizeof(struct CLUpdateShell) + clupdate.hdr.nexplosions*sizeof(struct CLUpdateExplosion), 0) == -1) {
    LOGFAIL(errno)
  }

CLEANUP
ERRHANDLER(0, -1)
END
}

