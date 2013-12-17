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

#include "Index.h"
#line 22 "Index.cc"

Index::Index(void) : indextype (NONE), fieldtype (NOFIELDTYPE),
                     maptype (Nomaptype), notNull (true), isunique (true),
                     indexmaptype (noindexmaptype), uniqueIntIndex (NULL),
                     nonuniqueIntIndex (NULL), unorderedIntIndex (NULL),
                     uniqueUintIndex (NULL), nonuniqueUintIndex (NULL),
                     unorderedUintIndex (NULL), uniqueBoolIndex (NULL),
                     nonuniqueBoolIndex (NULL), unorderedBoolIndex (NULL),
    uniqueFloatIndex (NULL), nonuniqueFloatIndex (NULL),
    unorderedFloatIndex (NULL), uniqueCharIndex (NULL),
    nonuniqueCharIndex (NULL), unorderedCharIndex (NULL),
    uniqueStringIndex (NULL), nonuniqueStringIndex (NULL),
    unorderedStringIndex (NULL), intIndexShadow (NULL),
    uintIndexShadow (NULL), boolIndexShadow (NULL),
    floatIndexShadow (NULL), charIndexShadow (NULL),
    stringIndexShadow (NULL), intLockQueue (NULL),
    uintLockQueue (NULL), boolLockQueue (NULL),
    floatLockQueue (NULL), charLockQueue (NULL),
    stringLockQueue (NULL)
{
    ;
}
// in lieu of constructor, since Field will call this as part of its constructor
void Index::makeindex(indextype_e indextypearg, fieldtype_e fieldtypearg)
{
    indextype = indextypearg;
    fieldtype = fieldtypearg;

    if (indextype==UNIQUE || indextype==UNORDERED || indextype==UNIQUENOTNULL ||
        indextype==UNORDEREDNOTNULL)
    {
        isunique=true;
    }
    else
    {
        isunique=false;
    }

    if (indextype==UNIQUENOTNULL || indextype==NONUNIQUENOTNULL ||
        indextype==UNORDEREDNOTNULL)
    {
        notNull=true;
    }
    else
    {
        notNull=false;
    }

    if (indextype==NONE)
    {
        return;
    }

    // 0 unique 1 nonunique 2 unordered
    if (indextype==UNIQUE || indextype==UNIQUENOTNULL)
    {
        maptype = Unique;
    }
    else if (indextype==NONUNIQUE || indextype==NONUNIQUENOTNULL)
    {
        maptype = Nonunique;
    }
    else if (indextype==UNORDERED || indextype==UNORDEREDNOTNULL)
    {
        maptype = Unordered;
    }

    switch (fieldtype)
    {
    case INT:
        indexmaptype=(indexmaptype_e)maptype;
        break;

    case UINT:
        indexmaptype=(indexmaptype_e)(3+maptype);
        break;

    case BOOL:
        indexmaptype=(indexmaptype_e)(6+maptype);
        break;

    case FLOAT:
        indexmaptype=(indexmaptype_e)(9+maptype);
        break;

    case CHAR:
        indexmaptype=(indexmaptype_e)(12+maptype);
        break;

    case CHARX:
        indexmaptype=(indexmaptype_e)(15+maptype);
        break;

    case VARCHAR:
        indexmaptype=(indexmaptype_e)(18+maptype);
        break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", fieldtype, __FILE__, __LINE__);
    }

    switch (indexmaptype)
    {
    case uniqueint:
        uniqueIntIndex = new uniqueIntMap;
        intLockQueue = new boost::unordered_map< int64_t,
                                                 std::queue<lockQueueIndexEntry> >;
        intIndexShadow = new unorderedIntMap;
        break;

    case nonuniqueint:
        nonuniqueIntIndex = new nonuniqueIntMap;
        break;

    case unorderedint:
        unorderedIntIndex = new unorderedIntMap;
        intLockQueue = new boost::unordered_map< int64_t,
                                                 std::queue<lockQueueIndexEntry> >;
        intIndexShadow = new unorderedIntMap;
        break;

    case uniqueuint:
        uniqueUintIndex = new uniqueUintMap;
        uintLockQueue = new boost::unordered_map< uint64_t,
                                                  std::queue<lockQueueIndexEntry> >;
        uintIndexShadow = new unorderedUintMap;
        break;

    case nonuniqueuint:
        nonuniqueUintIndex = new nonuniqueUintMap;
        break;

    case unordereduint:
        unorderedUintIndex = new unorderedUintMap;
        uintLockQueue = new boost::unordered_map< uint64_t,
                                                  std::queue<lockQueueIndexEntry> >;
        uintIndexShadow = new unorderedUintMap;
        break;

    case uniquebool:
        uniqueBoolIndex = new uniqueBoolMap;
        boolLockQueue = new boost::unordered_map< bool,
                                                  std::queue<lockQueueIndexEntry> >;
        boolIndexShadow = new unorderedBoolMap;
        break;

    case nonuniquebool:
        nonuniqueBoolIndex = new nonuniqueBoolMap;
        break;

    case unorderedbool:
        unorderedBoolIndex = new unorderedBoolMap;
        boolLockQueue = new boost::unordered_map< bool,
                                                  std::queue<lockQueueIndexEntry> >;
        boolIndexShadow = new unorderedBoolMap;
        break;

    case uniquefloat:
        uniqueFloatIndex = new uniqueFloatMap;
        floatLockQueue = new boost::unordered_map< long double,
                                                   std::queue<lockQueueIndexEntry> >;
        floatIndexShadow = new unorderedFloatMap;
        break;

    case nonuniquefloat:
        nonuniqueFloatIndex = new nonuniqueFloatMap;
        break;

    case unorderedfloat:
        unorderedFloatIndex = new unorderedFloatMap;
        floatLockQueue = new boost::unordered_map< long double,
                                                   std::queue<lockQueueIndexEntry> >;
        floatIndexShadow = new unorderedFloatMap;
        break;

    case uniquechar:
        uniqueCharIndex = new uniqueCharMap;
        charLockQueue = new boost::unordered_map< char,
                                                  std::queue<lockQueueIndexEntry> >;
        charIndexShadow = new unorderedCharMap;
        break;

    case nonuniquechar:
        nonuniqueCharIndex = new nonuniqueCharMap;
        break;

    case unorderedchar:
        unorderedCharIndex = new unorderedCharMap;
        charLockQueue = new boost::unordered_map< char,
                                                  std::queue<lockQueueIndexEntry> >;
        charIndexShadow = new unorderedCharMap;
        break;

    case uniquecharx:
        uniqueStringIndex = new uniqueStringMap;
        stringLockQueue = new boost::unordered_map< string,
                                                    std::queue<lockQueueIndexEntry> >;
        stringIndexShadow = new unorderedStringMap;
        break;

    case nonuniquecharx:
        nonuniqueStringIndex = new nonuniqueStringMap;
        break;

    case unorderedcharx:
        unorderedStringIndex = new unorderedStringMap;
        stringLockQueue = new boost::unordered_map< string,
                                                    std::queue<lockQueueIndexEntry> >;
        stringIndexShadow = new unorderedStringMap;
        break;

    case uniquevarchar:
        uniqueStringIndex = new uniqueStringMap;
        stringLockQueue = new boost::unordered_map< string,
                                                    std::queue<lockQueueIndexEntry> >;
        stringIndexShadow = new unorderedStringMap;
        break;

    case nonuniquevarchar:
        nonuniqueStringIndex = new nonuniqueStringMap;
        break;

    case unorderedvarchar:
        unorderedStringIndex = new unorderedStringMap;
        stringLockQueue = new boost::unordered_map< string,
                                                    std::queue<lockQueueIndexEntry> >;
        stringIndexShadow = new unorderedStringMap;
        break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

Index::~Index()
{
}

//int
locktype_e Index::checkAndLock(int64_t entry, int64_t rowid, int64_t engineid,
                               int64_t subtransactionid, int64_t pendingcmdid,
                               int64_t tacmdentrypoint)
{
    switch (indexmaptype)
    {
    case uniqueint:
        if (uniqueIntIndex->count(entry))
        {
            if (uniqueIntIndex->at(entry).subtransactionid)   // already locked
            {
                if (uniqueIntIndex->at(entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                intLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueIntIndex->at(entry).rowid = rowid;
            uniqueIntIndex->at(entry).engineid = engineid;
            uniqueIntIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unorderedint:
        if (unorderedIntIndex->count(entry))
        {
            if (unorderedIntIndex->at(entry).subtransactionid)   // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                intLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedIntIndex->at(entry).rowid = rowid;
            unorderedIntIndex->at(entry).engineid = engineid;
            unorderedIntIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK; // intent to add an index entry
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }

    return NOLOCK;
}

// uint
locktype_e Index::checkAndLock(uint64_t entry, int64_t rowid, int64_t engineid,
                               int64_t subtransactionid, int64_t pendingcmdid, int64_t tacmdentrypoint)
{
    switch (indexmaptype)
    {
    case uniqueuint:
        if (uniqueUintIndex->count(entry))
        {
            if (uniqueUintIndex->at(entry).subtransactionid)   // already locked
            {
                if (uniqueUintIndex->at(entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                uintLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueUintIndex->at(entry).rowid = rowid;
            uniqueUintIndex->at(entry).engineid = engineid;
            uniqueUintIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unordereduint:
        if (unorderedUintIndex->count(entry))
        {
            if (unorderedUintIndex->at(entry).subtransactionid)   // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                uintLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedUintIndex->at(entry).rowid = rowid;
            unorderedUintIndex->at(entry).engineid = engineid;
            unorderedUintIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }

    return NOLOCK;
}

//bool
locktype_e Index::checkAndLock(bool entry, int64_t rowid, int64_t engineid,
                               int64_t subtransactionid, int64_t pendingcmdid, int64_t tacmdentrypoint)
{
    switch (indexmaptype)
    {
    case uniquebool:
        if (uniqueBoolIndex->count(entry))
        {
            if (uniqueBoolIndex->at(entry).subtransactionid)   // already locked
            {
                if (uniqueBoolIndex->at(entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                boolLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueBoolIndex->at(entry).rowid = rowid;
            uniqueBoolIndex->at(entry).engineid = engineid;
            uniqueBoolIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unorderedbool:
        if (unorderedBoolIndex->count(entry))
        {
            if (unorderedBoolIndex->at(entry).subtransactionid)   // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                boolLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedBoolIndex->at(entry).rowid = rowid;
            unorderedBoolIndex->at(entry).engineid = engineid;
            unorderedBoolIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }

    return NOLOCK;
}

//float
locktype_e Index::checkAndLock(long double entry, int64_t rowid,
                               int64_t engineid, int64_t subtransactionid, int64_t pendingcmdid,
                               int64_t tacmdentrypoint)
{
    switch (indexmaptype)
    {
    case uniquefloat:
        if (uniqueFloatIndex->count(entry))
        {
            if (uniqueFloatIndex->at(entry).subtransactionid)   // already locked
            {
                if (uniqueFloatIndex->at(entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                floatLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueFloatIndex->at(entry).rowid = rowid;
            uniqueFloatIndex->at(entry).engineid = engineid;
            uniqueFloatIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unorderedfloat:
        if (unorderedFloatIndex->count(entry))
        {
            if (unorderedFloatIndex->at(entry).subtransactionid)   // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                floatLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedFloatIndex->at(entry).rowid = rowid;
            unorderedFloatIndex->at(entry).engineid = engineid;
            unorderedFloatIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }

    return NOLOCK;
}

//char
locktype_e Index::checkAndLock(char entry, int64_t rowid, int64_t engineid,
                               int64_t subtransactionid, int64_t pendingcmdid, int64_t tacmdentrypoint)
{
    switch (indexmaptype)
    {
    case uniquechar:
        if (uniqueCharIndex->count(entry))
        {
            if (uniqueCharIndex->at(entry).subtransactionid)   // already locked
            {
                if (uniqueCharIndex->at(entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                charLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueCharIndex->at(entry).rowid = rowid;
            uniqueCharIndex->at(entry).engineid = engineid;
            uniqueCharIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unorderedchar:
        if (unorderedCharIndex->count(entry))
        {
            if (unorderedCharIndex->at(entry).subtransactionid)   // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                charLockQueue->at(entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedCharIndex->at(entry).rowid = rowid;
            unorderedCharIndex->at(entry).engineid = engineid;
            unorderedCharIndex->at(entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }

    return NOLOCK;
}

//charx & varchar (string)
locktype_e Index::checkAndLock(string *entry, int64_t rowid, int64_t engineid,
                               int64_t subtransactionid, int64_t pendingcmdid, int64_t tacmdentrypoint)
{
    switch (indexmaptype)
    {
    case uniquecharx:
        if (uniqueStringIndex->count(*entry))
        {
            if (uniqueStringIndex->at(*entry).subtransactionid)   // already locked
            {
                if (uniqueStringIndex->at(*entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                stringLockQueue->at(*entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueStringIndex->at(*entry).rowid = rowid;
            uniqueStringIndex->at(*entry).engineid = engineid;
            uniqueStringIndex->at(*entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unorderedcharx:
        if (unorderedStringIndex->count(*entry))
        {
            if (unorderedStringIndex->at(*entry).subtransactionid) // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                stringLockQueue->at(*entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedStringIndex->at(*entry).rowid = rowid;
            unorderedStringIndex->at(*entry).engineid = engineid;
            unorderedStringIndex->at(*entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case uniquevarchar:
        if (uniqueStringIndex->count(*entry))
        {
            if (uniqueStringIndex->at(*entry).subtransactionid)   // already locked
            {
                if (uniqueStringIndex->at(*entry).subtransactionid==subtransactionid)
                {
                    return INDEXPENDINGLOCK;
                }

                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                stringLockQueue->at(*entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            uniqueStringIndex->at(*entry).rowid = rowid;
            uniqueStringIndex->at(*entry).engineid = engineid;
            uniqueStringIndex->at(*entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    case unorderedvarchar:
        if (unorderedStringIndex->count(*entry))
        {
            if (unorderedStringIndex->at(*entry).subtransactionid) // already locked
            {
                // do pending & return
                lockQueueIndexEntry queueEntry = {};
                queueEntry.pendingcmdid = pendingcmdid;
                queueEntry.tacmdentrypoint = tacmdentrypoint;
                queueEntry.entry.engineid = engineid;
                queueEntry.entry.rowid = rowid;
                queueEntry.entry.subtransactionid = subtransactionid;
                stringLockQueue->at(*entry).push(queueEntry);
                return INDEXPENDINGLOCK;
            }
            else
            {
                return NOLOCK; // unique constraint violation
            }
        }
        else
        {
            unorderedStringIndex->at(*entry).rowid = rowid;
            unorderedStringIndex->at(*entry).engineid = engineid;
            unorderedStringIndex->at(*entry).subtransactionid = subtransactionid;
            return INDEXLOCK;
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }

    return NOLOCK;
}

void Index::replace(int64_t entry, int64_t rowid, int64_t engineid)
{
    switch (indexmaptype)
    {
    case uniqueint:
        uniqueIntIndex->at(entry).rowid = rowid;
        uniqueIntIndex->at(entry).engineid = engineid;
        break;

    case nonuniqueint:
    {
        pair<multimap<int64_t, nonLockingIndexEntry_s>::iterator,
             multimap<int64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueIntMap::iterator it;

        iteratorRange = nonuniqueIntIndex->equal_range(entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unorderedint:
        unorderedIntIndex->at(entry).rowid = rowid;
        unorderedIntIndex->at(entry).engineid = engineid;
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replace(uint64_t entry, int64_t rowid, int64_t engineid)
{
    switch (indexmaptype)
    {
    case uniqueuint:
        uniqueUintIndex->at(entry).rowid = rowid;
        uniqueUintIndex->at(entry).engineid = engineid;
        break;

    case nonuniqueuint:
    {
        pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
             multimap<uint64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueUintMap::iterator it;

        iteratorRange = nonuniqueUintIndex->equal_range(entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unordereduint:
        unorderedUintIndex->at(entry).rowid = rowid;
        unorderedUintIndex->at(entry).engineid = engineid;
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replace(bool entry, int64_t rowid, int64_t engineid)
{
    switch (indexmaptype)
    {
    case uniquebool:
        uniqueBoolIndex->at(entry).rowid = rowid;
        uniqueBoolIndex->at(entry).engineid = engineid;
        break;

    case nonuniquebool:
    {
        pair<multimap<bool, nonLockingIndexEntry_s>::iterator,
             multimap<bool, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueBoolMap::iterator it;

        iteratorRange = nonuniqueBoolIndex->equal_range(entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unorderedbool:
        unorderedBoolIndex->at(entry).rowid = rowid;
        unorderedBoolIndex->at(entry).engineid = engineid;
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replace(long double entry, int64_t rowid, int64_t engineid)
{
    switch (indexmaptype)
    {
    case uniquefloat:
        uniqueFloatIndex->at(entry).rowid = rowid;
        uniqueFloatIndex->at(entry).engineid = engineid;
        break;

    case nonuniquefloat:
    {
        pair<multimap<long double, nonLockingIndexEntry_s>::iterator,
             multimap<long double, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueFloatMap::iterator it;

        iteratorRange = nonuniqueFloatIndex->equal_range(entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unorderedfloat:
        unorderedFloatIndex->at(entry).rowid = rowid;
        unorderedFloatIndex->at(entry).engineid = engineid;
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replace(char entry, int64_t rowid, int64_t engineid)
{
    switch (indexmaptype)
    {
    case uniquechar:
        uniqueCharIndex->at(entry).rowid = rowid;
        uniqueCharIndex->at(entry).engineid = engineid;
        break;

    case nonuniquechar:
    {
        pair<multimap<char, nonLockingIndexEntry_s>::iterator,
             multimap<char, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueCharMap::iterator it;

        iteratorRange = nonuniqueCharIndex->equal_range(entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unorderedchar:
        unorderedCharIndex->at(entry).rowid = rowid;
        unorderedCharIndex->at(entry).engineid = engineid;
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::replace(string *entry, int64_t rowid, int64_t engineid)
{
    trimspace(*entry);

    switch (indexmaptype)
    {
    case uniquecharx:
        uniqueStringIndex->at(*entry).rowid = rowid;
        uniqueStringIndex->at(*entry).engineid = engineid;
        break;

    case nonuniquecharx:
    {
        pair<multimap<string, nonLockingIndexEntry_s>::iterator,
             multimap<string, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueStringMap::iterator it;

        iteratorRange = nonuniqueStringIndex->equal_range(*entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unorderedcharx:
        unorderedStringIndex->at(*entry).rowid = rowid;
        unorderedStringIndex->at(*entry).engineid = engineid;
        break;

    case uniquevarchar:
        uniqueStringIndex->at(*entry).rowid = rowid;
        uniqueStringIndex->at(*entry).engineid = engineid;
        break;

    case nonuniquevarchar:
    {
        pair<multimap<string, nonLockingIndexEntry_s>::iterator,
             multimap<string, nonLockingIndexEntry_s>::iterator> iteratorRange;
        nonuniqueStringMap::iterator it;

        iteratorRange = nonuniqueStringIndex->equal_range(*entry);

        for (it=iteratorRange.first; it != iteratorRange.second; ++it)
        {
            if (it->second.rowid==rowid && it->second.engineid==engineid)
            {
                it->second.rowid = rowid;
                it->second.engineid = engineid;
            }
        }
    }
    break;

    case unorderedvarchar:
        unorderedStringIndex->at(*entry).rowid = rowid;
        unorderedStringIndex->at(*entry).engineid = engineid;
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotnulls(vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        uniqueIntMap::iterator it;

        for (it=uniqueIntIndex->begin(); it != uniqueIntIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniqueint:
    {
        nonuniqueIntMap::iterator it;

        for (it=nonuniqueIntIndex->begin();
             it != nonuniqueIntIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unorderedint:
    {
        unorderedIntMap::iterator it;

        for (it=unorderedIntIndex->begin(); it != unorderedIntIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniqueuint:
    {
        uniqueUintMap::iterator it;

        for (it=uniqueUintIndex->begin(); it != uniqueUintIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniqueuint:
    {
        nonuniqueUintMap::iterator it;

        for (it=nonuniqueUintIndex->begin();
             it != nonuniqueUintIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unordereduint:
    {
        unorderedUintMap::iterator it;

        for (it=unorderedUintIndex->begin();
             it != unorderedUintIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniquebool:
    {
        uniqueBoolMap::iterator it;

        for (it=uniqueBoolIndex->begin();
             it != uniqueBoolIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquebool:
    {
        nonuniqueBoolMap::iterator it;

        for (it=nonuniqueBoolIndex->begin();
             it != nonuniqueBoolIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unorderedbool:
    {
        unorderedBoolMap::iterator it;

        for (it=unorderedBoolIndex->begin();
             it != unorderedBoolIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniquefloat:
    {
        uniqueFloatMap::iterator it;

        for (it=uniqueFloatIndex->begin();
             it != uniqueFloatIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquefloat:
    {
        nonuniqueFloatMap::iterator it;

        for (it=nonuniqueFloatIndex->begin();
             it != nonuniqueFloatIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unorderedfloat:
    {
        unorderedFloatMap::iterator it;

        for (it=unorderedFloatIndex->begin(); it != unorderedFloatIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniquechar:
    {
        uniqueCharMap::iterator it;

        for (it=uniqueCharIndex->begin(); it != uniqueCharIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquechar:
    {
        nonuniqueCharMap::iterator it;

        for (it=nonuniqueCharIndex->begin(); it != nonuniqueCharIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unorderedchar:
    {
        unorderedCharMap::iterator it;

        for (it=unorderedCharIndex->begin(); it != unorderedCharIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniquecharx:
    {
        uniqueStringMap::iterator it;

        for (it=uniqueStringIndex->begin(); it != uniqueStringIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquecharx:
    {
        nonuniqueStringMap::iterator it;

        for (it=nonuniqueStringIndex->begin(); it != nonuniqueStringIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unorderedcharx:
    {
        unorderedStringMap::iterator it;

        for (it=unorderedStringIndex->begin(); it != unorderedStringIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniquevarchar:
    {
        uniqueStringMap::iterator it;

        for (it=uniqueStringIndex->begin(); it != uniqueStringIndex->end(); ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquevarchar:
    {
        nonuniqueStringMap::iterator it;

        for (it=nonuniqueStringIndex->begin(); it != nonuniqueStringIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case unorderedvarchar:
    {
        unorderedStringMap::iterator it;

        for (it=unorderedStringIndex->begin(); it != unorderedStringIndex->end();
             ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

// called by getequal() or getin()
void Index::getequal_f(int64_t input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        if (uniqueIntIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueIntIndex->at(input).rowid;
            entry.engineid = uniqueIntIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniqueint:
    {
        pair<multimap<int64_t, nonLockingIndexEntry_s>::iterator,
             multimap<int64_t, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueIntMap::iterator it;

        itRange = nonuniqueIntIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unorderedint:
    {
        if (unorderedIntIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedIntIndex->at(input).rowid;
            entry.engineid = unorderedIntIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::getequal_f(uint64_t input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        if (uniqueUintIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueUintIndex->at(input).rowid;
            entry.engineid = uniqueUintIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniqueuint:
    {
        pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
             multimap<uint64_t, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueUintMap::iterator it;

        itRange = nonuniqueUintIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unordereduint:
    {
        if (unorderedUintIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedUintIndex->at(input).rowid;
            entry.engineid = unorderedUintIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getequal_f(bool input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquebool:
    {
        if (uniqueBoolIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueBoolIndex->at(input).rowid;
            entry.engineid = uniqueBoolIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquebool:
    {
        pair<multimap<bool, nonLockingIndexEntry_s>::iterator,
             multimap<bool, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueBoolMap::iterator it;

        itRange = nonuniqueBoolIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unorderedbool:
    {
        if (unorderedBoolIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedBoolIndex->at(input).rowid;
            entry.engineid = unorderedBoolIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }

    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::getequal_f(long double input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        if (uniqueFloatIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueFloatIndex->at(input).rowid;
            entry.engineid = uniqueFloatIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquefloat:
    {
        pair<multimap<long double, nonLockingIndexEntry_s>::iterator,
             multimap<long double, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueFloatMap::iterator it;

        itRange = nonuniqueFloatIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unorderedfloat:
    {
        if (unorderedFloatIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedFloatIndex->at(input).rowid;
            entry.engineid = unorderedFloatIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }

    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getequal_f(char input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        if (uniqueCharIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueCharIndex->at(input).rowid;
            entry.engineid = uniqueCharIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquechar:
    {
        pair<multimap<char, nonLockingIndexEntry_s>::iterator,
             multimap<char, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueCharMap::iterator it;

        itRange = nonuniqueCharIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unorderedchar:
    {
        if (unorderedCharIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedCharIndex->at(input).rowid;
            entry.engineid = unorderedCharIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getequal_f(string input, vector<indexEntry_s> *returnEntries)
{
    trimspace(input);

    switch (indexmaptype)
    {
    case uniquecharx:
    {
        if (uniqueStringIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueStringIndex->at(input).rowid;
            entry.engineid = uniqueStringIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquecharx:
    {
        pair<multimap<string, nonLockingIndexEntry_s>::iterator,
             multimap<string, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueStringMap::iterator it;

        itRange = nonuniqueStringIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unorderedcharx:
    {
        if (unorderedStringIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedStringIndex->at(input).rowid;
            entry.engineid = unorderedStringIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case uniquevarchar:
    {
        if (uniqueStringIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = uniqueStringIndex->at(input).rowid;
            entry.engineid = uniqueStringIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniquevarchar:
    {
        pair<multimap<string, nonLockingIndexEntry_s>::iterator,
             multimap<string, nonLockingIndexEntry_s>::iterator> itRange;
        nonuniqueStringMap::iterator it;

        itRange = nonuniqueStringIndex->equal_range(input);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }
    }
    break;

    case unorderedvarchar:
    {
        if (unorderedStringIndex->count(input))
        {
            indexEntry_s entry;
            entry.rowid = unorderedStringIndex->at(input).rowid;
            entry.engineid = unorderedStringIndex->at(input).engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotequal(int64_t input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        uniqueIntMap::iterator it;

        for (it=uniqueIntIndex->begin(); it != uniqueIntIndex->end(); ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniqueint:
    {
        nonuniqueIntMap::iterator it;

        for (it=nonuniqueIntIndex->begin(); it != nonuniqueIntIndex->end(); ++it)
        {
            if (it->first != input)
            {
                returnEntries->push_back(it->second);
            }
        }
    }
    break;

    case unorderedint:
    {
        unorderedIntMap::iterator it;

        for (it=unorderedIntIndex->begin(); it != unorderedIntIndex->end(); ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotequal(uint64_t input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        uniqueUintMap::iterator it;

        for (it=uniqueUintIndex->begin(); it != uniqueUintIndex->end(); ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniqueuint:
    {
        nonuniqueUintMap::iterator it;

        for (it=nonuniqueUintIndex->begin(); it != nonuniqueUintIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                returnEntries->push_back(it->second);
            }
        }

    }
    break;

    case unordereduint:
    {
        unorderedUintMap::iterator it;

        for (it=unorderedUintIndex->begin(); it != unorderedUintIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotequal(bool input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquebool:
    {
        uniqueBoolMap::iterator it;

        for (it=uniqueBoolIndex->begin(); it != uniqueBoolIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniquebool:
    {
        nonuniqueBoolMap::iterator it;

        for (it=nonuniqueBoolIndex->begin(); it != nonuniqueBoolIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                returnEntries->push_back(it->second);
            }
        }

    }
    break;

    case unorderedbool:
    {
        unorderedBoolMap::iterator it;

        for (it=unorderedBoolIndex->begin(); it != unorderedBoolIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotequal(long double input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        uniqueFloatMap::iterator it;

        for (it=uniqueFloatIndex->begin(); it != uniqueFloatIndex->end(); ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniquefloat:
    {
        nonuniqueFloatMap::iterator it;

        for (it=nonuniqueFloatIndex->begin(); it != nonuniqueFloatIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                returnEntries->push_back(it->second);
            }
        }

    }
    break;

    case unorderedfloat:
    {
        unorderedFloatMap::iterator it;

        for (it=unorderedFloatIndex->begin(); it != unorderedFloatIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotequal(char input, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        uniqueCharMap::iterator it;

        for (it=uniqueCharIndex->begin(); it != uniqueCharIndex->end(); ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniquechar:
    {
        nonuniqueCharMap::iterator it;

        for (it=nonuniqueCharIndex->begin(); it != nonuniqueCharIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                returnEntries->push_back(it->second);
            }
        }

    }
    break;

    case unorderedchar:
    {
        unorderedCharMap::iterator it;

        for (it=unorderedCharIndex->begin(); it != unorderedCharIndex->end();
             ++it)
        {
            if (it->first != input)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotequal(string input, vector<indexEntry_s> *returnEntries)
{
    trimspace(input);

    switch (indexmaptype)
    {
    case uniquecharx:
    {
        uniqueStringMap::iterator it;

        for (it=uniqueStringIndex->begin(); it != uniqueStringIndex->end(); ++it)
        {
            if (it->first.compare(input))
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniquecharx:
    {
        nonuniqueStringMap::iterator it;

        for (it=nonuniqueStringIndex->begin(); it != nonuniqueStringIndex->end();
             ++it)
        {
            if (it->first.compare(input))
            {
                returnEntries->push_back(it->second);
            }
        }

    }
    break;

    case unorderedcharx:
    {
        unorderedStringMap::iterator it;

        for (it=unorderedStringIndex->begin(); it != unorderedStringIndex->end();
             ++it)
        {
            if (it->first.compare(input))
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case uniquevarchar:
    {
        uniqueStringMap::iterator it;

        for (it=uniqueStringIndex->begin(); it != uniqueStringIndex->end(); ++it)
        {
            if (it->first.compare(input))
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniquevarchar:
    {
        nonuniqueStringMap::iterator it;

        for (it=nonuniqueStringIndex->begin(); it != nonuniqueStringIndex->end();
             ++it)
        {
            if (it->first.compare(input))
            {
                returnEntries->push_back(it->second);
            }
        }

    }
    break;

    case unorderedvarchar:
    {
        unorderedStringMap::iterator it;

        for (it=unorderedStringIndex->begin(); it != unorderedStringIndex->end();
             ++it)
        {
            if (it->first.compare(input))
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

// lower_bound returns >=, upper_bound only returns <
void Index::comparison(int64_t input, operatortypes_e op,
                       vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        uniqueIntMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueIntIndex->begin();
            itEnd = uniqueIntIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueIntIndex->lower_bound(input);
            itEnd = uniqueIntIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case nonuniqueint:
    {
        nonuniqueIntMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueIntIndex->begin();
            itEnd = nonuniqueIntIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueIntIndex->lower_bound(input);
            itEnd = nonuniqueIntIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case unorderedint:
    {
        return; // unordered_map isn't sorted
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::comparison(uint64_t input, operatortypes_e op,
                       vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        uniqueUintMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueUintIndex->begin();
            itEnd = uniqueUintIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueUintIndex->lower_bound(input);
            itEnd = uniqueUintIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if (op==OPERATOR_GT)   // skip equal
            {
                if (it->first == input)
                {
                    continue;
                }

                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }

        if (op==OPERATOR_LTE)   // need equal, too
        {
            if (uniqueUintIndex->count(input))
            {
                indexEntry_s entry;
                entry.rowid = uniqueUintIndex->at(input).rowid;
                entry.engineid = uniqueUintIndex->at(input).engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniqueuint:
    {
        nonuniqueUintMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueUintIndex->begin();
            itEnd = nonuniqueUintIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueUintIndex->lower_bound(input);
            itEnd = nonuniqueUintIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if (op==OPERATOR_GT)   // skip equal
            {
                if (it->first == input)
                {
                    continue;
                }

                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }

        if (op==OPERATOR_LTE)   // need equal, too
        {
            pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
                 multimap<uint64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
            iteratorRange = nonuniqueUintIndex->equal_range(input);

            for (it=iteratorRange.first; it != iteratorRange.second; ++it)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case unordereduint:
    {
        return;
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::comparison(bool input, operatortypes_e op,
                       vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquebool:
    {
        uniqueBoolMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueBoolIndex->begin();
            itEnd = uniqueBoolIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueBoolIndex->lower_bound(input);
            itEnd = uniqueBoolIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if (op==OPERATOR_GT)   // skip equal
            {
                if (it->first == input)
                {
                    continue;
                }

                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }

        if (op==OPERATOR_LTE)   // need equal, too
        {
            if (uniqueBoolIndex->count(input))
            {
                indexEntry_s entry;
                entry.rowid = uniqueBoolIndex->at(input).rowid;
                entry.engineid = uniqueBoolIndex->at(input).engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case nonuniquebool:
    {
        nonuniqueBoolMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueBoolIndex->begin();
            itEnd = nonuniqueBoolIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueBoolIndex->lower_bound(input);
            itEnd = nonuniqueBoolIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if (op==OPERATOR_GT)   // skip equal
            {
                if (it->first == input)
                {
                    continue;
                }

                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }

        if (op==OPERATOR_LTE)   // need equal, too
        {
            pair<multimap<bool, nonLockingIndexEntry_s>::iterator,
                 multimap<bool, nonLockingIndexEntry_s>::iterator> iteratorRange;
            iteratorRange = nonuniqueBoolIndex->equal_range(input);

            for (it=iteratorRange.first; it != iteratorRange.second; ++it)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case unorderedbool:
    {
        return;
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::comparison(long double input, operatortypes_e op,
                       vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        uniqueFloatMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueFloatIndex->begin();
            itEnd = uniqueFloatIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueFloatIndex->lower_bound(input);
            itEnd = uniqueFloatIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case nonuniquefloat:
    {
        nonuniqueFloatMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueFloatIndex->begin();
            itEnd = nonuniqueFloatIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueFloatIndex->lower_bound(input);
            itEnd = nonuniqueFloatIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case unorderedfloat:
    {
        return;
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::comparison(char input, operatortypes_e op,
                       vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        uniqueCharMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueCharIndex->begin();
            itEnd = uniqueCharIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueCharIndex->lower_bound(input);
            itEnd = uniqueCharIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case nonuniquechar:
    {
        nonuniqueCharMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueCharIndex->begin();
            itEnd = nonuniqueCharIndex->upper_bound(input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueCharIndex->lower_bound(input);
            itEnd = nonuniqueCharIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case unorderedchar:
    {
        return;
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

// upper_bound & lower_bound seem to compile ok, so run with it I guess
void Index::comparison(string *input, operatortypes_e op,
                       vector<indexEntry_s> *returnEntries)
{
    trimspace(*input);

    switch (indexmaptype)
    {
    case uniquecharx:
    {
        uniqueStringMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueStringIndex->begin();
            itEnd = uniqueStringIndex->upper_bound(*input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueStringIndex->lower_bound(*input);
            itEnd = uniqueStringIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==*input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case nonuniquecharx:
    {
        nonuniqueStringMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueStringIndex->begin();
            itEnd = nonuniqueStringIndex->upper_bound(*input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueStringIndex->lower_bound(*input);
            itEnd = nonuniqueStringIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==*input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case unorderedcharx:
    {
        return;
    }
    break;

    case uniquevarchar:
    {
        uniqueStringMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = uniqueStringIndex->begin();
            itEnd = uniqueStringIndex->upper_bound(*input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = uniqueStringIndex->lower_bound(*input);
            itEnd = uniqueStringIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==*input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case nonuniquevarchar:
    {
        nonuniqueStringMap::iterator itBegin, itEnd, it;

        if (op==OPERATOR_LT || op==OPERATOR_LTE)
        {
            itBegin = nonuniqueStringIndex->begin();
            itEnd = nonuniqueStringIndex->upper_bound(*input);
        }
        else if (op==OPERATOR_GT || op==OPERATOR_GTE)
        {
            itBegin = nonuniqueStringIndex->lower_bound(*input);
            itEnd = nonuniqueStringIndex->end();
        }
        else
        {
            fprintf(logfile, "anomaly: %i %s %i\n", op, __FILE__, __LINE__);
        }

        // lower_bound: GTE
        // upper_bound: LT
        for (it=itBegin; it != itEnd; ++it)
        {
            if ((op==OPERATOR_GT || op==OPERATOR_LT) && it->first==*input)    // skip equal
            {
                continue;
            }

            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }
    }
    break;

    case unorderedvarchar:
    {
        return;
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

/* SQL92: "value1 BETWEEN value2 and value3 is equivalent" to
 * "value1 >= value2 AND value1 <= value3"
 */
void Index::between(int64_t lower, int64_t upper,
                    vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        uniqueIntMap &mapRef = *uniqueIntIndex;

        if (lower==upper)
        {
            if (mapRef.count(lower))
            {
                returnEntries->push_back({mapRef[lower].rowid, mapRef[lower].engineid});
            }

            return;
        }

        if (lower>upper)
        {
            return;
        }

        uniqueIntMap::const_iterator it;

        for (it = mapRef.lower_bound(lower); it != mapRef.lower_bound(upper);
             ++it)
        {
            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }

        if (mapRef.count(upper))
        {
            returnEntries->push_back({mapRef[upper].rowid, mapRef[upper].engineid});
        }
    }
    break;

    case nonuniqueint:
    {
        if (lower>upper)
        {
            return;
        }

        nonuniqueIntMap::iterator it;

        for (it = nonuniqueIntIndex->lower_bound(lower);
             it != nonuniqueIntIndex->upper_bound(upper); ++it)
        {
            if (it->first <= upper)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::between(uint64_t lower, uint64_t upper,
                    vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        uniqueUintMap::iterator itBegin, itEnd, it;
        itBegin = uniqueUintIndex->upper_bound(lower);
        itEnd = uniqueUintIndex->lower_bound(upper);

        if (uniqueUintIndex->count(lower))
        {
            indexEntry_s entry;
            entry.rowid = uniqueUintIndex->at(lower).rowid;
            entry.engineid = uniqueUintIndex->at(lower).engineid;
            returnEntries->push_back(entry);
        }

        for (it=itBegin; it != itEnd; ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    case nonuniqueuint:
    {
        nonuniqueUintMap::iterator itBegin, itEnd, it;
        itBegin = nonuniqueUintIndex->upper_bound(lower);
        itEnd = nonuniqueUintIndex->lower_bound(upper);

        pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
             multimap<uint64_t, nonLockingIndexEntry_s>::iterator> itRange;
        itRange = nonuniqueUintIndex->equal_range(lower);

        for (it=itRange.first; it != itRange.second; ++it)
        {
            returnEntries->push_back(it->second);
        }

        for (it=itBegin; it != itEnd; ++it)
        {
            indexEntry_s entry;
            entry.rowid = it->second.rowid;
            entry.engineid = it->second.engineid;
            returnEntries->push_back(entry);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::between(bool lower, bool upper, vector<indexEntry_s> *returnEntries)
{
    return;
}

// uniquefloat nonuniquefloat uniqueFloatMap uniqueFloatIndex
void Index::between(long double lower, long double upper,
                    vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        uniqueFloatMap &mapRef = *uniqueFloatIndex;

        if (lower==upper)
        {
            if (mapRef.count(lower))
            {
                returnEntries->push_back({mapRef[lower].rowid, mapRef[lower].engineid});
            }

            return;
        }

        if (lower>upper)
        {
            return;
        }

        uniqueFloatMap::const_iterator it;

        for (it = mapRef.lower_bound(lower); it != mapRef.lower_bound(upper);
             ++it)
        {
            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }

        if (mapRef.count(upper))
        {
            returnEntries->push_back({mapRef[upper].rowid, mapRef[upper].engineid});
        }
    }
    break;

    case nonuniquefloat:
    {
        if (lower>upper)
        {
            return;
        }

        nonuniqueFloatMap::iterator it;

        for (it = nonuniqueFloatIndex->lower_bound(lower);
             it != nonuniqueFloatIndex->upper_bound(upper); ++it)
        {
            if (it->first <= upper)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::between(char lower, char upper, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        uniqueCharMap &mapRef = *uniqueCharIndex;

        if (lower==upper)
        {
            if (mapRef.count(lower))
            {
                returnEntries->push_back({mapRef[lower].rowid, mapRef[lower].engineid});
            }

            return;
        }

        if (lower>upper)
        {
            return;
        }

        uniqueCharMap::const_iterator it;

        for (it = mapRef.lower_bound(lower);
             it != mapRef.lower_bound(upper); ++it)
        {
            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }

        if (mapRef.count(upper))
        {
            returnEntries->push_back({mapRef[upper].rowid, mapRef[upper].engineid});
        }
    }
    break;

    case nonuniquechar:
    {
        if (lower>upper)
        {
            return;
        }

        nonuniqueCharMap::iterator it;

        for (it = nonuniqueCharIndex->lower_bound(lower);
             it != nonuniqueCharIndex->upper_bound(upper); ++it)
        {
            if (it->first <= upper)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::between(string lower, string upper,
                    vector<indexEntry_s> *returnEntries)
{
    trimspace(lower);
    trimspace(upper);

    switch (indexmaptype)
    {
    case uniquecharx:
    {
        uniqueStringMap &mapRef = *uniqueStringIndex;

        if (lower==upper)
        {
            if (mapRef.count(lower))
            {
                returnEntries->push_back({mapRef[lower].rowid, mapRef[lower].engineid});
            }

            return;
        }

        if (lower>upper)
        {
            return;
        }

        uniqueStringMap::const_iterator it;

        for (it = mapRef.lower_bound(lower); it != mapRef.lower_bound(upper);
             ++it)
        {
            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }

        if (mapRef.count(upper))
        {
            returnEntries->push_back({mapRef[upper].rowid, mapRef[upper].engineid});
        }
    }
    break;

    case nonuniquecharx:
    {
        if (lower>upper)
        {
            return;
        }

        nonuniqueStringMap::iterator it;

        for (it = nonuniqueStringIndex->lower_bound(lower);
             it != nonuniqueStringIndex->upper_bound(upper); ++it)
        {
            if (it->first <= upper)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case uniquevarchar:
    {
        uniqueStringMap &mapRef = *uniqueStringIndex;

        if (lower==upper)
        {
            if (mapRef.count(lower))
            {
                returnEntries->push_back({mapRef[lower].rowid, mapRef[lower].engineid});
            }

            return;
        }

        if (lower>upper)
        {
            return;
        }

        uniqueStringMap::const_iterator it;

        for (it = mapRef.lower_bound(lower); it != mapRef.lower_bound(upper);
             ++it)
        {
            returnEntries->push_back({it->second.rowid, it->second.engineid});
        }

        if (mapRef.count(upper))
        {
            returnEntries->push_back({mapRef[upper].rowid, mapRef[upper].engineid});
        }
    }
    break;

    case nonuniquevarchar:
    {
        if (lower>upper)
        {
            return;
        }

        nonuniqueStringMap::iterator it;

        for (it = nonuniqueStringIndex->lower_bound(lower);
             it != nonuniqueStringIndex->upper_bound(upper); ++it)
        {
            if (it->first <= upper)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::regex(string *regexStr, vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquecharx:
    {
        uniqueStringMap::iterator itBegin, itEnd, it;
        getIterators(regexStr, uniqueStringIndex, &itBegin, &itEnd);
        pcrecpp::RE re(*regexStr);

        for (it=itBegin; it != itEnd; ++it)
        {
            // regex search o yeah!
            if (re.FullMatch(it->first)==true)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case unorderedcharx:
    {
        unorderedStringMap::iterator it;
        pcrecpp::RE re(*regexStr);

        for (it = unorderedStringIndex->begin();
             it != unorderedStringIndex->end(); ++it)
        {
            // regex search o yeah!
            if (re.FullMatch(it->first)==true)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquecharx:
    {
        nonuniqueStringMap::iterator itBegin, itEnd, it;
        getIterators(regexStr, nonuniqueStringIndex, &itBegin, &itEnd);
        pcrecpp::RE re(*regexStr);

        for (it=itBegin; it != itEnd; ++it)
        {
            // regex search o yeah!
            if (re.FullMatch(it->first)==true)
            {
                returnEntries->push_back(it->second);
            }
        }
    }
    break;

    case uniquevarchar:
    {
        uniqueStringMap::iterator itBegin, itEnd, it;
        getIterators(regexStr, uniqueStringIndex, &itBegin, &itEnd);
        pcrecpp::RE re(*regexStr);

        for (it=itBegin; it != itEnd; ++it)
        {
            // regex search o yeah!
            if (re.FullMatch(it->first)==true)
            {
                indexEntry_s entry;
                entry.rowid = it->second.rowid;
                entry.engineid = it->second.engineid;
                returnEntries->push_back(entry);
            }
        }
    }
    break;

    case unorderedvarchar:
    {
        unorderedStringMap::iterator it;
        pcrecpp::RE re(*regexStr);

        for (it = unorderedStringIndex->begin();
             it != unorderedStringIndex->end(); ++it)
        {
            // regex search o yeah!
            if (re.FullMatch(it->first)==true)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquevarchar:
    {
        nonuniqueStringMap::iterator itBegin, itEnd, it;
        getIterators(regexStr, nonuniqueStringIndex, &itBegin, &itEnd);
        pcrecpp::RE re(*regexStr);

        for (it=itBegin; it != itEnd; ++it)
        {
            // regex search o yeah!
            if (re.FullMatch(it->first)==true)
            {
                returnEntries->push_back(it->second);
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
        return;
    }
}

void Index::like(string &likeStr, vector<indexEntry_s> *returnEntries)
{
    trimspace(likeStr);
    like2Regex(likeStr);
    regex(&likeStr, returnEntries);
}

void Index::notlike(string &likeStr, vector<indexEntry_s> *returnEntries)
{
    trimspace(likeStr);
    like2Regex(likeStr);
    likeStr.insert(0, "^((?!");
    likeStr.append(").)*$");
    regex(&likeStr, returnEntries);
}

void Index::getnotin(vector<int64_t> &entries,
                     vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        uniqueIntMap::iterator it;

        for (it = uniqueIntIndex->begin(); it != uniqueIntIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unorderedint:
    {
        unorderedIntMap::iterator it;

        for (it = unorderedIntIndex->begin(); it != unorderedIntIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniqueint:
    {
        nonuniqueIntMap::iterator it;

        for (it = nonuniqueIntIndex->begin(); it != nonuniqueIntIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotin(vector<uint64_t> &entries,
                     vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        uniqueUintMap::iterator it;

        for (it = uniqueUintIndex->begin(); it != uniqueUintIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unordereduint:
    {
        unorderedUintMap::iterator it;

        for (it = unorderedUintIndex->begin();
             it != unorderedUintIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniqueuint:
    {
        nonuniqueUintMap::iterator it;

        for (it = nonuniqueUintIndex->begin(); it != nonuniqueUintIndex->end();
             ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotin(vector<bool> &entries,
                     vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquebool:
    {
        uniqueBoolMap::iterator it;

        for (it = uniqueBoolIndex->begin(); it != uniqueBoolIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unorderedbool:
    {
        unorderedBoolMap::iterator it;

        for (it = unorderedBoolIndex->begin();
             it != unorderedBoolIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquebool:
    {
        nonuniqueBoolMap::iterator it;

        for (it = nonuniqueBoolIndex->begin();
             it != nonuniqueBoolIndex->end();++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotin(vector<long double> &entries,
                     vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        uniqueFloatMap::iterator it;

        for (it = uniqueFloatIndex->begin(); it != uniqueFloatIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unorderedfloat:
    {
        unorderedFloatMap::iterator it;

        for (it = unorderedFloatIndex->begin();
             it != unorderedFloatIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquefloat:
    {
        nonuniqueFloatMap::iterator it;

        for (it = nonuniqueFloatIndex->begin();
             it != nonuniqueFloatIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotin(vector<char> &entries,
                     vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        uniqueCharMap::iterator it;

        for (it = uniqueCharIndex->begin(); it != uniqueCharIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unorderedchar:
    {
        unorderedCharMap::iterator it;

        for (it = unorderedCharIndex->begin();
             it != unorderedCharIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquechar:
    {
        nonuniqueCharMap::iterator it;

        for (it = nonuniqueCharIndex->begin();
             it != nonuniqueCharIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (entries[n] == it->first)
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnotin(vector<string> &entries,
                     vector<indexEntry_s> *returnEntries)
{
    switch (indexmaptype)
    {
    case uniquecharx:
    {
        uniqueStringMap::iterator it;

        for (it = uniqueStringIndex->begin();
             it != uniqueStringIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                trimspace(entries[n]);

                if (!entries[n].compare(it->first))
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unorderedcharx:
    {
        unorderedStringMap::iterator it;

        for (it = unorderedStringIndex->begin();
             it != unorderedStringIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (!entries[n].compare(it->first))
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquecharx:
    {
        nonuniqueStringMap::iterator it;

        for (it = nonuniqueStringIndex->begin();
             it != nonuniqueStringIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                trimspace(entries[n]);

                if (!entries[n].compare(it->first))
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case uniquevarchar:
    {
        uniqueStringMap::iterator it;

        for (it = uniqueStringIndex->begin();
             it != uniqueStringIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                trimspace(entries[n]);

                if (!entries[n].compare(it->first))
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case unorderedvarchar:
    {
        unorderedStringMap::iterator it;

        for (it = unorderedStringIndex->begin();
             it != unorderedStringIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (!entries[n].compare(it->first))
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    case nonuniquevarchar:
    {
        nonuniqueStringMap::iterator it;

        for (it = nonuniqueStringIndex->begin();
             it != nonuniqueStringIndex->end(); ++it)
        {
            bool isfound=false;

            for (size_t n=0; n < entries.size(); n++)
            {
                if (!entries[n].compare(it->first))
                {
                    isfound=true;
                    break;
                }
            }

            if (isfound==false)
            {
                returnEntries->push_back({it->second.rowid, it->second.engineid});
            }
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::commitRollback(int64_t input, int64_t subtransactionid,
                           enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
        switch (indexmaptype)
        {
        case uniqueint:
            if (!uniqueIntIndex->count(input))
            {
                return;
            }

            if (uniqueIntIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueIntIndex->at(input).subtransactionid = 0;
            break;

        case unorderedint:
            if (!unorderedIntIndex->count(input))
            {
                return;
            }

            if (unorderedIntIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            unorderedIntIndex->at(input).subtransactionid = 0;
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    case ROLLBACKCMD:
        switch (indexmaptype)
        {
        case uniqueint:
            if (!uniqueIntIndex->count(input))
            {
                return;
            }

            if (uniqueIntIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueIntIndex->erase(input);
            break;

        case unorderedint:
            if (!unorderedIntIndex->count(input))
            {
                return;
            }

            if (unorderedIntIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            unorderedIntIndex->erase(input);
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}

void Index::commitRollback(uint64_t input, int64_t subtransactionid,
                           enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
        switch (indexmaptype)
        {
        case uniqueuint:
            if (!uniqueUintIndex->count(input))
            {
                return;
            }

            if (uniqueUintIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueUintIndex->at(input).subtransactionid = 0;
            break;

        case unordereduint:
            if (!unorderedUintIndex->count(input))
            {
                return;
            }

            if (unorderedUintIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedUintIndex->at(input).subtransactionid = 0;
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    case ROLLBACKCMD:
        switch (indexmaptype)
        {
        case uniqueuint:
            if (!uniqueUintIndex->count(input))
            {
                return;
            }

            if (uniqueUintIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueUintIndex->erase(input);
            break;

        case unordereduint:
            if (!unorderedUintIndex->count(input))
            {
                return;
            }

            if (unorderedUintIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedUintIndex->erase(input);
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}

void Index::commitRollback(bool input, int64_t subtransactionid,
                           enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
        switch (indexmaptype)
        {
        case uniquebool:
            if (!uniqueBoolIndex->count(input))
            {
                return;
            }

            if (uniqueBoolIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueBoolIndex->at(input).subtransactionid = 0;
            break;

        case unorderedbool:
            if (!unorderedBoolIndex->count(input))
            {
                return;
            }

            if (unorderedBoolIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedBoolIndex->at(input).subtransactionid = 0;
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    case ROLLBACKCMD:
        switch (indexmaptype)
        {
        case uniquebool:
            if (!uniqueBoolIndex->count(input))
            {
                return;
            }

            if (uniqueBoolIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueBoolIndex->erase(input);
            break;

        case unorderedbool:
            if (!unorderedBoolIndex->count(input))
            {
                return;
            }

            if (unorderedBoolIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedBoolIndex->erase(input);
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}

void Index::commitRollback(long double input, int64_t subtransactionid,
                           enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
        switch (indexmaptype)
        {
        case uniquefloat:
            if (!uniqueFloatIndex->count(input))
            {
                return;
            }

            if (uniqueFloatIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueFloatIndex->at(input).subtransactionid = 0;
            break;

        case unorderedfloat:
            if (!unorderedFloatIndex->count(input))
            {
                return;
            }

            if (unorderedFloatIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedFloatIndex->at(input).subtransactionid = 0;
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    case ROLLBACKCMD:
        switch (indexmaptype)
        {
        case uniquefloat:
            if (!uniqueFloatIndex->count(input))
            {
                return;
            }

            if (uniqueFloatIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueFloatIndex->erase(input);
            break;

        case unorderedfloat:
            if (!unorderedFloatIndex->count(input))
            {
                return;
            }

            if (unorderedFloatIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedFloatIndex->erase(input);
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}

void Index::commitRollback(char input, int64_t subtransactionid,
                           enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
        switch (indexmaptype)
        {
        case uniquechar:
            if (!uniqueCharIndex->count(input))
            {
                return;
            }

            if (uniqueCharIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueCharIndex->at(input).subtransactionid = 0;
            break;

        case unorderedchar:
            if (!unorderedCharIndex->count(input))
            {
                return;
            }

            if (unorderedCharIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedCharIndex->at(input).subtransactionid = 0;
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    case ROLLBACKCMD:
        switch (indexmaptype)
        {
        case uniquechar:
            if (!uniqueCharIndex->count(input))
            {
                return;
            }

            if (uniqueCharIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueCharIndex->erase(input);
            break;

        case unorderedchar:
            if (!unorderedCharIndex->count(input))
            {
                return;
            }

            if (unorderedCharIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedCharIndex->erase(input);
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}
void Index::commitRollback(string input, int64_t subtransactionid,
                           enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
        switch (indexmaptype)
        {
        case uniquecharx:
            if (!uniqueStringIndex->count(input))
            {
                return;
            }

            if (uniqueStringIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueStringIndex->at(input).subtransactionid = 0;
            break;

        case unorderedcharx:
            if (!unorderedStringIndex->count(input))
            {
                return;
            }

            if (unorderedStringIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedStringIndex->at(input).subtransactionid = 0;
            break;

        case uniquevarchar:
            if (!uniqueStringIndex->count(input))
            {
                return;
            }

            if (uniqueStringIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueStringIndex->at(input).subtransactionid = 0;
            break;

        case unorderedvarchar:
            if (!unorderedStringIndex->count(input))
            {
                return;
            }

            if (unorderedStringIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedStringIndex->at(input).subtransactionid = 0;
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

        break;

    case ROLLBACKCMD:
        switch (indexmaptype)
        {
        case uniquecharx:
            if (!uniqueStringIndex->count(input))
            {
                return;
            }

            if (uniqueStringIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueStringIndex->erase(input);
            break;

        case unorderedcharx:
            if (!unorderedStringIndex->count(input))
            {
                return;
            }

            if (unorderedStringIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedStringIndex->erase(input);

        case uniquevarchar:
            if (!uniqueStringIndex->count(input))
            {
                return;
            }

            if (uniqueStringIndex->at(input).subtransactionid != subtransactionid)
            {
                return;
            }

            uniqueStringIndex->erase(input);
            break;

        case unorderedvarchar:
            if (!unorderedStringIndex->count(input))
            {
                return;
            }

            if (unorderedStringIndex->at(input).subtransactionid !=
                subtransactionid)
            {
                return;
            }

            unorderedStringIndex->erase(input);
            break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexmaptype, __FILE__,
                    __LINE__);
        }

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}

void Index::replaceUnique(int64_t newrowid, int64_t newengineid, int64_t input)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueIntIndex->operator [](input) = entry;
    }
    break;

    case unorderedint:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedIntIndex->operator [](input) = entry;
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replaceUnique(int64_t newrowid, int64_t newengineid, uint64_t input)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueUintIndex->operator [](input) = entry;
    }
    break;

    case unordereduint:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedUintIndex->operator [](input) = entry;
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replaceUnique(int64_t newrowid, int64_t newengineid, bool input)
{
    switch (indexmaptype)
    {
    case uniquebool:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueBoolIndex->operator [](input) = entry;
    }
    break;

    case unorderedbool:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedBoolIndex->operator [](input) = entry;
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replaceUnique(int64_t newrowid, int64_t newengineid,
                          long double input)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueFloatIndex->operator [](input) = entry;
    }
    break;

    case unorderedfloat:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedFloatIndex->operator [](input) = entry;
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replaceUnique(int64_t newrowid, int64_t newengineid, char input)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueCharIndex->operator [](input) = entry;
    }
    break;

    case unorderedchar:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedCharIndex->operator [](input) = entry;
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}
void Index::replaceUnique(int64_t newrowid, int64_t newengineid, string &input)
{
    trimspace(input);

    switch (indexmaptype)
    {
    case uniquecharx:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueStringIndex->operator [](input) = entry;
    }
    break;

    case unorderedcharx:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedStringIndex->operator [](input) = entry;
    }
    break;

    case uniquevarchar:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        uniqueStringIndex->operator [](input) = entry;
    }
    break;

    case unorderedvarchar:
    {
        lockingIndexEntry entry;
        entry.subtransactionid = 0;
        entry.rowid = newrowid;
        entry.engineid = newengineid;

        unorderedStringIndex->operator [](input) = entry;
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                             int64_t newrowid, int64_t newengineid, int64_t input)
{
    pair<multimap<int64_t, nonLockingIndexEntry_s>::iterator,
         multimap<int64_t, nonLockingIndexEntry_s>::iterator> itRange;
    nonuniqueIntMap::iterator it;
    itRange = nonuniqueIntIndex->equal_range(input);

    for (it = itRange.first; it != itRange.second; ++it)
    {
        if (it->second.rowid==oldrowid && it->second.engineid==oldengineid)
        {
            it->second.rowid=newrowid;
            it->second.engineid=newengineid;
        }
    }
}

void Index::replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                             int64_t newrowid, int64_t newengineid, uint64_t input)
{
    pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
         multimap<uint64_t, nonLockingIndexEntry_s>::iterator> itRange;
    nonuniqueUintMap::iterator it;
    itRange = nonuniqueUintIndex->equal_range(input);

    for (it = itRange.first; it != itRange.second; ++it)
    {
        if (it->second.rowid==oldrowid && it->second.engineid==oldengineid)
        {
            it->second.rowid=newrowid;
            it->second.engineid=newengineid;
        }
    }
}

void Index::replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                             int64_t newrowid, int64_t newengineid, bool input)
{
    pair<multimap<bool, nonLockingIndexEntry_s>::iterator,
         multimap<bool, nonLockingIndexEntry_s>::iterator> itRange;
    nonuniqueBoolMap::iterator it;
    itRange = nonuniqueBoolIndex->equal_range(input);

    for (it = itRange.first; it != itRange.second; ++it)
    {
        if (it->second.rowid==oldrowid && it->second.engineid==oldengineid)
        {
            it->second.rowid=newrowid;
            it->second.engineid=newengineid;
        }
    }
}

void Index::replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                             int64_t newrowid, int64_t newengineid, long double input)
{
    pair<multimap<long double, nonLockingIndexEntry_s>::iterator,
         multimap<long double, nonLockingIndexEntry_s>::iterator> itRange;
    nonuniqueFloatMap::iterator it;
    itRange = nonuniqueFloatIndex->equal_range(input);

    for (it = itRange.first; it != itRange.second; ++it)
    {
        if (it->second.rowid==oldrowid && it->second.engineid==oldengineid)
        {
            it->second.rowid=newrowid;
            it->second.engineid=newengineid;
        }
    }
}

void Index::replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                             int64_t newrowid, int64_t newengineid, char input)
{
    pair<multimap<char, nonLockingIndexEntry_s>::iterator,
         multimap<char, nonLockingIndexEntry_s>::iterator> itRange;
    nonuniqueCharMap::iterator it;
    itRange = nonuniqueCharIndex->equal_range(input);

    for (it = itRange.first; it != itRange.second; ++it)
    {
        if (it->second.rowid==oldrowid && it->second.engineid==oldengineid)
        {
            it->second.rowid=newrowid;
            it->second.engineid=newengineid;
        }
    }
}

void Index::replaceNonunique(int64_t oldrowid, int64_t oldengineid,
                             int64_t newrowid, int64_t newengineid, string &input)
{
    trimspace(input);

    pair<multimap<string, nonLockingIndexEntry_s>::iterator,
         multimap<string, nonLockingIndexEntry_s>::iterator> itRange;
    nonuniqueStringMap::iterator it;
    itRange = nonuniqueStringIndex->equal_range(input);

    for (it = itRange.first; it != itRange.second; ++it)
    {
        if (it->second.rowid==oldrowid && it->second.engineid==oldengineid)
        {
            it->second.rowid=newrowid;
            it->second.engineid=newengineid;
        }
    }
}

void Index::replaceNull(int64_t oldrowid, int64_t oldengineid,
                        int64_t newrowid, int64_t newengineid)
{
    vector<int64_t> v(2);
    v[0] = oldrowid;
    v[1] = oldengineid;
    nulls.erase(v);
    v[0] = newrowid;
    v[1] = newengineid;
    nulls.insert(v);
}

void Index::insertNonuniqueEntry(int64_t entry, int64_t rowid, int64_t engineid)
{
    nonLockingIndexEntry_s val = {};
    val.rowid = rowid;
    val.engineid = engineid;
    nonuniqueIntIndex->insert(pair<int64_t, nonLockingIndexEntry_s>(entry, val));
}

void Index::insertNonuniqueEntry(uint64_t entry, int64_t rowid,
                                 int64_t engineid)
{
    nonLockingIndexEntry_s val = {};
    val.rowid = rowid;
    val.engineid = engineid;
    nonuniqueUintIndex->insert(pair<uint64_t,
                               nonLockingIndexEntry_s>(entry, val));
}

void Index::insertNonuniqueEntry(bool entry, int64_t rowid, int64_t engineid)
{
    nonLockingIndexEntry_s val = {};
    val.rowid = rowid;
    val.engineid = engineid;
    nonuniqueBoolIndex->insert(pair<bool, nonLockingIndexEntry_s>(entry, val));
}

void Index::insertNonuniqueEntry(long double entry, int64_t rowid,
                                 int64_t engineid)
{
    nonLockingIndexEntry_s val = {};
    val.rowid = rowid;
    val.engineid = engineid;
    nonuniqueFloatIndex->insert(pair<long double,
                                nonLockingIndexEntry_s>(entry, val));
}

void Index::insertNonuniqueEntry(char entry, int64_t rowid, int64_t engineid)
{
    nonLockingIndexEntry_s val = {};
    val.rowid = rowid;
    val.engineid = engineid;
    nonuniqueCharIndex->insert(pair<char, nonLockingIndexEntry_s>(entry, val));
}

void Index::insertNonuniqueEntry(string *entry, int64_t rowid, int64_t engineid)
{
    trimspace(*entry);
    nonLockingIndexEntry_s val = {};
    val.rowid = rowid;
    val.engineid = engineid;
    nonuniqueStringIndex->insert(pair<string,
                                 nonLockingIndexEntry_s>(*entry, val));
}

void Index::deleteNonuniqueEntry(int64_t entry, int64_t rowid, int64_t engineid)
{
    pair<multimap<int64_t, nonLockingIndexEntry_s>::iterator,
         multimap<int64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
    nonuniqueIntMap::iterator it;

    iteratorRange = nonuniqueIntIndex->equal_range(entry);

    for (it=iteratorRange.first; it != iteratorRange.second; )
    {
        if (it->second.rowid==rowid && it->second.engineid==engineid)
        {
            nonuniqueIntIndex->erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Index::deleteNonuniqueEntry(uint64_t entry, int64_t rowid,
                                 int64_t engineid)
{
    pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
         multimap<uint64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
    nonuniqueUintMap::iterator it;

    iteratorRange = nonuniqueUintIndex->equal_range(entry);

    for (it=iteratorRange.first; it != iteratorRange.second; )
    {
        if (it->second.rowid==rowid && it->second.engineid==engineid)
        {
            nonuniqueUintIndex->erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Index::deleteNonuniqueEntry(bool entry, int64_t rowid, int64_t engineid)
{
    pair<multimap<bool, nonLockingIndexEntry_s>::iterator,
         multimap<bool, nonLockingIndexEntry_s>::iterator> iteratorRange;
    nonuniqueBoolMap::iterator it;

    iteratorRange = nonuniqueBoolIndex->equal_range(entry);

    for (it=iteratorRange.first; it != iteratorRange.second; )
    {
        if (it->second.rowid==rowid && it->second.engineid==engineid)
        {
            nonuniqueBoolIndex->erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Index::deleteNonuniqueEntry(long double entry, int64_t rowid, int64_t engineid)
{
    pair<multimap<long double, nonLockingIndexEntry_s>::iterator,
         multimap<long double, nonLockingIndexEntry_s>::iterator> iteratorRange;
    nonuniqueFloatMap::iterator it;

    iteratorRange = nonuniqueFloatIndex->equal_range(entry);

    for (it=iteratorRange.first; it != iteratorRange.second; )
    {
        if (it->second.rowid==rowid && it->second.engineid==engineid)
        {
            nonuniqueFloatIndex->erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Index::deleteNonuniqueEntry(char entry, int64_t rowid, int64_t engineid)
{
    pair<multimap<char, nonLockingIndexEntry_s>::iterator,
         multimap<char, nonLockingIndexEntry_s>::iterator> iteratorRange;
    nonuniqueCharMap::iterator it;

    iteratorRange = nonuniqueCharIndex->equal_range(entry);

    for (it=iteratorRange.first; it != iteratorRange.second; )
    {
        if (it->second.rowid==rowid && it->second.engineid==engineid)
        {
            nonuniqueCharIndex->erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Index::deleteNonuniqueEntry(string *entry, int64_t rowid, int64_t engineid)
{
    trimspace(*entry);

    pair<multimap<string, nonLockingIndexEntry_s>::iterator,
         multimap<string, nonLockingIndexEntry_s>::iterator> iteratorRange;
    nonuniqueStringMap::iterator it;

    iteratorRange = nonuniqueStringIndex->equal_range(*entry);

    for (it=iteratorRange.first; it != iteratorRange.second; )
    {
        if (it->second.rowid==rowid && it->second.engineid==engineid)
        {
            nonuniqueStringIndex->erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void Index::insertNullEntry(int64_t rowid, int64_t engineid)
{
    vector<int64_t> v(2);
    v[0] = rowid;
    v[1] = engineid;
    nulls.insert(v);
}

void Index::deleteNullEntry(int64_t rowid, int64_t engineid)
{
    vector<int64_t> v(2);
    v[0] = rowid;
    v[1] = engineid;
    nulls.erase(v);
}

void Index::deleteUniqueEntry(int64_t entry)
{
    switch (indexmaptype)
    {
    case uniqueint: //uniqueIntIndex
        uniqueIntIndex->erase(entry);
        break;

    case unorderedint:
        unorderedIntIndex->erase(entry);
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::deleteUniqueEntry(uint64_t entry)
{
    switch (indexmaptype)
    {
    case uniqueuint: //uniqueIntIndex
        uniqueUintIndex->erase(entry);
        break;

    case unordereduint:
        unorderedUintIndex->erase(entry);
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::deleteUniqueEntry(bool entry)
{
    switch (indexmaptype)
    {
    case uniquebool: //uniqueIntIndex
        uniqueBoolIndex->erase(entry);
        break;

    case unorderedbool:
        unorderedBoolIndex->erase(entry);
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::deleteUniqueEntry(long double entry)
{
    switch (indexmaptype)
    {
    case uniquefloat: //uniqueIntIndex
        uniqueFloatIndex->erase(entry);
        break;

    case unorderedfloat:
        unorderedFloatIndex->erase(entry);
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::deleteUniqueEntry(char entry)
{
    switch (indexmaptype)
    {
    case uniquechar: //uniqueIntIndex
        uniqueCharIndex->erase(entry);
        break;

    case unorderedchar:
        unorderedCharIndex->erase(entry);
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::deleteUniqueEntry(string *entry)
{
    trimspace(*entry);

    switch (indexmaptype)
    {
    case uniquecharx: //uniqueIntIndex
        uniqueStringIndex->erase(*entry);
        break;

    case unorderedcharx:
        unorderedStringIndex->erase(*entry);
        break;

    case uniquevarchar: //uniqueIntIndex
        uniqueStringIndex->erase(*entry);
        break;

    case unorderedvarchar:
        unorderedStringIndex->erase(*entry);
        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexmaptype, __FILE__, __LINE__);
    }
}

void Index::getnulls(vector<indexEntry_s> *returnEntries)
{
    if (notNull==true)   // very easy
    {
        return;
    }

    boost::unordered_set< vector<int64_t> >::iterator nullsIterator;
    indexEntry_s entry;

    for (nullsIterator = nulls.begin(); nullsIterator != nulls.end();
         ++nullsIterator)
    {
        entry.rowid = nullsIterator->at(0);
        entry.engineid = nullsIterator->at(1);
        returnEntries->push_back(entry);
    }
}

void Index::getall(vector<indexEntry_s> *returnEntries)
{
    getnotnulls(returnEntries);
    getnulls(returnEntries);
}

// apply unique index not null
bool Index::addifnotthere(fieldValue_s &val, int64_t rowid, int16_t engineid,
                          int64_t subtransactionid)
{
    switch (fieldtype)
    {
    case INT:
        if (checkifthere(val.value.integer)==true)
        {
            return false;
        }

        break;

    case UINT:
        if (checkifthere(val.value.uinteger)==true)
        {
            return false;
        }

        break;

    case BOOL:
        if (checkifthere(val.value.boolean)==true)
        {
            return false;
        }

        break;

    case FLOAT:
        if (checkifthere(val.value.floating)==true)
        {
            return false;
        }

        break;

    case CHAR:
        if (checkifthere(val.value.character)==true)
        {
            return false;
        }

        break;

    case CHARX:
        if (checkifthere(val.str)==true)
        {
            return false;
        }

        break;

    case VARCHAR:
        if (checkifthere(val.str)==true)
        {
            return false;
        }

        break;

    default:
        printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__, fieldtype);
    }

    lockingIndexEntry entry = {rowid, engineid, 0, subtransactionid};

    switch (indexmaptype)
    {
    case uniqueint:
        uniqueIntIndex->operator [](val.value.integer) = entry;
        break;

    case unorderedint:
        unorderedIntIndex->operator [](val.value.integer) = entry;
        break;

    case uniqueuint:
        uniqueUintIndex->operator [](val.value.uinteger) = entry;
        break;

    case unordereduint:
        unorderedUintIndex->operator [](val.value.uinteger) = entry;
        break;

    case uniquebool:
        uniqueBoolIndex->operator [](val.value.boolean) = entry;
        break;

    case unorderedbool:
        unorderedBoolIndex->operator [](val.value.boolean) = entry;
        break;

    case uniquefloat:
        uniqueFloatIndex->operator [](val.value.floating) = entry;
        break;

    case unorderedfloat:
        unorderedFloatIndex->operator [](val.value.floating) = entry;
        break;

    case uniquechar:
        uniqueCharIndex->operator [](val.value.character) = entry;
        break;

    case unorderedchar:
        unorderedCharIndex->operator [](val.value.character) = entry;
        break;

    case uniquecharx:
        uniqueStringIndex->operator [](val.str) = entry;
        break;

    case unorderedcharx:
        unorderedStringIndex->operator [](val.str) = entry;
        break;

    case uniquevarchar:
        uniqueStringIndex->operator [](val.str) = entry;
        break;

    case unorderedvarchar:
        unorderedStringIndex->operator [](val.str) = entry;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifthere(fieldValue_s &val)
{
    switch (fieldtype)
    {
    case INT:
        return checkifthere(val.value.integer);
        break;

    case UINT:
        return checkifthere(val.value.uinteger);
        break;

    case BOOL:
        return checkifthere(val.value.boolean);
        break;

    case FLOAT:
        return checkifthere(val.value.floating);
        break;

    case CHAR:
        return checkifthere(val.value.character);
        break;

    case CHARX:
        return checkifthere(val.str);
        break;

    case VARCHAR:
        return checkifthere(val.str);
        break;

    default:
        printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__,
               fieldtype);
    }

    return false;
}

bool Index::checkifthere(int64_t val)
{
    switch (indexmaptype)
    {
    case uniqueint:
    {
        if (!uniqueIntIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unorderedint:
    {
        if (!unorderedIntIndex->count(val))
        {
            return false;
        }
    }
    break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifthere(uint64_t val)
{
    switch (indexmaptype)
    {
    case uniqueuint:
    {
        if (!uniqueUintIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unordereduint:
    {
        if (!unorderedUintIndex->count(val))
        {
            return false;
        }
    }
    break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifthere(bool val)
{
    switch (indexmaptype)
    {
    case uniquebool:
    {
        if (!uniqueBoolIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unorderedbool:
    {
        if (!unorderedBoolIndex->count(val))
        {
            return false;
        }
    }
    break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifthere(long double val)
{
    switch (indexmaptype)
    {
    case uniquefloat:
    {
        if (!uniqueFloatIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unorderedfloat:
    {
        if (!unorderedFloatIndex->count(val))
        {
            return false;
        }
    }
    break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifthere(char val)
{
    switch (indexmaptype)
    {
    case uniquechar:
    {
        if (!uniqueCharIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unorderedchar:
    {
        if (!unorderedCharIndex->count(val))
        {
            return false;
        }
    }
    break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifthere(string &val)
{
    switch (indexmaptype)
    {
    case uniquecharx:
    {
        if (!uniqueStringIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unorderedcharx:
    {
        if (!unorderedStringIndex->count(val))
        {
            return false;
        }
    }
    break;

    case uniquevarchar:
    {
        if (!uniqueStringIndex->count(val))
        {
            return false;
        }
    }
    break;

    case unorderedvarchar:
    {
        if (!unorderedStringIndex->count(val))
        {
            return false;
        }
    }
    break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return true;
}

bool Index::checkifmatch(fieldValue_s &val, int64_t rowid, int64_t engineid)
{
    switch (fieldtype)
    {
    case INT:
        return checkifmatch(val.value.integer, rowid, engineid);
        break;

    case UINT:
        return checkifmatch(val.value.uinteger, rowid, engineid);
        break;

    case BOOL:
        return checkifmatch(val.value.boolean, rowid, engineid);
        break;

    case FLOAT:
        return checkifmatch(val.value.floating, rowid, engineid);
        break;

    case CHAR:
        return checkifmatch(val.value.character, rowid, engineid);
        break;

    case CHARX:
        return checkifmatch(val.str, rowid, engineid);
        break;

    case VARCHAR:
        return checkifmatch(val.str, rowid, engineid);
        break;

    default:
        printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__,
               fieldtype);
    }

    return false;
}

bool Index::checkifmatch(int64_t val, int64_t rowid, int64_t engineid)
{
    lockingIndexEntry entry;

    switch (indexmaptype)
    {
    case uniqueint:
        entry = uniqueIntIndex->at(val);
        break;

    case unorderedint:
        entry = unorderedIntIndex->at(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
        entry = {};
    }

    if (entry.rowid != rowid || entry.engineid != engineid)
    {
        return false;
    }

    return true;
}

bool Index::checkifmatch(uint64_t val, int64_t rowid, int64_t engineid)
{
    lockingIndexEntry entry;

    switch (indexmaptype)
    {
    case uniqueuint:
        entry = uniqueUintIndex->at(val);
        break;

    case unordereduint:
        entry = unorderedUintIndex->at(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
        entry = {};
    }

    if (entry.rowid != rowid || entry.engineid != engineid)
    {
        return false;
    }

    return true;
}

bool Index::checkifmatch(bool val, int64_t rowid, int64_t engineid)
{
    lockingIndexEntry entry;

    switch (indexmaptype)
    {
    case uniquebool:
        entry = uniqueBoolIndex->at(val);
        break;

    case unorderedbool:
        entry = unorderedBoolIndex->at(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
        entry = {};
    }

    if (entry.rowid != rowid || entry.engineid != engineid)
    {
        return false;
    }

    return true;
}

bool Index::checkifmatch(long double val, int64_t rowid, int64_t engineid)
{
    lockingIndexEntry entry;

    switch (indexmaptype)
    {
    case uniquefloat:
        entry = uniqueFloatIndex->at(val);
        break;

    case unorderedfloat:
        entry = unorderedFloatIndex->at(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
        entry = {};
    }

    if (entry.rowid != rowid || entry.engineid != engineid)
    {
        return false;
    }

    return true;
}

bool Index::checkifmatch(char val, int64_t rowid, int64_t engineid)
{
    lockingIndexEntry entry;

    switch (indexmaptype)
    {
    case uniquechar:
        entry = uniqueCharIndex->at(val);
        break;

    case unorderedchar:
        entry = unorderedCharIndex->at(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
        entry = {};
    }

    if (entry.rowid != rowid || entry.engineid != engineid)
    {
        return false;
    }

    return true;
}

bool Index::checkifmatch(string &val, int64_t rowid, int64_t engineid)
{
    lockingIndexEntry entry;

    switch (indexmaptype)
    {
    case uniquecharx:
        entry = uniqueStringIndex->at(val);
        break;

    case unorderedcharx:
        entry = unorderedStringIndex->at(val);
        break;

    case uniquevarchar:
        entry = uniqueStringIndex->at(val);
        break;

    case unorderedvarchar:
        entry = unorderedStringIndex->at(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
        entry = {};
    }

    if (entry.rowid != rowid || entry.engineid != engineid)
    {
        return false;
    }

    return true;
}

void Index::rm(fieldValue_s &val)
{
    switch (fieldtype)
    {
    case INT:
        rm(val.value.integer);
        break;

    case UINT:
        rm(val.value.uinteger);
        break;

    case BOOL:
        rm(val.value.boolean);
        break;

    case FLOAT:
        rm(val.value.floating);
        break;

    case CHAR:
        rm(val.value.character);
        break;

    case CHARX:
        rm(val.str);
        break;

    case VARCHAR:
        rm(val.str);
        break;

    default:
        printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__,
               fieldtype);
    }
}

void Index::rm(int64_t val)
{
    switch (indexmaptype)
    {
    case uniqueint:
        uniqueIntIndex->erase(val);
        break;

    case unorderedint:
        unorderedIntIndex->erase(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }
}

void Index::rm(uint64_t val)
{
    switch (indexmaptype)
    {
    case uniqueuint:
        uniqueUintIndex->erase(val);
        break;

    case unordereduint:
        unorderedUintIndex->erase(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }
}

void Index::rm(bool val)
{
    switch (indexmaptype)
    {
    case uniquebool:
        uniqueBoolIndex->erase(val);
        break;

    case unorderedbool:
        unorderedBoolIndex->erase(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }
}

void Index::rm(long double val)
{
    switch (indexmaptype)
    {
    case uniquefloat:
        uniqueFloatIndex->erase(val);
        break;

    case unorderedfloat:
        unorderedFloatIndex->erase(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }
}

void Index::rm(char val)
{
    switch (indexmaptype)
    {
    case uniquechar:
        uniqueCharIndex->erase(val);
        break;

    case unorderedchar:
        unorderedCharIndex->erase(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }
}

void Index::rm(string &val)
{
    switch (indexmaptype)
    {
    case uniquecharx:
        uniqueStringIndex->erase(val);
        break;

    case unorderedcharx:
        unorderedStringIndex->erase(val);
        break;

    case uniquevarchar:
        uniqueStringIndex->erase(val);
        break;

    case unorderedvarchar:
        unorderedStringIndex->erase(val);
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }
}

int64_t Index::getprevioussubtransactionid(fieldValue_s &val)
{
    switch (fieldtype)
    {
    case INT:
        return getprevioussubtransactionid(val.value.integer);
        break;

    case UINT:
        return getprevioussubtransactionid(val.value.uinteger);
        break;

    case BOOL:
        return getprevioussubtransactionid(val.value.boolean);
        break;

    case FLOAT:
        return getprevioussubtransactionid(val.value.floating);
        break;

    case CHAR:
        return getprevioussubtransactionid(val.value.character);
        break;

    case CHARX:
        return getprevioussubtransactionid(val.str);
        break;

    case VARCHAR:
        return getprevioussubtransactionid(val.str);
        break;

    default:
        printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__,
               fieldtype);
    }

    return -1;
}


int64_t Index::getprevioussubtransactionid(int64_t val)
{
    switch (indexmaptype)
    {
    case uniqueint:
        return uniqueIntIndex->at(val).previoussubtransactionid;
        break;

    case unorderedint:
        return unorderedIntIndex->at(val).previoussubtransactionid;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return -1;
}

int64_t Index::getprevioussubtransactionid(uint64_t val)
{
    switch (indexmaptype)
    {
    case uniqueuint:
        return uniqueUintIndex->at(val).previoussubtransactionid;
        break;

    case unordereduint:
        return unorderedUintIndex->at(val).previoussubtransactionid;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return -1;
}

int64_t Index::getprevioussubtransactionid(bool val)
{
    switch (indexmaptype)
    {
    case uniquebool:
        return uniqueBoolIndex->at(val).previoussubtransactionid;
        break;

    case unorderedbool:
        return unorderedBoolIndex->at(val).previoussubtransactionid;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return -1;
}

int64_t Index::getprevioussubtransactionid(long double val)
{
    switch (indexmaptype)
    {
    case uniquefloat:
        return uniqueFloatIndex->at(val).previoussubtransactionid;
        break;

    case unorderedfloat:
        return unorderedFloatIndex->at(val).previoussubtransactionid;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return -1;
}

int64_t Index::getprevioussubtransactionid(char val)
{
    switch (indexmaptype)
    {
    case uniquechar:
        return uniqueCharIndex->at(val).previoussubtransactionid;
        break;

    case unorderedchar:
        return unorderedCharIndex->at(val).previoussubtransactionid;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return -1;
}

int64_t Index::getprevioussubtransactionid(string &val)
{
    switch (indexmaptype)
    {
    case uniquecharx:
        return uniqueStringIndex->at(val).previoussubtransactionid;
        break;

    case unorderedcharx:
        return unorderedStringIndex->at(val).previoussubtransactionid;
        break;

    case uniquevarchar:
        return uniqueStringIndex->at(val).previoussubtransactionid;
        break;

    case unorderedvarchar:
        return unorderedStringIndex->at(val).previoussubtransactionid;
        break;

    default:
        printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
               indexmaptype);
    }

    return -1;
}


// end apply
