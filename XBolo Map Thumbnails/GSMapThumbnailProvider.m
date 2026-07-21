/*
 *  GSMapThumbnailProvider.m
 *  XBolo Map Thumbnails
 *
 *  Quick Look thumbnail extension for .map files, replacing the retired
 *  .qlgenerator plug-in.  Renders the classic mini-map: one colored cell
 *  per tile, cropped to the used area of the map, with bases, pillboxes,
 *  and player starts marked.  The parsing and palette are ported from the
 *  original GenerateThumbnailForURL.m.
 */

@import QuickLookThumbnailing;
@import CoreGraphics;

#define MAPFILEIDENT        ("BMAPBOLO")
#define MAPFILEIDENTLEN     (8)
#define CURRENTMAPVERSION   (1)
#define MAXPILLS            (16)
#define MAXBASES            (16)
#define MAXSTARTS           (16)

/* tile types as stored in the file's nibble runs */
enum {
  kWallTile         = 0,
  kRiverTile        = 1,
  kSwampTile        = 2,
  kCraterTile       = 3,
  kRoadTile         = 4,
  kForestTile       = 5,
  kRubbleTile       = 6,
  kGrassTile        = 7,
  kDamagedWallTile  = 8,
  kBoatTile         = 9,
  kMinedSwampTile   = 10,
  kMinedCraterTile  = 11,
  kMinedRoadTile    = 12,
  kMinedForestTile  = 13,
  kMinedRubbleTile  = 14,
  kMinedGrassTile   = 15,
};

struct BMAP_Preamble {
  uint8_t ident[8];
  uint8_t version;
  uint8_t npills;
  uint8_t nbases;
  uint8_t nstarts;
} __attribute__((__packed__));

struct BMAP_PillInfo {
  uint8_t x, y, owner, armour, speed;
} __attribute__((__packed__));

struct BMAP_BaseInfo {
  uint8_t x, y, owner, armour, shells, mines;
} __attribute__((__packed__));

struct BMAP_StartInfo {
  uint8_t x, y, dir;
} __attribute__((__packed__));

struct BMAP_Run {
  uint8_t datalen;  /* including this 4 byte header */
  uint8_t y;
  uint8_t startx;
  uint8_t endx;
} __attribute__((__packed__));

/* palette (RGB), same values as the original generator */
static const CGFloat kTileColors[16][3] = {
  [kWallTile]         = { 0.702, 0.475, 0.059 },  /* brown */
  [kRiverTile]        = { 0.0,   1.0,   1.0   },  /* cyan */
  [kSwampTile]        = { 0.0,   0.502, 0.502 },  /* dark cyan */
  [kCraterTile]       = { 0.502, 0.251, 0.0   },  /* dark brown */
  [kRoadTile]         = { 0.0,   0.0,   0.0   },  /* black */
  [kForestTile]       = { 0.0,   0.502, 0.0   },  /* dark green */
  [kRubbleTile]       = { 0.918, 0.647, 0.482 },  /* very light brown */
  [kGrassTile]        = { 0.0,   1.0,   0.0   },  /* green */
  [kDamagedWallTile]  = { 0.875, 0.596, 0.075 },  /* light brown */
  [kBoatTile]         = { 0.0,   0.0,   0.475 },  /* dark blue */
  [kMinedSwampTile]   = { 0.0,   0.502, 0.502 },
  [kMinedCraterTile]  = { 0.502, 0.251, 0.0   },
  [kMinedRoadTile]    = { 0.0,   0.0,   0.0   },
  [kMinedForestTile]  = { 0.0,   0.502, 0.0   },
  [kMinedRubbleTile]  = { 0.918, 0.647, 0.482 },
  [kMinedGrassTile]   = { 0.0,   1.0,   0.0   },
};

static const CGFloat kSeaColor[3]   = { 0.0,   0.0,   1.0   };
static const CGFloat kBaseColor[3]  = { 1.0,   1.0,   0.0   };
static const CGFloat kPillColor[3]  = { 1.0,   0.0,   0.0   };
static const CGFloat kStartColor[3] = { 0.502, 0.502, 0.502 };

static void setColor(CGContextRef ctx, const CGFloat rgb[3]) {
  CGContextSetRGBFillColor(ctx, rgb[0], rgb[1], rgb[2], 1.0);
}

static int readnibble(const void *buf, size_t i) {
  return i % 2 ? *((const uint8_t *)buf + i/2) & 0x0f
               : (*((const uint8_t *)buf + i/2) & 0xf0) >> 4;
}

/* draws one run; returns -1 on corrupt data */
static int drawrun(CGContextRef ctx, struct BMAP_Run run, const void *buf) {
  int x = run.startx;
  int offset = 0;

  while (x < run.endx) {
    int len;

    if (sizeof(struct BMAP_Run) + (offset + 2)/2 > run.datalen) return -1;
    len = readnibble(buf, offset++);

    if (len >= 0 && len <= 7) {  /* sequence of different tiles */
      len += 1;
      if (sizeof(struct BMAP_Run) + (offset + len + 1)/2 > run.datalen) return -1;
      for (int i = 0; i < len; i++) {
        int tile = readnibble(buf, offset++);
        if (tile < 0 || tile > 15) return -1;
        setColor(ctx, kTileColors[tile]);
        CGContextFillRect(ctx, CGRectMake(x++, run.y, 1, 1));
      }
    }
    else {  /* sequence of like tiles */
      len -= 6;
      if (sizeof(struct BMAP_Run) + (offset + 2)/2 > run.datalen) return -1;
      int tile = readnibble(buf, offset++);
      if (tile < 0 || tile > 15) return -1;
      setColor(ctx, kTileColors[tile]);
      CGContextFillRect(ctx, CGRectMake(x, run.y, len, 1));
      x += len;
    }
  }

  return 0;
}

/* renders the mini-map into a square context of the given size; always
   paints something (plain sea for unreadable files) and returns YES */
static BOOL drawMap(CGContextRef ctx, CGFloat size, NSData *data) {
  const void *buf = data.bytes;
  size_t nbytes = data.length;

  CGContextSetShouldAntialias(ctx, false);

  /* map space is 256 units square */
  CGContextScaleCTM(ctx, size/256.0, size/256.0);

  setColor(ctx, kSeaColor);
  CGContextFillRect(ctx, CGRectMake(0, 0, 256, 256));

  /* map rows run top-down */
  CGContextTranslateCTM(ctx, 0, 256);
  CGContextScaleCTM(ctx, 1, -1);

  if (nbytes < sizeof(struct BMAP_Preamble)) return YES;

  const struct BMAP_Preamble *preamble = buf;

  if (strncmp((const char *)preamble->ident, MAPFILEIDENT, MAPFILEIDENTLEN) != 0) return YES;
  if (preamble->version != CURRENTMAPVERSION) return YES;
  if (preamble->npills > MAXPILLS) return YES;
  if (preamble->nbases > MAXBASES) return YES;
  if (preamble->nstarts > MAXSTARTS) return YES;

  size_t headerlen = sizeof(struct BMAP_Preamble) +
    preamble->npills*sizeof(struct BMAP_PillInfo) +
    preamble->nbases*sizeof(struct BMAP_BaseInfo) +
    preamble->nstarts*sizeof(struct BMAP_StartInfo);

  if (nbytes < headerlen) return YES;

  const struct BMAP_PillInfo *pillInfos = (const struct BMAP_PillInfo *)(preamble + 1);
  const struct BMAP_BaseInfo *baseInfos = (const struct BMAP_BaseInfo *)(pillInfos + preamble->npills);
  const struct BMAP_StartInfo *startInfos = (const struct BMAP_StartInfo *)(baseInfos + preamble->nbases);
  const void *runData = (const void *)(startInfos + preamble->nstarts);
  size_t runDataLen = nbytes - headerlen;

  /* first pass: bounds of the used area, for cropping */
  int minx = 256, maxx = 0, miny = 256, maxy = 0;
  size_t offset = 0;

  for (;;) {
    struct BMAP_Run run;

    if (offset + sizeof(struct BMAP_Run) > runDataLen) break;
    memcpy(&run, (const uint8_t *)runData + offset, sizeof(run));
    if (run.datalen == 4 && run.y == 0xff && run.startx == 0xff && run.endx == 0xff) break;
    if (offset + run.datalen > runDataLen) break;

    if (run.y < miny) miny = run.y;
    if (run.y > maxy) maxy = run.y;
    if (run.startx < minx) minx = run.startx;
    if (run.endx > maxx) maxx = run.endx;

    offset += run.datalen;
  }

  minx = minx - 3 < 0 ? 0 : minx - 3;
  maxx = maxx + 3 > 256 ? 256 : maxx + 3;
  miny = miny - 3 < 0 ? 0 : miny - 3;
  maxy = maxy + 3 > 256 ? 256 : maxy + 3;

  if (maxx <= minx || maxy <= miny) return YES;  /* empty map: plain sea */

  /* scale the used area to fill the context, centered on the short axis */
  if (maxx - minx > maxy - miny) {
    CGContextScaleCTM(ctx, 256.0/(maxx - minx), 256.0/(maxx - minx));
    CGContextTranslateCTM(ctx, -minx, -miny + ((maxx - minx) - (maxy - miny))/2);
  }
  else {
    CGContextScaleCTM(ctx, 256.0/(maxy - miny), 256.0/(maxy - miny));
    CGContextTranslateCTM(ctx, -minx + ((maxy - miny) - (maxx - minx))/2, -miny);
  }

  /* second pass: draw terrain */
  offset = 0;

  for (;;) {
    struct BMAP_Run run;

    if (offset + sizeof(struct BMAP_Run) > runDataLen) break;
    memcpy(&run, (const uint8_t *)runData + offset, sizeof(run));
    if (run.datalen == 4 && run.y == 0xff && run.startx == 0xff && run.endx == 0xff) break;
    if (offset + run.datalen > runDataLen) break;

    if (drawrun(ctx, run, (const uint8_t *)runData + offset + sizeof(struct BMAP_Run)) == -1) {
      return YES;
    }

    offset += run.datalen;
  }

  /* markers */
  setColor(ctx, kBaseColor);
  for (int i = 0; i < preamble->nbases; i++) {
    CGContextFillRect(ctx, CGRectMake(baseInfos[i].x, baseInfos[i].y, 1, 1));
  }

  setColor(ctx, kPillColor);
  for (int i = 0; i < preamble->npills; i++) {
    CGContextFillRect(ctx, CGRectMake(pillInfos[i].x, pillInfos[i].y, 1, 1));
  }

  setColor(ctx, kStartColor);
  for (int i = 0; i < preamble->nstarts; i++) {
    CGContextFillRect(ctx, CGRectMake(startInfos[i].x, startInfos[i].y, 1, 1));
  }

  return YES;
}

@interface GSMapThumbnailProvider : QLThumbnailProvider
@end

@implementation GSMapThumbnailProvider

- (void)provideThumbnailForFileRequest:(QLFileThumbnailRequest *)request
                     completionHandler:(void (^)(QLThumbnailReply *, NSError *))handler {
  NSData *data = [NSData dataWithContentsOfURL:request.fileURL];

  if (data == nil) {
    handler(nil, [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadUnknownError userInfo:nil]);
    return;
  }

  CGFloat side = MIN(request.maximumSize.width, request.maximumSize.height);

  CGFloat scale = request.scale;

  handler([QLThumbnailReply replyWithContextSize:CGSizeMake(side, side)
                                    drawingBlock:^BOOL(CGContextRef context) {
    /* the context is in pixels (contextSize x scale); draw at pixel size */
    return drawMap(context, side * scale, data);
  }], nil);
}

@end
