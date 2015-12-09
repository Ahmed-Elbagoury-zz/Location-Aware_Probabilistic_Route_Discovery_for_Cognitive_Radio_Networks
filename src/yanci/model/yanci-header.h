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


#ifndef YANCIHEADER_H
#define YANCIHEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include <map>
#include <vector>
#include "ns3/log.h"

namespace ns3{
namespace yanci{

struct PacketIndicator
{
	Mac48Address address;
	uint32_t pid;
	uint32_t length;
};

struct RecpReportStruct
{
	//Ipv4Address address;
	Mac48Address address;
	uint32_t lastPkt;
	uint8_t bitMap;
};
struct AckBlockStruct
{
	Mac48Address address;
	uint32_t lastAck;
	uint8_t ackMap;
};

typedef struct RecpReportStruct RecpReport;
typedef struct AckBlockStruct AckBlock;

class YanciHeader : public Header
{
public:
	YanciHeader();
	YanciHeader(uint16_t seqnum) :
		m_localPktSeqNum (seqnum)
	{}
	virtual ~YanciHeader();
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual void Print(std::ostream &os) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual uint32_t GetSerializedSize (void) const;

	//bool AddIdNexthop(const Mac48Address & nexthop, uint32_t pktId);
	bool AddPacketIndicator(const Mac48Address & nexthop, uint32_t pid, uint32_t length);
	uint16_t GetEncodedNum() const;
	void SetEncodedNum(uint16_t encodedNum);
	//std::map<Mac48Address, uint32_t> GetIdNexthops() const;
	std::vector<PacketIndicator> GetPacketsIds() const;
	bool AmINext(const Mac48Address & mac, uint32_t &pid) const;
	bool AddRecpReport(const Mac48Address & address, uint32_t lastPkt, uint8_t bitMap);
	//bool AddRecpReport(Ipv4Address srcIp, uint32_t lastPkt, uint8_t bitMap);
	bool AddAckBlock(const Mac48Address & neighbor, uint32_t lastAck, uint8_t ackMap);
	void AddAckBlock(std::vector<AckBlock> ackBlock);
	uint16_t GetLocalPktSeqNum() const { return m_localPktSeqNum; }
	void SetLocalPktSeqNum(uint16_t seq) { m_localPktSeqNum = seq; }

	bool Search(const std::vector<RecpReport> reportVec, const Mac48Address & addr, RecpReport* report = 0) const;
	//bool Search(const std::vector<RecpReport> reportVec, const Ipv4Address addr, RecpReport* report = 0) const;
	bool Search(const std::vector<AckBlock> ackVec, const Mac48Address & mac, AckBlock* ackblock = 0) const;

	std::vector<RecpReport> GetRecpReports() const { return m_receptionReports;}
	uint16_t GetReportNum() const; 
	void SetReportNum(uint16_t reportNum);
	void SetAckNum(uint16_t ackNum);
	std::vector<AckBlock> GetAckBlocks() const { return m_ackBlocks; }
	bool IsBroadcast() const;
	uint32_t GetPacketSize(uint32_t pid) const;

private:
	uint16_t m_encodedNum;
	//std::map<Mac48Address, uint32_t> m_pidNexthops;
	std::vector<PacketIndicator> m_packetsIds;
	uint16_t m_reportNum;
	std::vector<RecpReport> m_receptionReports;
	uint16_t m_ackNum;
	uint16_t m_localPktSeqNum;
	std::vector<AckBlock> m_ackBlocks;

};

}//namespace yanci
}//namespace ns3

#endif
