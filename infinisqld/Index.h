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
 * @file   Index.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:22:15 2013
 * 
 * @brief  Index type class (UNIQUE, NONUNIQUE, etc.)
 */

#ifndef INFINISQLINDEX_H
#define INFINISQLINDEX_H

#include "gch.h"

typedef struct
{
    int64_t rowid;
    int16_t engineid;
    int64_t subtransactionid;
    int64_t previoussubtransactionid;
} lockingIndexEntry;

typedef struct
{
    int64_t pendingcmdid;
    int64_t tacmdentrypoint;
    lockingIndexEntry entry;
} lockQueueIndexEntry;

// buncha index types!
typedef std::map<int64_t, lockingIndexEntry> uniqueIntMap;
typedef std::multimap<int64_t, nonLockingIndexEntry_s> nonuniqueIntMap;
typedef boost::unordered_map<int64_t, lockingIndexEntry> unorderedIntMap;
typedef std::map<uint64_t, lockingIndexEntry> uniqueUintMap;
typedef std::multimap<uint64_t, nonLockingIndexEntry_s> nonuniqueUintMap;
typedef boost::unordered_map<uint64_t, lockingIndexEntry> unorderedUintMap;
typedef std::map<bool, lockingIndexEntry> uniqueBoolMap;
typedef std::multimap<bool, nonLockingIndexEntry_s> nonuniqueBoolMap;
typedef boost::unordered_map<bool, lockingIndexEntry> unorderedBoolMap;
typedef std::map<long double, lockingIndexEntry> uniqueFloatMap;
typedef std::multimap<long double, nonLockingIndexEntry_s> nonuniqueFloatMap;
typedef boost::unordered_map<long double, lockingIndexEntry> unorderedFloatMap;
typedef std::map<char, lockingIndexEntry> uniqueCharMap;
typedef std::multimap<char, nonLockingIndexEntry_s> nonuniqueCharMap;
typedef boost::unordered_map<char, lockingIndexEntry> unorderedCharMap;
typedef std::map<std::string, lockingIndexEntry> uniqueStringMap;
typedef std::multimap<std::string, nonLockingIndexEntry_s>
    nonuniqueStringMap;
typedef boost::unordered_map<std::string, lockingIndexEntry>
    unorderedStringMap;

// well, upon creation, the following should always be knowable:
// type,tableid,simple
class Index
{
public:
    Index();
    virtual ~Index();

    friend class Transaction;
    friend class Field;
    friend class SubTransaction;

    //private:

    // in lieu of constructor, call this in field creation
    void makeindex(indextype_e indextypearg, fieldtype_e fieldtypearg);

    template < typename T >
    void getequal(T input, std::vector<indexEntry_s> *returnEntries)
    {
        getequal_f(input, returnEntries);
    }

    template < typename T >
    void getin(T input, vector<indexEntry_s> *returnEntries)
    {
        // input needs to be a vector of the appropriate type
        for (size_t n=0; n < input->size(); n++)
        {
            getequal_f(input->at(n), returnEntries);
        }
    }

    // getequal_f always called by getequal or get in, which do their own
    // clearing of returnEntries vector
    void getnotequal(int64_t input, vector<indexEntry_s> *returnEntries);
    void getnotequal(uint64_t input, vector<indexEntry_s> *returnEntries);
    void getnotequal(bool input, vector<indexEntry_s> *returnEntries);
    void getnotequal(long double input, vector<indexEntry_s> *returnEntries);
    void getnotequal(char input, vector<indexEntry_s> *returnEntries);
    void getnotequal(string input, vector<indexEntry_s> *returnEntries);
    void getnotnulls(vector<indexEntry_s> *returnEntries);
    void comparison(int64_t input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    void comparison(uint64_t input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    void comparison(bool input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    void comparison(long double input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    void comparison(char input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    void comparison(string *input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    void between(int64_t lower, int64_t upper,
                 vector<indexEntry_s> *returnEntries);
    void between(uint64_t lower, uint64_t upper,
                 vector<indexEntry_s> *returnEntries);
    void between(bool lower, bool upper,
                 vector<indexEntry_s> *returnEntries);
    void between(long double lower, long double upper,
                 vector<indexEntry_s> *returnEntries);
    void between(char lower, char upper,
                 vector<indexEntry_s> *returnEntries);
    void between(string lower, string upper,
                 vector<indexEntry_s> *returnEntries);

    template <class T>
        void notbetween(T lower, T upper, vector<indexEntry_s> *returnEntries)
    {
        comparison(lower, OPERATOR_LT, returnEntries);
        comparison(upper, OPERATOR_GT, returnEntries);
    }

    void regex(string *regexStr, vector<indexEntry_s> *returnEntries);
    void like(string &likeStr, vector<indexEntry_s> *returnEntries);
    void notlike(string &likeStr, vector<indexEntry_s> *returnEntries);
    void getnotin(vector<int64_t> &entries,
                  vector<indexEntry_s> *returnEntries);
    void getnotin(vector<uint64_t> &entries,
                  vector<indexEntry_s> *returnEntries);
    void getnotin(vector<bool> &entries,
                  vector<indexEntry_s> *returnEntries);
    void getnotin(vector<long double> &entries,
                  vector<indexEntry_s> *returnEntries);
    void getnotin(vector<char> &entries,
                  vector<indexEntry_s> *returnEntries);
    void getnotin(vector<string> &entries,
                  vector<indexEntry_s> *returnEntries);

    void commitRollback(int64_t input, int64_t subtransactionid,
                        enginecmd_e cmd);
    void commitRollback(uint64_t input, int64_t subtransactionid,
                        enginecmd_e cmd);
    void commitRollback(bool input, int64_t subtransactionid,
                        enginecmd_e cmd);
    void commitRollback(long double input, int64_t subtransactionid,
                        enginecmd_e cmd);
    void commitRollback(char input, int64_t subtransactionid,
                        enginecmd_e cmd);
    void commitRollback(string input, int64_t subtransactionid,
                        enginecmd_e cmd);
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       int64_t input);
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       uint64_t input);
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       bool input);
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       long double input);
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       char input);
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       string &input);
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          int64_t input);
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          uint64_t input);
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          bool input);
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          long double input);
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          char input);
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          string &input);
    void replaceNull(int64_t oldrowid, int64_t oldengineid,
                     int64_t newrowid, int64_t newengineid);
    void getnulls(vector<indexEntry_s> *returnEntries);
    void insertNonuniqueEntry(int64_t entry, int64_t rowid, int64_t engineid);
    void insertNonuniqueEntry(uint64_t entry, int64_t rowid, int64_t engineid);
    void insertNonuniqueEntry(bool entry, int64_t rowid, int64_t engineid);
    void insertNonuniqueEntry(long double entry, int64_t rowid,
                              int64_t engineid);
    void insertNonuniqueEntry(char entry, int64_t rowid, int64_t engineid);
    void insertNonuniqueEntry(string *entry, int64_t rowid, int64_t engineid);
    void deleteNonuniqueEntry(int64_t entry, int64_t rowid, int64_t engineid);
    void deleteNonuniqueEntry(uint64_t entry, int64_t rowid, int64_t engineid);
    void deleteNonuniqueEntry(bool entry, int64_t rowid, int64_t engineid);
    void deleteNonuniqueEntry(long double entry, int64_t rowid,
                              int64_t engineid);
    void deleteNonuniqueEntry(char entry, int64_t rowid, int64_t engineid);
    void deleteNonuniqueEntry(string * engry, int64_t rowid, int64_t engineid);
    void deleteUniqueEntry(int64_t entry);
    void deleteUniqueEntry(uint64_t entry);
    void deleteUniqueEntry(bool entry);
    void deleteUniqueEntry(long double entry);
    void deleteUniqueEntry(char entry);
    void deleteUniqueEntry(string *entry);
    void insertNullEntry(int64_t rowid, int64_t engineid);
    void deleteNullEntry(int64_t rowid, int64_t engineid);
    void getall(vector<indexEntry_s> *returnEntries);

    locktype_e checkAndLock(int64_t entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    locktype_e checkAndLock(uint64_t entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    locktype_e checkAndLock(bool entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    locktype_e checkAndLock(long double entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    locktype_e checkAndLock(char entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    locktype_e checkAndLock(string *entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    void replace(int64_t entry, int64_t rowid, int64_t engineid);
    void replace(uint64_t entry, int64_t rowid, int64_t engineid);
    void replace(bool entry, int64_t rowid, int64_t engineid);
    void replace(long double entry, int64_t rowid, int64_t engineid);
    void replace(char entry, int64_t rowid, int64_t engineid);
    void replace(string *entry, int64_t rowid, int64_t engineid);
    void getequal_f(int64_t entry, vector<indexEntry_s> *returnEntries);
    void getequal_f(uint64_t entry, vector<indexEntry_s> *returnEntries);
    void getequal_f(bool entry, vector<indexEntry_s> *returnEntries);
    void getequal_f(long double entry, vector<indexEntry_s> *returnEntries);
    void getequal_f(char entry, vector<indexEntry_s> *returnEntries);
    void getequal_f(string entry, vector<indexEntry_s> *returnEntries);

    // apply unique index not null
    bool addifnotthere(fieldValue_s &val, int64_t rowid, int16_t engineid,
                       int64_t subtransactionid);
    bool checkifthere(fieldValue_s & val);
    bool checkifthere(int64_t val);
    bool checkifthere(uint64_t val);
    bool checkifthere(bool val);
    bool checkifthere(long double val);
    bool checkifthere(char val);
    bool checkifthere(string &val);
    bool checkifmatch(fieldValue_s &val, int64_t rowid, int64_t engineid);
    bool checkifmatch(int64_t val, int64_t rowid, int64_t engineid);
    bool checkifmatch(uint64_t val, int64_t rowid, int64_t engineid);
    bool checkifmatch(bool val, int64_t rowid, int64_t engineid);
    bool checkifmatch(long double val, int64_t rowid, int64_t engineid);
    bool checkifmatch(char val, int64_t rowid, int64_t rowid);
    bool checkifmatch(string &val, int64_t rowid, int64_t engineid);
    void rm(fieldValue_s &val);
    void rm(int64_t val);
    void rm(uint64_t val);
    void rm(bool val);
    void rm(long double val);
    void rm(char val);
    void rm(string &val);
    int64_t getprevioussubtransactionid(fieldValue_s &val);
    int64_t getprevioussubtransactionid(int64_t val);
    int64_t getprevioussubtransactionid(uint64_t val);
    int64_t getprevioussubtransactionid(bool val);
    int64_t getprevioussubtransactionid(long double val);
    int64_t getprevioussubtransactionid(char val);
    int64_t getprevioussubtransactionid(string &val);

    template <class T, class U, class V>
        void getIterators(string *regexStr, T mapPtr, U itBegin, V itEnd)
    {
        if (!regexStr->size())
        {
            return;
        }

        string tmpStr = *regexStr;

        if (tmpStr[0]=='^')
        {
            tmpStr.erase(0, 1);
        }

        size_t n = tmpStr.find_first_of("[\\^$.|?*+()", 0);

        if (!n)   // no beginning search string
        {
            *itBegin = mapPtr->begin();
            *itEnd = mapPtr->end();
            return;
        }

        string beginStr(tmpStr, 0, std::min(n, regexStr->size()));
        string endStr = beginStr;

        if (endStr[endStr.size()-1] == 255)
        {
            endStr.append(1, 1);
        }
        else
        {
            endStr[endStr.size()-1]++;
        }

        *itBegin = mapPtr->find(beginStr);

        if (*itBegin == mapPtr->end())
        {
            *itBegin = mapPtr->upper_bound(beginStr);
        }

        *itEnd = mapPtr->lower_bound(endStr);
    }

    indextype_e indextype;
    fieldtype_e fieldtype; // if simple, otherwise string
    maptype_e maptype;
    bool notNull;
    bool isunique;
    indexmaptype_e indexmaptype;
    // buncha index types! INT, UINT, BOOL, FLOAT, CHAR, CHARX, VARCHAR
    uniqueIntMap *uniqueIntIndex;
    nonuniqueIntMap *nonuniqueIntIndex;
    unorderedIntMap *unorderedIntIndex;
    uniqueUintMap *uniqueUintIndex;
    nonuniqueUintMap *nonuniqueUintIndex;
    unorderedUintMap *unorderedUintIndex;
    uniqueBoolMap *uniqueBoolIndex;
    nonuniqueBoolMap *nonuniqueBoolIndex;
    unorderedBoolMap *unorderedBoolIndex;
    uniqueFloatMap *uniqueFloatIndex;
    nonuniqueFloatMap *nonuniqueFloatIndex;
    unorderedFloatMap *unorderedFloatIndex;
    uniqueCharMap *uniqueCharIndex;
    nonuniqueCharMap *nonuniqueCharIndex;
    unorderedCharMap *unorderedCharIndex;
    uniqueStringMap *uniqueStringIndex;
    nonuniqueStringMap *nonuniqueStringIndex;
    unorderedStringMap *unorderedStringIndex;
    unorderedIntMap *intIndexShadow;
    unorderedUintMap *uintIndexShadow;
    unorderedBoolMap *boolIndexShadow;
    unorderedFloatMap *floatIndexShadow;
    unorderedCharMap *charIndexShadow;
    unorderedStringMap *stringIndexShadow;
    // these are used for either "unique" or unordered" indices
    boost::unordered_map< int64_t, std::queue<lockQueueIndexEntry> >
        *intLockQueue;
    boost::unordered_map< uint64_t, std::queue<lockQueueIndexEntry> >
        *uintLockQueue;
    boost::unordered_map< bool, std::queue<lockQueueIndexEntry> >
        *boolLockQueue;
    boost::unordered_map< long double, std::queue<lockQueueIndexEntry> >
        *floatLockQueue;
    boost::unordered_map< char, std::queue<lockQueueIndexEntry> >
        *charLockQueue;
    boost::unordered_map< std::string, std::queue<lockQueueIndexEntry> >
        *stringLockQueue;
    // // field [0] is rowid, [1] is engineid, just need 1 type,
    // since the entry is null
    // // and might as well make it with the index, instead of having it
    // as a pointer
    // // just less code...
    boost::unordered_set< vector<int64_t> > nulls;
};

#endif  /* INFINISQLINDEX_H */
