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

#ifndef YANCINEIGHBOR_H
#define YANCINEIGHBOR_H

#include "ns3/log.h"
#include <deque>
#include "ns3/ipv4-address.h"
#include "ns3/wifi-mac-header.h"
#include "yanci-queue.h"
#include <set>
#include <deque>

namespace ns3{
namespace yanci{

class Neighbor
{
public:
	Neighbor();
	Neighbor(const Mac48Address & mac);
	Neighbor(const Ipv4Address & ip);
	~Neighbor();
	/*
	QueueEntry* RemoveVirtualQueueEntry(bool isLong);
	QueueEntry* GetVirtualQueueEntry(const bool isLong) const;
	void AddVirtualQueueEntry(bool isLong, QueueEntry * vqe);
	*/
	void AddVirtualQueueEntry(QueueEntry * vqe);
	void AddVirtualQueueEntryFront(QueueEntry * vqe);

	QueueEntry* GetVirtualQueueEntry() const;
	QueueEntry* RemoveVirtualQueueEntry();
	bool RemoveVirtualQueueEntry(uint32_t packetId);
	Ipv4Address GetIp() const { return m_ipAddress; }
	void SetIp(const Ipv4Address & addr );
	Mac48Address GetMac() const { return m_macAddress; }
	void SetMac(const Mac48Address & mac) ;
	uint32_t GetSeqCounter() const {return m_NeighborSeqnoCounter; }
	void SetSeqCounter(uint32_t seq);
	void IncSeqCounter();
	bool operator< (const Neighbor& neighbor) const;
	bool operator== (const Neighbor& neighbor) const;
	/*
	double GetProbability() const;
	void SetProbability( double prob );
	*/
	//uint32_t SetLastAckFromN( uint32_t ack);
	bool SetLastAckFromN(uint32_t last, uint8_t map);
	uint32_t GetLastAckFromN() const { return m_lastAckFromN; }
	//uint8_t SetAckMapFromN(uint8_t ackMap); 
	uint8_t GetAckMapFromN() const { return m_ackMapFromN; }
	void SetLastAckToN (uint32_t last);
	uint32_t GetLastAckToN () const { return m_lastAckToN; }
	uint8_t GetAckMapToN() const { return m_ackMapToN; }
	//void SetAckMapToN(uint8_t map);

	//for reception list:
	//bool DoUpdate(uint32_t lastPkt, uint8_t map);
	bool DoUpdate(uint32_t last);
	bool IsUpdated() const;
	void ResetUpdate();
	inline uint32_t GetLastPkt() const { return m_lastPkt; }
	inline uint8_t GetRecpMap() const { return m_recpMap; }
	void PrintVirtualQueue() const; //(std::ostream& os) const;

private:
	void Init();
	friend std::ostream & operator<< (std::ostream & os, const Neighbor& neighbor);

	Ipv4Address m_ipAddress;
	Mac48Address m_macAddress;
	uint32_t m_NeighborSeqnoCounter;
	//double m_prob;
	//for Acks
	uint32_t m_lastAckFromN;
	uint8_t m_ackMapFromN;
	uint32_t m_lastAckToN;
	uint8_t m_ackMapToN;
	//for Reception reports:
	bool m_update;
	uint32_t m_lastPkt; //the last pkt received from this neighbor!
	uint8_t m_recpMap;

	/*
	std::deque<QueueEntry *> m_virtualQueueShort;
	std::deque<QueueEntry *> m_virtualQueueLong;
	*/
	std::deque<QueueEntry *> m_virtualQueue;
};

std::ostream & operator<< (std::ostream & os, const Neighbor& neighbor);


class Neighbors
{
public:
	Neighbors();
	~Neighbors();
	
	//typedef std::set<Neighbor>::iterator NeighborIterator;
	typedef std::deque<Neighbor>::iterator NeighborIterator;

	bool AddNeighbor(const Neighbor& neighbor);
	bool AddNeighbor(Neighbor& neighbor, const Ipv4Address & ipAddr, const Ipv4Mask & mask);
	//bool RemoveNeighbor(const Neighbor& neighbor);
	void RemoveNeighbor(const Ipv4Address & ip);
	//NeighborIterator SearchNeighbor(const Ipv4Address ip) const;
	//NeighborIterator SearchNeighbor(const Mac48Address mac) const;
	//inline NeighborIterator SearchNeighbor(const Neighbor neighbor) const {return m_neighbors.find(neighbor); }
	//bool SearchNeighbor(const Ipv4Address ip, Neighbor & neighbor) const;
	//bool SearchNeighbor(const Mac48Address ip, Neighbor & neighbor) const;
	uint32_t Size() const { return m_neighbors.size(); }
	void Print(std::ostream& os) const ;
	int32_t SearchNeighbor(const Ipv4Address & ip) const;
	int32_t SearchNeighbor(const Mac48Address & mac) const;
	NeighborIterator At(int32_t pos);

	/*
	Neighbor SearchNeighbor(const Ipv4Address ip) const;
	Neighbor SearchNeighbor(const Mac48Address mac) const;
	Neighbor SearchNeighbor(const Neighbor neighbor) const;
	*/
	//std::deque<Neighbor> GetNeighborSet() const;

private:
	//std::set<Neighbor> m_neighbors;
	std::deque<Neighbor> m_neighbors;
};

}//namespace yanci
}//namespace ns3

#endif
