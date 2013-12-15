/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 
 * InfiniSQL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFINISQLLARX_H
#define INFINISQLLARX_H

#include <stdint.h>
#include "parser.h"
#include <string.h>
#include <stdio.h>

class Larxer;

struct perlarxer
{
    void *scaninfo;
    class Larxer *larxerPtr;
};

int yyparse(struct perlarxer *);

void flexinit(struct perlarxer *);
void flexbuffer(char *, size_t, void *);
void flexdestroy(void *);

#endif /* INFINISQLLARX_H */
