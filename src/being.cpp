/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#include <stdio.h>

#include "astar.h"
#include "being.h"

NODE *player_node = NULL;
NODE *head = NULL; // First node of the list
unsigned int b_count = 0; // Number of beings in the list

/** Create a path node */
PATH_NODE::PATH_NODE(unsigned short x, unsigned short y):
  next(NULL)
{
  this->x = x;
  this->y = y;
}

/** Return a* path */
PATH_NODE *calculate_path(unsigned short src_x, unsigned short src_y, unsigned short dest_x, unsigned short dest_y) {
    return find_path(1, src_x, src_y, dest_x, dest_y);
}

/** Returns the first node of the list */
NODE *get_head() {
  return head;
}

/** Creates a empty node */
NODE::NODE():
  id(0), job(0),
  next(NULL),
  action(0), frame(0),
  path(NULL),
  speech(NULL), speech_time(0),
  tick_time(0), speed(150),
  emotion(0), emotion_time(0),
  text_x(0), text_y(0),
  hair_style(1), hair_color(1),
  weapon(0)
{
  memset(coordinates, 0, 3);
  speech_color = makecol(255,255,255);
}

/** Returns number of beings in the list */
unsigned int get_count() {
  return b_count;
}

/** Removes all beings from the list */
void empty() {
  NODE *node = head;
  NODE *next;
  while(node) {
    next = node->next;
    delete node;
    node = next;
  }
  b_count = 0;
  head = NULL;
}

/** Add a node to the list */
void add_node(NODE *node) {
  NODE *temp = head;
  if(temp) {
    while(temp->next)
      temp = temp->next;
    temp->next = node;
  } else head = node;
  b_count++;
}

/** Remove a node */
void remove_node(unsigned int id) {
  unsigned int temp;
  NODE *node_old, *node_new;
  node_old = head;
  node_new = NULL;
    while(node_old) {
    temp = get_id(node_old);
        if(temp==id) {
      if(node_new==NULL) {
        head = node_old->next;
        delete node_old;
        b_count--;
        return;
      } else {
                node_new->next = node_old->next;
        delete node_old;
        b_count--;
        return;
      }
        } else {
      node_new = node_old;
      node_old = node_old->next;
    }
  }
}

/** Returns the id of a being in the list */
unsigned int get_id(NODE *node) {
  return node->id;
}

/** Find a NPC id based on its coordinates */
unsigned int find_npc(unsigned short x, unsigned short y) {
  NODE *node = head;
  while(node) {
    if((node->job>=46)&&(node->job<=125)) { // Check if is a NPC (only low job ids)
      if((get_x(node->coordinates)==x)&&(get_y(node->coordinates)==y))
        return node->id;
      else node = node->next;
		} else {
			node = node->next;
			//alert("id","","","","",0,0);
		}
  }
  return 0;
}

/** Find a MONSTER id based on its coordinates */
unsigned int find_monster(unsigned short x, unsigned short y) {
  NODE *node = head;
  while(node) {
    if(node->job>200) { // Check if is a MONSTER
			if(get_x(node->coordinates)==x && get_y(node->coordinates)==y)
        return node->id;
    }
    node = node->next;
  }
  return 0;
}

/** Return a specific id node */
NODE *find_node(unsigned int id) {
  NODE *node = head;
  while(node)
    if(node->id==id)
      return node;
    else node = node->next;
  return NULL;
}

/** Sort beings in vertical order using bubble sort */
void sort() {
  NODE *p, *q, *r, *s, *temp;
  s = NULL;

  while(s!=head->next) {
    r = p = head;
    q = p->next;

    while(p!=s) {
      if(get_y(p->coordinates)>get_y(q->coordinates)) {
        if(p==head) {
          temp = q->next;
          q->next = p;
          p->next = temp;
          head = q;
          r = q;
        } else {
          temp = q->next;
          q->next = p;
          p->next = temp;
          r->next = q;
          r = q;
        }
      } else {
        r = p;
        p = p->next;
      }
      q = p->next;
      if(q==s)s = p;
    }
  }
}

/** Remove all path nodes from a being */
void empty_path(NODE *node) {
	if(node) {
    PATH_NODE *temp = node->path;
    PATH_NODE *next;
    while(temp) {
      next = temp->next;
      delete temp;
      temp = next;
    }
    node->path = NULL;
	}
}
