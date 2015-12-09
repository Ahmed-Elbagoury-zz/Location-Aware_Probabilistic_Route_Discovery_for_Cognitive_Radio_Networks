/*
 * Copyright (c) 2012 Yang CHI, CDMC, University of Cincinnati
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Yang CHI <chiyg@mail.uc.edu>
 */

#include "yanci-packet-info.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("YanciPacketInfo");

namespace ns3{
namespace yanci{

PacketInfo::PacketInfo(){}
PacketInfo::~PacketInfo(){}

/*
uint32_t
PacketInfo::Hash(Ipv4Address address, uint32_t seq_no) const
{
	uint32_t addressInt = address.Get();
	uint64_t key = addressInt << 32 + seq_no;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}
*/

/*
double
PacketInfo::GetProbability(uint32_t packetId, Ipv4Address ip) const
{
}

double
PacketInfo::GetProbability(uint32_t packetId, Mac48Address mac) const
{}
*/
/*
double
PacketInfo::GetProbability(uint32_t packetId, Neighbor neighbor) const
{
	NS_LOG_FUNCTION_NOARGS();
	//ProbabilityDeque probDeque;
	Probability prob;
	std::map<uint32_t, Probability>::const_iterator iter = m_packetInfo.find(packetId);
	//std::map<uint32_t, ProbabilityDeque>::const_iterator iter = m_packetInfo.find(packetId);

	if(iter == m_packetInfo.end())
		return -1;
	else
		prob = (*iter).second;

	Probability::const_iterator probIter = prob.find(neighbor);
	if(probIter == prob.end())
		return -2;
	else
		return (*probIter).second;
}
*/

/*
void
PacketInfo::SetProbability(uint32_t packetId, Neighbor neighbor, double probability)
{
}
*/

bool
PacketInfo::GetItem(uint32_t packetId, const Mac48Address & mac)
//PacketInfo::GetItem(uint32_t packetId, Ipv4Address ip)
{
	//std::map<uint32_t, Ipv4Address>::const_iterator iter = m_packetInfo.find(packetId);
	std::map<uint32_t, Mac48Address>::const_iterator iter = m_packetInfo.find(packetId);
	if(iter == m_packetInfo.end())
		return false;
	//if (iter->second == ip)
	if(iter->second == mac)
		return true;
	else
		return false;
}

/*
void
PacketInfo::SetItem(const YanciHeader & header, const Mac48Address & mac)
{
	std::map<Mac48Address, uint32_t> idnext = header.GetIdNexthops();
	std::map<Mac48Address, uint32_t>::const_iterator id_iter;
	for(id_iter = idnext.begin(); id_iter != idnext.end(); id_iter++)
	{
		m_packetInfo.insert(std::make_pair(id_iter->second, mac));
	}
}
*/

void
PacketInfo::SetItem(const YanciHeader & header, const Mac48Address & mac)
{
	std::vector<PacketIndicator> packetsIds = header.GetPacketsIds();
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = packetsIds.begin(); iter != packetsIds.end(); iter++)
		m_packetInfo.insert(std::make_pair(iter->pid, iter->address));
}

void
//PacketInfo::SetItem(uint32_t packetId, Ipv4Address ip)
PacketInfo::SetItem(uint32_t packetId, const Mac48Address & mac)
{
	//NS_LOG_FUNCTION(this<<packetId<<ip);
	NS_LOG_FUNCTION(this<<packetId<<mac);
	//m_packetInfo.insert(std::make_pair(packetId, ip));
	m_packetInfo.insert(std::make_pair(packetId, mac));
}

void
PacketInfo::Print(std::ostream &os) const
{
	//std::map<uint32_t, Ipv4Address>::const_iterator iter;
	std::map<uint32_t, Mac48Address>::const_iterator iter;
	for(iter = m_packetInfo.begin(); iter != m_packetInfo.end(); iter++)
	{
		os<<iter->second << " has "<<iter->first<<"\n";
	}
}

}//namespace yanci
}//namespace ns3
