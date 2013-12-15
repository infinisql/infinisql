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

#ifndef INFINISQLAPPLIER_H
#define INFINISQLAPPLIER_H

class TransactionAgent;

class Applier
{
public:
  Applier(class TransactionAgent *, int64_t, Topology::addressStruct, int64_t);
  virtual ~Applier();

  void ackedApply(class MessageAckApply &);

  int64_t applierid;
  class TransactionAgent *taPtr;
  int64_t domainid;
  Topology::addressStruct sourceAddr;
  int64_t partitioncount;
};

#endif  /* INFINISQLAPPLIER_H */

