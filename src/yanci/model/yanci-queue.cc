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
 *
 * Authors: Yang CHI <chiyg@mail.uc.edu>
 */

#include "yanci-queue.h"

NS_LOG_COMPONENT_DEFINE("YanciQueue");

namespace ns3{
namespace yanci{

QueueEntry::QueueEntry()
{
	NS_LOG_FUNCTION(this);
	m_packet = Create<Packet>();
}
QueueEntry::QueueEntry(Ptr<Packet> packet)
{	
	NS_LOG_FUNCTION(this<<packet);
	m_packet = packet->Copy();
}
QueueEntry::~QueueEntry(){}
Queue::Queue()
{
	NS_LOG_FUNCTION(this<<"Queue constructed");
	m_max = 400;
}
Queue::~Queue(){}

bool
Queue::EnqueueBack(const QueueEntry & entry)
{
	if(m_queue.size() == m_max)
	{
		NS_LOG_FUNCTION(this<<"Queue full"<<m_queue.size());
		return false;
	}
	std::list<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
		if(iter->GetPacketId() == entry.GetPacketId())
			return false;
	m_queue.push_back(entry);
	NS_LOG_FUNCTION(this<<m_queue.size()<<entry.GetPacketId()<<entry.GetSequence());
	return true;
}

bool
Queue::EnqueueFront(const QueueEntry & entry)
{
	if(m_queue.size() == m_max)
	{
		NS_LOG_FUNCTION(this<<"Queue full"<<m_queue.size());
		return false;
	}
std::list<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
		if(iter->GetPacketId() == entry.GetPacketId())
			return false;
	m_queue.push_front(entry);
	NS_LOG_FUNCTION(this<<m_queue.size());
	return true;
}

QueueEntry *
Queue::LastPosition() 
{
	NS_LOG_FUNCTION_NOARGS();
	//return &(m_queue.at(m_queue.size() - 1));
	return &(m_queue.back());
}

QueueEntry *
Queue::FirstPosition() 
{
	NS_LOG_FUNCTION_NOARGS();
	//return &(m_queue.at(0));
	return &(m_queue.front());
}

QueueEntry
Queue::Dequeue()
{
	QueueEntry entry;
	Mac48Address mac;
	if (!m_queue.empty())
	{
		entry = m_queue.front();
		mac = entry.GetDestMac();
		NS_LOG_FUNCTION(this<<mac<<entry.GetPacketId()<<m_queue.size());
		m_queue.pop_front();
	}
	return entry;
}

QueueEntry
Queue::Front() const
{
	QueueEntry entry;
	Mac48Address mac;
	if (!m_queue.empty())
	{
		entry = m_queue.front();
		mac = entry.GetDestMac();
		NS_LOG_FUNCTION(this<<mac<<entry.GetPacketId()<<m_queue.size());
	}
	return entry;
}

bool
Queue::Erase(uint32_t pid)
{
	NS_LOG_FUNCTION(this<<pid);
	//Print(std::cout);
	std::list<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter!= m_queue.end(); iter++)
	{
		if(iter->GetPacketId() == pid)
		{
			m_queue.erase(iter);
			return true;
		}
	}
	return false;
}

/*
bool
Queue::Erase(Mac48Address mac, uint32_t ipSeq)
{
	std::deque<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
	{
		if(((*iter).GetMacHeader().GetAddr2() == mac) && ((*iter).GetSequence() == ipSeq))
		{
			m_queue.erase(iter);
			return true;
		}
	}
	return false;
}
*/

bool
Queue::Erase(const Ipv4Address & ip, uint16_t ipSeq)
{
	NS_LOG_FUNCTION(this<<ip<<ipSeq);
	//std::deque<QueueEntry>::iterator iter;
	std::list<QueueEntry>::iterator iter;

	for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
	{
		if(!iter->GetSequence())
			iter->SetSequence(iter->GetIpHeader().GetIdentification());
		if(iter->GetNexthop() == ip && iter->GetSequence() == ipSeq)
		{
			m_queue.erase(iter);
			return true;
		}
	}
	return false;
}

bool
Queue::Erase(const Mac48Address & mac, uint16_t ipSeq)
{
	NS_LOG_FUNCTION(this<<mac<<ipSeq);
	std::list<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
	{
		if(iter->GetDestMac() == mac && iter->GetSequence() == ipSeq)
		{
			m_queue.erase(iter);
			return true;
		}
	}
	return false;
}

bool
Queue::Erase(const Ipv4Address & ip, const Mac48Address & mac, uint16_t seq)
{
	NS_LOG_FUNCTION(this<<ip<<mac<<seq);
	std::list<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
	{
		if(iter->GetSequence() == seq && (iter->GetDestMac() == mac || iter->GetNexthop() == ip))
		{
			m_queue.erase(iter);
			return true;
		}
	}
	return false;
}

void
Queue::Clean(const Mac48Address & mac, uint16_t seq)
{
	NS_LOG_FUNCTION(this<<mac<<seq<<m_queue.size());
	std::list<QueueEntry>::iterator iter;
	for(iter = m_queue.begin(); iter != m_queue.end(); )
	{
		if(iter->GetDestMac() == mac && iter->GetSequence() < seq)
			iter = m_queue.erase(iter);
		else
			iter++;
	}
}

/*
uint32_t
QueueEntry::Hash(Ipv4Address address, uint32_t seq_no) const
{
	uint32_t addressInt = address.Get();
	uint64_t key = addressInt;
	key = (key << 32) + seq_no;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}

uint32_t
QueueEntry::Hash() const
{
	return Hash(m_ipHeader.GetSource(), m_ipSeqNo);
}
*/

void
QueueEntry::SetSrcMac(const Mac48Address & addr)
{
	NS_LOG_FUNCTION(this);
	m_srcMac = addr;
}

void
QueueEntry::SetDestMac(const Mac48Address & addr)
{
	NS_LOG_FUNCTION(this);
	m_destMac = addr;
}

Mac48Address
QueueEntry::GetSrcMac() const
{
	return m_srcMac;
}

Mac48Address
QueueEntry::GetDestMac() const
{
	return m_destMac;
}
Ipv4Address
QueueEntry::GetIPSrc() const
{
	return m_ipAddress;
}

void
QueueEntry::SetIPSrc(const Ipv4Address & addr)
{
	NS_LOG_FUNCTION(this);
	m_ipAddress = addr;
}

Ipv4Header
QueueEntry::GetIpHeader() const
{
	return m_ipHeader;
}

void
QueueEntry::SetIpHeader(const Ipv4Header & header)
{
	NS_LOG_FUNCTION(this);
	m_ipHeader = header;
	m_ipSeqNo = header.GetIdentification();
	m_ipAddress = header.GetSource();
	m_ipDest = header.GetDestination();
}

void
QueueEntry::SetNexthop()
{
	NS_LOG_FUNCTION(this);
	m_ipDest = m_ipHeader.GetDestination();
}

void
QueueEntry::SetNexthop(const Ipv4Address & addr)
{
	m_ipDest = addr;
}

Ipv4Address
QueueEntry::GetNexthop() const
{
	return m_ipDest;
}

Ptr<Packet>
QueueEntry::GetPacket() const
{
	NS_LOG_FUNCTION(this);
	return m_packet;
}

void
QueueEntry::SetPacketId()
{
	NS_LOG_FUNCTION(this);
	m_packetId = Hash(m_ipAddress, m_ipSeqNo);
	//m_packetId = Hash(m_srcMac, m_ipSeqNo);
}

void
QueueEntry::SetPacketId(uint32_t id)
{
	NS_LOG_FUNCTION(this);
	m_packetId = id;
}

/*
QueueEntry
Queue::At(int pos) const
{
	return m_queue.at(pos);
}
*/

QueueEntry&
QueueEntry::operator= (const QueueEntry& ent)
{
	this->m_packet = ent.m_packet;
	this->m_macHeader = ent.m_macHeader;
	this->m_ipAddress= ent.m_ipAddress;
	this->m_ipDest = ent.m_ipDest;
	this->m_ipHeader = ent.m_ipHeader;
	this->m_yanciHeader = ent.m_yanciHeader;
	this->m_ipSeqNo = ent.m_ipSeqNo;
	this->m_packetId = ent.m_packetId;
	this->m_protocolNumber = ent.m_protocolNumber;
	this->m_srcMac = ent.m_srcMac;
	this->m_destMac = ent.m_destMac;

	return *this;
}

void
Queue::Print(std::ostream &os) const
{
	NS_LOG_FUNCTION_NOARGS();
	os<<"Current queue size is "<<m_queue.size()<<std::endl;
	//std::deque<QueueEntry>::const_iterator iter;
	std::list<QueueEntry>::const_iterator iter;
	for(iter = m_queue.begin(); iter!= m_queue.end(); iter++)
	{
		os<<iter->GetPacketId()<<" "<<iter->GetSequence()<<" "<<iter->GetDestMac()
			<< " "<<iter->GetNexthop()<<std::endl;
	}
	os.flush();
}

}//namespace yanci
}//namespace ns3
