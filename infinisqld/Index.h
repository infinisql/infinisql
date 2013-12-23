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
/** 
 * @brief create INDEX object
 *
 * Each field has optionally 1 index associated
 *
 */
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
    /** 
     * @brief in lieu of constructor, creates Index object
     *
     * @param indextypearg index type
     * @param fieldtypearg field type
     */
    void makeindex(indextype_e indextypearg, fieldtype_e fieldtypearg);

    /** 
     * @brief return index entries matching equality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    template < typename T >
    void getequal(T input, std::vector<indexEntry_s> *returnEntries)
    {
        getequal_f(input, returnEntries);
    }

    /** 
     * @brief return index entries matching IN expression
     *
     * @param input list of IN values to compare
     * @param returnEntries matching rows
     */
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
    /** 
     * @brief return index entries matching inequality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    void getnotequal(int64_t input, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching inequality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    void getnotequal(uint64_t input, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching inequality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    void getnotequal(bool input, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching inequality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    void getnotequal(long double input, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching inequality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    void getnotequal(char input, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching inequality expression
     *
     * @param input value to compare
     * @param returnEntries matching rows
     */
    void getnotequal(string input, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return all non-NULL entries
     *
     * @param returnEntries matching rows
     */
    void getnotnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching <,>,<=, or >= expression
     *
     * @param input value to compare
     * @param op operator
     * @param returnEntries matching rows
     */
    void comparison(int64_t input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching <,>,<=, or >= expression
     *
     * @param input value to compare
     * @param op operator
     * @param returnEntries matching rows
     */
    void comparison(uint64_t input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching <,>,<=, or >= expression
     *
     * @param input value to compare
     * @param op operator
     * @param returnEntries matching rows
     */
    void comparison(bool input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching <,>,<=, or >= expression
     *
     * @param input value to compare
     * @param op operator
     * @param returnEntries matching rows
     */
    void comparison(long double input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching <,>,<=, or >= expression
     *
     * @param input value to compare
     * @param op operator
     * @param returnEntries matching rows
     */
    void comparison(char input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching <,>,<=, or >= expression
     *
     * @param input value to compare
     * @param op operator
     * @param returnEntries matching rows
     */
    void comparison(string *input, operatortypes_e op,
                    vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching BETWEEN expression
     *
     * BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> BETWEEN lower AND upper" is equivalent to:
     * "<fieldvalue> >= lower AND <fieldvalue> <= upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    void between(int64_t lower, int64_t upper,
                 vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching BETWEEN expression
     *
     * BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> BETWEEN lower AND upper" is equivalent to:
     * "<fieldvalue> >= lower AND <fieldvalue> <= upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    void between(uint64_t lower, uint64_t upper,
                 vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching BETWEEN expression
     *
     * BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> BETWEEN lower AND upper" is equivalent to:
     * "<fieldvalue> >= lower AND <fieldvalue> <= upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    void between(bool lower, bool upper,
                 vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching BETWEEN expression
     *
     * BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> BETWEEN lower AND upper" is equivalent to:
     * "<fieldvalue> >= lower AND <fieldvalue> <= upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    void between(long double lower, long double upper,
                 vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching BETWEEN expression
     *
     * BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> BETWEEN lower AND upper" is equivalent to:
     * "<fieldvalue> >= lower AND <fieldvalue> <= upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    void between(char lower, char upper,
                 vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching BETWEEN expression
     *
     * BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> BETWEEN lower AND upper" is equivalent to:
     * "<fieldvalue> >= lower AND <fieldvalue> <= upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    void between(string lower, string upper,
                 vector<indexEntry_s> *returnEntries);

    /** 
     * @brief return index entries matching NOT BETWEEN expression
     *
     * NOT BETWEEN is implemented according to SQL-92 spec:
     * "<fieldvalue> NOT BETWEEN lower and UPPER" is equivalent to:
     * "<fieldvalue> < lower OR <fieldvalue> > upper"
     *
     * @param lower lower range
     * @param upper upper range
     * @param returnEntries matching rows
     */
    template <class T>
        void notbetween(T lower, T upper, vector<indexEntry_s> *returnEntries)
    {
        comparison(lower, OPERATOR_LT, returnEntries);
        comparison(upper, OPERATOR_GT, returnEntries);
    }

    /** 
     * @brief return index entries matching regex search
     *
     * PCRE is regex library
     *
     * @param regexStr regular expression
     * @param returnEntries matching rows
     */
    void regex(string *regexStr, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching LIKE search
     *
     * implemented by calling regex method
     *
     * @param likeStr LIKE operand
     * @param returnEntries matching rows
     */
    void like(string &likeStr, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries maching NOT LIKE search
     *
     * @param likeStr NOT LIKE operand
     * @param returnEntries matching rows
     */
    void notlike(string &likeStr, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching NOT IN expression
     *
     * @param input list of NOT IN values to compare
     * @param returnEntries matching rows
     */    
    void getnotin(vector<int64_t> &entries,
                  vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching NOT IN expression
     *
     * @param input list of NOT IN values to compare
     * @param returnEntries matching rows
     */    
    void getnotin(vector<uint64_t> &entries,
                  vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching NOT IN expression
     *
     * @param input list of NOT IN values to compare
     * @param returnEntries matching rows
     */    
    void getnotin(vector<bool> &entries,
                  vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching NOT IN expression
     *
     * @param input list of NOT IN values to compare
     * @param returnEntries matching rows
     */    
    void getnotin(vector<long double> &entries,
                  vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching NOT IN expression
     *
     * @param input list of NOT IN values to compare
     * @param returnEntries matching rows
     */    
    void getnotin(vector<char> &entries,
                  vector<indexEntry_s> *returnEntries);
    /** 
     * @brief return index entries matching NOT IN expression
     *
     * @param input list of NOT IN values to compare
     * @param returnEntries matching rows
     */    
    void getnotin(vector<string> &entries,
                  vector<indexEntry_s> *returnEntries);

    /** 
     * @brief commit or rollback entry
     *
     * @param input field value
     * @param subtransactionid subtransactionid
     * @param cmd commit or rollback
     */
    void commitRollback(int64_t input, int64_t subtransactionid,
                        enginecmd_e cmd);
    /** 
     * @brief commit or rollback entry
     *
     * @param input field value
     * @param subtransactionid subtransactionid
     * @param cmd commit or rollback
     */
    void commitRollback(uint64_t input, int64_t subtransactionid,
                        enginecmd_e cmd);
    /** 
     * @brief commit or rollback entry
     *
     * @param input field value
     * @param subtransactionid subtransactionid
     * @param cmd commit or rollback
     */
    void commitRollback(bool input, int64_t subtransactionid,
                        enginecmd_e cmd);
    /** 
     * @brief commit or rollback entry
     *
     * @param input field value
     * @param subtransactionid subtransactionid
     * @param cmd commit or rollback
     */
    void commitRollback(long double input, int64_t subtransactionid,
                        enginecmd_e cmd);
    /** 
     * @brief commit or rollback entry
     *
     * @param input field value
     * @param subtransactionid subtransactionid
     * @param cmd commit or rollback
     */
    void commitRollback(char input, int64_t subtransactionid,
                        enginecmd_e cmd);
    /** 
     * @brief commit or rollback entry
     *
     * @param input field value
     * @param subtransactionid subtransactionid
     * @param cmd commit or rollback
     */
    void commitRollback(string input, int64_t subtransactionid,
                        enginecmd_e cmd);
    /** 
     * @brief replace index value
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       int64_t input);
    /** 
     * @brief replace index value
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       uint64_t input);
    /** 
     * @brief replace index value
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       bool input);
    /** 
     * @brief replace index value
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       long double input);
    /** 
     * @brief replace index value
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       char input);
    /** 
     * @brief replace index value
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceUnique(int64_t newrowid, int64_t newengineid,
                       string &input);
    /** 
     * @brief replace index value in non-unique index
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          int64_t input);
    /** 
     * @brief replace index value in non-unique index
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          uint64_t input);
    /** 
     * @brief replace index value in non-unique index
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          bool input);
    /** 
     * @brief replace index value in non-unique index
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          long double input);
    /** 
     * @brief replace index value in non-unique index
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          char input);
    /** 
     * @brief replace index value in non-unique index
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     * @param input field value
     */
    void replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                          int64_t newrowid, int64_t newengineid,
                          string &input);
    /** 
     * @brief replace NULL entry
     *
     * this activity points an existing index entry to a new row
     * used when 1st field of a row is updated, as that activity is
     * implemented as a DELETE then INSERT of the row
     *
     * @param oldrowid rowid to replace
     * @param oldengineid engineid to replace
     * @param newrowid new rowid
     * @param newengineid new engineid
     */
    void replaceNull(int64_t oldrowid, int64_t oldengineid,
                     int64_t newrowid, int64_t newengineid);
    
    /** 
     * @brief return index entries matching IS NULL expression
     *
     * @param returnEntries matching rows
     */
    void getnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief insert entry into non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNonuniqueEntry(int64_t entry, int64_t rowid, int64_t engineid);
    void getnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief insert entry into non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNonuniqueEntry(uint64_t entry, int64_t rowid, int64_t engineid);
    void getnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief insert entry into non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNonuniqueEntry(bool entry, int64_t rowid, int64_t engineid);
    void getnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief insert entry into non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNonuniqueEntry(long double entry, int64_t rowid,
                              int64_t engineid);
    void getnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief insert entry into non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNonuniqueEntry(char entry, int64_t rowid, int64_t engineid);
    void getnulls(vector<indexEntry_s> *returnEntries);
    /** 
     * @brief insert entry into non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNonuniqueEntry(string *entry, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry from non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNonuniqueEntry(int64_t entry, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry from non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNonuniqueEntry(uint64_t entry, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry from non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNonuniqueEntry(bool entry, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry from non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNonuniqueEntry(long double entry, int64_t rowid,
                              int64_t engineid);
    /** 
     * @brief delete entry from non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNonuniqueEntry(char entry, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry from non-unique index
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNonuniqueEntry(string * engry, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry from unique index
     *
     * @param entry field value
     */
    void deleteUniqueEntry(int64_t entry);
    /** 
     * @brief delete entry from unique index
     *
     * @param entry field value
     */
    void deleteUniqueEntry(uint64_t entry);
    /** 
     * @brief delete entry from unique index
     *
     * @param entry field value
     */
    void deleteUniqueEntry(bool entry);
    /** 
     * @brief delete entry from unique index
     *
     * @param entry field value
     */
    void deleteUniqueEntry(long double entry);
    /** 
     * @brief delete entry from unique index
     *
     * @param entry field value
     */
    void deleteUniqueEntry(char entry);
    /** 
     * @brief delete entry from unique index
     *
     * @param entry field value
     */
    void deleteUniqueEntry(string *entry);
    /** 
     * @brief insert entry with NULL value
     *
     * @param rowid rowid
     * @param engineid engineid
     */
    void insertNullEntry(int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry with NULL value
     *
     * @param rowid rowid
     * @param engineid engineid
     */
    void deleteNullEntry(int64_t rowid, int64_t engineid);
    /** 
     * @brief SELECT *
     *
     * @param returnEntries matching rows
     */
    void getall(vector<indexEntry_s> *returnEntries);

    /** 
     * @brief (not used 12/22/13) check existence of entry and lock
     *
     * creates pending lock if not acquirable
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command in calling Transaction
     * @param tacmdentrypoint entry point for pending command
     *
     * @return type of lock acquired (or pending)
     */
    locktype_e checkAndLock(int64_t entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    /** 
     * @brief (not used 12/22/13) check existence of entry and lock
     *
     * creates pending lock if not acquirable
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command in calling Transaction
     * @param tacmdentrypoint entry point for pending command
     *
     * @return type of lock acquired (or pending)
     */
    locktype_e checkAndLock(uint64_t entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    /** 
     * @brief (not used 12/22/13) check existence of entry and lock
     *
     * creates pending lock if not acquirable
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command in calling Transaction
     * @param tacmdentrypoint entry point for pending command
     *
     * @return type of lock acquired (or pending)
     */
    locktype_e checkAndLock(bool entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    /** 
     * @brief (not used 12/22/13) check existence of entry and lock
     *
     * creates pending lock if not acquirable
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command in calling Transaction
     * @param tacmdentrypoint entry point for pending command
     *
     * @return type of lock acquired (or pending)
     */
    locktype_e checkAndLock(long double entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    /** 
     * @brief (not used 12/22/13) check existence of entry and lock
     *
     * creates pending lock if not acquirable
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command in calling Transaction
     * @param tacmdentrypoint entry point for pending command
     *
     * @return type of lock acquired (or pending)
     */
    locktype_e checkAndLock(char entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    /** 
     * @brief (not used 12/22/13) check existence of entry and lock
     *
     * creates pending lock if not acquirable
     *
     * @param entry field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command in calling Transaction
     * @param tacmdentrypoint entry point for pending command
     *
     * @return type of lock acquired (or pending)
     */
    locktype_e checkAndLock(string *entry, int64_t rowid, int64_t engineid,
                            int64_t subtransactionid, int64_t pendingcmdid,
                            int64_t tacmdentrypoint);
    /** 
     * @brief perform activity to check for equality
     *
     * @param entry field value
     * @param returnEntries matchine rows
     */
    void getequal_f(int64_t entry, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief perform activity to check for equality
     *
     * @param entry field value
     * @param returnEntries matchine rows
     */
    void getequal_f(uint64_t entry, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief perform activity to check for equality
     *
     * @param entry field value
     * @param returnEntries matchine rows
     */
    void getequal_f(bool entry, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief perform activity to check for equality
     *
     * @param entry field value
     * @param returnEntries matchine rows
     */
    void getequal_f(long double entry, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief perform activity to check for equality
     *
     * @param entry field value
     * @param returnEntries matchine rows
     */
    void getequal_f(char entry, vector<indexEntry_s> *returnEntries);
    /** 
     * @brief perform activity to check for equality
     *
     * @param entry field value
     * @param returnEntries matchine rows
     */
    void getequal_f(string entry, vector<indexEntry_s> *returnEntries);

    // apply unique index not null
    /** 
     * @brief create index entry as part of synchronous replication
     *
     * @param val field value
     * @param rowid rowid
     * @param engineid engineid
     * @param subtransactionid subtransactionid
     *
     * @return 
     */
    bool addifnotthere(fieldValue_s &val, int64_t rowid, int16_t engineid,
                       int64_t subtransactionid);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(fieldValue_s & val);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(int64_t val);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(uint64_t val);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(bool val);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(long double val);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(char val);
    /** 
     * @brief check for entry as part of synchronous replication
     *
     * @param val field value
     *
     * @return match or no match
     */
    bool checkifthere(string &val);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(fieldValue_s &val, int64_t rowid, int64_t engineid);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(int64_t val, int64_t rowid, int64_t engineid);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(uint64_t val, int64_t rowid, int64_t engineid);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(bool val, int64_t rowid, int64_t engineid);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(long double val, int64_t rowid, int64_t engineid);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(char val, int64_t rowid, int64_t rowid);
    /** 
     * @brief check if entry matches row as part of synchronous replication
     *
     * @param val field entry
     * @param rowid rowid
     * @param engineid engineid
     *
     * @return match or no match
     */
    bool checkifmatch(string &val, int64_t rowid, int64_t engineid);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(fieldValue_s &val);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(int64_t val);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(uint64_t val);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(bool val);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(long double val);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(char val);
    /** 
     * @brief delete entry as part of synchronous replication
     *
     * @param val field value
     */
    void rm(string &val);
    /** 
     * @brief return previous subtransactionid for field
     *
     * part of synchronous replication
     *
     * @param val field value
     *
     * @return 
     */
    int64_t getprevioussubtransactionid(fieldValue_s &val);
    int64_t getprevioussubtransactionid(int64_t val);
    int64_t getprevioussubtransactionid(uint64_t val);
    int64_t getprevioussubtransactionid(bool val);
    int64_t getprevioussubtransactionid(long double val);
    int64_t getprevioussubtransactionid(char val);
    int64_t getprevioussubtransactionid(string &val);

    /** 
     * @brief set iterators part of regex query
     *
     * @param regexStr regex
     * @param mapPtr map to search
     * @param itBegin beginning iterator
     * @param itEnd ending iterator
     */
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
