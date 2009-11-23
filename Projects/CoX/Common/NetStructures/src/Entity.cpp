/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Entity.cpp 289 2006-10-03 14:52:25Z nemerle $
 */

#define _USE_MATH_DEFINES
#include <math.h>
#ifdef WIN32
#include "xmmintrin.h"
#else
#define _copysign(x,y) ((x)* ((y<0.0)? -1.0 : 1.0))
#endif
#include "Entity.h"
#include <limits>
#include <sstream>
float AngleDequantize(int value,int numb_bits)
{
	int max_val = 1<<numb_bits;
	float v = M_PI*((float)value/max_val) - M_PI/2;
	if(v<(-M_PI/2))
		return -1.0f;
	else if(v>(M_PI/2))
		return 1.0f;
	else if(v<0.00001)
		return 0.0f;
	return sinf(v);
}
u32 AngleQuantize(float value,int numb_bits)
{
	int max_val = 1<<numb_bits;
	float v = fabs(value)>1.0f ? _copysign(1.0f,value) : value ;
	v  = (asinf(v)+M_PI)/(2*M_PI); // maps -1..1 to 0..1
	v *= max_val;
//	assert(v<=max_val);
	return (u32)v;
}
int Entity::getOrientation(BitStream &bs)
{
	float fval;
	int update_qrot;
	update_qrot = getBitsConditional(bs,3);
	if(!update_qrot)
		return 0;
	bool recv_older = false;
	for(int i=0; i<3; i++)
	{
		if(update_qrot&(1<<i))
		{
			fval = AngleDequantize(bs.GetBits(9),9);
			if(current_client_packet_id>pkt_id_QrotUpdateVal[i])
			{
				pkt_id_QrotUpdateVal[i] = current_client_packet_id;
				qrot.q[i] = fval;				
			}
			else
				recv_older=true;
		}
	}
	//RestoreFourthQuatComponent(pEnt->qrot);
	//NormalizeQuaternion(pEnt->qrot)
		return recv_older==false;
}
void Entity::storeOrientation(BitStream &bs) const
{
	// if(updateNeeded())
	u8 updates;
	updates = ((int)update_rot(0)) | (((int)update_rot(1))<<1) | (((int)update_rot(2))<<2);
	storeBitsConditional(bs,3,updates); //frank 7,0,0.1,0
	//NormalizeQuaternion(pEnt->qrot)
	// 
	//RestoreFourthQuatComponent(pEnt->qrot);
	for(int i=0; i<3; i++)
	{
		if(update_rot(i))
		{
			bs.StoreBits(9,AngleQuantize(qrot.q[i],9));	 // normalized quat, 4th param is recoverable from the first 3
		}
	}
}
static inline u32 quantize_float(float v)
{
	return u32(floorf(v*64)+0x800000);
}
void Entity::storePosition(BitStream &bs) const
{
//	float x = pos.vals.x;
	u32 packed;
	u32 diff=0; // changed bits are '1'
	bs.StoreBits(3,7); // frank -> 7,-60.5,0,180
	for(int i=0; i<3; i++)
	{
		packed = quantize_float(pos.v[i]);
		packed = packed<0xFFFFFF ? packed : 0xFFFFFF;
		//diff = packed ^ prev_pos[i]; // changed bits are '1'
		bs.StoreBits(24,packed);
	}
}
void Entity::storePosUpdate(BitStream &bs) const
{
	storePosition(bs);
	// if(is_update)
	if(false)
	{
		bs.StoreBits(1,0); // not extra_info
	}
	storeOrientation(bs);
}
void Entity::storeUnknownBinTree(BitStream &bs) const
{
	bs.StoreBits(1,1);

}
void Entity::sendStateMode(BitStream &bs) const
{
	bs.StoreBits(1,m_state_mode_send); // no state mode
	if(m_state_mode_send)
	{
		storePackedBitsConditional(bs,3,m_state_mode);
	}
}
void Entity::sendSeqMoveUpdate(BitStream &bs) const
{
	bs.StoreBits(1,m_seq_update); // no seq update
	if(m_seq_update)
	{
		storePackedBitsConditional(bs,8,m_seq_upd_num1); // move index
		storePackedBitsConditional(bs,4,m_seq_upd_num2); //maxval is 255

	}
}
void Entity::sendSeqTriggeredMoves(BitStream &bs) const
{
	bs.StorePackedBits(1,0); // num moves
}
void Entity::sendNetFx(BitStream &bs) const
{
	bs.StorePackedBits(1,m_num_fx); // num fx
	//NetFx.serializeto();
	for(int i=0; i<m_num_fx; i++)
	{
		bs.StoreBits(8,m_fx1[i]); // net_id
		bs.StoreBits(32,m_fx2[i]); // command
		bs.StoreBits(1,0);
		storePackedBitsConditional(bs,10,0xCB8);
/*
		storeBitsConditional(bs,4,0);
		storePackedBitsConditional(bs,12,0xCB8);
		bs.StoreBits(1,0);
		if(false)
		{
			bs.StoreBits(32,0);
		}
*/
		storeBitsConditional(bs,4,0);
		storeBitsConditional(bs,32,0);
		storeFloatConditional(bs,0.0);
		storeFloatConditional(bs,10.0);
		storeBitsConditional(bs,4,10);
		storeBitsConditional(bs,32,0);
		int val=0;
		storeBitsConditional(bs,2,val);
		if(val==1)
		{
			bs.StoreFloat(0.0);
			bs.StoreFloat(0.0);
			bs.StoreFloat(0.0);
		}
		else
		{
			if(val)
			{
				//"netbug"
			}
			else
			{
				storePackedBitsConditional(bs,8,0);
				bs.StorePackedBits(2,0);

			}
		}
		storeBitsConditional(bs,2,0);
		if(false)
		{
			bs.StoreFloat(0);
			bs.StoreFloat(0);
			bs.StoreFloat(0);
		}
		else
		{
			storePackedBitsConditional(bs,12,0x19b);
		}
	}
}
void MobEntity::sendCostumes( BitStream &bs ) const
{
	int npc_costume_type_idx=0;
	int costume_idx=0;
	storePackedBitsConditional(bs,2,m_costume_type);
	if((m_costume_type!=2)&&(m_costume_type!=4))
	{
		ACE_ASSERT(false);
		return;
	}
	if(m_costume_type==2)
	{
		bs.StorePackedBits(12,npc_costume_type_idx);
		bs.StorePackedBits(1,costume_idx);
	}
	else
	{
		bs.StoreString(m_costume_seq);
	}
}

void PlayerEntity::sendCostumes(BitStream &bs) const
{
	storePackedBitsConditional(bs,2,m_costume_type);
	if(m_costume_type!=1)
	{
		ACE_ASSERT(false);
		return;
	}
	if(m_type==ENT_PLAYER)
	{
		bs.StoreBits(1,m_current_costume_set);
		if(m_current_costume_set)
		{
			bs.StoreBits(32,m_current_costume_idx);
			bs.StoreBits(32,m_costumes.size());
		}
		bs.StoreBits(1,m_multiple_costumes);
		if(m_multiple_costumes)
		{
			for(size_t idx=0; idx<=m_costumes.size(); idx++)
			{
				m_costumes[idx]->serializeto(bs);
			}
		}
		else
		{
			m_costumes[m_current_costume_idx]->serializeto(bs);
		}
		//m_costume=m_costumes[m_current_costume_idx]; // this is only set if ent!=playerPtr
	}
	else
	{
		m_costume->serializeto(bs); // m_costume is the costume that is rendered
	}
	bs.StoreBits(1,m_supergroup_costume);
	if(m_supergroup_costume)
	{
		m_sg_costume->serializeto(bs);
		bs.StoreBits(1,m_using_sg_costume);
	}
}

PlayerEntity::PlayerEntity()
{
	m_costume_type=1;
	m_multiple_costumes=false;
	m_current_costume_idx=0;
	m_current_costume_set=false;
	m_supergroup_costume=false;
	m_sg_costume=0;
	m_using_sg_costume=false;
}
MobEntity::MobEntity()
{
	m_costume_type=2;
}
void Entity::sendXLuency(BitStream &bs,float val) const
{
	storeBitsConditional(bs,8,min(static_cast<int>(u8(val*255)),255)); // upto here everything is ok
}
void Entity::sendTitles(BitStream &bs) const
{
	bs.StoreBits(1,m_has_titles); // no titles
	if(m_has_titles)
	{
		if(m_type==ENT_PLAYER)
		{
			bs.StoreString(m_name);
			bs.StoreBits(1,0); // ent_player2->flag_F4
			storeStringConditional(bs,"");//max 32 chars
			storeStringConditional(bs,"");//max 32 chars
			storeStringConditional(bs,"");//max 128 chars
		}
		else // unused
		{
			bs.StoreString("");
			bs.StoreBits(1,0);
			storeStringConditional(bs,"");
			storeStringConditional(bs,"");
			storeStringConditional(bs,"");
		}
	}
}
void Entity::sendRagDoll(BitStream &bs) const
{
	int num_bones=0; //NPC->0 bones
	storeBitsConditional(bs,5,num_bones);
	if(num_bones)
		bs.StorePackedBits(1,0); // no titles
}
void Entity::sendOnOddSend(BitStream &bs,bool is_odd) const
{
	bs.StoreBits(1,is_odd);
}
void Entity::sendAllyID(BitStream &bs)
{
	bs.StorePackedBits(2,0);
	bs.StorePackedBits(4,0); // NPC->0

}
void Entity::sendPvP(BitStream &bs)
{
	bs.StoreBits(1,0);
	bs.StoreBits(1,0);
	bs.StorePackedBits(5,0);
	bs.StoreBits(1,0);
}
void Entity::sendEntCollision(BitStream &bs) const
{
	bs.StoreBits(1,0); // 1/0 only
}
void Entity::sendNoDrawOnClient(BitStream &bs) const
{
	bs.StoreBits(1,0); // 1/0 only
}
void Entity::sendContactOrPnpc(BitStream &bs) const
{
	// frank 1
	bs.StoreBits(1,m_contact); // 1/0 only
}
void Entity::sendPetName(BitStream &bs) const
{
	storeStringConditional(bs,"");
}
void Entity::sendAFK(BitStream &bs) const
{
	bool is_away=false;
	bool away_string=false;
	bs.StoreBits(1,is_away); // 1/0 only
	if(is_away)
	{
		bs.StoreBits(1,away_string); // 1/0 only
		if(away_string)
			bs.StoreString("");
	}
}
void Entity::sendOtherSupergroupInfo(BitStream &bs) const
{
	bs.StoreBits(1,m_SG_info); // UNFINISHED
	if(m_SG_info) // frank has info
	{
		bs.StorePackedBits(2,field_78);
		if(field_78)
		{
			bs.StoreString("");//64 chars max
			bs.StoreString("");//128 chars max -> hash table key from the CostumeString_HTable
			bs.StoreBits(32,0);
			bs.StoreBits(32,0);
		}
	}
}
void Entity::sendLogoutUpdate(BitStream &bs) const
{
	bool is_logout=false;
	bs.StoreBits(1,is_logout);
	if(is_logout)
	{
		bs.StoreBits(1,0); // flags_1[1] set in entity 
		storePackedBitsConditional(bs,5,0); // time to logout, multiplied by 30
	}
}

void Entity::serializeto( BitStream &bs ) const
{
	//////////////////////////////////////////////////////////////////////////

	// entity creation
	bs.StoreBits(1,m_create); // checkEntCreate_varD14
	if(m_create)
	{
		bs.StoreBits(1,var_129C); // checkEntCreate_var_129C / ends creation destroys seq and returns NULL

		if(var_129C)
			return;
		bs.StorePackedBits(12,field_64);//  this will be put in  of created entity
		bs.StorePackedBits(2,m_type);
		if(m_type==ENT_PLAYER)
		{
			bs.StoreBits(1,m_create_player);
			if(m_create_player)
				bs.StorePackedBits(1,0x123); // var_1190: this will be put in field_C8 of created entity 
			bs.StorePackedBits(20,0);//bs.StorePackedBits(20,m_db_id);
		}
		else
		{
			bool val=false;
			bs.StoreBits(1,val);
			if(val)
			{
				bs.StorePackedBits(12,0); // entity idx
				bs.StorePackedBits(12,0); // entity idx
			}
		}
		if(m_type==ENT_PLAYER || m_type==ENT_CRITTER)
		{
			bs.StorePackedBits(1,m_origin_idx);
			bs.StorePackedBits(1,m_class_idx);
			bool val=false;
			bs.StoreBits(1,val);
			if(val)
			{
				bs.StoreBits(1,0); // entplayer_flgFE0
				storeStringConditional(bs,"");
				storeStringConditional(bs,"");
				storeStringConditional(bs,"");
			}
		}
		bs.StoreBits(1,m_hasname);
		if(m_hasname)
			bs.StoreString(m_name);
		bs.StoreBits(1,0); //var_94 if set Entity.field_1818/field_1840=0 else field_1818/field_1840 = 255,2
		bs.StoreBits(32,field_60); // this will be put in field_60 of created entity 
		bs.StoreBits(1,m_hasgroup_name);
		if(m_hasgroup_name)
		{
			bs.StorePackedBits(2,0);// this will be put in field_1830 of created entity 
			bs.StoreString(m_group_name);
		}
	}
	// End of entrecv_442C60
	//////////////////////////////////////////////////////////////////////////
	// creation ends here

	bs.StoreBits(1,might_have_rare); //var_C

	if(might_have_rare)
	{
		bs.StoreBits(1,m_rare_bits);
	}
	if(m_rare_bits)
		sendStateMode(bs);
	storePosUpdate(bs);
	if(might_have_rare)
	{
		sendSeqMoveUpdate(bs);
	}
	if(m_rare_bits)
		sendSeqTriggeredMoves(bs);
	// NPC -> m_pchar_things=0 ?
	bs.StoreBits(1,m_pchar_things);
	if(m_pchar_things)
	{
		sendNetFx(bs);
	}
	if(m_rare_bits)
	{
		sendCostumes(bs);
		sendXLuency(bs,0.5f);
		sendTitles(bs);
	}
	if(m_pchar_things)
	{
		sendCharacterStats(bs);
		sendBuffs(bs);
		sendTargetUpdate(bs);
	}
	if(m_rare_bits)
	{
		sendOnOddSend(bs,m_odd_send); // is one on client end
		sendWhichSideOfTheForce(bs);
		sendEntCollision(bs);
		sendNoDrawOnClient(bs);
		sendAFK(bs);
		sendOtherSupergroupInfo(bs);
		sendLogoutUpdate(bs);
	}
}
void Entity::sendBuffs(BitStream &bs) const
{
	bs.StoreBits(1,0); // nothing here for now
}
void Entity::sendCharacterStats(BitStream &bs) const
{
	bs.StoreBits(1,0); // nothing here for now
}
void Entity::sendTargetUpdate(BitStream &bs) const
{
	bs.StoreBits(1,0); // nothing here for now
}
void Entity::sendWhichSideOfTheForce(BitStream &bs) const
{
	bs.StoreBits(1,0);
	bs.StoreBits(1,0);
	// flags 3 & 2 of entity flags_1
}
bool Entity::update_rot( int axis ) const /* returns true if given axis needs updating */
{
	if(axis==axis)
		return true;
	return false;
}

void Avatar::GetCharBuildInfo( BitStream &src )
{
	src.GetString(m_class_name);
	src.GetString(m_origin_name);
	m_powers.push_back(get_power_info(src)); // primary_powerset power
	m_powers.push_back(get_power_info(src)); // secondary_powerset power
	m_trays.serializefrom(src);

}

void Avatar::DumpPowerPoolInfo( const PowerPool_Info &pool_info )
{
	for(int i=0; i<3; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Pool_id[%d]: 0x%08x\n"),i,pool_info.id[i]));
	}
}

void Avatar::DumpBuildInfo()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),m_class_name.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),m_origin_name.c_str()));
	DumpPowerPoolInfo(m_powers[0]);
	DumpPowerPoolInfo(m_powers[1]);
}

void PlayerEntity::serializefrom_newchar( BitStream &src )
{
	int val = src.GetPackedBits(1); //2
	m_char.GetCharBuildInfo(src);
	m_costume = new MapCostume;
	m_costume->serializefrom(src);
	m_costumes.push_back(m_costume);
	int t = src.GetBits(1); // The -> 1
	src.GetString(m_battle_cry);
	src.GetString(m_character_description);
}

void Entity::InsertUpdate( PosUpdate pup )
{
	m_update_idx++;
	m_update_idx %=64;
	m_pos_updates[m_update_idx]=pup;
}

void Entity::dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Tray------------------\n")));
	m_char.m_trays.dump();
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Costume---------------\n")));
	m_costume->dump();
}

Entity::Entity() : m_char(0)
{
	field_60=0;
	field_64=0;
	field_78=0;
	m_state_mode_send=0;
	m_state_mode=0;
	m_seq_update=0;
	m_has_titles=false;
	m_SG_info=false;
}

PowerPool_Info Avatar::get_power_info( BitStream &src )
{
	PowerPool_Info res;
	res.id[0] = src.GetPackedBits(3);
	res.id[1] = src.GetPackedBits(3);
	res.id[2] = src.GetPackedBits(3);
	return res;
}
void Avatar::sendTray(BitStream &bs) const
{
	m_trays.serializeto(bs);
}
void Avatar::sendTrayMode(BitStream &bs) const
{
	bs.StoreBits(1,0);
}
void Avatar::sendEntStrings(BitStream &bs) const
{
	bs.StoreString(m_ent->m_battle_cry); //max 128
	bs.StoreString(m_ent->m_character_description); //max 1024
}
void Avatar::sendWindow(BitStream &bs) const
{
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0); // visible ?
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bool a=false;
	bs.StoreBits(1,a);
	if(a)
	{
		bs.StorePackedBits(1,0);
		bs.StorePackedBits(1,0);
	}
	//storeFloatConditional(bs,1.0f);
}
void Avatar::sendTeamBuffMode(BitStream &bs) const
{
	bs.StoreBits(1,0);
}
void Avatar::sendDockMode(BitStream &bs) const
{
	bs.StoreBits(32,0); // unused on the client
	bs.StoreBits(32,0); // 
}
void Avatar::sendChatSettings(BitStream &bs) const
{
	int i;
	bs.StoreFloat(0.8); // window transparency ?
	bs.StorePackedBits(1,1);
	bs.StorePackedBits(1,2);
	bs.StorePackedBits(1,3);
/*
	bs.StorePackedBits(1,4);
	bs.StorePackedBits(1,5);
	bs.StorePackedBits(1,6);
	for(i=0; i<5; i++)
	{
		bs.StorePackedBits(1,1);
		bs.StorePackedBits(1,2);
		bs.StorePackedBits(1,3);
		bs.StoreFloat(1.0f);
	}
	for(i=0; i<10; i++)
	{
		bs.StoreString("TestChat1");
		bs.StorePackedBits(1,1);
		bs.StorePackedBits(1,2);
		bs.StorePackedBits(1,3);
		bs.StorePackedBits(1,4);
		bs.StorePackedBits(1,5);
	}
	for(i=0; i<10; i++)
	{
		bs.StoreString("TestChat2");
		bs.StorePackedBits(1,1);
	}
*/
}
void Avatar::sendDescription(BitStream &bs) const
{
	bs.StoreString("Desc1");
	bs.StoreString("Desc2");

}
void Avatar::sendTitles(BitStream &bs) const
{
	bs.StoreBits(1,1);
	bs.StoreString("Tz1");
	bs.StoreString("Tz2");
	bs.StoreString("Tz3");
}
void Avatar::sendKeybinds(BitStream &bs) const
{
	bs.StoreString("Keybinds");
	for(int i=0; i<256; i++)
	{
		bs.StoreString(""); //w = +forward
		bs.StoreBits(32,0);
		bs.StoreBits(32,0);
	}

}
void Avatar::sendFriendList(BitStream &bs) const
{
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
}
void Avatar::serializeto(BitStream &bs) const
{
	u8 arr[16]={0};
	//////////////////////////////////////////////////////////////////////////
	// character send
	//////////////////////////////////////////////////////////////////////////
	send_character(bs);
	//////////////////////////////////////////////////////////////////////////
	// full stats
	//////////////////////////////////////////////////////////////////////////
	sendFullStats(bs);

	sendBuffs(bs);
	bool has_sidekick=false;
	bs.StoreBits(1,has_sidekick);
	if(has_sidekick)
	{
		bool is_mentor=false; // this flag might mean something totally different :)
		bs.StoreBits(1,is_mentor);
		bs.StorePackedBits(20,0); // sidekick partner idx -> 10240
	}
	sendTray(bs);
	sendTrayMode(bs);
	bs.StoreString(m_ent->m_name); // maxlength 32
	sendEntStrings(bs);
	for(int i=0; i<35; i++)
	{
		bs.StorePackedBits(1,i); // window index
		sendWindow(bs);
	}
//	bs.StoreBits(10,0); // lfg
//	bs.StoreBits(1,0); // super group mode
	bs.StoreBits(1,0); // pent->player_ppp.field_540
	bs.StoreBits(1,0); // pEnt->player_ppp.field_984C
	sendTeamBuffMode(bs);
	sendDockMode(bs);
	sendChatSettings(bs);
	sendTitles(bs);
	sendDescription(bs);
	u8 auth_data[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	bs.StoreBitArray(auth_data,128);
	sendKeybinds(bs);
	bs.StoreBits(1,m_full_options);
	if(m_full_options)
	{
		sendOptions(bs);
	}
	else
	{
		bs.StoreBits(1,m_options.mouse_invert);
		bs.StoreFloat(m_options.mouselook_scalefactor);
		bs.StoreFloat(m_options.degrees_for_turns);
	}
	bs.StoreBits(1,m_first_person_view_toggle);
	sendFriendList(bs);
}
Avatar::Avatar(Entity *ent)
{
	m_ent = ent;
	m_full_options=false;
	m_first_person_view_toggle=false;
	m_player_collisions=0;
	m_options.mouse_invert=false;
	m_options.degrees_for_turns=1.0f;
	m_options.mouselook_scalefactor=1.0f;
	m_class_name = "Class_Blaster";
	m_origin_name= "Science";

}
void sendPower(BitStream &bs,int a,int b,int c)
{
	bs.StorePackedBits(3,a);
	bs.StorePackedBits(3,b);
	bs.StorePackedBits(3,c);
}
void sendPowers(BitStream &bs)
{
	bs.StorePackedBits(4,0); // count
	for(int i=0; i<0; i++)
	{
		size_t num_powers=0;
		bs.StorePackedBits(5,0);
		bs.StorePackedBits(4,num_powers);
		for(size_t idx=0; idx<num_powers; ++idx)
		{
			size_t num_somethings=0;
			sendPower(bs,0,0,0);

			bs.StorePackedBits(5,0);
			bs.StoreFloat(1.0);
			bs.StorePackedBits(4,num_somethings);

			for(size_t idx2=0; idx2<num_somethings; ++idx2)
			{
				sendPower(bs,0,0,0);
				bs.StorePackedBits(5,0);
				bs.StorePackedBits(2,0);
			}
		}
	}
}
void sendPowers_main_tray(BitStream &bs)
{
	size_t max_num_cols=3;
	size_t max_num_rows=1;
	bs.StorePackedBits(3,max_num_cols); // count
	bs.StorePackedBits(3,max_num_rows); // count
	for(int i=0; i<max_num_cols; i++)
	{
		for(size_t idx=0; idx<max_num_rows; ++idx)
		{
			bool is_power=false;
			bs.StoreBits(1,is_power);
			if(is_power)
			{
				sendPower(bs,0,0,0);
			}
		}
	}
}

void sendBoosts(BitStream &bs)
{
	size_t num_boosts=0;
	bs.StorePackedBits(5,num_boosts); // count
	for(size_t idx=0; idx<num_boosts; ++idx)
	{
		bool set_boost=false;
		bs.StorePackedBits(3,0); // bost idx
		bs.uStoreBits(1,set_boost); // 1 set, 0 clear
		if(set_boost)
		{
			sendPower(bs,0,0,0);
			bs.StorePackedBits(5,0); // bost idx
			bs.StorePackedBits(2,0); // bost idx
		}
	}
	// boosts
}
void sendUnk2(BitStream &bs)
{
	bs.StorePackedBits(5,0); // count
}
void sendUnk3(BitStream &bs) // inventory ?
{
	bs.StorePackedBits(3,0); // count
}
void Avatar::send_character(BitStream &bs) const
{
	bs.StoreString(m_class_name); // class name
	bs.StoreString(m_origin_name); // origin name
	bs.StorePackedBits(5,0); // ?
	// powers/stats ?
	sendPowers(bs);
	sendPowers_main_tray(bs);
	sendBoosts(bs);
}
void Avatar::sendFullStats(BitStream &bs) const
{
	// if sendAbsolutoOverride 

	// this uses the character schema from the xml -> FullStats and children

	// CurrentAttributes
	bs.StoreBits(1,1);
	bs.StorePackedBits(1,0); // CurrentAttribs entry idx
	{
			// nested into CurrentAttribs:LiveAttribs
			bs.StoreBits(1,1); // has more data
			bs.StorePackedBits(1,0); // HitPoints entry
			// field type 0xA, param 2
			// Type15_Params 2 1.0 1.0 7
			if(1) // absolute values
			{
				bs.StorePackedBits(7,45); // character health/1.0
			}
			else
			{
				// StoreOptionalSigned(
					// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			}
			bs.StoreBits(1,1); // has more data
			bs.StorePackedBits(1,1); // EndurancePoints entry
			// field type 0xA, param 2
			if(1) // absolute values
			{
				bs.StorePackedBits(7,45); // character end/1.0
			}
			else
			{
				// StoreOptionalSigned(
				// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			}
			bs.StoreBits(1,0); // nest out
	}
	bs.StoreBits(1,1); // has more data
	bs.StorePackedBits(1,1); // MaxAttribs entry idx
	{
		// nested into MaxAttribs:LiveAttribs
		bs.StoreBits(1,1); // has more data
			bs.StorePackedBits(1,0); // HitPoints entry
			// field type 0xA, param 2
			// Type15_Params 2 1.0 1.0 7
			if(1) // absolute values
			{
				bs.StorePackedBits(7,45); // character health/1.0
			}
			else
			{
				// StoreOptionalSigned(
				// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			}
		bs.StoreBits(1,1); // has more data
			bs.StorePackedBits(1,1); // EndurancePoints entry
			// field type 0xA, param 2
			if(1) // absolute values
			{
				bs.StorePackedBits(7,45); // character end/1.0
			}
			else
			{
				// StoreOptionalSigned(
				// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			}
		bs.StoreBits(1,0); // nest out
	}
	bs.StoreBits(1,1); // has more data
	bs.StorePackedBits(1,2); // SendLevels entry idx
	{
		// nested into SendLevels:LiveLevels
		bs.StoreBits(1,1); // has more data
			bs.StorePackedBits(1,0); // Level entry
			// field type 0x5, param 4
			if(1) // absolute values
			{
				bs.StorePackedBits(4,1); // 
			}
			else
			{
				// StoreOptionalSigned(
				// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
				// send prev_lev-new_lev
			}
		bs.StoreBits(1,1); // has more data
			bs.StorePackedBits(1,1); // CombatLevel entry
			if(1) // absolute values
			{
				bs.StorePackedBits(4,1); 
			}
			else
			{
				// StoreOptionalSigned(
				// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			}
		bs.StoreBits(1,0); // nest out
	}
	bs.StoreBits(1,1); // has more data
	bs.StorePackedBits(1,3); // Experience
	{
		// field type 0x5, param 16
		if(1) // absolute values
		{
			bs.StorePackedBits(16,1); // 
		}
		else
		{
			// StoreOptionalSigned(
			// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			// send prev_lev-new_lev
		}
	}
	bs.StoreBits(1,1); // has more data
	bs.StorePackedBits(1,4); // ExperienceDebt
	{
		// field type 0x5, param 16
		if(1) // absolute values
		{
			bs.StorePackedBits(16,1); // 
		}
		else
		{
			// StoreOptionalSigned(
			// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			// send prev_lev-new_lev
		}
	}
	bs.StoreBits(1,1); // has more data
	bs.StorePackedBits(1,5); // Influence
	{
		// field type 0x5, param 16
		if(1) // absolute values
		{
			bs.StorePackedBits(16,0); // 
		}
		else
		{
			// StoreOptionalSigned(
			// Bits(1) ?( Bits(1) ? -packedBits(1) : PackedBits(1) ) : 0
			// send prev_lev-new_lev
		}
	}
	bs.StoreBits(1,0); // has more data, nest out from the root
}
void Avatar::sendBuffs(BitStream &bs) const
{
	size_t num_buffs=0;
	bs.StorePackedBits(5,num_buffs);
	for(size_t idx=0; idx<num_buffs; ++idx)
	{
		sendPower(bs,0,0,0);
	}
}
void Avatar::sendOptions(BitStream &bs) const
{
	bs.StoreFloat(m_options.mouselook_scalefactor);//MouseFlt1
	bs.StoreFloat(m_options.degrees_for_turns);//MouseFlt2
	bs.StoreBits(1,m_options.mouse_invert);//MouseSet1 
	bs.StoreBits(1,0);//g_DimChatWindow 
	bs.StoreBits(1,0); //g_DimNavWindow
	bs.StoreBits(1,1);//g_ToolTips
	bs.StoreBits(1,1);//g_AllowProfanity
	bs.StoreBits(1,1);//g_ChatBalloons
	bs.StoreBits(3,0);//dword_729E58
	bs.StoreBits(3,0);//dword_729E5C
	bs.StoreBits(3,0);//dword_729E60
	bs.StoreBits(3,0);//dword_729E68
	bs.StoreBits(3,0);//dword_729E6C
	bs.StoreBits(3,0);//dword_729E70
	bs.StoreBits(3,0);//dword_729E74
	bs.StoreBits(3,0);//dword_729E78
	bs.StoreBits(3,0);//dword_729E7C
	bs.StorePackedBits(5,2);//v2 = 
//	if ( v1 >= 5 )
//	{
//		word_91A7A4 = v2;
//		word_91A7A0 = v2;
//	}

}



void MapCostume::GetCostume( BitStream &src )
{
	this->m_costume_type = 1;
	m_body_type = src.GetPackedBits(3); // 0:male normal
	a = src.GetBits(32); // rgb ?

	split.m_height = src.GetFloat();
	split.m_physique = src.GetFloat();

	m_non_default_costme_p = src.GetBits(1);
	m_num_parts = src.GetPackedBits(4);
	for(int costume_part=0; costume_part<m_num_parts;costume_part++)
	{
		CostumePart part(m_non_default_costme_p);
		part.serializefrom(src);
		m_parts.push_back(part);
	}
}

void MapCostume::dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Costume \n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    body type: 0x%08x\n"),m_body_type));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    a: 0x%08x\n"),a));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Height %f\n"),split.m_height));			
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Physique %f\n"),split.m_physique));			
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    ****** %d Parts *******\n"),m_num_parts));		
	for(int i=0; i<m_num_parts; i++)
	{
		if(m_parts[i].m_full_part)
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%s,%s,%s,%s,0x%08x,0x%08x,%s,%s\n"),m_parts[i].name_0.c_str(),
			m_parts[i].name_1.c_str(),m_parts[i].name_2.c_str(),m_parts[i].name_3.c_str(),
			m_parts[i].m_colors[0],m_parts[i].m_colors[1],
			m_parts[i].name_4.c_str(),m_parts[i].name_5.c_str()
			));		
		else
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%s,%s,%s,%s,0x%08x,0x%08x,0x%08x,0x%08x,%s,%s\n"),m_parts[i].name_0.c_str(),
			m_parts[i].name_1.c_str(),m_parts[i].name_2.c_str(),m_parts[i].name_3.c_str(),
			m_parts[i].m_colors[0],m_parts[i].m_colors[1],m_parts[i].m_colors[2],m_parts[i].m_colors[3],
			m_parts[i].name_4.c_str(),m_parts[i].name_5.c_str()
			));		
	}
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    *************\n")));
}

void MapCostume::serializefrom( BitStream &src )
{
	GetCostume(src);
}
void MapCostume::serializeto( BitStream &bs ) const
{
	SendCommon(bs);
}
class FullCostume : public MapCostume
{
public:
	void serializeto( BitStream &bs ) const
	{
		storePackedBitsConditional(bs,2,1);
		bs.StoreBits(1,0);
		if(false)
		{
		}
		bs.StoreBits(1,0);
	}
};
void MapCostume::SendCommon(BitStream &bs) const
{
	bs.StorePackedBits(3,m_body_type); // 0:male normal
	bs.StoreBits(32,a); // rgb ?

	bs.StoreFloat(split.m_height);
	bs.StoreFloat(split.m_physique);

	bs.StoreBits(1,m_non_default_costme_p);
	bs.StorePackedBits(4,m_num_parts);
	for(int costume_part=0; costume_part<m_num_parts;costume_part++)
	{
		CostumePart part=m_parts[costume_part];
		part.serializeto(bs);
	}
}