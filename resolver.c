/*
 *  resolver.c
 *  XBolo
 *
 *  Created by Robert Chrzanowski on 11/12/09.
 *  Copyright 2009 Robert Chrzanowski. All rights reserved.
 *
 */

#include "resolver.h"
#include "io.h"
#include "errchk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <math.h>
#include <netinet/tcp.h>

#include <fcntl.h>

#include <pthread.h>
#include <unistd.h>
#include <netdb.h>

struct GetHostByName {
  char *hostname;
  int pipe;
} ;

struct GetHostByNameResult {
  int err;
  struct in_addr addr;
} ;

static void *gethostbynamethread(struct GetHostByName *info);

/* returns a file descriptor that will return struct GetHostByNameResult */
int nslookup(const char *hostname) {
  struct GetHostByName *info = NULL;
  int fildes[2] = { -1, -1 };
  pthread_t thread;
  int err;

  assert(hostname != NULL);

  if ((info = (struct GetHostByName *)malloc(sizeof(struct GetHostByName))) == NULL) {
    ERRLOG(errno);
    goto fail;
  }

  if ((info->hostname = (char *)malloc(strlen(hostname) + 1)) == NULL) {
    ERRLOG(errno);
    goto fail;
  }

  strcpy(info->hostname, hostname);

  if (pipe(fildes)) {
    fildes[0] = -1;
    fildes[1] = -1;
    ERRLOG(errno);
    goto fail;
  }

  info->pipe = fildes[1];

  if ((err = pthread_create(&thread, NULL, (void *)gethostbynamethread, info))) {
    ERRLOG(err);
    goto fail;
  }

  /* the thread owns these now */
  fildes[1] = -1;
  info = NULL;

  if ((err = pthread_detach(thread))) {
    ERRLOG(err);
    goto fail;
  }

  return fildes[0];

fail:
  err = errno;

  if (fildes[1] != -1) {
    closesock(fildes + 1);
  }

  if (fildes[0] != -1) {
    closesock(fildes);
  }

  if (info) {
    if (info->hostname) {
      free(info->hostname);
    }

    free(info);
  }

  errno = err;
  return -1;
}

int nslookupresult(int pipe, struct in_addr *addr) {
  struct GetHostByNameResult result;

  assert(pipe != -1);
  assert(addr != NULL);

  if (readblock(pipe, &result, sizeof(result)) == -1) {
    return ERRLOG(errno);
  }

  if (result.err) {
    errno = result.err;
    return -1;
  }

  *addr = result.addr;
  return 0;
}

int nslookupcancel(int pipe) {
  if (closesock(&pipe)) {
    return ERRLOG(errno);
  }

  return 0;
}

void *gethostbynamethread(struct GetHostByName *info) {
  struct hostent *hostent;
  struct GetHostByNameResult result;

  assert(info != NULL);

  bzero(&result, sizeof(result));
  result.err = 0;

  /* use udp */
  sethostent(0);

  for (;;) {
    hostent = gethostbyname(info->hostname);

    if (hostent == NULL) {
      if (h_errno == TRY_AGAIN) {
        continue;
      }
      else if (h_errno == HOST_NOT_FOUND) {
        result.err = EHOSTNOTFOUND;
      }
      else if (h_errno == NO_RECOVERY) {
        result.err = EHOSTNORECOVERY;
      }
      else if (h_errno == NO_DATA) {
        result.err = EHOSTNODATA;
      }
      else {
        assert(0);
      }

      break;
    }
    else {
      result.addr.s_addr = *(uint32_t *)(hostent->h_addr_list[0]);
      break;
    }
  }

  endhostent();

  writeblock(info->pipe, &result, sizeof(result));

  if (info->pipe != -1) {
    closesock(&info->pipe);
  }

  if (info->hostname) {
    free(info->hostname);
  }

  free(info);

  pthread_exit(NULL);
}
