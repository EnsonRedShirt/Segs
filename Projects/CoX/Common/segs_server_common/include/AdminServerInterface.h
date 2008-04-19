/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminServerInterface.h 301 2006-12-26 15:50:44Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef ADMINSERVERINTERFACE_H
#define ADMINSERVERINTERFACE_H

#include <ace/INET_Addr.h>
#ifndef WIN32
#include <ext/hash_map>
using namespace __gnu_cxx;
#else
#include <hash_map>
#endif // WIN32
#include <list>
#include "ServerHandle.h"
#include "RoamingServer.h"
/************************************************************************/
/* Design consideration:                                                */
/* Current implementation gets knowledge it shouldn't possess,          */
/* AdminServerInterface::GetClientByLogin returns all details known     */
/* about given client.													*/
/************************************************************************/

// this is interface for calling AdminServer services, currently it's an Interface pattern
// in future it can be a Remote Proxy

class IClient;

class IMapServer;
class IGameServer;

class IAdminServer : public RoamingServer
{
public:
virtual	int                         GetBlockedIpList(std::list<int> &)=0;
virtual	bool                        Logout(const IClient *client) const=0;
virtual	bool                        Login(const IClient *client,const ACE_INET_Addr &client_addr)=0;
virtual	bool                        ValidPassword(const IClient *client, const char *password)=0;

virtual	void                        FillClientInfo(IClient *client)=0;
virtual	int                         SaveAccount(const char *username, const char *password)=0;
virtual	int                         RemoveAccount(IClient *client)=0;

virtual	int                         AddIPBan(const ACE_INET_Addr &client_addr)=0;
virtual	void                        InvalidGameServerConnection(const ACE_INET_Addr &)=0;

virtual	bool                        ReadConfig(const std::string &name)=0;
virtual	bool                        Run(void)=0;
virtual	bool                        ShutDown(const std::string &reason="No particular reason")=0;

virtual	ServerHandle<IGameServer>    RegisterMapServer(const ServerHandle<IMapServer> &map_h )=0;
virtual	int                         GetAccessKeyForServer(const ServerHandle<IMapServer> &h_server )=0;


};
class AdminServerInterface : public Server
{
public:
	AdminServerInterface(IAdminServer *srv);
	~AdminServerInterface(void);

	bool ReadConfig(const std::string &name);
	bool Run(void);
	bool ShutDown(const std::string &reason);

    ServerHandle<IGameServer> RegisterMapServer(const ServerHandle<IMapServer> &map_h);
    int GetAccessKeyForServer(const ServerHandle<IMapServer> &h_server);


	int GetBlockedIpList(std::list<int> &addreses); // called from auth server during user authentication, might be useful for automatical firewall rules update
	void FillClientInfo(IClient *); 
	bool Login(IClient  *client,const ACE_INET_Addr &client_addr); // Records given 'client' as logged in from 'addr'.
    int	 SaveAccount(const char *username, const char *password);  // Save account
	bool Logout(IClient  *client); // Records given 'client' as logged out in from 'addr'.
	bool ValidPassword(const IClient *client, const char *password); // If 'pass' is a valid password for client, return true
	void InvalidGameServerConnection(const ACE_INET_Addr &from);
	void RunCommand(const char *); //magical entry point to internal workings of all the servers ??
protected:
	IAdminServer *m_server;
};

#endif // ADMINSERVERINTERFACE_H