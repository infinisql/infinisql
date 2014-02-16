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
 * @file   UserSchemaManager.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 09:07:08 2014
 * 
 * @brief  Actor that maintains user and domain authentication information
 * and schema definitions.
 *
 * There is 1 UserSchemaManager per replica, and the master replica's
 * UserSchemaManager replicates to each slave.
 */

#include "UserSchemaManager.h"
#include "../transaction/trans.h"

UserSchemaManager::UserSchemaManager(Actor::identity_s identity)
    : Actor(identity)
{
}

void UserSchemaManager::operator()()
{
    int waitfor=100;

    while(1)
    {
        myTopology.update();
        sendObBatch();

        for (int inmsg=0; inmsg < 10; ++inmsg)
        {
            getmsg(waitfor);
            if (msgrcv==nullptr)
            {
                waitfor=100;
                break;
            }
            waitfor=0;

            switch(msgrcv->message.topic)
            {
            case Message::TOPIC_USERSCHEMA:
            {
                MessageUserSchema &msgRef=*(MessageUserSchema *)msgrcv;
                
            }
            break;                

            default:
                LOG("can't handle topic " << msgrcv->message.topic);
            }
        }
    }
}
