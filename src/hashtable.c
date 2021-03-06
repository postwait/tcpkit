/**
 *   tcpkit --  toolkit to analyze tcp packet
 *   Copyright (C) 2018  @git-hulk
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 **/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"


hashtable *hashtable_create(int nbucket) {
   if (nbucket <= 0) return NULL;
   hashtable *ht = malloc(sizeof(*ht));
   if (!ht) return NULL;
   ht->free = NULL;
   ht->nbucket = nbucket;
   ht->buckets = calloc(nbucket, sizeof(entry*));
   if (!ht->buckets) {
      free(ht);
      return NULL;
   }
   return ht;
}

void hashtable_destroy(hashtable *ht) {
   int i;
   if (!ht) return;
   entry *current,*next;
   for (i = 0; i < ht->nbucket; i++) {
       current = ht->buckets[i];
       while (current) {
           next = current->next;
           free(current->key);
           ht->free ? ht->free(current->value) : free(current->value);
           free(current);
           current = next;
       }
   }
   free(ht->buckets);
   free(ht);
}

unsigned int hash_function(const void *key, int len) {
    uint32_t seed = 5381;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    uint32_t h = seed ^ len;

    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

int hashtable_add(hashtable *ht, char *key, void *value) {
    int bucket;
    entry *next;

    bucket = hash_function(key, strlen(key)) % ht->nbucket;
    next = ht->buckets[bucket];
    while (next) {
        if (!strncasecmp(key, next->key, strlen(next->key))) return 0;
    }
    entry *e = malloc(sizeof(*e));
    e->value = value;
    e->key = strdup(key);
    e->next = ht->buckets[bucket];
    ht->buckets[bucket] = e;
    return 1;
}

void *hashtable_get(hashtable *ht, char *key) {
    int bucket;
    entry *current;

    bucket = hash_function(key, strlen(key)) % ht->nbucket;
    current = ht->buckets[bucket];
    while(current) {
        if (!strncasecmp(key, current->key, strlen(current->key))) {
           return current->value;
        }
        current = current->next;
    }
    return NULL;
}

int hashtable_del(hashtable *ht, char *key) {
    int bucket;
    entry *next, *prev;

    bucket = hash_function(key, strlen(key)) % ht->nbucket;
    prev = next = ht->buckets[bucket];
    while (next) {
        if (!strncasecmp(key, next->key, strlen(next->key))) {
            prev->next = next->next;
            free(next->key);
            ht->free ? ht->free(next->value):free(next->value);
            free(next);
            if (prev == ht->buckets[bucket]) {
               ht->buckets[bucket] = NULL;
            }
            return 1;
        }
        prev = next;
        next = next->next;
    }
    return 0;
}
