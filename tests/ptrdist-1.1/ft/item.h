/*
 * item.h
 *
 * The author of this software is Alain K\"{a}gi.
 *
 * Copyright (c) 1993 by Alain K\"{a}gi and the University of Wisconsin
 * Board of Trustees.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR THE UNIVERSITY OF
 * WISCONSIN MAKE ANY REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING
 * THE MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 *
 * ------------------------------------------------------------------------
 *
 * Interface between graph and Fheap.
 *
 * ------------------------------------------------------------------------
 *
 * $Id: item.h,v 1.2 1993/03/15 04:25:10 alain Exp alain $
 *
 */

#ifndef _item_h
#define _item_h

#include "graph.h"

typedef Vertices Item;

#define NULL_ITEM	NULL_VERTEX

int LessThan(Item *, Item *);
int Equal(Item *, Item *);
Item * Subtract(Item *, int);

#endif
