/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL (tm). It is available either under the
 * GNU Affero Public License or under a commercial license. Contact the
 * copyright holder for information about a commercial license if terms
 * of the GNU Affero Public License do not suit you.
 *
 * This copy of InfiniSQL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with InfiniSQL. It should be in the top level of the source
 * directory in a file entitled "COPYING".
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INDEX_HPP
#define INDEX_HPP

#include "infinisql_gch.h"

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
typedef map<int64_t, lockingIndexEntry> uniqueIntMap;
typedef multimap<int64_t, nonLockingIndexEntry_s> nonuniqueIntMap;
typedef boost::unordered_map<int64_t, lockingIndexEntry> unorderedIntMap;
typedef map<uint64_t, lockingIndexEntry> uniqueUintMap;
typedef multimap<uint64_t, nonLockingIndexEntry_s> nonuniqueUintMap;
typedef boost::unordered_map<uint64_t, lockingIndexEntry> unorderedUintMap;
typedef map<bool, lockingIndexEntry> uniqueBoolMap;
typedef multimap<bool, nonLockingIndexEntry_s> nonuniqueBoolMap;
typedef boost::unordered_map<bool, lockingIndexEntry> unorderedBoolMap;
typedef map<long double, lockingIndexEntry> uniqueFloatMap;
typedef multimap<long double, nonLockingIndexEntry_s> nonuniqueFloatMap;
typedef boost::unordered_map<long double, lockingIndexEntry> unorderedFloatMap;
typedef map<char, lockingIndexEntry> uniqueCharMap;
typedef multimap<char, nonLockingIndexEntry_s> nonuniqueCharMap;
typedef boost::unordered_map<char, lockingIndexEntry> unorderedCharMap;
typedef map<string, lockingIndexEntry> uniqueStringMap;
typedef multimap<string, nonLockingIndexEntry_s> nonuniqueStringMap;
typedef boost::unordered_map<string, lockingIndexEntry> unorderedStringMap;

// well, upon creation, the following should always be knowable:
// type,tableid,simple
class Index
{
public:
  Index(void);
  virtual ~Index();

  friend class Transaction;
  friend class Field;
  friend class SubTransaction;

  //private:

  // in lieu of constructor, call this in field creation
  void makeindex(indextype_e, fieldtype_e);

  template < typename T >
  void getequal(T input, vector<indexEntry_s> *returnEntries)
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
  void getnotequal(int64_t, vector<indexEntry_s> *);
  void getnotequal(uint64_t, vector<indexEntry_s> *);
  void getnotequal(bool, vector<indexEntry_s> *);
  void getnotequal(long double, vector<indexEntry_s> *);
  void getnotequal(char, vector<indexEntry_s> *);
  void getnotequal(string, vector<indexEntry_s> *);
  void getnotnulls(vector<indexEntry_s> *);
  void comparison(int64_t, operatortypes_e, vector<indexEntry_s> *);
  void comparison(uint64_t, operatortypes_e, vector<indexEntry_s> *);
  void comparison(bool, operatortypes_e, vector<indexEntry_s> *);
  void comparison(long double, operatortypes_e, vector<indexEntry_s> *);
  void comparison(char, operatortypes_e, vector<indexEntry_s> *);
  void comparison(string *, operatortypes_e, vector<indexEntry_s> *);
  void between(int64_t, int64_t, vector<indexEntry_s> *);
  void between(uint64_t, uint64_t, vector<indexEntry_s> *);
  void between(bool, bool, vector<indexEntry_s> *);
  void between(long double, long double, vector<indexEntry_s> *);
  void between(char, char, vector<indexEntry_s> *);
  void between(string, string, vector<indexEntry_s> *);

  template <class T>
  void notbetween(T lower, T upper, vector<indexEntry_s> *returnEntries)
  {
    comparison(lower, OPERATOR_LT, returnEntries);
    comparison(upper, OPERATOR_GT, returnEntries);
  }

  void regex(string *, vector<indexEntry_s> *);
  void like(string &, vector<indexEntry_s> *);
  void notlike(string &, vector<indexEntry_s> *);
  void getnotin(vector<int64_t> &, vector<indexEntry_s> *);
  void getnotin(vector<uint64_t> &, vector<indexEntry_s> *);
  void getnotin(vector<bool> &, vector<indexEntry_s> *);
  void getnotin(vector<long double> &, vector<indexEntry_s> *);
  void getnotin(vector<char> &, vector<indexEntry_s> *);
  void getnotin(vector<string> &, vector<indexEntry_s> *);

  void commitRollback(int64_t, int64_t, enginecmd_e);
  void commitRollback(uint64_t, int64_t, enginecmd_e);
  void commitRollback(bool, int64_t, enginecmd_e);
  void commitRollback(long double, int64_t, enginecmd_e);
  void commitRollback(char, int64_t, enginecmd_e);
  void commitRollback(string, int64_t, enginecmd_e);
  void replaceUnique(int64_t, int64_t, int64_t);
  void replaceUnique(int64_t, int64_t, uint64_t);
  void replaceUnique(int64_t, int64_t, bool);
  void replaceUnique(int64_t, int64_t, long double);
  void replaceUnique(int64_t, int64_t, char);
  void replaceUnique(int64_t, int64_t, string &);
  void replaceNonunique(int64_t, int64_t, int64_t, int64_t, int64_t);
  void replaceNonunique(int64_t, int64_t, int64_t, int64_t, uint64_t);
  void replaceNonunique(int64_t, int64_t, int64_t, int64_t, bool);
  void replaceNonunique(int64_t, int64_t, int64_t, int64_t, long double);
  void replaceNonunique(int64_t, int64_t, int64_t, int64_t, char);
  void replaceNonunique(int64_t, int64_t, int64_t, int64_t, string &);
  void replaceNull(int64_t, int64_t, int64_t, int64_t);
  void getnulls(vector<indexEntry_s> *);
  void insertNonuniqueEntry(int64_t, int64_t, int64_t);
  void insertNonuniqueEntry(uint64_t, int64_t, int64_t);
  void insertNonuniqueEntry(bool, int64_t, int64_t);
  void insertNonuniqueEntry(long double, int64_t, int64_t);
  void insertNonuniqueEntry(char, int64_t, int64_t);
  void insertNonuniqueEntry(string *, int64_t, int64_t);
  void deleteNonuniqueEntry(int64_t, int64_t, int64_t);
  void deleteNonuniqueEntry(uint64_t, int64_t, int64_t);
  void deleteNonuniqueEntry(bool, int64_t, int64_t);
  void deleteNonuniqueEntry(long double, int64_t, int64_t);
  void deleteNonuniqueEntry(char, int64_t, int64_t);
  void deleteNonuniqueEntry(string *, int64_t, int64_t);
  void deleteUniqueEntry(int64_t);
  void deleteUniqueEntry(uint64_t);
  void deleteUniqueEntry(bool);
  void deleteUniqueEntry(long double);
  void deleteUniqueEntry(char);
  void deleteUniqueEntry(string *);
  void insertNullEntry(int64_t, int64_t);
  void deleteNullEntry(int64_t, int64_t);
  void getall(vector<indexEntry_s> *);

  locktype_e checkAndLock(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t);
  locktype_e checkAndLock(uint64_t, int64_t, int64_t, int64_t, int64_t,
                          int64_t);
  locktype_e checkAndLock(bool, int64_t, int64_t, int64_t, int64_t, int64_t);
  locktype_e checkAndLock(long double, int64_t, int64_t, int64_t, int64_t,
                          int64_t);
  locktype_e checkAndLock(char, int64_t, int64_t, int64_t, int64_t, int64_t);
  locktype_e checkAndLock(string *, int64_t, int64_t, int64_t, int64_t,
                          int64_t);

  void replace(int64_t, int64_t, int64_t);
  void replace(uint64_t, int64_t, int64_t);
  void replace(bool, int64_t, int64_t);
  void replace(long double, int64_t, int64_t);
  void replace(char, int64_t, int64_t);
  void replace(string *, int64_t, int64_t);
  void getequal_f(int64_t, vector<indexEntry_s> *);
  void getequal_f(uint64_t, vector<indexEntry_s> *);
  void getequal_f(bool, vector<indexEntry_s> *);
  void getequal_f(long double, vector<indexEntry_s> *);
  void getequal_f(char, vector<indexEntry_s> *);
  void getequal_f(string, vector<indexEntry_s> *);

  // apply unique index not null
  bool addifnotthere(fieldValue_s &, int64_t, int16_t, int64_t);
  bool checkifthere(fieldValue_s &);
  bool checkifthere(int64_t);
  bool checkifthere(uint64_t);
  bool checkifthere(bool);
  bool checkifthere(long double);
  bool checkifthere(char);
  bool checkifthere(string &);
  bool checkifmatch(fieldValue_s &, int64_t, int64_t);
  bool checkifmatch(int64_t, int64_t, int64_t);
  bool checkifmatch(uint64_t, int64_t, int64_t);
  bool checkifmatch(bool, int64_t, int64_t);
  bool checkifmatch(long double, int64_t, int64_t);
  bool checkifmatch(char, int64_t, int64_t);
  bool checkifmatch(string &, int64_t, int64_t);
  void rm(fieldValue_s &);
  void rm(int64_t);
  void rm(uint64_t);
  void rm(bool);
  void rm(long double);
  void rm(char);
  void rm(string &);
  int64_t getprevioussubtransactionid(fieldValue_s &);
  int64_t getprevioussubtransactionid(int64_t);
  int64_t getprevioussubtransactionid(uint64_t);
  int64_t getprevioussubtransactionid(bool);
  int64_t getprevioussubtransactionid(long double);
  int64_t getprevioussubtransactionid(char);
  int64_t getprevioussubtransactionid(string &);

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
  boost::unordered_map< string, std::queue<lockQueueIndexEntry> >
  *stringLockQueue;
  // field [0] is rowid, [1] is engineid, just need 1 type, since the entry is null
  // and might as well make it with the index, instead of having it as a pointer
  // just less code...
  boost::unordered_set< vector<int64_t> > nulls;
};

#endif  /* INDEX_HPP */
