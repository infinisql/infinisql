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
 * @file   Lightning.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 23:47:10 2014
 * 
 * @brief  interface to Symas' Lightning MDB
 */

#include "Lightning.h"
#include "main.h"
#line 31 "Lightning.cc"

Lightning::Lightning()
{
    MDB_env *env;
    LOG("mdb_env_create results: " << mdb_env_create(&env));
    LOG("mdb_env_open results: " << mdb_env_open(env, "/tmp/lmdb", 0, S_IRUSR|S_IWUSR));
    MDB_txn *txn;
    LOG("mdb_txn_begin results: " << mdb_txn_begin(env, nullptr, 0, &txn));
    MDB_dbi dbi;
    LOG("mdb_dbi_open results: " << mdb_dbi_open(txn, nullptr, 0, &dbi));
    LOG("mdb_txn_commit results: " << mdb_txn_commit(txn));
    LOG("mdb_txn_begin results: " << mdb_txn_begin(env, nullptr, 0, &txn));
    MDB_val mkey, mval;
    string key="haha";
    string val="hoho";
    mkey.mv_size=key.size();
    mkey.mv_data=(void *)key.c_str();
    mval.mv_size=val.size();
    mval.mv_data=(void *)val.c_str();
    LOG("mdb_put results: " << mdb_put(txn, dbi, &mkey, &mval, 0));
    LOG("mdb_txn_commit results: " << mdb_txn_commit(txn));
    string key2="hehe";
    mkey.mv_size=key2.size();
    mkey.mv_data=(void *)key2.c_str();
    mval.mv_size=0;
    mval.mv_data=nullptr;
    LOG("mdb_txn_begin results: " << mdb_txn_begin(env, nullptr, 0, &txn));
    LOG("mdb_put results: " << mdb_put(txn, dbi, &mkey, &mval, 0));
    LOG("mdb_txn_commit results: " << mdb_txn_commit(txn));
    
    LOG("mdb_txn_begin results: " << mdb_txn_begin(env, nullptr, 0, &txn));
    mkey.mv_size=key.size();
    mkey.mv_data=(void *)key.c_str();
    LOG("mdb_get results: " << mdb_get(txn, dbi, &mkey, &mval));
    LOG("mdb_txn_commit results: " << mdb_txn_commit(txn));
    string res((const char *)mval.mv_data, mval.mv_size);
    LOG("val: " << res);
    
    LOG("mdb_txn_begin results: " << mdb_txn_begin(env, nullptr, 0, &txn));
    mkey.mv_size=key2.size();
    mkey.mv_data=(void *)key2.c_str();
    LOG("mdb_get results: " << mdb_get(txn, dbi, &mkey, &mval));
    LOG("mdb_txn_commit results: " << mdb_txn_commit(txn));
    LOG("val size: " << mval.mv_size);
    
    


}

Lightning::~Lightning()
{
    
}
