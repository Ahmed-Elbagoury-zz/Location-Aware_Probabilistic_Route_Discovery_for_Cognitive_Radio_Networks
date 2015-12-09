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

#include "yanci-neighbor.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE("YanciNeighbor");

namespace ns3{
namespace yanci{

Neighbor::Neighbor()
{
	NS_LOG_FUNCTION(this<<"Neighbor object constructed");
	Init();
}

Neighbor::Neighbor(const Mac48Address & mac)
{
	NS_LOG_FUNCTION(this<<"Neighbor object constructed"<<mac);
	Init();
	//m_macAddresses.push_back(mac);
	m_macAddress = mac;
}

Neighbor::Neighbor(const Ipv4Address & ip)
{
	NS_LOG_FUNCTION(this<<"Neighbor object constructed"<<ip);
	Init();
	m_ipAddress = ip;
}

Neighbor::~Neighbor()
{
	NS_LOG_FUNCTION(this<<"Neighbor object destroyed");
}
Neighbors::Neighbors(){}
Neighbors::~Neighbors(){}

void
Neighbor::Init()
{
	NS_LOG_FUNCTION(this);
	m_lastAckFromN = 0;
	m_ackMapFromN = 0;
	m_lastAckToN = 0;
	m_ackMapToN = 0;
	m_update = false;
	m_lastPkt = 0;
	m_recpMap = 0;
	m_NeighborSeqnoCounter = 0;
	//m_macAddresses.clear();
}

/*
void
Neighbor::AddMac(const Mac48Address mac)
{
	std::deque<Mac48Address>::const_iterator iter = find(m_macAddresses.begin(), m_macAddresses.end(), mac);
	if(iter != m_macAddresses.end())
		m_macAddresses.push_back(mac);
}
*/

/*
bool
Neighbor::AddVirtualQueueEntry(const QueueEntry& entry)
{}
*/

/*
QueueEntry
Neighbor::RemoveVirtualQueueEntry(bool isLong)
{
	QueueEntry entry;
	if(isLong)
	{
		entry = m_queueLong.front();
		m_queueLong.pop_front();
		return entry;
	}
	else {
		entry = m_queueShort.front();
		m_queueShort.pop_front();
		return entry;
	}
}
*/

/*
void
Neighbor::SetProbability(double prob)
{
	NS_LOG_FUNCTION_NOARGS();
	m_prob = prob;
}

double
Neighbor::GetProbability() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_prob;
}
*/

/*
QueueEntry*
Neighbor::RemoveVirtualQueueEntry(const bool isLong)
{
	NS_LOG_FUNCTION_NOARGS();
	QueueEntry* ventry;
	if(isLong)
	{
		if(!m_virtualQueueLong.size())
			return 0;
		ventry = m_virtualQueueLong.front();
		m_virtualQueueLong.pop_front();
	}
	else
	{
		if(!m_virtualQueueLong.size())
			return 0;
		ventry = m_virtualQueueShort.front();
		m_virtualQueueShort.pop_front();
	}
	return ventry;
}*/

QueueEntry*
Neighbor::RemoveVirtualQueueEntry()
{
	//NS_LOG_FUNCTION(this<<m_virtualQueue.size()<<m_macAddress);
	//PrintVirtualQueue();
	QueueEntry* ventry;
	if(!m_virtualQueue.size())
		return 0;
	ventry = m_virtualQueue.front();
	NS_LOG_FUNCTION(this<<ventry->GetDestMac()<<m_macAddress<<ventry->GetPacketId());
	NS_ASSERT(ventry->GetDestMac() == m_macAddress);
	m_virtualQueue.pop_front();
	//NS_LOG_FUNCTION(this<<m_virtualQueue.size());
	return ventry;
}

/*
void
Neighbor::RemoveVirtualQueueEntry(uint32_t packetId)
{
	NS_LOG_FUNCTION(this<<packetId<<m_virtualQueueLong.size()<<m_virtualQueueShort.size());

	std::deque<QueueEntry *>::iterator iter;
	for(iter = m_virtualQueueShort.begin(); iter != m_virtualQueueShort.end(); iter++)
	{
		if((**iter).GetPacketId() == packetId)
		{
			m_virtualQueueShort.erase(iter);
			break;
		}
	}
	for(iter = m_virtualQueueLong.begin(); iter != m_virtualQueueLong.end(); iter++)
	{
		if((**iter).GetPacketId() == packetId)
		{
			m_virtualQueueLong.erase(iter);
			break;
		}
	}
}
*/

bool
Neighbor::RemoveVirtualQueueEntry(uint32_t packetId)
{
	NS_LOG_FUNCTION(this<<packetId<<m_virtualQueue.size()<<m_macAddress);
	//PrintVirtualQueue();
	bool isRemoved = false;
	std::deque<QueueEntry *>::iterator iter;
	for(iter = m_virtualQueue.begin(); iter!= m_virtualQueue.end(); iter++)
	{
		NS_LOG_FUNCTION(this<<(**iter).GetDestMac()<<m_macAddress);
		NS_ASSERT((**iter).GetDestMac() == m_macAddress);
		if ((**iter).GetPacketId() == packetId)
		{
			m_virtualQueue.erase(iter);
			isRemoved = true;
			return isRemoved;
		}
	}
	NS_LOG_FUNCTION(this<<packetId<<m_virtualQueue.size());
	return isRemoved;
}

/*
QueueEntry*
Neighbor::GetVirtualQueueEntry(const bool isLong) const
{
	if(isLong)
		return m_virtualQueueLong.front();
	else
		return m_virtualQueueShort.front();
}
*/

QueueEntry*
Neighbor::GetVirtualQueueEntry() const
{
	if(!m_virtualQueue.size())
		return 0;
	else 
		return m_virtualQueue.front();
}

/*
void
Neighbor::AddVirtualQueueEntry(bool isLong, QueueEntry * vqe)
{
	NS_LOG_FUNCTION(this<<m_ipAddress<<isLong);
	uint32_t id = vqe->GetPacketId();
	if(isLong)
	{
		std::deque<QueueEntry *>::const_iterator iter;
		for(iter = m_virtualQueueLong.begin(); iter< m_virtualQueueLong.end(); iter++)
		{
			if((**iter).GetPacketId() == id)
				NS_FATAL_ERROR(this<<"Duplicate packet with id: "<<id);
		}
		if(iter == m_virtualQueueLong.end())
			m_virtualQueueLong.push_back(vqe);
	}
	else
	{
		std::deque<QueueEntry *>::const_iterator iter;
		for(iter = m_virtualQueueShort.begin(); iter< m_virtualQueueShort.end(); iter++)
		{
			if((**iter).GetPacketId() == id)
				NS_FATAL_ERROR(this<<"Duplicate packet with id: "<<id);
		}
		if(iter == m_virtualQueueShort.end())
			m_virtualQueueShort.push_back(vqe);
	}
}
*/

void
Neighbor::AddVirtualQueueEntryFront(QueueEntry * vqe)
{
	NS_LOG_FUNCTION(this<<m_macAddress<<vqe->GetPacketId());
	//PrintVirtualQueue();
	NS_ASSERT(m_macAddress == vqe->GetDestMac());
	uint32_t id = vqe->GetPacketId();
	std::deque<QueueEntry *>::const_iterator iter;
	for(iter = m_virtualQueue.begin(); iter != m_virtualQueue.end(); iter++)
	{
		if ((**iter).GetPacketId() == id)
			NS_FATAL_ERROR(this<<"Duplicate packet with id: "<<id);
	}
	if(iter == m_virtualQueue.end())
		m_virtualQueue.push_front(vqe);
}

void
Neighbor::AddVirtualQueueEntry(QueueEntry * vqe)
{
	NS_LOG_FUNCTION(this<<m_macAddress<<vqe->GetPacketId());
	//PrintVirtualQueue();
	NS_ASSERT(m_macAddress == vqe->GetDestMac());
	uint32_t id = vqe->GetPacketId();
	std::deque<QueueEntry *>::const_iterator iter;
	for(iter = m_virtualQueue.begin(); iter != m_virtualQueue.end(); iter++)
	{
		if ((**iter).GetPacketId() == id)
			NS_FATAL_ERROR(this<<"Duplicate packet with id: "<<id);
	}
	if(iter == m_virtualQueue.end())
		m_virtualQueue.push_back(vqe);
}

void
Neighbor::SetIp(const Ipv4Address & addr)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ipAddress = addr;
}

bool
Neighbor::operator< (const Neighbor& neighbor) const
{
	return m_ipAddress<neighbor.m_ipAddress;
}

bool
Neighbor::operator== (const Neighbor& neighbor) const
{
	return m_ipAddress == neighbor.m_ipAddress;
}

void
Neighbor::SetSeqCounter(uint32_t seq)
{
	m_NeighborSeqnoCounter = seq;
}

void
Neighbor::IncSeqCounter()
{
	SetSeqCounter(GetSeqCounter()+1);
}

bool
Neighbor::SetLastAckFromN(uint32_t last, uint8_t map)
{
	NS_LOG_FUNCTION(this<<last<<m_lastAckFromN);
	if(last > m_lastAckFromN)
	{
		m_lastAckFromN = last;
		m_ackMapFromN = map;
		return true;
	}
	else 
		return false;
	//m_lastAckFromN = last > m_lastAckFromN ? last : m_lastAckFromN;
	//return m_lastAckFromN;
}

/*
uint8_t
Neighbor::SetAckMapFromN(uint8_t map)
{
	m_ackMapFromN = map;
	return m_ackMapFromN;
}
*/

void
Neighbor::SetLastAckToN(uint32_t last)
{
	NS_LOG_FUNCTION(this<<m_lastAckToN<<(int)m_ackMapToN<<last);
	if(last <= m_lastAckToN)
		return ;
	m_ackMapToN <<= 1;
	m_ackMapToN++;
	m_ackMapToN <<= (last - m_lastAckToN - 1);
	m_lastAckToN = last;
}

/*
void
Neighbor::SetAckMapToN(uint8_t map)
{
	m_ackMapToN = map;
}
*/

/*
bool
Neighbor::DoUpdate(uint32_t lastPkt, uint8_t map)
{
	if(lastPkt < m_lastPkt)
	{
		if(lastPkt < m_lastPkt - 7)
			return false;
		else
		{
			m_recpMap = m_recpMap | (1 << (m_lastPkt - lastPkt));
			m_recpMap = m_recpMap | (map << (m_lastPkt - lastPkt));
		}
	}
	else
	{
		m_recpMap <<= lastPkt - m_lastPkt;
		//m_recpMap++;
		m_recpMap = map | m_recpMap;
		m_lastPkt = lastPkt;
		m_update = true;
	}
	return true;
}
*/

bool
Neighbor::DoUpdate(uint32_t lastPkt)
{
	if(lastPkt < m_lastPkt)
	{
		if(lastPkt < m_lastPkt - 8)
			return false;
		else
			m_recpMap = m_recpMap | (1 << (m_lastPkt - lastPkt  - 1));
	}
	else
	{
		//m_recpMap = m_recpMap | 1;
		m_recpMap <<= 1;
		m_recpMap |= 1;
		m_recpMap <<= (lastPkt - m_lastPkt - 1);
		m_lastPkt = lastPkt;
		m_update = true;
	}
	return true;
}

bool
Neighbor::IsUpdated() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_update;
}

void
Neighbor::ResetUpdate()
{
	m_update = false;
}

void
Neighbor::SetMac(const Mac48Address & mac)
{
	m_macAddress = mac;
}

/*
bool
Neighbor::HasMac(const Mac48Address mac) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::deque<Mac48Address>::const_iterator iter = find (m_macAddresses.begin(), m_macAddresses.end(), mac);
	if (iter != m_macAddresses.end())
		return true;
	else return false;
}

void
Neighbor::PrintMac(std::ostream& os) const
{
	std::deque<Mac48Address>::const_iterator iter;
	for(iter = m_macAddresses.begin(); iter != m_macAddresses.end(); iter++)
	{
		os<<(*iter)<<" ";
	}
	os<<std::endl;
}*/

/*****************************************
 * Neighbors
 * **************************************/

void
Neighbors::Print(std::ostream& os) const
{
	NS_LOG_FUNCTION(this);
	os<<"Printing out all "<<m_neighbors.size()<<" neighbors: \n";
	std::deque<Neighbor>::const_iterator iter;
	for(iter = m_neighbors.begin(); iter!= m_neighbors.end(); iter++)
	{
		os<<iter->GetIp()<<" ";
		os<<iter->GetMac()<<std::endl;
		//std::deque<Mac48Address>::const_iterator maciter;
		//iter->PrintMac(os);
	}
}

bool
Neighbors::AddNeighbor(const Neighbor& neighbor)
{
	NS_LOG_FUNCTION(this);
	std::deque<Neighbor>::const_iterator iter;
	for(iter = m_neighbors.begin(); iter != m_neighbors.end(); iter++)
		if(iter->GetMac() == neighbor.GetMac())
			return false;
	m_neighbors.push_back(neighbor);
	return true;
}

bool
Neighbors::AddNeighbor(Neighbor& neighbor, const Ipv4Address & ipAddr, const Ipv4Mask & mask)
{
	NS_LOG_FUNCTION(this<<ipAddr);
	if(!ipAddr.IsMulticast() && !ipAddr.IsBroadcast() && !ipAddr.IsLocalMulticast() && !ipAddr.IsSubnetDirectedBroadcast(mask))
	{
		neighbor.SetIp(ipAddr);
		//m_neighbors.insert(neighbor);
		m_neighbors.push_back(neighbor);
		return true;
	}
	else
		return false;
}

void
Neighbors::RemoveNeighbor(const Ipv4Address & ip)
{
	std::deque<Neighbor>::iterator iter;
	for(iter = m_neighbors.begin(); iter!=m_neighbors.end(); iter++)
		if(iter->GetIp().IsEqual(ip))
			m_neighbors.erase(iter);
}

/*
bool
Neighbors::RemoveNeighbor(const Neighbor& neighbor)
{
	NeighborIterator iter = SearchNeighbor(neighbor);
	if(iter == m_neighbors.end())
		return false;
	else 
	{
		m_neighbors.erase(iter);
		return true;
	}
}

Neighbors::NeighborIterator
Neighbors::SearchNeighbor(const Ipv4Address ip) const
{
	NS_LOG_FUNCTION_NOARGS();
	NeighborIterator iter;
	NS_LOG_DEBUG("Neighbors::SearchNeighbor.\tEntering for loop");
	for(iter = m_neighbors.begin(); iter != m_neighbors.end(); iter++)
	{
		if(iter->GetIp().IsEqual(ip))
		{
			NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tFound neighbor. Returning from for loop.");
			return iter;
		}
	}
	NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tNot found neighbor. Returning from for loop.");
	return iter;
}
*/
int32_t
Neighbors::SearchNeighbor(const Ipv4Address & ip) const
{
	NS_LOG_FUNCTION_NOARGS();
	for(int32_t i = 0; i<(int)m_neighbors.size(); i++)
	{
		if (m_neighbors[i].GetIp().IsEqual(ip))
			return i;
	}
	return -1;
}

int32_t
Neighbors::SearchNeighbor(const Mac48Address & mac) const
{
	NS_LOG_FUNCTION(mac);
	//Print(std::cout);
	std::deque<Neighbor>::const_iterator neighborIter;
	int32_t pos = 0;
	for(neighborIter = m_neighbors.begin(); neighborIter != m_neighbors.end(); neighborIter++)
	{
		if(neighborIter->GetMac() == mac)
			return pos;
		else pos++;
		/*
		NS_LOG_DEBUG((pos+1)<<" time in SearchNeighbor for loop");
		bool found = neighborIter->HasMac(mac);
		if(found)
		{
			NS_LOG_DEBUG("Found neighbor");
			return pos;
		}
		else
		{
			NS_LOG_DEBUG("Not found and continue to next neighbor");
			pos++;
		}
		*/
	}
	NS_LOG_DEBUG("Not found and return "<<mac);
	return -1;
}
/*
bool
Neighbors::SearchNeighbor(const Ipv4Address ip, Neighbor & neighbor) const
{
	NS_LOG_FUNCTION_NOARGS();
	NeighborIterator iter;
	NS_LOG_DEBUG("Neighbors::SearchNeighbor.\tEntering for loop");
	for(iter = m_neighbors.begin(); iter != m_neighbors.end(); iter++)
	{
		if(iter->GetIp().IsEqual(ip))
		{
			NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tFound neighbor. Returning from for loop.");
			neighbor = *iter;
			return true;
		}
		else
			NS_LOG_DEBUG("Ip not eq: "<<ip<<iter->GetIp());
	}
	NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tNot found neighbor. Returning from for loop.");
	return false;
}
Neighbors::NeighborIterator
Neighbors::SearchNeighbor(const Mac48Address mac) const
{
	NS_LOG_FUNCTION_NOARGS();
	NeighborIterator iter;
	NS_LOG_DEBUG("Neighbors::SearchNeighbor.\tEntering for loop");
	for(iter = m_neighbors.begin(); iter != m_neighbors.end(); iter++)
	{
		//if (iter->GetMac() == mac)
		std::deque<Mac48Address>::iterator qeIter;
		for(qeIter = iter->GetMac().begin(); qeIter != iter->GetMac().end(); qeIter++)
			if((*qeIter) == mac)
			{
				NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tFound neighbor. Returning from for loop.");
				return iter;
			}
	}
	NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tNot found neighbor. Returning from for loop.");
	return iter;
}
bool
Neighbors::SearchNeighbor(const Mac48Address mac, Neighbor & neighbor) const
{
	NS_LOG_FUNCTION_NOARGS();
	NeighborIterator iter;
	NS_LOG_DEBUG("Neighbors::SearchNeighbor.\tEntering for loop");
	for(iter = m_neighbors.begin(); iter != m_neighbors.end(); iter++)
	{
		//if (iter->GetMac() == mac)
		std::deque<Mac48Address>::iterator qeIter;
		for(qeIter = iter->GetMac().begin(); qeIter != iter->GetMac().end(); qeIter++)
			if((*qeIter) == mac)
			{
				NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tFound neighbor. Returning from for loop.");
				neighbor = *iter;
				return true;
			}
	}
	NS_LOG_DEBUG("Neighbor::SearchNeighbor().\tNot found neighbor. Returning from for loop.");
	return false;
}
*/

//std::set<Neighbor>
/*
std::deque<Neighbor>
Neighbors::GetNeighborSet() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_neighbors;
}
*/

/*
Neighbor
Neighbors::SearchNeighbor(const Neighbor neighbor) const
{
	return *(m_neighors.find(neighbor));
}
*/

std::ostream &
operator<<(std::ostream & os, const Neighbor& neighbor)
{
	//uint8_t *mac = new uint8_t[6];
	
	os<< "This neighbor's ip is: ";
	neighbor.m_ipAddress.Print(os);

	/*
	os<< "Whis neighbor's mac is: ";
	neighbor.m_macAddress.CopyTo(mac);
	for(int i = 0; i<6; i++)
		os<<mac[i]<<":";
		*/
	os<<std::endl;

	os<<"This neighbor's seq_num_counter is: " << neighbor.m_NeighborSeqnoCounter << std::endl;

	return os;
}

Neighbors::NeighborIterator
Neighbors::At(int32_t pos)
{
	NS_LOG_FUNCTION_NOARGS();
	return m_neighbors.begin()+pos;
}

void
Neighbor::PrintVirtualQueue() const
{
	NS_LOG_FUNCTION(this<<"The length of the long queue is "<<m_virtualQueue.size());
	NS_LOG_FUNCTION(this<<"Now printing out all dest macs");
	std::deque<QueueEntry *>::const_iterator iter;
	for(iter = m_virtualQueue.begin(); iter != m_virtualQueue.end(); iter++)
	{
		NS_LOG_FUNCTION(this<<"printing all virtual queue entry: "<<(*iter)->GetDestMac()<<(*iter)->GetPacketId());
	}
}

}//namespace yanci
}//namespace ns3
