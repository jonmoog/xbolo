/*
 *  client_logic.c
 *  XBolo
 *
 *  Client-side game simulation: movement, builders, shells, collisions, visibility and tile mapping.
 */

#include "client_internal.h"


/* builder state shared with the protocol modules */
int buildertask;
int collisionowner;

static Pointi target;


float maxspeed(int x, int y) {
  int pill, base;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  if ((pill = clientfindpill(x, y)) != -1) {
    if (client.pills[pill].armour > 0) {
      return 0.0;
    }
    else {
      return 3.125;
    }
  }

  if ((base = clientfindbase(x, y)) != -1) {
    return 3.125;
  }

  switch (client.terrain[y][x]) {
  case kRiverTerrain:  /* river */
  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRubbleTerrain:  /* mined rubble */
    return RUBBLEMAXSPEED;

  case kForestTerrain:  /* forest */
  case kMinedForestTerrain:  /* mined forest */
    return FORESTMAXSPEED;

  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
  case kMinedGrassTerrain:  /* mined grass */
    return GRASSMAXSPEED;

  case kRoadTerrain:  /* road */
  case kBoatTerrain:  /* river w/boat */
  case kMinedRoadTerrain:  /* mined road */
    return ROADMAXSPEED;

  case kSeaTerrain:  /* sea */
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
  case kMinedSeaTerrain:  /* mined sea */
  default:
    return 0.0;
  }
}


float maxturnspeed(int x, int y) {
  int pill, base;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  if ((pill = clientfindpill(x, y)) != -1) {
    if (client.pills[pill].armour > 0) {
      return 0.0;
    }
    else {
      return 2.5;
    }
  }

  if ((base = clientfindbase(x, y)) != -1) {
    return 2.5;
  }

  switch (client.terrain[y][x]) {
  case kRiverTerrain:  /* river */
  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRubbleTerrain:  /* mined rubble */
    return 0.625;

  case kForestTerrain:  /* forest */
  case kMinedForestTerrain:  /* mined forest */
    return 1.25;

  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
  case kMinedGrassTerrain:  /* mined grass */
  case kRoadTerrain:  /* road */
  case kBoatTerrain:  /* river w/boat */
  case kMinedRoadTerrain:  /* mined road */
    return 2.5;

  case kSeaTerrain:  /* sea */
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
  case kMinedSeaTerrain:  /* mined sea */
  default:
    return 0.0;
  }
}


float builderspeed(int x, int y, int player) {
  int pill, base;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

  if ((pill = clientfindpill(x, y)) != -1) {
    if (client.pills[pill].armour > 0) {
      return 0.0;
    }
    else {
      return BUILDERMAXSPEED;
    }
  }

  if ((base = clientfindbase(x, y)) != -1) {
    if (
      client.bases[base].owner == NEUTRAL || client.bases[base].armour == 0 ||
      testalliance(client.bases[base].owner, player)
    ) {
      return BUILDERMAXSPEED;
    }
    else {
      return 0.0;
    }
  }

  switch (client.terrain[y][x]) {
  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRubbleTerrain:  /* mined rubble */
    return BUILDERMAXSPEED*0.25;

  case kForestTerrain:  /* forest */
  case kMinedForestTerrain:  /* mined forest */
    return BUILDERMAXSPEED*0.5;

  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
  case kMinedGrassTerrain:  /* mined grass */
  case kRoadTerrain:  /* road */
  case kBoatTerrain:  /* river w/boat */
  case kMinedRoadTerrain:  /* mined road */
    return BUILDERMAXSPEED;

  case kRiverTerrain:  /* river */
  case kSeaTerrain:  /* sea */
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
  case kMinedSeaTerrain:  /* mined sea */
    return 0.0;

  default:
    assert(0);
  }
}


float buildertargetspeed(int x, int y) {
  int pill, base;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

  if ((pill = clientfindpill(x, y)) != -1) {
    return BUILDERMAXSPEED;
  }

  if ((base = clientfindbase(x, y)) != -1) {
    return BUILDERMAXSPEED;
  }

  switch (client.terrain[y][x]) {
  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRubbleTerrain:  /* mined rubble */
    return BUILDERMAXSPEED*0.25;

  case kForestTerrain:  /* forest */
  case kMinedForestTerrain:  /* mined forest */
    return BUILDERMAXSPEED*0.5;

  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
  case kMinedGrassTerrain:  /* mined grass */
  case kRoadTerrain:  /* road */
  case kBoatTerrain:  /* river w/boat */
  case kMinedRoadTerrain:  /* mined road */
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
  case kRiverTerrain:  /* river */
    return BUILDERMAXSPEED;

  case kSeaTerrain:  /* sea */
  case kMinedSeaTerrain:  /* mined sea */
    return 0.0;

  default:
    assert(0);
    return 0.0;
  }
}


int decreasevis(Recti r) {
  int x, y;

  assert(r.size.width > 0 && r.size.height > 0);

TRY
  r = intersectionrect(makerect(0, 0, WIDTH, WIDTH), r);

  for (y = minyrect(r); y <= maxyrect(r); y++) {
    for (x = minxrect(r); x <= maxxrect(r); x++) {
      if (--client.fog[y][x] == 0) {
        Pointi *p;

        if ((p = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
        p->x = x;
        p->y = y;
        if (addlist(&client.changedtiles, p) == -1) LOGFAIL(errno)
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int increasevis(Recti r) {
  int x, y;

  assert(r.size.width > 0 && r.size.height > 0);

TRY
  r = intersectionrect(makerect(0, 0, WIDTH, WIDTH), r);

  for (y = minyrect(r); y <= maxyrect(r); y++) {
    for (x = minxrect(r); x <= maxxrect(r); x++) {
      client.fog[y][x]++;
    }
  }

  r = insetrect(r, -1, -1);
  r = intersectionrect(makerect(0, 0, WIDTH, WIDTH), r);

  for (y = minyrect(r); y <= maxyrect(r); y++) {
    for (x = minxrect(r); x <= maxxrect(r); x++) {
      if (client.fog[y][x] <= 1) {
        client.seentiles[y][x] = fogtilefor(x, y, client.seentiles[y][x]);
      }
    }
  }

  for (y = minyrect(r); y <= maxyrect(r); y++) {
    for (x = minxrect(r); x <= maxxrect(r); x++) {
      Pointi *p;
      int image;

      image = mapimage(client.seentiles, x, y);

      if (image != client.images[y][x] || client.fog[y][x] == 1) {
        client.images[y][x] = image;
        if ((p = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
        p->x = x;
        p->y = y;
        if (addlist(&client.changedtiles, p) == -1) LOGFAIL(errno)
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int isshore(int x, int y) {
  int i;

  if (x < 0 || x >= WIDTH || y < 0 || y >= WIDTH) {
    return 0;
  }

  for (i = 0; i < client.nbases; i++) {
    if (client.bases[i].x == x && client.bases[i].y == y) {
      return 1;
    }
  }

  switch (client.terrain[y][x]) {
	case kSeaTerrain:
	case kRiverTerrain:
	case kMinedSeaTerrain:
    return 0;

	case kBoatTerrain:
	case kWallTerrain:
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
    return 1;

  default:
    assert(0);
    return 0;
  }
}


int tankmovelogic(int player) {
TRY
  if (client.players[player].connected) {
    if (client.players[player].dead) {
      if (player == client.player) {
        client.respawncounter++;

        if (client.respawncounter < EXPLODETICKS) {
          client.players[player].tank = add2f(client.players[player].tank, mul2f(dir2vec(client.players[player].kickdir), client.players[player].kickspeed/TICKSPERSEC));

          /* collisions with terrain */
          collisionowner = player;
          client.players[player].tank = collisiondetect(client.players[player].tank, TANKRADIUS, tankcollision);

          if (client.respawncounter%5 == 0) {
            struct Explosion *explosion;

            switch (client.terrain[w2t(client.players[player].tank.y)][w2t(client.players[player].tank.x)]) {
            case kSeaTile:
            case kMinedSeaTile:
              break;

            default:
              if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
              explosion->point = client.players[client.player].tank;
              explosion->counter = 0;
              if (addlist(&client.players[client.player].explosions, explosion)) LOGFAIL(errno)
              if (killpointbuilder(explosion->point)) LOGFAIL(errno)
              break;
            }
          }
        }
        else if (client.respawncounter == EXPLODETICKS) {
          if (client.mines >= 32) {
            if (superboom()) LOGFAIL(errno)
          }
          else if (client.mines > 0 || client.shells > 0) {
            if (smallboom()) LOGFAIL(errno)
          }
        }
        else if (client.respawncounter >= RESPAWN_TICKS) {
          if (spawn()) LOGFAIL(errno)
        }
      }
    }
    else {
      float max;
      /* turn */
      if ((client.players[player].inputflags & TURNLMASK) && !(client.players[player].inputflags & TURNRMASK)) {
        if(client.players[player].turnspeed < 0)
          client.players[player].turnspeed = 0;
        max = client.players[player].boat ? MAXANGULARVELOCITY : maxturnspeed((int)client.players[client.player].tank.x, (int)client.players[client.player].tank.y);

        if (client.players[player].turnspeed > max) {
          client.players[player].turnspeed -= ANGULARACCEL/TICKSPERSEC;

          if (client.players[player].turnspeed < max) {
            client.players[player].turnspeed = max;
          }
        }
        else {
          client.players[player].turnspeed += ANGULARACCEL/TICKSPERSEC;

          if (client.players[player].turnspeed > max) {
            client.players[player].turnspeed = max;
          }
        }
      }
      else if ((client.players[player].inputflags & TURNRMASK) && !(client.players[player].inputflags & TURNLMASK)) {
        if(client.players[player].turnspeed > 0)
          client.players[player].turnspeed = 0;
        max = client.players[player].boat ? MAXANGULARVELOCITY : maxturnspeed((int)client.players[client.player].tank.x, (int)client.players[client.player].tank.y);
          
        if (client.players[player].turnspeed < -max) {
          client.players[player].turnspeed += ANGULARACCEL/TICKSPERSEC;
            
          if (client.players[player].turnspeed > -max) {
            client.players[player].turnspeed = -max;
          }
        }
        else {
          client.players[player].turnspeed -= ANGULARACCEL/TICKSPERSEC;
            
          if (client.players[player].turnspeed < -max) {
            client.players[player].turnspeed = -max;
          }
        }
      }
      else {
        client.players[player].turnspeed = 0.0;
      }

      client.players[player].dir += client.players[player].turnspeed/TICKSPERSEC;

      if (client.players[player].dir > k2Pif) {
        client.players[player].dir -= k2Pif*floorf(client.players[player].dir/k2Pif);
      }
      else if (client.players[player].dir < 0.0) {
        client.players[player].dir += k2Pif*floorf(client.players[player].dir/-k2Pif + 1.0);
      }

      /* acceleration */
      max = client.players[player].boat ? BOATMAXSPEED : maxspeed((int)client.players[client.player].tank.x, (int)client.players[client.player].tank.y);

      /* accelerate forward */
      if ((client.players[player].inputflags & ACCELMASK) && !(client.players[player].inputflags & BRAKEMASK)) {
        if (client.players[player].speed < max) {
          client.players[player].speed += ACCEL/TICKSPERSEC;

          if (client.players[player].speed > max) {
            client.players[player].speed = max;
          }
        }
        else {
          client.players[player].speed -= ACCEL/TICKSPERSEC;

          if (client.players[player].speed < max) {
            client.players[player].speed = max;
          }
        }
      } /* accelerate backward */
      else if (!(client.players[player].inputflags & ACCELMASK) && (client.players[player].inputflags & BRAKEMASK)) {
        client.players[player].speed -= ACCEL/TICKSPERSEC;

        if (client.players[player].speed < 0.0) {
          client.players[player].speed = 0.0;
        }
      }
      else if (client.players[player].speed > max) {
        client.players[player].speed -= ACCEL/TICKSPERSEC;

        if (client.players[player].speed < max) {
          client.players[player].speed = max;
        }
      }

      client.players[player].tank = add2f(client.players[player].tank, div2f(add2f(mul2f(dir2vec(rounddir(client.players[player].dir)), client.players[player].speed), mul2f(dir2vec(client.players[player].kickdir), client.players[player].kickspeed)), TICKSPERSEC));

      /* reduce kickspeed */
      client.players[player].kickspeed -= 12.0/TICKSPERSEC;

      if (client.players[player].kickspeed < 0.0) {
        client.players[player].kickspeed = 0.0;
      }

      /* shore push */
      if (client.players[player].boat) {
        int x, y, fxc, cxc, fyc, cyc;
        float fx, fy, cx, cy;
        Vec2f push = make2f(0.0, 0.0);

        x = (int)client.players[player].tank.x;
        y = (int)client.players[player].tank.y;
        fx = client.players[player].tank.x - x;
        fy = client.players[player].tank.y - y;
        cx = 1.0 - fx;
        cy = 1.0 - fy;
        fxc = (fx < TANKRADIUS) && isshore(x - 1, y);
        cxc = ((1.0 - fx) < TANKRADIUS) && isshore(x + 1, y);
        fyc = (fy < TANKRADIUS) && isshore(x, y - 1);
        cyc = ((1.0 - fy) < TANKRADIUS) && isshore(x, y + 1);

        if (!fxc && !fyc && (((fx*fx + fy*fy) < (TANKRADIUS*TANKRADIUS)) && isshore(x - 1, y - 1))) {
          push.x = fx;
          push.y = fy;
        }
        else if (!cxc && !fyc && (((cx*cx + fy*fy) < (TANKRADIUS*TANKRADIUS)) && isshore(x + 1, y - 1))) {
          push.x -= cx;
          push.y = fy;
        }
        else if (!fxc && !cyc && (((fx*fx + cy*cy) < (TANKRADIUS*TANKRADIUS)) && isshore(x - 1, y + 1))) {
          push.x = fx;
          push.y -= cy;
        }
        else if (!cxc && !cyc && (((cx*cx + cy*cy) < (TANKRADIUS*TANKRADIUS)) && isshore(x + 1, y + 1))) {
          push.x -= cx;
          push.y -= cy;
        }
        else {
          if (fxc) {
            if (fyc) {
              push.x = fy;
              push.y = fx;
            }
            else if (cyc) {
              push.x = cy;
              push.y = -fx;
            }
            else {
              push.x = fx;
              push.y = 0.0;
            }
          }
          else if (cxc) {
            if (fyc) {
              push.x = -fy;
              push.y = cx;
            }
            else if (cyc) {
              push.x = -cy;
              push.y = -cx;
            }
            else {
              push.x = -cx;
              push.y = 0.0;
            }
          }
          else {
            if (fyc) {
              push.x = 0.0;
              push.y = fy;
            }
            else if (cyc) {
              push.x = 0.0;
              push.y = -cy;
            }
          }
        }

        if (mag2f(push) > 0.00001) {
          float f;

          f = mag2f(prj2f(push, mul2f(dir2vec(client.players[player].dir), client.players[player].speed)));

          if (f < PUSHFORCE) {
            client.players[player].tank = add2f(client.players[player].tank, mul2f(unit2f(push), PUSHFORCE/TICKSPERSEC));
          }

          if (!((client.players[player].inputflags & ACCELMASK) && !(client.players[player].inputflags & BRAKEMASK))) {
            client.players[player].speed -= ACCEL/TICKSPERSEC;

            if (client.players[player].speed < 0.0) {
              client.players[player].speed = 0.0;
            }
          }
        }
      }

      /* collisions with terrain */
      collisionowner = player;
      client.players[player].tank = collisiondetect(client.players[player].tank, TANKRADIUS, tankcollision);
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int tanklocallogic(Pointi old) {
  int pill, base, i, j, x, y;
  Pointi new;

TRY

  if (old.x < 0 || old.x >= WIDTH || old.y < 0 || old.y >= WIDTH) {
    SUCCESS
  }

  /* local tank logic */
  if (client.players[client.player].connected) {
    if (!client.players[client.player].dead) {
      Pointi tmp;

      tmp = makepoint(client.players[client.player].tank.x, client.players[client.player].tank.y);

      /* collisions with tanks */
      for (i = 0; i < MAXPLAYERS; i++) {
        if (i != client.player && client.players[i].connected && !client.players[i].dead) {
          Vec2f diff;
          float mag;

          diff = sub2f(client.players[client.player].tank, client.players[i].tank);
          mag = mag2f(diff);
          if (mag < TANKRADIUS*2.0) {
            if (mag < 0.00001) {
              client.players[client.player].tank = add2f(client.players[i].tank, mul2f(tan2f((random()%16)*(kPif/8.0)), TANKRADIUS*2.0));
            }
            else {
              client.players[client.player].tank = add2f(client.players[i].tank, mul2f(diff, (TANKRADIUS*2.0)/mag));
            }
          }
        }
      }

      new = makepoint(client.players[client.player].tank.x, client.players[client.player].tank.y);

      if (!isequalpoint(new, tmp)) {
        if (increasevis(makerect(new.x - 14, new.y - 14, 29, 29))) LOGFAIL(errno)
        if (decreasevis(makerect(tmp.x - 14, tmp.y - 14, 29, 29))) LOGFAIL(errno)
      }
    }
    else {
      new = makepoint(client.players[client.player].tank.x, client.players[client.player].tank.y);
    }

    /* show hidden mines within 1 square */
    x = w2t(client.players[client.player].tank.x) - 1;
    y = w2t(client.players[client.player].tank.y) - 1;
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        if (testhiddenmine(x + j, y + i)) LOGFAIL(errno)
      }
    }

    enter(new, old);

    if (new.x < 0 || new.x >= WIDTH || new.y < 0 || new.y >= WIDTH) {
      pill = -1;
    }
    else {
      pill = clientfindpill(new.x, new.y);
    }

    if (new.x < 0 || new.x >= WIDTH || new.y < 0 || new.y >= WIDTH) {
      base = -1;
    }
    else {
      base = clientfindbase(new.x, new.y);
    }
    
    /* refuel or drain */
    if (!client.players[client.player].dead) {
      /* drain resources on river terrain */
      if (!client.players[client.player].boat &&
          pill == -1 && base == -1 && client.terrain[new.y][new.x] == kRiverTerrain &&
          client.players[client.player].speed <= 0.5859375
        ) {
        client.draincounter++;

        if (client.playsound) {
          client.playsound(kBubblesSound);
        }

        if (client.draincounter >= DRAINTICKS) {
          client.draincounter = 0;
          client.shells--;

          if (client.shells < 0) {
            client.shells = 0;
          }

          client.mines--;

          if (client.mines < 0) {
            client.mines = 0;
          }

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
      }
      else {
        client.draincounter = 0;
      }

      if (!client.refueling) {
        if (base != -1) {
          client.refueling = 1;
          client.refuelingbase = base;
          client.refuelingcounter = 0;
        }
      }
      else if (isequalpoint(new, old)) {
        int armour;

        client.refuelingcounter++;

        if (
          client.armour < MAXARMOUR && client.bases[client.refuelingbase].armour > MINBASEARMOUR &&
          (armour = MIN(MAXARMOUR - client.armour, MIN(client.bases[client.refuelingbase].armour - MINBASEARMOUR, MINBASEARMOUR))) > 0
        ) {
          if (client.refuelingcounter >= REFUELARMOURTICKS) {
            sendclrefuel(client.refuelingbase, armour, 0, 0);
            client.bases[client.refuelingbase].armour -= armour;
            client.armour += armour;
            client.refuelingcounter = 0;

            if (client.settankstatus) {
              client.settankstatus();
            }
          }
        }
        else if (client.shells < MAXSHELLS && client.bases[client.refuelingbase].shells >= MINBASESHELLS) {
          if (client.refuelingcounter >= REFUELSHELLSTICKS) {
            if (client.shells > MAXSHELLS - MINBASESHELLS) {
              sendclrefuel(client.refuelingbase, 0, MAXSHELLS - client.shells, 0);
              client.bases[client.refuelingbase].shells -= MAXSHELLS - client.shells;
              client.shells += MAXSHELLS - client.shells;
            }
            else {
              sendclrefuel(client.refuelingbase, 0, MINBASESHELLS, 0);
              client.bases[client.refuelingbase].shells -= MINBASESHELLS;
              client.shells += MINBASESHELLS;
            }

            client.refuelingcounter = 0;

            if (client.settankstatus) {
              client.settankstatus();
            }
          }
        }
        else if (client.mines < MAXMINES && client.bases[client.refuelingbase].mines >= MINBASEMINES) {
          if (client.refuelingcounter >= REFUELMINESTICKS) {
            if (client.mines > MAXMINES - MINBASEMINES) {
              sendclrefuel(client.refuelingbase, 0, 0, MAXMINES - client.mines);
              client.bases[client.refuelingbase].mines -= MAXMINES - client.mines;
              client.mines += MAXMINES - client.mines;
            }
            else {
              sendclrefuel(client.refuelingbase, 0, 0, MINBASEMINES);
              client.bases[client.refuelingbase].mines -= MINBASEMINES;
              client.mines += MINBASEMINES;
            }

            client.refuelingcounter = 0;

            if (client.settankstatus) {
              client.settankstatus();
            }
          }
        }
      }
      else {
        client.refueling = 0;
        client.refuelingbase = -1;
        client.refuelingcounter = 0;
      }

      /* increase range */
      if ((client.players[client.player].inputflags & INCREMASK) && !(client.players[client.player].inputflags & DECREMASK)) {
        client.range += DRANGE/TICKSPERSEC;

        if (client.range > MAXRANGE) {
          client.range = MAXRANGE;
        }
      }  /* decrease range */
      else if (!(client.players[client.player].inputflags & INCREMASK) && (client.players[client.player].inputflags & DECREMASK)) {
        client.range -= DRANGE/TICKSPERSEC;

        if (client.range < MINRANGE) {
          client.range = MINRANGE;
        }
      }

      /* fire shell */
      if ((client.players[client.player].inputflags & SHOOTMASK) && (client.shellcounter > TICKSPERSEC/SHELLRATE)) {
        if (client.shells > 0) {
          struct Shell *shell;

          if ((shell = (struct Shell *)malloc(sizeof(struct Shell))) == NULL) LOGFAIL(errno)
          shell->owner = client.player;
          shell->point = add2f(client.players[client.player].tank, mul2f(dir2vec(client.players[client.player].dir), 0.5));
          shell->boat = client.players[client.player].boat;
          shell->pill = 0;
          shell->dir = client.players[client.player].dir;
          shell->range = client.range - 0.5;
          addlist(&client.players[client.player].shells, shell);
          client.shells--;

          client.shellcounter = 0;

          if (client.settankstatus) {
            client.settankstatus();
          }

          if (client.playsound) {
            client.playsound(kTankShotSound);
          }

          client.tankshotsound = 1;
        }
      }

      client.shellcounter++;
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int testhiddenmine(int x, int y) {
TRY
  if (mag2f(sub2f(make2f(x + 0.5, y + 0.5), client.players[client.player].tank)) <= 2.0) {
    switch (client.terrain[y][x]) {
    case kMinedSwampTerrain:
      if (refresh(x, y)) LOGFAIL(errno)
      break;

    case kMinedCraterTerrain:
      if (refresh(x, y)) LOGFAIL(errno)
      break;

    case kMinedRoadTerrain:
      if (refresh(x, y)) LOGFAIL(errno)
      break;

    case kMinedForestTerrain:
      if (refresh(x, y)) LOGFAIL(errno)
      break;

    case kMinedRubbleTerrain:
      if (refresh(x, y)) LOGFAIL(errno)
      break;

    case kMinedGrassTerrain:
      if (refresh(x, y)) LOGFAIL(errno)
      break;

    default:
      break;
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}




int tanktest(int x, int y) {
  int i;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

  for (i = 0; i < MAXPLAYERS; i++) {
    if (client.players[i].connected && !client.players[i].dead && (int)client.players[i].tank.x == x && (int)client.players[i].tank.y == y) {
      return 1;
    }
  }

  return 0;
}


int tankonaboattest(int x, int y) {
  int i;
  
  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);
  
  for (i = 0; i < MAXPLAYERS; i++) {
    if (client.players[i].connected) {
      if (!client.players[i].dead && client.players[i].boat && (int)client.players[i].tank.x == x && (int)client.players[i].tank.y == y) {
        return 1;
      }
    }
  }
  
  return 0;
}


int builderlogic(int player) {
  int i;
  Vec2f diff;
  float mag;

TRY
  if (!client.players[player].connected) {
    SUCCESS
  }

  switch (client.players[player].builderstatus) {
  case kBuilderReady:
    if (player == client.player) {
      client.buildertask = getbuildertaskforcommand(client.nextbuildercommand, client.nextbuildertarget);
      client.nextbuildercommand = BUILDERNILL;

      switch (client.buildertask) {
      case kBuilderDoNothing:
        break;

      case kBuilderGetTree:
        client.players[player].buildertarget = client.nextbuildertarget;
        client.nextbuildertarget = makepoint(0, 0);
        diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
        mag = mag2f(diff);
        client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));
        client.players[player].builderstatus = kBuilderGoto;
        client.buildermines = 0;
        client.buildertrees = 0;
        client.builderpill = NOPILL;

        if (client.settankstatus) {
          client.settankstatus();
        }

        break;

      case kBuilderBuildRoad:
        if (client.trees >= ROADTREES) {
          client.players[player].buildertarget = client.nextbuildertarget;
          client.nextbuildertarget = makepoint(0, 0);
          diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
          mag = mag2f(diff);
          client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));
          client.players[player].builderstatus = kBuilderGoto;
          client.buildermines = 0;
          client.buildertrees = ROADTREES;
          client.trees -= ROADTREES;
          client.builderpill = NOPILL;

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
        else {
          client.nextbuildertarget = makepoint(0, 0);

          if (client.printmessage) {
            client.printmessage(MSGGAME, "You need more trees.");
          }
        }

        break;

      case kBuilderBuildWall:
        if (client.trees >= WALLTREES) {
          client.players[player].buildertarget = client.nextbuildertarget;
          client.nextbuildertarget = makepoint(0, 0);
          diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
          mag = mag2f(diff);
          client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));            
          client.players[player].builderstatus = kBuilderGoto;
          client.buildermines = 0;
          client.buildertrees = WALLTREES;
          client.trees -= WALLTREES;
          client.builderpill = NOPILL;

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
        else {
          client.nextbuildertarget = makepoint(0, 0);

          if (client.printmessage) {
            client.printmessage(MSGGAME, "You need more trees.");
          }
        }

        break;

      case kBuilderBuildBoat:
        if (client.trees >= BOATTREES) {
          client.players[player].buildertarget = client.nextbuildertarget;
          client.nextbuildertarget = makepoint(0, 0);
          diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
          mag = mag2f(diff);
          client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));
          client.players[player].builderstatus = kBuilderGoto;
          client.buildermines = 0;
          client.buildertrees = BOATTREES;
          client.trees -= BOATTREES;
          client.builderpill = NOPILL;

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
        else {
          client.nextbuildertarget = makepoint(0, 0);

          if (client.printmessage) {
            client.printmessage(MSGGAME, "You need more trees.");
          }
        }

        break;

      case kBuilderBuildPill:
        if (client.trees >= 4) {
          for (i = 0; i < client.npills; i++) {
            if (client.pills[i].owner == player && client.pills[i].armour == ONBOARD) {
              client.players[player].buildertarget = client.nextbuildertarget;
              client.nextbuildertarget = makepoint(0, 0);
              diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
              mag = mag2f(diff);
              client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));
              client.players[player].builderstatus = kBuilderGoto;
              client.buildermines = 0;

              if (client.trees >= PILLTREES) {
                client.buildertrees = PILLTREES;
                client.trees -= PILLTREES;
              }
              else {
                client.buildertrees = client.trees;
                client.trees = 0;
              }

              client.builderpill = i;

              if (client.settankstatus) {
                client.settankstatus();
              }

              break;
            }
          }

          if (i == client.npills) {
            client.nextbuildertarget = makepoint(0, 0);

            if (client.printmessage) {
              client.printmessage(MSGGAME, "You need a pill.");
            }
          }
        }
        else {
          client.nextbuildertarget = makepoint(0, 0);

          if (client.printmessage) {
            client.printmessage(MSGGAME, "You need more trees.");
          }
        }

        break;

      case kBuilderRepairPill:
        if (client.trees > 0) {
          int needed;

          client.players[player].buildertarget = client.nextbuildertarget;
          client.nextbuildertarget = makepoint(0, 0);
          diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
          mag = mag2f(diff);
          client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));
          client.players[player].builderstatus = kBuilderGoto;
          client.buildermines = 0;

          switch (client.seentiles[client.players[player].buildertarget.y][client.players[player].buildertarget.x]) {
          case kFriendlyPill00Tile:
          case kFriendlyPill01Tile:
          case kFriendlyPill02Tile:
          case kFriendlyPill03Tile:
          case kFriendlyPill04Tile:
          case kFriendlyPill05Tile:
          case kFriendlyPill06Tile:
          case kFriendlyPill07Tile:
          case kFriendlyPill08Tile:
          case kFriendlyPill09Tile:
          case kFriendlyPill10Tile:
          case kFriendlyPill11Tile:
          case kFriendlyPill12Tile:
          case kFriendlyPill13Tile:
          case kFriendlyPill14Tile:
            needed = (kFriendlyPill15Tile - client.seentiles[client.players[player].buildertarget.y][client.players[player].buildertarget.x] + 3)/4;
            break;

          case kHostilePill00Tile:
          case kHostilePill01Tile:
          case kHostilePill02Tile:
          case kHostilePill03Tile:
          case kHostilePill04Tile:
          case kHostilePill05Tile:
          case kHostilePill06Tile:
          case kHostilePill07Tile:
          case kHostilePill08Tile:
          case kHostilePill09Tile:
          case kHostilePill10Tile:
          case kHostilePill11Tile:
          case kHostilePill12Tile:
          case kHostilePill13Tile:
          case kHostilePill14Tile:
            needed = (kHostilePill15Tile - client.seentiles[client.players[player].buildertarget.y][client.players[player].buildertarget.x] + 3)/4;
            break;

          default:
            needed = 0;
            break;
          }

          if (client.trees > needed) {
            client.buildertrees += needed;
            client.trees -= needed;
          }
          else {
            client.buildertrees += client.trees;
            client.trees = 0;
          }

          client.builderpill = NOPILL;

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
        else {
          client.nextbuildertarget = makepoint(0, 0);

          if (client.printmessage) {
            client.printmessage(MSGGAME, "You need more trees.");
          }
        }

        break;

      case kBuilderPlaceMine:
        if (client.mines > 0) {
          client.players[player].buildertarget = client.nextbuildertarget;
          client.nextbuildertarget = makepoint(0, 0);
          diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].tank);
          mag = mag2f(diff);
          client.players[player].builder = mag <= (TANKRADIUS - BUILDERRADIUS) ? make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5) : add2f(client.players[player].tank, mul2f(diff, (TANKRADIUS - BUILDERRADIUS)/mag));            
          client.players[player].builderstatus = kBuilderGoto;
          client.buildermines = 1;
          client.mines -= 1;
          client.builderpill = NOPILL;

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
        else {
          client.nextbuildertarget = makepoint(0, 0);

          if (client.printmessage) {
            client.printmessage(MSGGAME, "You need more mines.");
          }
        }

        break;

      default:
        break;
      }
    }

    break;

  case kBuilderGoto:
    diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].builder);

    if (mag2f(diff) < 0.00001) {
      if (player == client.player) {
        switch (client.buildertask) {
        case kBuilderGetTree:
          if (sendclgrabtrees(client.players[player].buildertarget.x, client.players[player].buildertarget.y) == -1) LOGFAIL(errno)
          client.players[player].builderstatus = kBuilderWork;
          break;

        case kBuilderBuildRoad:
          if (!tankonaboattest(client.players[player].buildertarget.x, client.players[player].buildertarget.y)) {
            if (sendclbuildroad(client.players[player].buildertarget.x, client.players[player].buildertarget.y, client.buildertrees) == -1) LOGFAIL(errno)
            client.players[player].builderstatus = kBuilderWork;
          }
          else {
            client.players[player].builderstatus = kBuilderWait;
          }

          break;

        case kBuilderBuildWall:
          if (!tanktest(client.players[player].buildertarget.x, client.players[player].buildertarget.y)) {
            if (sendclbuildwall(client.players[player].buildertarget.x, client.players[player].buildertarget.y, client.buildertrees) == -1) LOGFAIL(errno)
            client.players[player].builderstatus = kBuilderWork;
          }
          else {
            client.players[player].builderstatus = kBuilderWait;
          }

          break;

        case kBuilderBuildBoat:
          if (!tanktest(client.players[player].buildertarget.x, client.players[player].buildertarget.y)) {
            if (sendclbuildboat(client.players[player].buildertarget.x, client.players[player].buildertarget.y, client.buildertrees) == -1) LOGFAIL(errno)
            client.players[player].builderstatus = kBuilderWork;
          }
          else {
            client.players[player].builderstatus = kBuilderWait;
          }

          break;

        case kBuilderBuildPill:
          if (!tanktest(client.players[player].buildertarget.x, client.players[player].buildertarget.y)) {
            if (sendclbuildpill(client.players[player].buildertarget.x, client.players[player].buildertarget.y, client.buildertrees, client.builderpill) == -1) LOGFAIL(errno)
            client.players[player].builderstatus = kBuilderWork;
          }
          else {
            client.players[player].builderstatus = kBuilderWait;
          }

          break;

        case kBuilderRepairPill:
          if (!tanktest(client.players[player].buildertarget.x, client.players[player].buildertarget.y)) {
            if (sendclrepairpill(client.players[player].buildertarget.x, client.players[player].buildertarget.y, client.buildertrees) == -1) LOGFAIL(errno)
            client.players[player].builderstatus = kBuilderWork;
          }
          else {
            client.players[player].builderstatus = kBuilderWait;
          }

          break;

        case kBuilderPlaceMine:
          if (sendclplacemine(client.players[player].buildertarget.x, client.players[player].buildertarget.y) == -1) LOGFAIL(errno)
          client.players[player].builderstatus = kBuilderWork;
          break;

        default:
          assert(0);
          break;
        }
      }
    }
    else {
      float speed;

      if (isequalpoint(makepoint(client.players[player].builder.x, client.players[player].builder.y), client.players[player].buildertarget)) {
        speed = buildertargetspeed(client.players[player].builder.x, client.players[player].builder.y);
      }
      else {
        if (client.players[player].boat && mag2f(sub2f(client.players[player].builder, client.players[player].tank)) < TANKRADIUS + BUILDERRADIUS) {
          speed = BUILDERMAXSPEED;
        }
        else {
          speed = builderspeed(client.players[player].builder.x, client.players[player].builder.y, player);
        }
      }

      if (mag2f(diff) > speed/TICKSPERSEC) {
        diff = mul2f(diff, speed/(TICKSPERSEC*mag2f(diff)));

        target = client.players[player].buildertarget;
        buildertask = client.buildertask;
        collisionowner = player;
        diff = sub2f(collisiondetect(add2f(client.players[player].builder, diff), BUILDERRADIUS, buildercollision), client.players[player].builder);

        if (mag2f(diff) <= 0.00128*speed) {
          client.players[player].builderstatus = kBuilderReturn;
        }
        else {
          client.players[player].builder = collisiondetect(add2f(client.players[player].builder, mul2f(diff, speed/(TICKSPERSEC*mag2f(diff)))), BUILDERRADIUS, buildercollision);
        }
      }
      else {
        client.players[player].builder = make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5);
      }
    }

    break;

  case kBuilderWork:
    break;

  case kBuilderWait:
    if (client.players[player].builderwait++ > BUILDERBUILDTIME) {
      client.players[player].builderstatus = kBuilderReturn;
    }

    break;

  case kBuilderReturn:
    if (!client.players[player].dead) {
      float speed;
      int col;

      if (
        mag2f(sub2f(client.players[player].builder, client.players[player].tank)) <= 1.5*(TANKRADIUS + BUILDERRADIUS)
      ) {
        speed = BUILDERMAXSPEED;
        col = 0;
      }
      else {
        if (isequalpoint(makepoint(client.players[player].builder.x, client.players[player].builder.y), client.players[player].buildertarget)) {
          speed = buildertargetspeed(client.players[player].builder.x, client.players[player].builder.y);
        }
        else {
          speed = builderspeed(client.players[player].builder.x, client.players[player].builder.y, player);
        }
        col = 1;
      }

      diff = sub2f(client.players[player].tank, client.players[player].builder);

      /* builder enters tank */
      if (mag2f(diff) <= TANKRADIUS - BUILDERRADIUS) {
        client.players[player].builderstatus = kBuilderReady;
        client.players[player].buildertarget = makepoint(0, 0);

        if (player == client.player) {
          client.buildertask = kBuilderDoNothing;
          client.mines += client.buildermines;
          client.trees += client.buildertrees;
          client.buildermines = 0;
          client.buildertrees = 0;
          client.builderpill = NOPILL;

          if (client.mines > MAXMINES) {
            client.mines = MAXMINES;
          }

          if (client.trees > MAXTREES) {
            client.trees = MAXTREES;
          }

          if (client.settankstatus) {
            client.settankstatus();
          }
        }
      }
      else {
        diff = mul2f(diff, speed/(TICKSPERSEC*mag2f(diff)));

        if (col) {
          target = client.players[player].buildertarget;
          buildertask = client.buildertask;
          collisionowner = player;
          diff = sub2f(collisiondetect(add2f(client.players[player].builder, diff), BUILDERRADIUS, buildercollision), client.players[player].builder);

          if (mag2f(diff) >= 0.00001) {
            client.players[player].builder = collisiondetect(add2f(client.players[player].builder, mul2f(diff, speed/(TICKSPERSEC*mag2f(diff)))), BUILDERRADIUS, buildercollision);
          }
        }
        else {
          client.players[player].builder = add2f(client.players[player].builder, diff);
        }

        if (player == client.player && !circlesquare(client.players[player].builder, BUILDERRADIUS, client.players[player].buildertarget)) {
          client.buildertask = kBuilderDoNothing;
        }
      }
    }

    break;

  case kBuilderParachute:
    diff = sub2f(make2f(client.players[player].buildertarget.x + 0.5, client.players[player].buildertarget.y + 0.5), client.players[player].builder);

    if (mag2f(diff) < 0.001) {
      client.players[player].buildertarget = makepoint(0, 0);
      client.players[player].builderstatus = kBuilderReturn;
    }
    else {
      if (mag2f(diff) > PARACHUTESPEED/TICKSPERSEC) {
        diff = mul2f(diff, PARACHUTESPEED/(TICKSPERSEC*mag2f(diff)));
      }

      client.players[player].builder = add2f(client.players[player].builder, diff);
    }

    break;

  default:
    break;
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int pilllogic(Vec2f old) {
TRY
  if (client.players[client.player].dead) {
    SUCCESS;
  }

    int i;

    for (i = 0; i < client.npills; i++) {
      if (
        !client.players[client.player].dead &&
        client.pills[i].armour != ONBOARD &&
        client.pills[i].armour > 0 &&
        (client.pills[i].owner == NEUTRAL || !testalliance(client.pills[i].owner, client.player))
      ) {
        Vec2f pill, diff;
        float mag;

        pill = make2f(client.pills[i].x + 0.5, client.pills[i].y + 0.5);
        diff = sub2f(client.players[client.player].tank, pill);
        mag = mag2f(diff);

        if ((mag <= 2.0 || forestvis(client.players[client.player].tank) > 0.25) && mag <= 8.0) {
          int j;

          for (j = 0; j < MAXPLAYERS; j++) {
            if (
              j != client.player &&
              client.players[j].connected &&
              !client.players[j].dead &&
              (client.pills[i].owner == NEUTRAL || !testalliance(client.pills[i].owner, j)) &&
              mag2f(sub2f(client.players[j].tank, pill)) < mag &&
              (mag2f(sub2f(client.players[j].tank, pill)) <= 2.0 || forestvis(client.players[j].tank) > 0.25)
            ) {
              break;
            }
          }

          if (!(j < MAXPLAYERS)) {
            client.pills[i].counter++;

            if (client.pills[i].counter >= client.pills[i].speed) {
              struct Shell *shell;
              Vec2f vel, compi, compj;
              int col;

              vel = mul2f(sub2f(client.players[client.player].tank, old), TICKSPERSEC);
              compi = sub2f(vel, prj2f(diff, vel));
              compj = mul2f(unit2f(diff), sqrtf(fabsf((float)((SHELLVEL*SHELLVEL) - dot2f(compi, compi)))));  /* fabsf is a cludge */

              if ((shell = (struct Shell *)malloc(sizeof(struct Shell))) == NULL) LOGFAIL(errno)
              shell->owner = client.pills[i].owner;
              shell->point = add2f(pill, mul2f(diff, 0.70711219/mag));
              shell->boat = 0;
              shell->pill = 1;
              shell->dir = vec2dir(add2f(compi, compj));

              shell->range = 8.5 - 0.70711219;

              if ((col = shellcollisiontest(shell, client.player)) == -1) LOGFAIL(errno)

              if (col) {
                free(shell);
              }
              else {
                addlist(&client.players[client.player].shells, shell);
              }

              client.pills[i].counter = 0;

              if (client.playsound) {
                client.playsound(kPillShotSound);
              }

              client.pillshotsound = 1;
            }
          }
          else {
            client.pills[i].counter = 0;
          }
        }
      }
      else {
        client.pills[i].counter = 0;
      }
    }

CLEANUP
ERRHANDLER(0, -1)
END
}


int shellcollisiontest(struct Shell *shell, int player) {
  Pointi p;
  int pill, base, ret;
  struct Explosion *explosion = NULL;

  assert(shell != NULL);
  assert(shell->point.x >= 0.0);
  assert(shell->point.x < FWIDTH);
  assert(shell->point.y >= 0.0);
  assert(shell->point.y < FWIDTH);

TRY
  p = makepoint(shell->point.x, shell->point.y);

  if ((pill = clientfindpill(p.x, p.y)) != -1) {
    if (client.pills[pill].armour > 0) {
      if (player == client.player) {
        sendcldamage(p.x, p.y, shell->boat);
        if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
        explosion->point.x = p.x + 0.5;
        explosion->point.y = p.y + 0.5;
        explosion->counter = 0;
        addlist(&client.explosions, explosion);
        explosion = NULL;
        if (killsquarebuilder(p)) LOGFAIL(errno)
      }

      ret = 1;
    }
    else {
      ret = 0;
    }

    SUCCESS
  }

  if ((base = clientfindbase(p.x, p.y)) != -1) {
    if (!shell->pill) {
      if (shell->boat) {
        if (player == client.player) {
          if (
            client.bases[base].owner != NEUTRAL && shell->owner != NEUTRAL && client.bases[base].armour >= MINBASEARMOUR &&
            !testalliance(client.bases[base].owner, shell->owner)
          ) {
            sendcldamage(p.x, p.y, shell->boat);
            if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
            explosion->point.x = p.x + 0.5;
            explosion->point.y = p.y + 0.5;
            explosion->counter = 0;
            addlist(&client.explosions, explosion);
            explosion = NULL;
            if (killsquarebuilder(p)) LOGFAIL(errno)
          }
          else {
            if ((explosion = (void *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
            explosion->point = shell->point;
            explosion->counter = 0;
            addlist(&client.players[client.player].explosions, explosion);
            explosion = NULL;
            if (killpointbuilder(shell->point)) LOGFAIL(errno)
          }
        }

        ret = 1;
      }
      else {
        if (
          client.bases[base].owner != NEUTRAL && shell->owner != NEUTRAL && client.bases[base].armour >= MINBASEARMOUR &&
          !testalliance(client.bases[base].owner, shell->owner)
        ) {
          if (player == client.player) {
            sendcldamage(p.x, p.y, shell->boat);
            if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
            explosion->point.x = p.x + 0.5;
            explosion->point.y = p.y + 0.5;
            explosion->counter = 0;
            addlist(&client.explosions, explosion);
            explosion = NULL;
            if (killsquarebuilder(p)) LOGFAIL(errno)
          }

          ret = 1;
        }
        else {
          ret = 0;
        }
      }
    }
    else {
      ret = 0;
    }

    SUCCESS
  }

  if (shell->boat) {
    switch (client.terrain[p.y][p.x]) {
    case kSeaTerrain:  /* sea */
    case kRiverTerrain:  /* river */
    case kMinedSeaTerrain:  /* mined sea */
    case kCraterTerrain:  /* crater */
      ret = 0;
      break;

    case kWallTerrain:  /* wall */
    case kSwampTerrain0:  /* swamp */
    case kSwampTerrain1:  /* swamp */
    case kSwampTerrain2:  /* swamp */
    case kSwampTerrain3:  /* swamp */
    case kForestTerrain:  /* forest */
    case kRubbleTerrain0:  /* rubble */
    case kRubbleTerrain1:  /* rubble */
    case kRubbleTerrain2:  /* rubble */
    case kRubbleTerrain3:  /* rubble */
    case kGrassTerrain0:  /* grass */
    case kGrassTerrain1:  /* grass */
    case kGrassTerrain2:  /* grass */
    case kGrassTerrain3:  /* grass */
    case kDamagedWallTerrain0:  /* damaged wall */
    case kDamagedWallTerrain1:  /* damaged wall */
    case kDamagedWallTerrain2:  /* damaged wall */
    case kDamagedWallTerrain3:  /* damaged wall */
    case kBoatTerrain:  /* river w/boat */
    case kMinedSwampTerrain:  /* mined swamp */
    case kMinedCraterTerrain:  /* mined crater */
    case kMinedRoadTerrain:  /* mined road */
    case kMinedForestTerrain:  /* mined forest */
    case kMinedRubbleTerrain:  /* mined rubble */
    case kMinedGrassTerrain:  /* mined grass */
      if (player == client.player) {
        sendcldamage(p.x, p.y, shell->boat);
        if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
        explosion->point.x = p.x + 0.5;
        explosion->point.y = p.y + 0.5;
        explosion->counter = 0;
        addlist(&client.explosions, explosion);
        explosion = NULL;
        if (killsquarebuilder(p)) LOGFAIL(errno)
      }

      ret = 1;
      break;

    case kRoadTerrain:  /* road */
      if (player == client.player) {
        if (
          (isWaterLikeTerrain(client.terrain[p.y][p.x - 1]) && isWaterLikeTerrain(client.terrain[p.y][p.x + 1])) ||
          (isWaterLikeTerrain(client.terrain[p.y - 1][p.x]) && isWaterLikeTerrain(client.terrain[p.y + 1][p.x]))
        ) {
          sendcldamage(p.x, p.y, shell->boat);
          if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
          explosion->point.x = p.x + 0.5;
          explosion->point.y = p.y + 0.5;
          explosion->counter = 0;
          addlist(&client.explosions, explosion);
          explosion = NULL;
          if (killsquarebuilder(p)) LOGFAIL(errno)
        }
        else {
          if ((explosion = (void *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
          explosion->point = shell->point;
          explosion->counter = 0;
          addlist(&client.players[client.player].explosions, explosion);
          explosion = NULL;
          if (killpointbuilder(shell->point)) LOGFAIL(errno)
        }
      }

      ret = 1;
      break;

    default:
      ret = 0;
      break;
    }
  }
  else {
    switch (client.terrain[p.y][p.x]) {
    case kSeaTerrain:  /* sea */
    case kRiverTerrain:  /* river */
    case kMinedSeaTerrain:  /* mined sea */
    case kSwampTerrain0:  /* swamp */
    case kSwampTerrain1:  /* swamp */
    case kSwampTerrain2:  /* swamp */
    case kSwampTerrain3:  /* swamp */
    case kCraterTerrain:  /* crater */
    case kRoadTerrain:  /* road */
    case kRubbleTerrain0:  /* rubble */
    case kRubbleTerrain1:  /* rubble */
    case kRubbleTerrain2:  /* rubble */
    case kRubbleTerrain3:  /* rubble */
    case kGrassTerrain0:  /* grass */
    case kGrassTerrain1:  /* grass */
    case kGrassTerrain2:  /* grass */
    case kGrassTerrain3:  /* grass */
    case kMinedSwampTerrain:  /* mined swamp */
    case kMinedCraterTerrain:  /* mined crater */
    case kMinedRoadTerrain:  /* mined road */
    case kMinedRubbleTerrain:  /* mined rubble */
    case kMinedGrassTerrain:  /* mined grass */
      ret = 0;
      break;

    case kWallTerrain:  /* wall */
    case kForestTerrain:  /* forest */
    case kDamagedWallTerrain0:  /* damaged wall */
    case kDamagedWallTerrain1:  /* damaged wall */
    case kDamagedWallTerrain2:  /* damaged wall */
    case kDamagedWallTerrain3:  /* damaged wall */
    case kBoatTerrain:  /* river w/boat */
    case kMinedForestTerrain:  /* mined forest */
      if (player == client.player) {
        sendcldamage(p.x, p.y, shell->boat);
        if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
        explosion->point.x = p.x + 0.5;
        explosion->point.y = p.y + 0.5;
        explosion->counter = 0;
        addlist(&client.explosions, explosion);
        explosion = NULL;
        if (killsquarebuilder(p)) LOGFAIL(errno)
      }

      ret = 1;
      break;

    default:
      ret = 0;
      break;
    }
  }

CLEANUP
  switch (ERROR) {
  case 0:
    RETURN(ret)
//    RETURN(0)  /* use with caution */

  default:
    if (explosion) {
      free(explosion);
    }

    RETERR(-1)
  }
END
}


int shelllogic(int player) {
  struct ListNode *node;
  struct Shell *shell;
  struct Explosion *explosion = NULL;

TRY
  /* move shells */
  if (client.players[player].connected) {
    node = nextlist(&client.players[player].shells);

    while (node != NULL) {
      shell = ptrlist(node);

      if (shell->range < SHELLVEL/TICKSPERSEC) {
        shell->point = add2f(shell->point, mul2f(dir2vec(shell->dir), shell->range));
        shell->range = 0.0;
      }
      else {
        shell->point = add2f(shell->point, mul2f(dir2vec(shell->dir), SHELLVEL/TICKSPERSEC));
        shell->range -= SHELLVEL/TICKSPERSEC;
      }

      node = nextlist(node);
    }

    /* test for collisions with pills, bases and terrain */
    node = nextlist(&client.players[player].shells);

    while (node != NULL) {
      int col;

      shell = ptrlist(node);

      if ((col = shellcollisiontest(shell, player)) == -1) LOGFAIL(errno)

      if (col) {
        node = removelist(node, free);
      }
      else {
        node = nextlist(node);
      }
    }

    /* test for collisions with tanks */
    if (!client.players[player].dead) {
      node = nextlist(&client.players[client.player].shells);

      while (node != NULL) {
        shell = ptrlist(node);

        if (mag2f(sub2f(shell->point, client.players[player].tank)) <= TANKRADIUS) {
          if ((explosion = (void *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
          explosion->point = shell->point;
          explosion->counter = 0;
          addlist(&client.players[client.player].explosions, explosion);
          explosion = NULL;
          if (killpointbuilder(shell->point)) LOGFAIL(errno)
          client.players[player].kickdir = shell->dir;
          client.players[player].kickspeed = KICKFORCE;

          if (player == client.player) {
            client.players[client.player].boat = 0;

            client.armour -= 5;

            if (client.armour < 0) {
              client.armour = 0;
              killtank();
            }

            if (client.settankstatus) {
              client.settankstatus();
            }
          }
          else {
            if (sendclhittank(player, shell->dir)) LOGFAIL(errno)
          }

          if (client.playsound) {
            client.playsound(kHitTankSound);
          }

          node = removelist(node, free);
        }
        else {
          node = nextlist(node);
        }
      }
    }

    /* explode shells */
    node = nextlist(&client.players[player].shells);

    while (node != NULL) {
      shell = ptrlist(node);

      if (shell->range <= 0.0) {        
        explosion = (void *)malloc(sizeof(struct Explosion));
        if (explosion == NULL) LOGFAIL(errno)
        explosion->point = shell->point;
        explosion->counter = 0;
        addlist(&client.players[player].explosions, explosion);
        explosion = NULL;
        if (killpointbuilder(shell->point)) LOGFAIL(errno)

        if (player == client.player) {
          sendcltouch(w2t(shell->point.x), w2t(shell->point.y));
        }

        node = removelist(node, free);
      }
      else {
        node = nextlist(node);
      }
    }
  }

CLEANUP
  if (explosion) {
    free(explosion);
  }

ERRHANDLER(0, -1)
END
}


int explosionlogic(int player) {
  struct ListNode *node;

TRY
  if (player == -1) {
    node = nextlist(&client.explosions);

    while (node != NULL) {
      struct Explosion *explosion;

      explosion = ptrlist(node);
      explosion->counter++;

      if (explosion->counter > EXPLOSIONTICKS) {
        node = removelist(node, free);
      }
      else {
        node = nextlist(node);
      }
    }
  }
  else {
    if (client.players[player].connected) {
      node = nextlist(&client.players[player].explosions);

      while (node != NULL) {
        struct Explosion *explosion;

        explosion = ptrlist(node);
        explosion->counter++;

        if (explosion->counter > EXPLOSIONTICKS) {
          node = removelist(node, free);
        }
        else {
          node = nextlist(node);
        }
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int killtank() {
TRY
  if (!client.players[client.player].dead) {
    int j, pills;
    pills = 0;

    for (j = 0; j < client.npills; j++) {
      if (client.pills[j].owner == client.player && j != client.builderpill && client.pills[j].armour == ONBOARD) {
        pills |= 1 << j;
      }
    }

    if (sendcldroppills(client.players[client.player].tank.x, client.players[client.player].tank.y, pills)) LOGFAIL(errno)

    client.deaths++;
    client.players[client.player].dead = 1;
    client.players[client.player].boat = 0;
    client.respawncounter = 0;

    if (client.settankstatus) {
      client.settankstatus();
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int drown() {
TRY
  if (!client.players[client.player].dead || client.respawncounter <= EXPLODETICKS) {
    client.players[client.player].boat = 0;
    client.players[client.player].kickspeed = 0.0;
    client.respawncounter = EXPLODETICKS + 1;

    /* play sound */
    if (client.playsound) {
      client.playsound(kSinkSound);
    }

    client.sinksound = 1;
  }

  if (!client.players[client.player].dead) {
    int j, pills;
    pills = 0;

    for (j = 0; j < client.npills; j++) {
      if (client.pills[j].owner == client.player && j != client.builderpill && client.pills[j].armour == ONBOARD) {
        pills |= 1 << j;
      }
    }

    if (sendcldroppills(client.players[client.player].tank.x, client.players[client.player].tank.y, pills)) LOGFAIL(errno)

    client.deaths++;
    client.players[client.player].dead = 1;

    if (client.settankstatus) {
      client.settankstatus();
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int smallboom() {
TRY
  if (!client.players[client.player].dead || client.respawncounter <= EXPLODETICKS) {
    client.players[client.player].boat = 0;
    client.players[client.player].kickspeed = 0.0;
    client.respawncounter = EXPLODETICKS + 1;
    if (sendclsmallboom(w2t(client.players[client.player].tank.x), w2t(client.players[client.player].tank.y))) LOGFAIL(errno)
  }

  if (!client.players[client.player].dead) {
    int j, pills;
    pills = 0;

    for (j = 0; j < client.npills; j++) {
      if (client.pills[j].owner == client.player && j != client.builderpill && client.pills[j].armour == ONBOARD) {
        pills |= 1 << j;
      }
    }

    if (sendcldroppills(client.players[client.player].tank.x, client.players[client.player].tank.y, pills)) LOGFAIL(errno)
    client.deaths++;
    client.players[client.player].dead = 1;

    if (client.settankstatus) {
      client.settankstatus();
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int superboom() {
  int x, y;
  Vec2f point;
  struct Explosion *explosion = NULL;

TRY
  if (!client.players[client.player].dead || client.respawncounter <= EXPLODETICKS) {
    client.players[client.player].boat = 0;
    client.players[client.player].kickspeed = 0.0;
    client.respawncounter = EXPLODETICKS + 1;

    /* superboom! */
    x = (int)client.players[client.player].tank.x;

    if (client.players[client.player].tank.x - ((float)((int)client.players[client.player].tank.x)) < 0.5) {
      x--;
    }

    y = (int)client.players[client.player].tank.y;

    if (client.players[client.player].tank.y - ((float)((int)client.players[client.player].tank.y)) < 0.5) {
      y = ((int)client.players[client.player].tank.y) - 1;
    }

    if (sendclsuperboom(x, y)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = x + 0.5;
    explosion->point.y = y + 0.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(x, y))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = x + 1.5;
    explosion->point.y = y + 0.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(x + 1, y))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = x + 0.5;
    explosion->point.y = y + 1.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(x, y + 1))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    explosion->point.x = x + 1.5;
    explosion->point.y = y + 1.5;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killsquarebuilder(makepoint(x + 1, y + 1))) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = x + 0.25;
    point.y = y + 1.0;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = x + 1.0;
    point.y = y + 0.25;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = x + 1.75;
    point.y = y + 1.0;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = x + 1.0;
    point.y = y + 1.75;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
    point.x = x + 1.0;
    point.y = y + 1.0;
    explosion->point = point;
    explosion->counter = 0;
    if (addlist(&client.explosions, explosion)) LOGFAIL(errno)
    explosion = NULL;
    if (killpointbuilder(point)) LOGFAIL(errno)

    /* play sound */
    if (client.playsound) {
      if (client.fog[y][x] > 0) {
        client.playsound(kSuperBoomSound);
      }
      else {
        client.playsound(kFarSuperBoomSound);
      }
    }
  }

  if (!client.players[client.player].dead) {
    int j, pills;
    pills = 0;

    for (j = 0; j < client.npills; j++) {
      if (client.pills[j].owner == client.player && j != client.builderpill && client.pills[j].armour == ONBOARD) {
        pills |= 1 << j;
      }
    }

    if (sendcldroppills(client.players[client.player].tank.x, client.players[client.player].tank.y, pills)) LOGFAIL(errno)
    client.deaths++;
    client.players[client.player].dead = 1;
  }

CLEANUP
  if (explosion) {
    free(explosion);
  }

ERRHANDLER(0, -1)
END
}


int enter(Pointi new, Pointi old) {
  int pill, base;

  assert(new.x >= 0 && new.x < WIDTH && new.y >= 0 && new.y < WIDTH && old.x >= 0 && old.x < WIDTH && old.y >= 0 && old.y < WIDTH);

TRY
//  if (!(new.x >= 0 && new.x < WIDTH && new.y >= 0 && new.y < WIDTH && old.x >= 0 && old.x < WIDTH && old.y >= 0 && old.y < WIDTH)) {
//    SUCCESS
//  }

  if ((pill = clientfindpill(new.x, new.y)) != -1) {
    if (client.pills[pill].armour > 0) {
      if (superboom()) LOGFAIL(errno)
    }
    else {
      if (!client.players[client.player].dead) {
        if (!isequalpoint(new, old)) {
          if (sendclgrabtile(new.x, new.y)) LOGFAIL(errno)
        }

        switch (client.terrain[new.y][new.x]) {
        case kSwampTerrain0:  /* swamp */
        case kSwampTerrain1:  /* swamp */
        case kSwampTerrain2:  /* swamp */
        case kSwampTerrain3:  /* swamp */
        case kCraterTerrain:  /* crater */
        case kRoadTerrain:  /* road */
        case kForestTerrain:  /* forest */
        case kRubbleTerrain0:  /* rubble */
        case kRubbleTerrain1:  /* rubble */
        case kRubbleTerrain2:  /* rubble */
        case kRubbleTerrain3:  /* rubble */
        case kGrassTerrain0:  /* grass */
        case kGrassTerrain1:  /* grass */
        case kGrassTerrain2:  /* grass */
        case kGrassTerrain3:  /* grass */
          /* drop boat */
          if (client.players[client.player].boat && !isequalpoint(new, old)) {
            client.players[client.player].boat = 0;
            sendcldropboat(old.x, old.y);
          }

          break;

        default:
          break;
        }
      }
    }

    SUCCESS
  }

  if ((base = clientfindbase(new.x, new.y)) != -1) {
    if (!client.players[client.player].dead) {  /* if not dead */
      if (!isequalpoint(new, old)) {  /* if entered square */
        if /* if base is neutral or not in alliance */
          (
            client.bases[base].owner == NEUTRAL ||
            !testalliance(client.bases[base].owner, client.player)
          ) {
          if (sendclgrabtile(new.x, new.y)) LOGFAIL(errno)
        }

        if (client.players[client.player].boat) {  /* if have boat */
          client.players[client.player].boat = 0;
          sendcldropboat(old.x, old.y);
        }
      }
    }

    SUCCESS
  }

  switch (client.terrain[new.y][new.x]) {
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
    if (superboom()) LOGFAIL(errno)
    SUCCESS

  case kSeaTerrain:  /* sea */
    if (!client.players[client.player].boat) {
      drown();
    }

    SUCCESS

  case kRiverTerrain:  /* river */
    SUCCESS

  case kForestTerrain:  /* forest */
    if (client.players[client.player].dead && client.respawncounter < EXPLODETICKS && !isequalpoint(new, old)) {
      struct Explosion *explosion;
      if (sendcldamage(new.x, new.y, 0)) LOGFAIL(errno)
      if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
      explosion->point.x = new.x + 0.5;
      explosion->point.y = new.y + 0.5;
      explosion->counter = 0;
      addlist(&client.explosions, explosion);
      if (killsquarebuilder(new)) LOGFAIL(errno)
    }

  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRoadTerrain:  /* road */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
    /* drop boat */
    if (!client.players[client.player].dead && client.players[client.player].boat && !isequalpoint(new, old)) {
      client.players[client.player].boat = 0;
      sendcldropboat(old.x, old.y);
    }
  
    /* plant mines */
    if (!client.players[client.player].dead && client.players[client.player].inputflags & LMINEMASK && client.mines > 0 && !isequalpoint(new, old)) {
      client.mines--;
      if (sendcldropmine(new.x, new.y) == -1) LOGFAIL(errno)
    }

    SUCCESS

  case kBoatTerrain:  /* river w/boat */
    if (!isequalpoint(new, old)) {
      if (client.players[client.player].boat) {
        struct Explosion *explosion;
        if (sendcldamage(new.x, new.y, 0)) LOGFAIL(errno)
        if ((explosion = (struct Explosion *)malloc(sizeof(struct Explosion))) == NULL) LOGFAIL(errno)
        explosion->point.x = new.x + 0.5;
        explosion->point.y = new.y + 0.5;
        explosion->counter = 0;
        addlist(&client.explosions, explosion);
        if (killsquarebuilder(new)) LOGFAIL(errno)
      }
      else {
        if (sendclgrabtile(new.x, new.y)) LOGFAIL(errno)
      }
    }

    SUCCESS

  case kMinedSeaTerrain:  /* mined sea */
    if (!isequalpoint(new, old)) {
      if (sendclgrabtile(new.x, new.y)) LOGFAIL(errno)
    }

    drown();
    SUCCESS

  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRoadTerrain:  /* mined road */
  case kMinedForestTerrain:  /* mined forest */
  case kMinedRubbleTerrain:  /* mined rubble */
  case kMinedGrassTerrain:  /* mined grass */
    if (!isequalpoint(new, old)) {
      if (sendclgrabtile(new.x, new.y)) LOGFAIL(errno)
    }

    SUCCESS

  default:
    assert(0);
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int spawn() {
  int start, i, j;
  int weights[MAX_STARTS], range, index;

TRY
  for (i = 0; i < client.nstarts; i++) {
    weights[i] = 1;

    for (j = 0; j < client.nbases; j++) {
      if (client.bases[j].owner == NEUTRAL || testalliance(client.bases[j].owner, client.player)) {
        float dist = mag2f(sub2f(make2f(client.starts[i].x + 0.5, client.starts[i].y + 0.5), make2f(client.bases[j].x + 0.5, client.bases[j].y + 0.5)));

        if (dist < 8.5) {
          if (weights[i] < 3) {
            weights[i] = 3;
          }
        }
        else if (dist < 17) {
          if (weights[i] < 2) {
            weights[i] = 2;
          }
        }
      }
    }

    for (j = 0; j < client.npills; j++) {
      if (!testalliance(client.pills[j].owner, client.player)) {
        if (mag2f(sub2f(make2f(client.starts[i].x + 0.5, client.starts[i].y + 0.5), make2f(client.pills[j].x + 0.5, client.pills[j].y + 0.5))) < 8.5) {
          weights[i] = 0;
        }
      }
    }
  }

  range = 0;

  for (i = 0; i < client.nstarts; i++) {
    range += weights[i];
  }

  /* all starting locations are spiked need to reassess */
  if (range == 0) {
    for (i = 0; i < client.nstarts; i++) {
      weights[i] = 1;

      for (j = 0; j < client.nbases; j++) {
        if (client.bases[j].owner == NEUTRAL || testalliance(client.bases[j].owner, client.player)) {
          float dist = mag2f(sub2f(make2f(client.starts[i].x + 0.5, client.starts[i].y + 0.5), make2f(client.bases[j].x + 0.5, client.bases[j].y + 0.5)));

          if (dist < 8.5) {
            if (weights[i] < 3) {
              weights[i] = 3;
            }
          }
          else if (dist < 17) {
            if (weights[i] < 2) {
              weights[i] = 2;
            }
          }
        }
      }
    }

    range = 0;

    for (i = 0; i < client.nstarts; i++) {
      range += weights[i];
    }
  }

  index = random()%range;
  range = 0;

  for (i = 0; i < client.nstarts; i++) {
    range += weights[i];

    if (range > index) {
      break;
    }
  }

  start = i;

  client.players[client.player].dead = 0;
  client.players[client.player].tank.x = client.starts[start].x + 0.5;
  client.players[client.player].tank.y = client.starts[start].y + 0.5;
  client.players[client.player].dir = client.starts[start].dir*(kPif/8.0);
  client.players[client.player].speed = 0.0;
  client.players[client.player].turnspeed = 0.0;
  client.players[client.player].kickspeed = 0.0;
  client.players[client.player].kickdir = 0.0;
  client.range = MAXRANGE;
  client.players[client.player].boat = 1;

  if (client.gametype == kDominationGameType) {
    if (client.game.domination.type == kOpenGame) {
      client.shells = MAXSHELLS;
      client.mines = MAXMINES;
      client.armour = MAXARMOUR;
      client.trees = MAXTREES;
    }
    else if (client.game.domination.type == kTournamentGame) {
      client.shells = 0;

      for (i = 0; i < client.nbases; i++) {
        if (client.bases[i].owner == NEUTRAL) {
          client.shells++;
        }
      }

      client.shells *= 2;
      client.mines = 0;
      client.armour = MAXARMOUR;
      client.trees = 0;
    }
    else if (client.game.domination.type == kStrictGame) {
      client.shells = 0;
      client.mines = 0;
      client.armour = MAXARMOUR;
      client.trees = 0;
    }
    else {
      assert(0);
    }
  }
  else {
    assert(0);
  }

  client.spawned = 1;

  if (client.settankstatus) {
    client.settankstatus();
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int tilefor(int x, int y) {
  int i;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  for (i = 0; i < client.npills; i++) {
    if (client.pills[i].armour != ONBOARD && client.pills[i].x == x && client.pills[i].y == y) {
      if (client.pills[i].owner != NEUTRAL && testalliance(client.pills[i].owner, client.player)) {
        return kFriendlyPill00Tile + client.pills[i].armour;
      }
      else {
        return kHostilePill00Tile + client.pills[i].armour;
      }
    }
  }

  for (i = 0; i < client.nbases; i++) {
    if (client.bases[i].x == x && client.bases[i].y == y) {
      if (client.bases[i].owner == NEUTRAL) {
        return kNeutralBaseTile;
      }
      else if (testalliance(client.bases[i].owner, client.player)) {
        return kFriendlyBaseTile;
      }
      else {
        return kHostileBaseTile;
      }
    }
  }

  return terraintotile(client.terrain[y][x]);
  
}


int fogtilefor(int x, int y, int tile) {
  int pill, base;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

  if ((pill = clientfindpill(x, y)) != -1) {
    if (client.pills[pill].owner != NEUTRAL && testalliance(client.pills[pill].owner, client.player)) {
      return kFriendlyPill00Tile + client.pills[pill].armour;
    }
    else {
      return kHostilePill00Tile + client.pills[pill].armour;
    }
  }

  if ((base = clientfindbase(x, y)) != -1) {
    if (client.bases[base].owner == NEUTRAL) {
      return kNeutralBaseTile;
    }
    else if (testalliance(client.bases[base].owner, client.player)) {
      return kFriendlyBaseTile;
    }
    else {
      return kHostileBaseTile;
    }
  }

  switch (client.terrain[y][x]) {
  case kSeaTerrain:  /* sea */
    return kSeaTile;

  case kMinedSeaTerrain:  /* mined sea */
    return kMinedSeaTile;

  case kWallTerrain:  /* wall */
    return kWallTile;

  case kRiverTerrain:  /* river */
    return kRiverTile;

  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
    return kSwampTile;

  case kMinedSwampTerrain:  /* mined swamp */
    if (client.hiddenmines && tile != kMinedSwampTile) {
      return kSwampTile;
    }
    else {
      return kMinedSwampTile;
    }

  case kCraterTerrain:  /* crater */
    return kCraterTile;

  case kMinedCraterTerrain:  /* mined crater */
    if (client.hiddenmines && tile != kMinedCraterTile) {
      return kCraterTile;
    }
    else {
      return kMinedCraterTile;
    }

  case kRoadTerrain:  /* road */
    return kRoadTile;

  case kMinedRoadTerrain:  /* mined road */
    if (client.hiddenmines && tile != kMinedRoadTile) {
      return kRoadTile;
    }
    else {
      return kMinedRoadTile;
    }

  case kForestTerrain:  /* forest */
    return kForestTile;

  case kMinedForestTerrain:  /* mined forest */
    if (client.hiddenmines && tile != kMinedForestTile && tile != kMinedGrassTile) {
      return kForestTile;
    }
    else {
      return kMinedForestTile;
    }

  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
    return kRubbleTile;

  case kMinedRubbleTerrain:  /* mined rubble */
    if (client.hiddenmines && tile != kMinedRubbleTile) {
      return kRubbleTile;
    }
    else {
      return kMinedRubbleTile;
    }

  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
    return kGrassTile;

  case kMinedGrassTerrain:  /* mined grass */
    if (client.hiddenmines && tile != kMinedGrassTile && tile != kMinedForestTile) {
      return kGrassTile;
    }
    else {
      return kMinedGrassTile;
    }

  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
    return kDamagedWallTile;

  case kBoatTerrain:  /* river w/boat */
    return kBoatTile;

  default:
    assert(0);
    return -1;
  }
}


int refresh(int x, int y) {
  int i, j;
  Recti rect;
  int image;
  Pointi *p;
  int seentile;

  assert(x >= 0 && x < WIDTH && y >= 0 && y < WIDTH);

TRY
  if (client.fog[y - 1][x - 1] > 0 || client.fog[y - 1][x    ] > 0 || client.fog[y - 1][x + 1] > 0 ||
      client.fog[y    ][x - 1] > 0 || client.fog[y    ][x    ] > 0 || client.fog[y    ][x + 1] > 0 ||
      client.fog[y + 1][x - 1] > 0 || client.fog[y + 1][x    ] > 0 || client.fog[y + 1][x + 1] > 0) {
    seentile = tilefor(x, y);

    if (seentile != client.seentiles[y][x]) {
      client.seentiles[y][x] = seentile;
      rect = intersectionrect(makerect(x - 1, y - 1, 3, 3), makerect(0, 0, WIDTH, WIDTH));

      for (i = minxrect(rect); i <= maxxrect(rect); i++) {
        for (j = minyrect(rect); j <= maxyrect(rect); j++) {
          if (((image = mapimage(client.seentiles, i, j)) != client.images[j][i]) || (i == x && j == y)) {
            client.images[j][i] = image;
            if ((p = (Pointi *)malloc(sizeof(Pointi))) == NULL) LOGFAIL(errno)
            p->x = i;
            p->y = j;
            if (addlist(&client.changedtiles, p) == -1) LOGFAIL(errno)
          }
        }
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


void clearchangedtiles() {
  clearlist(&client.changedtiles, free);
}


int requestalliance(uint16_t withplayers) {
  struct CLSetAlliance clsetalliance;
  int i, j;
  char *text = NULL;
  uint16_t xor;

TRY
  xor = client.players[client.player].alliance ^ (client.players[client.player].alliance | withplayers);
  client.players[client.player].alliance |= withplayers;
  clsetalliance.type = CLSETALLIANCE;
  clsetalliance.alliance = htons(client.players[client.player].alliance);
  if (writebuf(&client.sendbuf, &clsetalliance, sizeof(clsetalliance)) == -1) LOGFAIL(errno)
  if (sendbuf(&client.sendbuf, client.cntlsock) == -1) LOGFAIL(errno)

  /* their alliance bit has changed */
  for (i = 0; i < MAXPLAYERS; i++) {
    /* my alliance bit is set */
    if (client.players[i].connected && (xor & (1 << i))) {
      /* their alliance bit is set */
      if (client.players[i].alliance & (1 << client.player)) {
        if (client.printmessage) {
          if (asprintf(&text, "alliance accepted with %s", client.players[i].name) == -1) LOGFAIL(errno)
          client.printmessage(MSGGAME, text);
          free(text);
          text = NULL;
        }

        if (client.setplayerstatus) {
          client.setplayerstatus(i);
        }

        for (j = 0; j < client.nbases; j++) {
          if (client.bases[j].owner == i) {
            refresh(client.bases[j].x, client.bases[j].y);

            if (client.setbasestatus) {
              client.setbasestatus(j);
            }
          }
        }

        for (j = 0; j < client.npills; j++) {
          if (client.pills[j].owner == i) {
            /* fog of war */
            if (client.pills[j].armour != ONBOARD && client.pills[j].armour > 0) {
              if (increasevis(makerect(client.pills[j].x - 7, client.pills[j].y - 7, 15, 15))) LOGFAIL(errno)
            }

            refresh(client.pills[j].x, client.pills[j].y);

            if (client.setpillstatus) {
              client.setpillstatus(j);
            }
          }
        }

        if (increasevis(makerect(((int)client.players[i].tank.x) - 14, ((int)client.players[i].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
      }
      /* their alliance bit is unset */
      else {
        if (client.printmessage) {
          if (asprintf(&text, "requested alliance with %s", client.players[i].name) == -1) LOGFAIL(errno)
          client.printmessage(MSGGAME, text);
          free(text);
          text = NULL;
        }
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int leavealliance(uint16_t withplayers) {
  struct CLSetAlliance clsetalliance;
  int i, j;
  char *text = NULL;
  uint16_t xor;

TRY
  xor = client.players[client.player].alliance ^ (client.players[client.player].alliance & (~withplayers | (1 << client.player)));
  client.players[client.player].alliance &= ~withplayers | (1 << client.player);
  clsetalliance.type = CLSETALLIANCE;
  clsetalliance.alliance = htons(client.players[client.player].alliance);
  if (writebuf(&client.sendbuf, &clsetalliance, sizeof(clsetalliance)) == -1) LOGFAIL(errno)
  if (sendbuf(&client.sendbuf, client.cntlsock) == -1) LOGFAIL(errno)

  /* their alliance bit has changed */
  for (i = 0; i < MAXPLAYERS; i++) {
    /* my alliance bit is unset */
    if (client.players[i].connected && (xor & (1 << i))) {
      /* their alliance bit is set */
      if (client.players[i].alliance & (1 << client.player)) {
        if (client.printmessage) {
          if (asprintf(&text, "left alliance with %s", client.players[i].name) == -1) LOGFAIL(errno)
          client.printmessage(MSGGAME, text);
          free(text);
          text = NULL;
        }

        if (client.setplayerstatus) {
          client.setplayerstatus(i);
        }

        for (j = 0; j < client.nbases; j++) {
          if (client.bases[j].owner == i) {
            refresh(client.bases[j].x, client.bases[j].y);
            if (client.setbasestatus) {
              client.setbasestatus(j);
            }
          }
        }

        for (j = 0; j < client.npills; j++) {
          if (client.pills[j].owner == i) {
            /* fog of war */
            if (client.pills[j].armour != ONBOARD) {
              if (refresh(client.pills[j].x, client.pills[j].y)) LOGFAIL(errno)

              if (client.pills[j].armour > 0) {
                if (decreasevis(makerect(client.pills[j].x - 7, client.pills[j].y - 7, 15, 15))) LOGFAIL(errno)
              }
            }

            if (client.setpillstatus) {
              client.setpillstatus(j);
            }
          }
        }

        if (decreasevis(makerect(((int)client.players[i].tank.x) - 14, ((int)client.players[i].tank.y) - 14, 29, 29)) == -1) LOGFAIL(errno)
      }
    }
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int keyevent(int mask, int set) {
  Pointi tanksqr;
  int gotlock = 0;

TRY
  if (lockclient()) LOGFAIL(errno)
  gotlock = 1;

  if (set) {
    client.players[client.player].inputflags |= mask;
  }
  else {
    client.players[client.player].inputflags &= ~mask;
  }

  tanksqr = makepoint(w2t(client.players[client.player].tank.x), w2t(client.players[client.player].tank.y));

  if (!client.players[client.player].dead) {
    int i;

    for (i = 0; i < client.npills; i++) {
      if (client.pills[i].armour != ONBOARD && client.pills[i].x == (int)client.players[client.player].tank.x && client.pills[i].y == (int)client.players[client.player].tank.y) {
        break;
      }
    }

    if (!(i < client.npills)) {
      for (i = 0; i < client.nbases; i++) {
        if (client.bases[i].x == (int)client.players[client.player].tank.x && client.bases[i].y == (int)client.players[client.player].tank.y) {
          break;
        }
      }

      if (!(i < client.nbases)) {
        switch (client.terrain[(int)client.players[client.player].tank.y][(int)client.players[client.player].tank.x]) {
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
          /* plant first mine */
          if (set && mask == LMINEMASK && client.mines > 0) {
            client.mines--;
            if (sendcldropmine((int)client.players[client.player].tank.x, (int)client.players[client.player].tank.y) == -1) LOGFAIL(errno)
          }

        break;

        default:
          break;
        }
      }
    }
  }

  if (unlockclient()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  if (gotlock) {
    unlockclient();
  }

ERRHANDLER(0, -1)
END
}


void buildercommand(int command, Pointi p) {
  if (client.players[client.player].builderstatus != kBuilderParachute && client.nextbuildercommand == BUILDERNILL && !client.players[client.player].dead) {
    client.nextbuildercommand = command;
    client.nextbuildertarget = p;
  }
}


int getbuildertaskforcommand(int command, Pointi at) {
  switch (command) {
  case BUILDERNILL:
    return kBuilderDoNothing;
    
  case BUILDERTREE:
    switch (client.seentiles[at.y][at.x]) {
    case kForestTile:
    case kMinedForestTile:
      return kBuilderGetTree;

    default:
      return kBuilderDoNothing;
    }

  case BUILDERROAD:
    switch (client.seentiles[at.y][at.x]) {
    case kForestTile:
    case kMinedForestTile:
      return kBuilderGetTree;

    case kRiverTile:
    case kSwampTile:
    case kCraterTile:
    case kRubbleTile:
    case kGrassTile:
      return kBuilderBuildRoad;

    case kMinedSwampTile:
    case kMinedCraterTile:
    case kMinedRubbleTile:
    case kMinedGrassTile:
      if (client.printmessage) {
        client.printmessage(MSGGAME, "Your builder cannot do that.  It would kill him.");
      }

      return kBuilderDoNothing;

    default:
      return kBuilderDoNothing;
    }

  case BUILDERWALL:
    switch (client.seentiles[at.y][at.x]) {
    case kForestTile:
    case kMinedForestTile:
      return kBuilderGetTree;

    case kSwampTile:
    case kCraterTile:
    case kRoadTile:
    case kRubbleTile:
    case kGrassTile:
    case kDamagedWallTile:
      return kBuilderBuildWall;

    case kRiverTile:
      return kBuilderBuildBoat;

    case kMinedSwampTile:
    case kMinedCraterTile:
    case kMinedRoadTile:
    case kMinedRubbleTile:
    case kMinedGrassTile:
      if (client.printmessage) {
        client.printmessage(MSGGAME, "Your builder cannot do that.  It would kill him.");
      }

      return kBuilderDoNothing;

    default:
      return kBuilderDoNothing;
    }

  case BUILDERPILL:
    switch (client.seentiles[at.y][at.x]) {
    case kForestTile:
    case kMinedForestTile:
      return kBuilderGetTree;

    case kSwampTile:
    case kCraterTile:
    case kRoadTile:
    case kRubbleTile:
    case kGrassTile:
      return kBuilderBuildPill;

    case kMinedSwampTile:
    case kMinedCraterTile:
    case kMinedRoadTile:
    case kMinedRubbleTile:
    case kMinedGrassTile:
      if (client.printmessage) {
        client.printmessage(MSGGAME, "Your builder cannot do that.  It would kill him.");
      }

      return kBuilderDoNothing;

    case kFriendlyPill00Tile:
    case kFriendlyPill01Tile:
    case kFriendlyPill02Tile:
    case kFriendlyPill03Tile:
    case kFriendlyPill04Tile:
    case kFriendlyPill05Tile:
    case kFriendlyPill06Tile:
    case kFriendlyPill07Tile:
    case kFriendlyPill08Tile:
    case kFriendlyPill09Tile:
    case kFriendlyPill10Tile:
    case kFriendlyPill11Tile:
    case kFriendlyPill12Tile:
    case kFriendlyPill13Tile:
    case kFriendlyPill14Tile:
    case kHostilePill00Tile:
    case kHostilePill01Tile:
    case kHostilePill02Tile:
    case kHostilePill03Tile:
    case kHostilePill04Tile:
    case kHostilePill05Tile:
    case kHostilePill06Tile:
    case kHostilePill07Tile:
    case kHostilePill08Tile:
    case kHostilePill09Tile:
    case kHostilePill10Tile:
    case kHostilePill11Tile:
    case kHostilePill12Tile:
    case kHostilePill13Tile:
    case kHostilePill14Tile:
      return kBuilderRepairPill;

    default:
      return kBuilderDoNothing;
    }

  case BUILDERMINE:
    switch (client.seentiles[at.y][at.x]) {
    case kSwampTile:
    case kCraterTile:
    case kRoadTile:
    case kForestTile:
    case kRubbleTile:
    case kGrassTile:
      return kBuilderPlaceMine;

    case kMinedSwampTile:
    case kMinedCraterTile:
    case kMinedRoadTile:
    case kMinedForestTile:
    case kMinedRubbleTile:
    case kMinedGrassTile:
      if (client.printmessage) {
        client.printmessage(MSGGAME, "Your builder cannot do that.  It would kill him.");
      }

      return kBuilderDoNothing;

    default:
      return kBuilderDoNothing;
    }

  default:
    assert(0);
  }
}


int sendmessage(const char *text, int to) {
  int gotlock = 0;
  int i;
  struct CLSendMesg clsendmesg;

TRY
  /* client lock */
  if (lockclient()) LOGFAIL(errno)
  gotlock = 1;

  clsendmesg.type = CLSENDMESG;
  clsendmesg.to = to;

  switch (to) {
  case MSGEVERYONE:
    clsendmesg.mask = htons(0xffff);  /* ok */
    break;

  case MSGALLIES:
    clsendmesg.mask = htons(client.players[client.player].alliance);
    break;

  case MSGNEARBY:
    clsendmesg.mask = htons(0x00);  /* ok */

    for (i = 0; i < MAXPLAYERS; i++) {
      if (mag2f(sub2f(client.players[client.player].tank, client.players[i].tank)) < 8.5) {
        clsendmesg.mask |= 1 << i;
      }
    }

    clsendmesg.mask = htons(clsendmesg.mask);
    break;

  default:
    assert(0);
    break;
  }

  if (writebuf(&client.sendbuf, &clsendmesg, sizeof(clsendmesg)) == -1) LOGFAIL(errno)
  if (writebuf(&client.sendbuf, text, strlen(text) + 1) == -1) LOGFAIL(errno)
  if (sendbuf(&client.sendbuf, client.cntlsock) == -1) LOGFAIL(errno)

  /* client unlock */
  if (unlockclient()) LOGFAIL(errno)
  gotlock = 0;

CLEANUP
  if (gotlock) {
    unlockclient();
  }

ERRHANDLER(0, -1)
END
}


int w2t(float f) {
  return f;
}


float rounddir(float dir) {
  return (kPif/8.0)*floor(dir/(kPif/8.0) + 0.5);
}


int tankcollision(Pointi square) {
  int i;

  if (square.x < 0 || square.x >= WIDTH || square.y < 0 || square.y >= WIDTH) {
    return 1;
  }

  for (i = 0; i < client.npills; i++) {
    if (client.pills[i].armour != ONBOARD && client.pills[i].x == square.x && client.pills[i].y == square.y) {
      return client.pills[i].armour > 0;
    }
  }

  for (i = 0; i < client.nbases; i++) {
    if (client.bases[i].x == square.x && client.bases[i].y == square.y) {
      return
        client.bases[i].owner != NEUTRAL &&
        !testalliance(client.bases[i].owner, collisionowner) &&
        client.bases[i].armour >= MINBASEARMOUR;
    }
  }

  switch (client.terrain[square.y][square.x]) {
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
    return 1;

  case kSeaTerrain:  /* sea */
  case kRiverTerrain:  /* river */
  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRoadTerrain:  /* road */
  case kForestTerrain:  /* forest */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
  case kBoatTerrain:  /* river w/boat */
  case kMinedSeaTerrain:  /* mined sea */
  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRoadTerrain:  /* mined road */
  case kMinedForestTerrain:  /* mined forest */
  case kMinedRubbleTerrain:  /* mined rubble */
  case kMinedGrassTerrain:  /* mined grass */
    return 0;

  default:
    assert(0);
  }
}


int buildercollision(Pointi square) {
  int pill, base;

  if (square.x < 0 || square.x >= WIDTH || square.y < 0 || square.y >= WIDTH) {
    return 1;
  }

  if ((pill = clientfindpill(square.x, square.y)) != -1) {
    return !((isequalpoint(square, target) && buildertask == kBuilderRepairPill) || (client.pills[pill].armour == 0));
  }

  if ((base = clientfindbase(square.x, square.y)) != -1) {
    return client.bases[base].owner != NEUTRAL && !testalliance(client.bases[base].owner, collisionowner) && client.bases[base].armour > MINBASEARMOUR;
  }

  switch (client.terrain[square.y][square.x]) {
  case kWallTerrain:  /* wall */
  case kDamagedWallTerrain0:  /* damaged wall */
  case kDamagedWallTerrain1:  /* damaged wall */
  case kDamagedWallTerrain2:  /* damaged wall */
  case kDamagedWallTerrain3:  /* damaged wall */
    return !isequalpoint(square, target) || buildertask != kBuilderBuildWall;

  case kRiverTerrain:  /* river */
    return !isequalpoint(square, target) || (buildertask != kBuilderBuildBoat && buildertask != kBuilderBuildRoad);

  case kSeaTerrain:  /* sea */
  case kMinedSeaTerrain:  /* mined sea */
    return 1;

  case kSwampTerrain0:  /* swamp */
  case kSwampTerrain1:  /* swamp */
  case kSwampTerrain2:  /* swamp */
  case kSwampTerrain3:  /* swamp */
  case kCraterTerrain:  /* crater */
  case kRoadTerrain:  /* road */
  case kForestTerrain:  /* forest */
  case kRubbleTerrain0:  /* rubble */
  case kRubbleTerrain1:  /* rubble */
  case kRubbleTerrain2:  /* rubble */
  case kRubbleTerrain3:  /* rubble */
  case kGrassTerrain0:  /* grass */
  case kGrassTerrain1:  /* grass */
  case kGrassTerrain2:  /* grass */
  case kGrassTerrain3:  /* grass */
  case kBoatTerrain:  /* river w/boat */
  case kMinedSwampTerrain:  /* mined swamp */
  case kMinedCraterTerrain:  /* mined crater */
  case kMinedRoadTerrain:  /* mined road */
  case kMinedForestTerrain:  /* mined forest */
  case kMinedRubbleTerrain:  /* mined rubble */
  case kMinedGrassTerrain:  /* mined grass */
    return 0;

  default:
    assert(0);
    return 0;
  }
}


int circlesquare(Vec2f point, float radius, Pointi square) {
  if (point.x < square.x) {
    if (point.y < square.y) {
      return mag2f(sub2f(make2f(square.x, square.y), point)) < radius;
    }
    else if (point.y > square.y + 1) {
      return mag2f(sub2f(make2f(square.x, square.y + 1), point)) < radius;
    }
    else {
      return point.x + radius > square.x;
    }
  }
  else if (point.x > square.x + 1) {
    if (point.y < square.y) {
      return mag2f(sub2f(make2f(square.x + 1, square.y), point)) < radius;
    }
    else if (point.y > square.y + 1) {
      return mag2f(sub2f(make2f(square.x + 1, square.y + 1), point)) < radius;
    }
    else {
      return point.x - radius < square.x + 1;
    }
  }
  else {
    if (point.y < square.y) {
      return point.y + radius > square.y;
    }
    else if (point.y > square.y + 1) {
      return point.y - radius < square.y + 1;
    }
    else {
      return 1;
    }
  }
}


Vec2f collisiondetect(Vec2f p, float radius, int (*func)(Pointi square)) {
  int ix, iy;
  int lxc, hxc, lyc, hyc;
  float lx, hx, ly, hy, fx, fy;
  float sqr, sca, r2;

  ix = (int)p.x;
  iy = (int)p.y;
  fx = (float)ix;
  fy = (float)iy;
  lx = p.x - fx;
  hx = 1.0 - lx;
  ly = p.y - fy;
  hy = 1.0 - ly;
  r2 = radius*radius;

  lxc = lx < radius && func(makepoint(ix - 1, iy));
  hxc = hx < radius && func(makepoint(ix + 1, iy));
  lyc = ly < radius && func(makepoint(ix, iy - 1));
  hyc = hy < radius && func(makepoint(ix, iy + 1));

  if (lxc) {
    if (hxc) {
      p.x = fx + 0.5;
    }
    else {
      p.x = fx + radius;
    }
  }
  else if (hxc) {
    p.x = fx + (1.0 - radius);
  }

  if (lyc) {
    if (hyc) {
      p.x = fy + 0.5;
    }
    else {
      p.y = fy + radius;
    }
  }
  else if (hyc) {
    p.y = fy + (1.0 - radius);
  }

  if (!lxc && !lyc && (sqr = lx*lx + ly*ly) < r2 && func(makepoint(ix - 1, iy - 1))) {
    sca = radius/sqrtf(sqr);
    p.x = fx + sca*lx;
    p.y = fy + sca*ly;
  }

  if (!hxc && !lyc && (sqr = hx*hx + ly*ly) < r2 && func(makepoint(ix + 1, iy - 1))) {
    sca = radius/sqrtf(sqr);
    p.x = fx + (1.0 - sca*hx);
    p.y = fy + sca*ly;
  }

  if (!lxc && !hyc && (sqr = lx*lx + hy*hy) < r2 && func(makepoint(ix - 1, iy + 1))) {
    sca = radius/sqrtf(sqr);
    p.x = fx + sca*lx;
    p.y = fy + (1.0 - sca*hy);
  }

  if (!hxc && !hyc && (sqr = hx*hx + hy*hy) < r2 && func(makepoint(ix + 1, iy + 1))) {
    sca = radius/sqrtf(sqr);
    p.x = fx + (1.0 - sca*hx);
    p.y = fy + (1.0 - sca*hy);
  }

  return p;
}


int killsquarebuilder(Pointi p) {
TRY
  switch (client.players[client.player].builderstatus) {
  case kBuilderGoto:
  case kBuilderWork:
  case kBuilderWait:
  case kBuilderReturn:
    if (isequalpoint(makepoint(w2t(client.players[client.player].builder.x), w2t(client.players[client.player].builder.y)), p)) {
      killbuilder();
    }

    break;

  case kBuilderReady:
  case kBuilderParachute:
  default:
    break;
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int killpointbuilder(Vec2f p) {
TRY
  switch (client.players[client.player].builderstatus) {
  case kBuilderGoto:
  case kBuilderWork:
  case kBuilderWait:
  case kBuilderReturn:
    if (mag2f(sub2f(client.players[client.player].builder, p)) < EXPLOSIONRADIUS) {
      killbuilder();
    }

    break;

  case kBuilderReady:
  case kBuilderParachute:
  default:
    break;
  }

CLEANUP
ERRHANDLER(0, -1)
END
}


int killbuilder() {
  int start;

TRY
  if (client.builderpill != NOPILL) {
    /* drop pill */
    if (sendcldroppills(client.players[client.player].builder.x, client.players[client.player].builder.y, 1 << client.builderpill)) LOGFAIL(errno)
    client.builderpill = NOPILL;
  }

  start = random()%client.nstarts;
  client.players[client.player].builderstatus = kBuilderParachute;
  client.players[client.player].builder.x = client.starts[start].x + 0.5;
  client.players[client.player].builder.y = client.starts[start].y + 0.5;
  client.nextbuildercommand = BUILDERNILL;
  client.nextbuildertarget = makepoint(0, 0);
  client.buildertask = kBuilderDoNothing;
  client.buildermines = 0;
  client.buildertrees = 0;
  client.players[client.player].buildertarget = makepoint(0, 0);

  client.players[client.player].buildertarget.x = client.players[client.player].tank.x;
  client.players[client.player].buildertarget.y = client.players[client.player].tank.y;

  /* play sound */
  if (client.playsound) {
    client.playsound(kBuilderDeathSound);
  }
  if (client.printmessage) {
      char *text;
      if (asprintf(&text, "%s just lost his builder", client.players[client.player].name) == -1) LOGFAIL(errno)
          client.printmessage(MSGGAME, text);
      free(text);
  }

  client.builderdeathsound = 1;

CLEANUP
ERRHANDLER(0, -1)
END
}


int clientfindpill(int x, int y) {
  int i;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  for (i = 0; i < client.npills; i++) {
    if (client.pills[i].armour != ONBOARD && client.pills[i].x == x && client.pills[i].y == y) {
      return i;
    }
  }

  return -1;
}


int clientfindbase(int x, int y) {
  int i;

  assert(x >= 0);
  assert(x < WIDTH);
  assert(y >= 0);
  assert(y < WIDTH);

  for (i = 0; i < client.nbases; i++) {
    if (client.bases[i].x == x && client.bases[i].y == y) {
      return i;
    }
  }

  return -1;
}


int testalliance(int p1, int p2) {
  return client.players[p1].used && client.players[p2].used && (client.players[p1].alliance & (1 << p2)) && (client.players[p2].alliance & (1 << p1));
}

