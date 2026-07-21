/*
 *  list.c
 *  XBolo
 *
 *  Created by Robert Chrzanowski on 11/7/04.
 *  Copyright 2004 Robert Chrzanowski. All rights reserved.
 *
 */

#include "list.h"
#include "errchk.h"

#include <stdlib.h>
#include <assert.h>

int initlist(struct ListNode *list) {
  assert(list != NULL);
  list->prev = NULL;
  list->next = NULL;
  list->ptr = NULL;
  return 0;
}


int addlist(struct ListNode *list, void *ptr) {
  struct ListNode *node;

  assert(list != NULL);

  if ((node = (struct ListNode *)malloc(sizeof(struct ListNode))) == NULL) {
    return ERRLOG(errno);
  }

  node->prev = list;
  node->next = list->next;
  node->prev->next = node;

  if (node->next != NULL) {
    node->next->prev = node;
  }

  node->ptr = ptr;
  return 0;
}


struct ListNode *removelist(struct ListNode *node, void (*releasefunc)(void *)) {
  struct ListNode *next;

  node->prev->next = node->next;

  if (node->next != NULL) {
    node->next->prev = node->prev;
  }

  next = node->next;

  releasefunc(node->ptr);
  free(node);

  return next;
}

void clearlist(struct ListNode *list, void (*releasefunc)(void *)) {
  struct ListNode *node, *next;

  assert(list != NULL);

  for (node = list->next; node != NULL; node = next) {
    next = node->next;
    releasefunc(node->ptr);
    free(node);
  }

  list->next = NULL;
}

struct ListNode *nextlist(struct ListNode *node) {
  assert(node != NULL);
  return node->next;
}

struct ListNode *prevlist(struct ListNode *node) {
  assert(node != NULL);
  return node->prev;
}

void *ptrlist(struct ListNode *node) {
  assert(node != NULL);
  return node->ptr;
}
