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

#ifndef YANCIQUEUE_H
#define YANCIQUEUE_H

#include "ns3/packet.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include <deque>
#include <list>
#include "yanci-header.h"
#include "yanci-hash.h"
#include "ns3/log.h"

namespace ns3{
namespace yanci{

class QueueEntry
{
public:
	QueueEntry();
	QueueEntry(Ptr<Packet> packet);
	~QueueEntry();
	//uint32_t Hash(Ipv4Address address, uint32_t seq_no) const;
	//uint32_t Hash() const;
	void SetPacketId();
	void SetPacketId(uint32_t id);
	inline uint32_t GetPacketId() const { return m_packetId; }
	/*
	inline void SetSequence(uint32_t seq) { m_ipSeqNo = seq; }
	inline uint32_t GetSequence() const { return m_ipSeqNo; }
	*/
	inline void SetSequence(uint16_t seq) { m_ipSeqNo = seq; }
	inline uint16_t GetSequence() const { return m_ipSeqNo; }
	inline uint32_t Size() const {return m_packet->GetSize(); }
	inline void SetPacket(Ptr<Packet> packet) { m_packet = packet->Copy(); }
	Ptr<Packet> GetPacket() const;
	inline void SetMacHeader(const WifiMacHeader & macHeader) { m_macHeader = macHeader; }
	WifiMacHeader GetMacHeader() const { return m_macHeader; }
	void SetYanciHeader(const YanciHeader & yanciHeader) { m_yanciHeader = yanciHeader; }
	YanciHeader GetYanciHeader() const { return m_yanciHeader; }
	inline bool operator< (const QueueEntry& entry) const { return m_packetId < entry.m_packetId; }
	QueueEntry& operator= (const QueueEntry& ent);
	inline void SetProtocolNumber(const uint16_t num) { m_protocolNumber = num; }
	inline uint16_t GetProtocolNumber() const { return m_protocolNumber; }

	void SetSrcMac(const Mac48Address & addr);
	Mac48Address GetSrcMac() const; 
	void SetDestMac(const Mac48Address & addr);
	Mac48Address GetDestMac() const;
	Ipv4Address GetIPSrc() const;
	void SetIPSrc(const Ipv4Address & addr);
	Ipv4Header GetIpHeader() const;
	void SetIpHeader(const Ipv4Header & header);
	void SetNexthop(const Ipv4Address & addr);
	void SetNexthop();
	Ipv4Address GetNexthop() const;

private:
	Ptr<Packet> m_packet;
	WifiMacHeader m_macHeader;
	Mac48Address m_srcMac;
	Mac48Address m_destMac;
	Ipv4Address m_ipAddress; // src ip addr
	Ipv4Address m_ipDest; //dest ip addr
	Ipv4Header m_ipHeader;
	YanciHeader m_yanciHeader;
	//uint32_t m_ipSeqNo;
	uint16_t m_ipSeqNo;
	uint32_t m_packetId;
	uint16_t m_protocolNumber;
};

class Queue
{
public:
	Queue();
	~Queue();
	bool EnqueueBack(const QueueEntry & entry);
	bool EnqueueFront(const QueueEntry & entry);
	bool Erase(uint32_t pid);
	//bool Erase(Mac48Address mac, uint32_t ipSeq);
	bool Erase(const Ipv4Address & ip, uint16_t ipSeq);
	bool Erase(const Mac48Address & mac, uint16_t ipSeq);
	bool Erase(const Ipv4Address & ip, const Mac48Address & mac, uint16_t seq);
	void Clean(const Mac48Address & mac, uint16_t seq);
	QueueEntry Dequeue();
	QueueEntry Front() const;
	inline uint32_t Size() const { return m_queue.size(); }
	QueueEntry* LastPosition() ;
	QueueEntry* FirstPosition() ;
	//QueueEntry At(int pos) const ;
	void Print(std::ostream &os) const;
private:
	//std::deque<QueueEntry> m_queue;
	std::list<QueueEntry> m_queue;
	uint32_t m_max;
};

}//namespace yanci
}//namespace ns3

#endif
