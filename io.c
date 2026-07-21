/*
 *  io.c
 *  XBolo
 *
 *  Created by Robert Chrzanowski.
 *  Copyright 2004 Robert Chrzanowski. All rights reserved.
 *
 */

#include "io.h"
#include "errchk.h"

#include <stdlib.h>
#include <string.h>

ssize_t readblock(int d, void *buf, size_t nbytes) {
  size_t n = 0;

  while (n < nbytes) {
    ssize_t r;

    if ((r = read(d, buf + n, nbytes - n)) == -1) {
      if (errno != EINTR) {
        return ERRLOG(errno);
      }

      continue;
    }

    if (r == 0) {  /* incomplete bytes */
      errno = EPIPE;
      return -1;
    }

    n += r;
  }

  return nbytes;
}

ssize_t writeblock(int d, const void *buf, size_t nbytes) {
  size_t n = 0;

  while (n < nbytes) {
    ssize_t r;

    if ((r = write(d, buf + n, nbytes - n)) == -1) {
      if (errno != ENOBUFS) {
        return ERRLOG(errno);
      }

      usleep(100);
      continue;
    }

    n += r;
  }

  return nbytes;
}


ssize_t writestr(int d, const char *str) {
  if (writeblock(d, str, strlen(str)) == -1) {
    return ERRLOG(errno);
  }

  return 0;
}

int sendblock(int s, const void *msg, size_t len) {
  size_t b = 0;

  while (b < len) {
    ssize_t r;

    if ((r = send(s, msg + b, len - b, 0)) == -1) {
      if (errno != ENOBUFS) {
        return ERRLOG(errno);
      }

      usleep(100);
      continue;
    }

    b += r;
  }

  return (int)b;
}

int sendstr(int s, const char *str) {
  ssize_t bytes;

  if ((bytes = sendblock(s, str, strlen(str) + 1)) == -1) {
    return ERRLOG(errno);
  }

  return (int)bytes;
}

int sendblockto(int s, const void *msg, size_t *len, int flags, const struct sockaddr *to, int tolen) {
  size_t b = 0;

  while (b < *len) {
    ssize_t r;

    if ((r = sendto(s, msg + b, (*len) - b, flags, to, tolen)) == -1) {
      if (errno != ENOBUFS) {
        *len = b;  /* report how much was sent, even on failure */
        return ERRLOG(errno);
      }

      usleep(100);
      continue;
    }

    b += r;
  }

  *len = b;
  return 0;
}

ssize_t recvblock(int s, void *buf, size_t len) {
  size_t n = 0;

  while (n < len) {
    ssize_t r;

    if ((r = recv(s, buf + n, len - n, MSG_WAITALL)) == -1) {
      if (errno != EINTR) {
        return ERRLOG(errno);
      }
      else {
        continue;
      }
    }

    /* Connection closed. EPIPE is normally returned for when send()ing
       on a closed pipe but it seems apropriate here since the socket
       was closed before the expected number of bytes could be recevied. */
    if (r == 0) {
      errno = EPIPE;
      return -1;
    }

    n += r;
  }

  return n;
}

ssize_t recvblockpeek(int s, void *buf, size_t len) {
  ssize_t r;

  do {
    if ((r = recv(s, buf, len, MSG_PEEK | MSG_WAITALL)) == -1) {
      if (errno != EINTR) {
        return ERRLOG(errno);
      }
      else {
        continue;
      }
    }

    /* Connection closed. EPIPE is normally returned for when send()ing
       on a closed pipe but it seems apropriate here since the socket
       was closed before the expected number of bytes could be recevied. */
    if (r == 0) {
      errno = EPIPE;
      return -1;
    }
  } while (r < len);

  return r;
}

ssize_t recvstr(int s, char **str) {
  char *buf;
  size_t bufsize = 8;
  size_t len = 0;

  if ((buf = (char *)malloc(bufsize)) == NULL) {
    *str = NULL;
    return ERRLOG(errno);
  }

  do {
    /* allocate more memory if needed */
    if (len + 1 >= bufsize) {
      char *newbuf;

      bufsize *= 2;

      if ((newbuf = realloc(buf, bufsize)) == NULL) {
        free(buf);
        *str = NULL;
        return ERRLOG(errno);
      }

      buf = newbuf;
    }

    /* receive one byte at a time */
    for (;;) {
      if (recv(s, buf + len, 1, MSG_WAITALL) == -1) {
        if (errno != EINTR) {
          int err = errno;

          free(buf);
          *str = NULL;
          return ERRLOG(err);
        }
        else {
          continue;
        }
      }
      else {
        break;
      }
    }

    len++;
  } while (buf[len - 1] != '\0');

  *str = buf;
  return len;
}

ssize_t recvblockfrom(int s, void *buf, size_t *len, int flags, struct sockaddr *from, socklen_t *fromlen) {
  ssize_t n = 0;

  while (n < *len) {
    ssize_t r;

    if ((r = recvfrom(s, buf + n, *len - n, flags, from, (socklen_t *)fromlen)) == -1) {
      if (errno != EINTR) {
        *len = n;  /* report how much was received, even on failure */
        return ERRLOG(errno);
      }

      continue;
    }

    if (r == 0) {
      break;
    }

    n += r;
  }

  *len = n;
  return 0;
}

int closesock(int *sock) {
  assert(sock);
  assert(*sock != -1);

  while (close(*sock)) {
    if (errno != EINTR) {
      return ERRLOG(errno);
    }
  }

  *sock = -1;
  return 0;
}
