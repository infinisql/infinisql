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

/**
 * @file   Applier.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:03:48 2013
 * 
 * @brief  Applies transactional data modifications as part of synchronous
 * replication.
 */

#ifndef INFINISQLAPPLIER_H
#define INFINISQLAPPLIER_H

class TransactionAgent;

/** 
 * @brief On replicas, this class replicates "applies" transactions received
 * by TransactionAgent
 *
 * The receiving TransactionAgent begins the process by sending data to
 * each Engine, and then creating an Applier object and putting it in a map.
 * Engines apply replicated transaction data in strict order based on
 * subtransactionid.
 */
class Applier
{
public:
  Applier(class TransactionAgent *taPtrarg, int64_t domainidarg,
          Topology::addressStruct sourceAddrarg, int64_t partitionidarg);
  virtual ~Applier();

  void ackedApply(class MessageAckApply &msg);

  int64_t applierid;
  class TransactionAgent *taPtr;
  int64_t domainid;
  Topology::addressStruct sourceAddr;
  int64_t partitioncount;
};

#endif  /* INFINISQLAPPLIER_H */
