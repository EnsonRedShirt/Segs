/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientManager.h 319 2007-01-26 17:03:18Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include "GameProtocol.h"
#include "GameProtocolHandler.h"
#include "Client.h"
#ifndef WIN32
#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
namespace __gnu_cxx {
	template<> struct hash<u64> { size_t operator()(u64 __x) const { return ((__x>>32) ^ __x) &0xFFFFFFFF; } }; 
};
#else
#include <hash_map>
#include <hash_set>
using namespace stdext;
#endif // WIN32

class ClientManager
{
public:
	static Client *CreateClient(){return new Client;};
};
template <class CLIENT_CLASS>
class ClientStore
{
	//	boost::object_pool<CLIENT_CLASS> m_pool;
	hash_map<u32,CLIENT_CLASS *> m_expected_clients;
	hash_map<u64,CLIENT_CLASS *> m_clients; // this maps client's id to it's object
	hash_map<u32,CLIENT_CLASS *> m_connected_clients_cookie; // this maps client's id to it's object

	u32 create_cookie(const ACE_INET_Addr &from,u64 id)
	{
		u64 res = ((from.hash()+id&0xFFFFFFFF)^(id>>32));
		ACE_DEBUG ((LM_WARNING,ACE_TEXT ("(%P|%t) create_cookie still needs a good algorithm.0x%08x\n"),res));
		return (u32)res;
	};

public:

	CLIENT_CLASS *getById(u64 id)
	{
		if(m_clients.find(id)==m_clients.end())
			return NULL;
		return m_clients[id];
	}

	CLIENT_CLASS *getByCookie(u32 cookie)
	{
		if(m_connected_clients_cookie.find(cookie)!=m_connected_clients_cookie.end())
		{
			return m_connected_clients_cookie[cookie];
		}
		return NULL;
	}

	CLIENT_CLASS *getExpectedByCookie(u32 cookie)
	{
		// we got cookie check if it's an expected client
		if(m_expected_clients.find(cookie)!=m_expected_clients.end())
		{
			return m_expected_clients[cookie];
		}
		return NULL;
	}

	u32 ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level)
	{
		CLIENT_CLASS * exp;
		u32 cook = create_cookie(from,id);
		// we already expect this client
		if(m_expected_clients.find(cook)!=m_expected_clients.end())
		{
			return cook;
		}
		if(getByCookie(cook)) // already connected ?!
		{
			//
			return ~0; // invalid cookie
		}
		exp = new CLIENT_CLASS;
		exp->setAccessLevel(access_level);
		exp->setId(id);
		exp->setState(IClient::CLIENT_EXPECTED);
		m_expected_clients[cook] = exp;
		m_clients[id] = exp;
		return cook;
	}
};

#endif // CLIENTMANAGER_H