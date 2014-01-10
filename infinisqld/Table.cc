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
 * @file   Table.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:52:19 2013
 * 
 * @brief  Table class. Has Fields and Indices.
 */

#include "Table.h"
#line 30 "Table.cc"

Table::Table(int64_t idarg) : id(idarg)
{
    if (id)   // if a real table, create a shadow table with id 0
    {
        shadowTable = new class Table(0);
    }

    name = "";
    rowsize = 0;
}

void Table::setname(string namearg)
{
    name = namearg;
}

string *Table::getname()
{
    return &name;
}

int64_t Table::addfield(fieldtype_e type, int64_t length, string name,
                        indextype_e indextype)
{
    class Field newfield(type, length, indextype, name);

    fields.push_back(newfield);
    columnaNameToFieldMap[name] = fields.size() - 1;

    if (rowsize != -1)
    {
        rowsize += sizeof(bool);

        switch (type)
        {
        case INT:
            rowsize += sizeof(int64_t);
            break;

        case UINT:
            rowsize += sizeof(uint64_t);
            break;

        case BOOL:
            rowsize += sizeof(bool);
            break;

        case FLOAT:
            rowsize += sizeof(long double);

        case CHAR:
            rowsize += sizeof(char);
            break;

        case CHARX:
            rowsize += length;
            break;

        case VARCHAR:
            rowsize = -1;
            break;

        default:
            fprintf(logfile, "anomaly: %i %s %i\n", type, __FILE__, __LINE__);
        }
    }

    return fields.size() - 1;
}

bool Table::makerow(vector<fieldValue_s> *fieldVal, string *res)
{
    vector<fieldValue_s> &fieldValRef = *fieldVal;

    if (fieldValRef.size() != fields.size())
    {
        fprintf(logfile, "%s %i anomaly fieldValRef.size() %lu fields.size() %lu\n", __FILE__, __LINE__, (unsigned long)fieldValRef.size(),
                (unsigned long)fields.size());
        return false;
    }

    if (rowsize != -1)
    {
        res->assign(rowsize, 0);
    }
    else
    {
        int64_t rsize = 0;
        bool hasvarchar=false;

        for (size_t n=0; n<fieldValRef.size(); n++)
        {
            rsize += sizeof(bool);
            rowsize += sizeof(bool);

            switch (fields[n].type)
            {
            case INT:
                rsize += sizeof(int64_t);
                break;

            case UINT:
                rsize += sizeof(uint64_t);
                break;

            case BOOL:
                rsize += sizeof(bool);
                break;

            case FLOAT:
                rsize += sizeof(long double);
                break;

            case CHAR:
                rsize += sizeof(char);
                break;

            case CHARX:
                rsize += fields[n].length;
                break;

            case VARCHAR:
                rsize += sizeof(int64_t);
                rsize += fieldValRef[n].str.length();
                hasvarchar=true;
                break;

            default:
                fprintf(logfile, "anomaly: %i %s %i\n", fields[n].type, __FILE__,
                        __LINE__);
            }

        }

        if (hasvarchar==true)
        {
            rowsize=-1;
        }

        res->assign(rsize, 0);
    }

    int64_t pos = 0;

    for (size_t n=0; n<fields.size(); n++)
    {
        if (fieldValRef[n].isnull==false)
        {
            res->operator [](pos++)=0;
        }
        else
        {
            res->operator[](pos++)=1;
            continue;
        }

        switch (fields[n].type)
        {
        case INT:
            memcpy(&res->operator [](pos), &fieldValRef[n].value.integer,
                   sizeof(fieldValRef[n].value.integer));
            pos += sizeof(fieldValRef[n].value.integer);
            break;

        case UINT:
            memcpy(&res->operator [](pos), &fieldValRef[n].value.uinteger,
                   sizeof(fieldValRef[n].value.uinteger));
            pos += sizeof(fieldValRef[n].value.uinteger);
            break;

        case BOOL:
            memcpy(&res->operator [](pos), &fieldValRef[n].value.boolean,
                   sizeof(fieldValRef[n].value.boolean));
            pos += sizeof(fieldValRef[n].value.boolean);
            break;

        case FLOAT:
            memcpy(&res->operator [](pos), &fieldValRef[n].value.floating,
                   sizeof(fieldValRef[n].value.floating));
            pos += sizeof(fieldValRef[n].value.floating);
            break;

        case CHAR:
            memcpy(&res->operator [](pos), &fieldValRef[n].value.character,
                   sizeof(fieldValRef[n].value.character));
            pos += sizeof(fieldValRef[n].value.character);
            break;

        case CHARX:

            // pad if necessary
            if (fieldValRef[n].str.length() < (size_t)fields[n].length)
            {
                fieldValRef[n].str.append(fields[n].length-
                                          fieldValRef[n].str.length(), ' ');
            }

            fieldValRef[n].str.copy(&res->operator [](pos), fields[n].length, 0);
            pos += fields[n].length;
            break;

        case VARCHAR:
        {
            int64_t varcharlength = fieldValRef[n].str.length();
            memcpy(&res->operator [](pos),
                   &varcharlength, sizeof(varcharlength));
            pos += sizeof(varcharlength);
            memcpy(&res->operator [](pos), fieldValRef[n].str.c_str(),
                   varcharlength);
            pos += varcharlength;
        }
        break;

        default:
            fprintf(logfile, "anomaly: %i %s %i\n", fields[n].type, __FILE__,
                    __LINE__);
        }
    }

    return true;
}

void Table::getrows(vector<int64_t> rowids, locktype_e locktype,
                    int64_t subtransactionid, int64_t pendingcmdid,
                    vector<returnRow_s> *returnRows,
                    vector<int64_t> *lockPendingRowids,
                    int64_t tacmdentrypoint)
{
    if (locktype != NOLOCK)
    {
        lockPendingRowids->clear();
    }

    rowdata_s *currentRowPtr;

    for (size_t n=0; n < rowids.size(); n++)
    {
        int64_t rid = rowids[n];

        if (rows.count(rid))
        {
            currentRowPtr = rows[rid];

            switch (locktype)
            {
            case NOLOCK:
            {
                if (getinsertflag(currentRowPtr->flags)==false)
                {
                    returnRow_s r = {};
                    r.rowid = rid;
                    r.row = currentRowPtr->row;
                    returnRows->push_back(r);
                }
            }
            break;

            case READLOCK:
            {
                switch (getlocktype(currentRowPtr->flags))
                {
                case NOLOCK: // lock it and add create readlockHolders
                {
                    returnRow_s r = {};
                    r.rowid = rid;
                    r.row = currentRowPtr->row;
                    returnRows->push_back(r);
                    setreadlock(&currentRowPtr->flags);
                    currentRowPtr->readlockHolders =
                        new boost::unordered_set<int64_t>;
                    printf("%s %i ROWID %li READLOCKHOLDERS->insert %li\n",
                           __FILE__, __LINE__, rid, subtransactionid);
                    currentRowPtr->readlockHolders->insert(subtransactionid);
                }
                break;

                case READLOCK:
                {
                    printf("%s %i ROWID %li READLOCKHOLDERS->insert %li\n",
                           __FILE__, __LINE__, rid, subtransactionid);
                    currentRowPtr->readlockHolders->insert(subtransactionid);
                    returnRow_s r = {};
                    r.rowid = rid;
                    r.row = currentRowPtr->row;
                    returnRows->push_back(r);
                }
                break;

                case WRITELOCK:
                {
                    // make it pending
                    lockQueueRowEntry qEntry = {};
                    // populate qEntry, d'accord
                    qEntry.pendingcmdid = pendingcmdid;
                    qEntry.tacmdentrypoint = tacmdentrypoint;
                    qEntry.subtransactionid = subtransactionid;
                    qEntry.locktype = READLOCK;
                    lockQueue[rid].push(qEntry);
                    lockPendingRowids->push_back(rid);
                }
                break;

                default:
                    fprintf(logfile, "anomaly: %i %s %i\n",
                            getlocktype(currentRowPtr->flags), __FILE__,
                            __LINE__);
                }
            }
            break;

            case WRITELOCK:
            {
                switch (getlocktype(currentRowPtr->flags))
                {
                case NOLOCK:
                {
                    // lock it & return
                    setwritelock(&currentRowPtr->flags);
                    currentRowPtr->writelockHolder = subtransactionid;
                    returnRow_s r = {};
                    r.rowid = rid;
                    r.row = currentRowPtr->row;
                    returnRows->push_back(r);
                }
                break;

                case READLOCK:
                {
                    // lock pending
                    lockQueueRowEntry qEntry = {};
                    // populate qEntry, d'accord
                    qEntry.pendingcmdid = pendingcmdid;
                    qEntry.tacmdentrypoint = tacmdentrypoint;
                    qEntry.subtransactionid = subtransactionid;
                    qEntry.locktype = WRITELOCK;
                    lockQueue[rid].push(qEntry);
                    lockPendingRowids->push_back(rid);
                }
                break;

                case WRITELOCK:
                {
                    // lock pending
                    lockQueueRowEntry qEntry = {};
                    // populate qEntry, d'accord
                    qEntry.pendingcmdid = pendingcmdid;
                    qEntry.tacmdentrypoint = tacmdentrypoint;
                    qEntry.subtransactionid = subtransactionid;
                    qEntry.locktype = WRITELOCK;
                    lockQueue[rid].push(qEntry);
                    lockPendingRowids->push_back(rid);
                }
                break;

                default:
                    fprintf(logfile, "anomaly: %i %s %i\n",
                            getlocktype(currentRowPtr->flags), __FILE__,
                            __LINE__);
                }
            }
            break;

            default:
                fprintf(logfile, "anomaly: %i %s %i\n", locktype, __FILE__,
                        __LINE__);
            }
        }
    }
}

int64_t Table::updaterow(int64_t rowid, int64_t subtransactionid, string *row)
{
    if (!rows.count(rowid))
    {
        return STATUS_NOTOK;
    }

    rowdata_s &currentRowRef = *rows[rowid];

    if ((getlocktype(currentRowRef.flags) != WRITELOCK) ||
        (currentRowRef.writelockHolder != subtransactionid))
    {
        fprintf(logfile, "anomaly %i %li %s %i\n", currentRowRef.flags,
                currentRowRef.writelockHolder, __FILE__, __LINE__);
        return STATUS_NOTOK;
    }

    // should probably validate the row is not garbage, but o well    
    rowdata_s *nrow=new rowdata_s();
    nrow->row=*row;
    shadowTable->rows[rowid]=nrow;

    return STATUS_OK;
}

// this means there's a replacement row, so index hits that refer here
// need to be forwarded on
int64_t Table::deleterow(int64_t rowid, int64_t subtransactionid,
                         int64_t forward_rowid, int64_t forward_engineid)
{
    int64_t status = deleterow(rowid, subtransactionid);

    if (status)
    {
        return status;
    }

    rowdata_s &currentRowRef = *rows[rowid];

    setreplacedeleteflag(&currentRowRef.flags);
    forwarderEntry forwarder;
    forwarder.rowid = forward_rowid;
    forwarder.engineid = forward_engineid;
    forwarderMap[rowid] = forwarder;
    return STATUS_OK;
}

void Table::selectrows(vector<int64_t> *rowids, locktype_e locktype,
                       int64_t subtransactionid, int64_t pendingcmdid,
                       vector<returnRow_s> *returnRows, int64_t tacmdentrypoint)
{
    vector<returnRow_s> &returnRowsRef = *returnRows;
    size_t numrowids = rowids->size();
    returnRowsRef.reserve(numrowids);
    returnRow_s workrow;
    int64_t rowid;

    switch (locktype)
    {
    case NOLOCK:
    {
        for (size_t n=0; n<numrowids; n++)
        {
            rowid = rowids->at(n);
            workrow.rowid = rowid;

            if (!rows.count(rowid))
            {
                workrow.row.clear();
                workrow.locktype = NOTFOUNDLOCK;
            }
            else
            {
                switch (getlocktype(rows[rowid]->flags))
                {
                case NOLOCK:
                    break;

                case READLOCK:
                    if (rows[rowid]->readlockHolders->count(subtransactionid))
                    {
                        continue;
                    }

                    break;

                case WRITELOCK:
                    if (rows[rowid]->writelockHolder==subtransactionid)
                    {
                        continue;
                    }

                    break;

                default:
                    printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                           getlocktype(rows[rowid]->flags));
                    continue;
                }

                workrow.row = rows[rowid]->row;
                workrow.locktype = NOLOCK;
            }

            returnRowsRef.push_back(workrow);
        }
    }
    break;

    case READLOCK:
    {
        for (size_t n=0; n<numrowids; n++)
        {
            rowid = rowids->at(n);
            workrow.rowid = rowid;

            if (!rows.count(rowid))
            {
                workrow.row.clear();
                workrow.locktype = NOTFOUNDLOCK;
            }
            else
            {
                switch (getlocktype(rows[rowid]->flags))
                {
                case NOLOCK: // lock it & return row
                    setreadlock(&rows[rowid]->flags);
                    rows[rowid]->readlockHolders =
                        new boost::unordered_set<int64_t>;
                    rows[rowid]->readlockHolders->insert(subtransactionid);
                    workrow.row = rows[rowid]->row;
                    workrow.locktype = READLOCK;
                    break;

                case READLOCK:
                    if (rows[rowid]->readlockHolders->count(subtransactionid))
                    {
                        continue;
                    }
                    else
                    {
                        rows[rowid]->readlockHolders->insert(subtransactionid);
                        workrow.row = rows[rowid]->row;
                        workrow.locktype = READLOCK;
                    }

                    break;

                case WRITELOCK: // pending
                {
                    if (rows[rowid]->writelockHolder==subtransactionid)
                    {
                        continue;
                    }

                    if (assignToLockQueue(rowid, READLOCK, subtransactionid,
                                          pendingcmdid,
                                          tacmdentrypoint)==NOTFOUNDLOCK)
                    {
                        continue;
                    }

                    workrow.row.clear();
                    workrow.locktype = PENDINGLOCK;
                }
                break;

                default:
                    fprintf(logfile, "anomaly: %i %s %i\n",
                            getlocktype(rows[rowid]->flags), __FILE__, __LINE__);
                }
            }

            returnRowsRef.push_back(workrow);
        }
    }
    break;

    case WRITELOCK:
    {
        for (size_t n=0; n<numrowids; n++)
        {
            rowid = rowids->at(n);
            workrow.rowid = rowid;

            if (!rows.count(rowid))
            {
                workrow.row.clear();
                workrow.locktype = NOTFOUNDLOCK;
            }
            else
            {
                switch (getlocktype(rows[rowid]->flags))
                {
                case NOLOCK: // lock it & return row
                    setwritelock(&rows[rowid]->flags);
                    rows[rowid]->writelockHolder = subtransactionid;
                    workrow.row = rows[rowid]->row;
                    workrow.locktype = WRITELOCK;
                    break;

                case READLOCK: // pending
                    if (assignToLockQueue(rowid, WRITELOCK, subtransactionid,
                                          pendingcmdid,
                                          tacmdentrypoint)==NOTFOUNDLOCK)
                    {
                        continue;
                    }

                    workrow.row.clear();
                    workrow.locktype = PENDINGLOCK;
                    break;

                case WRITELOCK: // pending
                    if (subtransactionid == rows[rowid]->writelockHolder)
                    {
                        workrow.row = rows[rowid]->row;
                        workrow.locktype = WRITELOCK;
                    }
                    else
                    {
                        if (assignToLockQueue(rowid, WRITELOCK, subtransactionid,
                                              pendingcmdid,
                                              tacmdentrypoint)==NOTFOUNDLOCK)
                        {
                            continue;
                        }

                        workrow.row.clear();
                        workrow.locktype = PENDINGLOCK;
                    }

                    break;

                default:
                    fprintf(logfile, "anomaly: %i %s %i\n",
                            getlocktype(rows[rowid]->flags), __FILE__, __LINE__);
                }
            }

            returnRowsRef.push_back(workrow);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", locktype, __FILE__, __LINE__);
    }
}

void Table::commitRollbackUnlock(int64_t rowid, int64_t subtransactionid,
                                 enginecmd_e cmd)
{
    switch (cmd)
    {
    case COMMITCMD:
    {
        rowdata_s &currentRowRef = *rows[rowid];

        if (getlocktype(currentRowRef.flags)==READLOCK) // just unlock
        {
            if (!currentRowRef.readlockHolders->count(subtransactionid))
            {
                printf("%s %i anomaly subtransactionid %li\n", __FILE__,
                       __LINE__, subtransactionid);
                return; // bogus request
            }

            currentRowRef.readlockHolders->erase(subtransactionid);

            if (currentRowRef.readlockHolders->empty()==true)
            {
                clearlockedflag(&currentRowRef.flags);
                delete currentRowRef.readlockHolders;
                currentRowRef.readlockHolders = NULL;
            }

            return;
        }

        if ((getlocktype(currentRowRef.flags) != WRITELOCK) ||
            (currentRowRef.writelockHolder != subtransactionid))
        {
            printf("%s %i anomaly flags %i subtransactionid %li writelockHolder %li rowid %li cmd %i flags %i\n", __FILE__, __LINE__, (int)currentRowRef.flags,
                   subtransactionid, currentRowRef.writelockHolder, rowid, cmd,
                   (int)currentRowRef.flags);
            return; // bogus request
        }

        if (getdeleteflag(currentRowRef.flags)==true)
        {
            if (getreplacedeleteflag(currentRowRef.flags)==true)
            {
                forwarderMap.erase(rowid);
            }

            delete &currentRowRef;
            rows.erase(rowid);

            if (shadowTable->rows.count(rowid))
            {
                delete shadowTable->rows[rowid];
                shadowTable->rows.erase(rowid);
            }

            return;
        }

        if (currentRowRef.readlockHolders != NULL)
        {
            printf("%s %i anomaly: readlockHolders\n", __FILE__, __LINE__);
            boost::unordered_set<int64_t>::const_iterator it;

            for (it = currentRowRef.readlockHolders->begin();
                 it != currentRowRef.readlockHolders->end(); ++it)
            {
                printf("%s %i readlockHolder %li\n", __FILE__, __LINE__, *it);
            }

            delete currentRowRef.readlockHolders;
            currentRowRef.readlockHolders=NULL;
        }

        // only change the row if changes have been made, this covers in case a
        // locked row didn't get modified, but still part of a commit
        if (shadowTable->rows.count(rowid))
        {
            rowdata_s &shadowRowRef = *shadowTable->rows[rowid];
            shadowRowRef.previoussubtransactionid=currentRowRef.writelockHolder;
            shadowRowRef.flags=0;
            shadowRowRef.writelockHolder=0;
            shadowRowRef.readlockHolders=NULL;

            if (rows[rowid]==shadowTable->rows[rowid]) // insert
            {
                shadowTable->rows.erase(rowid);
            }
            else // update
            {
                delete &currentRowRef;
                rows[rowid]=&shadowRowRef;
                shadowTable->rows.erase(rowid);
            }
        }
        else
        {
            printf("%s %i anomaly\n", __FILE__, __LINE__);
            currentRowRef.flags=0;
            currentRowRef.writelockHolder=0;
        }

        /*
          if (shadowTable->rows.count(rowid))
          {
          if (currentRowRef.previoussubtransactionid==0)
          {
          currentRowRef.previoussubtransactionid = currentRowRef.writelockHolder;
          currentRowRef.row = shadowTable->rows[rowid].row;
          }
          else
          { // if update row uses string::copy() then the container's memory will be consumed
          currentRowRef.previoussubtransactionid = currentRowRef.writelockHolder;
          memcpy(&currentRowRef.row[0], &shadowTable->rows[rowid].row[0],
          currentRowRef.row.size());
          }
          shadowTable->rows.erase(rowid);
          }
          currentRowRef.flags = 0;
          currentRowRef.writelockHolder = 0;
        */
    }
    break;

    case ROLLBACKCMD:
    {
        if (!rows.count(rowid))
        {
            return;
        }

        rowdata_s &currentRowRef = *rows[rowid];

        if ((currentRowRef.writelockHolder != subtransactionid) &&
            (getlocktype(currentRowRef.flags) == WRITELOCK))
        {
            fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
            return; // bogus request
        }

        if (getinsertflag(currentRowRef.flags)==true)
        {
            delete &currentRowRef;
            rows.erase(rowid);
            shadowTable->rows.erase(rowid);
            // drain Q
            return;
        }

        if (getdeleteflag(currentRowRef.flags)==true)
        {
            if (getreplacedeleteflag(currentRowRef.flags)==true)
            {
                clearreplacedeleteflag(&currentRowRef.flags);
                forwarderMap.erase(rowid);
            }

            cleardeleteflag(&currentRowRef.flags);

            if (shadowTable->rows.count(rowid))
            {
                printf("%s %i anomaly rowid %li\n", __FILE__, __LINE__, rowid);
                delete shadowTable->rows[rowid];
                shadowTable->rows.erase(rowid);
            }
        }

        currentRowRef.writelockHolder = 0;

        if (getlocktype(currentRowRef.flags)==READLOCK)
        {
            currentRowRef.readlockHolders->erase(subtransactionid);

            if (currentRowRef.readlockHolders->empty()==true)
            {
                delete currentRowRef.readlockHolders;
                currentRowRef.readlockHolders = NULL;
            }
        }

        currentRowRef.flags = 0;

        if (currentRowRef.readlockHolders != NULL)
        {
            setreadlock(&currentRowRef.flags);
        }

        if (shadowTable->rows.count(rowid))
        {
            delete shadowTable->rows[rowid];
            shadowTable->rows.erase(rowid);
        }

        // process Q
    }
    break;

    case UNLOCKCMD:
    {
        rowdata_s &currentRowRef = *rows[rowid];

        if (getlocktype(currentRowRef.flags) != READLOCK)
        {
            fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
            return; // bogus request
        }

        if (!currentRowRef.readlockHolders->count(subtransactionid))
        {
            fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
            return; // bogus request
        }

        currentRowRef.readlockHolders->erase(subtransactionid);

        if (currentRowRef.readlockHolders->empty()==true)
        {
            clearlockedflag(&currentRowRef.flags);
            delete currentRowRef.readlockHolders;
            currentRowRef.readlockHolders = NULL;
        }
    }
    break;

    // REVERTCMD likely broken somehow
    case REVERTCMD:
    {
        // do what rollback does, but don't unlock
        if (!rows.count(rowid))
        {
            return;
        }

        rowdata_s &currentRowRef = *rows[rowid];

        if ((currentRowRef.writelockHolder != subtransactionid) ||
            (getlocktype(currentRowRef.flags) != WRITELOCK))
        {
            fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
            return; // bogus request
        }

        if (getinsertflag(currentRowRef.flags)==true)
        {
            rows.erase(rowid);
        }
        else
        {
            currentRowRef.flags = 0;
        }

        forwarderMap.erase(rowid);
        shadowTable->rows.erase(rowid);
    }
    break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", cmd, __FILE__, __LINE__);
    }
}

bool Table::unmakerow(string *rowstring, vector<fieldValue_s> *resultFields)
{
    vector<fieldValue_s> &resultFieldsRef = *resultFields;
    string &rowstringRef = *rowstring;

    size_t pos = 0;
    size_t  numfields = fields.size();
    resultFieldsRef.resize(numfields, fieldValue_s());
    size_t rowsize = rowstringRef.size();

    for (size_t n=0; n < numfields; n++)
    {
        if (pos >= rowsize-1)   // remember the bool for isnull or not
        {
            return false;
        }

        if (rowstringRef[pos++] == 0)
        {
            resultFieldsRef[n].isnull = false;
        }
        else
        {
            resultFieldsRef[n].isnull = true;
            continue;
        }

        switch (fields[n].type)
        {
        case INT:
            memcpy(&resultFieldsRef[n].value.integer, &rowstringRef[pos],
                   sizeof(resultFieldsRef[n].value.integer));
            pos += sizeof(resultFieldsRef[n].value.integer);
            break;

        case UINT:
            memcpy(&resultFieldsRef[n].value.uinteger, &rowstringRef[pos],
                   sizeof(resultFieldsRef[n].value.uinteger));
            pos += sizeof(resultFieldsRef[n].value.uinteger);
            break;

        case BOOL:
            memcpy(&resultFieldsRef[n].value.boolean, &rowstringRef[pos],
                   sizeof(resultFieldsRef[n].value.boolean));
            pos += sizeof(resultFieldsRef[n].value.boolean);
            break;

        case FLOAT:
            memcpy(&resultFieldsRef[n].value.floating, &rowstringRef[pos],
                   sizeof(resultFieldsRef[n].value.floating));
            pos += sizeof(resultFieldsRef[n].value.floating);
            break;

        case CHAR:
            memcpy(&resultFieldsRef[n].value.character, &rowstringRef[pos],
                   sizeof(resultFieldsRef[n].value.character));
            pos += sizeof(resultFieldsRef[n].value.character);
            break;

        case CHARX: // makerow() already padded this field to the length of the
            // char(x)
            resultFieldsRef[n].str.assign(rowstringRef, pos, fields[n].length);

            if (resultFieldsRef[n].str.size() < fields[n].length)
            {
                // this should not be, but pad it anyway
                printf("%s %i rowstring.size() %lu field %lu resultFieldsRef[n].str.size() %lu fields[n].length %lu\n", __FILE__, __LINE__,
                       (unsigned long)rowstring->size(), (unsigned long)n,
                       resultFieldsRef[n].str.size(), fields[n].length);
                resultFieldsRef[n].str.append(fields[n].length -
                                              resultFieldsRef[n].str.size(), 0);
            }

            pos += fields[n].length;
            break;

        case VARCHAR:
        {
            int64_t varcharlength;
            memcpy(&varcharlength, &rowstringRef[pos], sizeof(varcharlength));
            pos += sizeof(varcharlength);
            resultFieldsRef[n].str.assign(rowstringRef, pos, varcharlength);
            pos += varcharlength;
        }
        break;

        default:
            fprintf(logfile, "anomaly: %i %s %i\n", fields[n].type, __FILE__,
                    __LINE__);
        }
    }

    return true;
}

int64_t Table::getnextrowid()
{
    return ++nextrowid;
}

void Table::newrow(int64_t newrowid, int64_t subtransactionid, string &row)
{
    rowdata_s *nrow = new rowdata_s();
    //  nrow->flags = 0;
    setwritelock(&nrow->flags);
    nrow->writelockHolder = subtransactionid;
    setinsertflag(&nrow->flags);
    //  nrow.previoussubtransactionid = 0;
    nrow->row = row;
    //  nrow.readlockHolders=NULL;

    rows[newrowid] = nrow;
    shadowTable->rows[newrowid] = nrow;
}

int64_t Table::deleterow(int64_t rowid, int64_t subtransactionid)
{
    if (!rows.count(rowid))
    {
        return STATUS_NOTOK;
    }

    rowdata_s &currentRowRef = *rows[rowid];

    if ((getlocktype(currentRowRef.flags) != WRITELOCK) ||
        (currentRowRef.writelockHolder != subtransactionid))
    {
        fprintf(logfile, "anomaly %i %li %s %i\n", currentRowRef.flags,
                currentRowRef.writelockHolder, __FILE__, __LINE__);
        return STATUS_NOTOK;
    }

    setdeleteflag(&currentRowRef.flags);
    return STATUS_OK;
}

locktype_e Table::assignToLockQueue(int64_t rowid, locktype_e locktype,
                                    int64_t subtransactionid, int64_t pendingcmdid, int64_t tacmdentrypoint)
{
    if (getinsertflag(rows[rowid]->flags)==true)
    {
        return NOTFOUNDLOCK;
    }

    lockQueueRowEntry entry;
    entry.tacmdentrypoint = tacmdentrypoint;
    entry.locktype = locktype;
    entry.pendingcmdid = pendingcmdid;
    entry.subtransactionid = subtransactionid;

    lockQueue[rowid].push(entry);

    return locktype;
}
