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


#include "yanci-header.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE("YanciHeader");

namespace ns3
{
namespace yanci
{

YanciHeader::YanciHeader()
{
	NS_LOG_FUNCTION_NOARGS();
	m_encodedNum = 0;
	m_reportNum = 0;
	m_ackNum = 0;
}
YanciHeader::~YanciHeader(){}

TypeId
YanciHeader::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::yanci::YanciHeader")
		.SetParent<Header>()
		.AddConstructor<YanciHeader>()
		;
	return tid;
}

TypeId
YanciHeader::GetInstanceTypeId(void) const
{
	return GetTypeId();
}

void
YanciHeader::Print (std::ostream &os) const
{
	os << "ENCODED NUM " << m_encodedNum << ", ";
	/*
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{
		os << (*iter).second << ", " << (*iter).first << ", ";
	}
	*/
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = m_packetsIds.begin(); iter != m_packetsIds.end(); iter++)
	{
		os << (*iter).address << ", "<<(*iter).pid<<", "<<(*iter).length<<", ";
	}
	os << "REPORT NUM " << m_reportNum << ", ";
	std::vector<RecpReport>::const_iterator RecpRepIter;
	for(RecpRepIter = m_receptionReports.begin(); RecpRepIter != m_receptionReports.end(); RecpRepIter++)
	{
		os << (*RecpRepIter).address << ", " << (*RecpRepIter).lastPkt << ", " << (int)(*RecpRepIter).bitMap << ", ";
	}

	os << "ACK NUM " << m_ackNum << ", ";
	os << "LOCAL PKT SEQ NUM " << m_localPktSeqNum << ", ";
	std::vector<AckBlock>::const_iterator ackBlockIter;
	for(ackBlockIter = m_ackBlocks.begin(); ackBlockIter != m_ackBlocks.end(); ackBlockIter++)
	{
		os << (*ackBlockIter).address << ", " << (*ackBlockIter).lastAck << ", " << (int)(*ackBlockIter).ackMap ;
	}
}

void
YanciHeader::Serialize(Buffer::Iterator start) const
{
	start.WriteHtonU16 (m_encodedNum);
	/*
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{	
		WriteTo(start, (*iter).first);
		start.WriteHtonU32 ((*iter).second);
	}
	*/
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = m_packetsIds.begin(); iter != m_packetsIds.end(); iter++)
	{
		WriteTo(start, iter->address);
		start.WriteHtonU32(iter->pid);
		start.WriteHtonU32(iter->length);
	}
	start.WriteHtonU16 (m_reportNum);
	std::vector<RecpReport>::const_iterator RecpRepIter;
	for(RecpRepIter = m_receptionReports.begin(); RecpRepIter != m_receptionReports.end(); RecpRepIter++)
	{
		WriteTo(start, (*RecpRepIter).address);
		start.WriteHtonU32 ((*RecpRepIter).lastPkt);
		start.WriteU8 ((*RecpRepIter).bitMap);
	}

	start.WriteHtonU16(m_ackNum);
	start.WriteHtonU16(m_localPktSeqNum);
	std::vector<AckBlock>::const_iterator ackBlockIter;
	for(ackBlockIter = m_ackBlocks.begin(); ackBlockIter != m_ackBlocks.end(); ackBlockIter++)
	{
		WriteTo(start, (*ackBlockIter).address);
		start.WriteHtonU32((*ackBlockIter).lastAck);
		start.WriteU8((*ackBlockIter).ackMap);
	}
}

uint32_t
YanciHeader::GetSerializedSize() const
{
	return 2 //m_encodedNum, uint16_t
		//+ (6+4)*(uint32_t)m_pidNexthops.size()//(4+4)*(uint32_t)m_pidNexthops.size() // packet id -> 4, nexthop -> 4
		+ (6+4+4)*(uint32_t)m_packetsIds.size()// packet id -> 4, nexthop -> 4, packet length -> 4
		+ 2 // m_reportNum, uint16_t
		//+ (4+4+1)*(uint32_t)m_receptionReports.size() //
		+ (6+4+1)*(uint32_t)m_receptionReports.size()
		+ 2 //m_ackNum, uint16_t
		+ 2 //4 //m_localPktSeqNum
		+ (6+4+1)*(uint32_t)m_ackBlocks.size();
}

uint32_t
YanciHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator bufIter = start;
	m_encodedNum = bufIter.ReadNtohU16();
	//m_pidNexthops.clear();
	m_packetsIds.clear();
	//uint32_t pktId;
	//Mac48Address add;
	PacketIndicator indi;
	for(int i = 0; i<m_encodedNum; i++)
	{	
		//ReadFrom(bufIter, add);
		ReadFrom(bufIter, indi.address);
		//pktId = bufIter.ReadNtohU32();
		indi.pid = bufIter.ReadNtohU32();
		indi.length = bufIter.ReadNtohU32();
		//m_pidNexthops.insert(std::make_pair(add, pktId));
		m_packetsIds.push_back(indi);
	}

	m_reportNum = bufIter.ReadNtohU16();
	RecpReport report;
	m_receptionReports.clear();
	for(int i = 0; i<m_reportNum; i++)
	{
		ReadFrom(bufIter, report.address);
		report.lastPkt = bufIter.ReadNtohU32();
		report.bitMap = bufIter.ReadU8();
		m_receptionReports.push_back(report);
	}

	m_ackNum = bufIter.ReadNtohU16();
	m_localPktSeqNum = bufIter.ReadNtohU16();
	AckBlock ack;
	m_ackBlocks.clear();
	for(int i = 0; i<m_ackNum; i++)
	{
		ReadFrom(bufIter, ack.address);
		ack.lastAck = bufIter.ReadNtohU32();
		ack.ackMap = bufIter.ReadU8();
		m_ackBlocks.push_back(ack);
	}

	uint32_t dist = bufIter.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	return dist;
}

uint16_t
YanciHeader::GetReportNum() const
{
	NS_LOG_FUNCTION(this<<m_reportNum<<m_receptionReports.size());
	return m_reportNum;
}

/*
bool
YanciHeader::AddIdNexthop(const Mac48Address & nexthop, uint32_t pktId)
{
	NS_LOG_FUNCTION(this<<m_pidNexthops.size());
	if(m_pidNexthops.find(nexthop) != m_pidNexthops.end())
		return false;
	NS_ASSERT(GetEncodedNum() < 65536);//2^16 = 65536
	m_pidNexthops.insert(std::make_pair(nexthop, pktId));
	m_encodedNum++;
	return true;
}
*/

bool
YanciHeader::AddPacketIndicator(const Mac48Address & address, uint32_t pid, uint32_t length)
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = m_packetsIds.begin(); iter != m_packetsIds.end(); iter++)
	{
		if(iter->address == address)
			return false;
	}
	NS_ASSERT(GetEncodedNum() < 65536);
	PacketIndicator indi = {address, pid, length};
	m_packetsIds.push_back(indi);
	m_encodedNum++;
	return true;
}

uint16_t
YanciHeader::GetEncodedNum() const
{
	NS_LOG_FUNCTION_NOARGS();
	//NS_ASSERT(m_pidNexthops.size() == m_encodedNum);
	NS_ASSERT(m_packetsIds.size() == m_encodedNum);
	return m_encodedNum;
}

void
YanciHeader::SetEncodedNum(uint16_t encodedNum)
{
	m_encodedNum = encodedNum;
}

/*
std::map<Mac48Address, uint32_t>
YanciHeader::GetIdNexthops() const
{
	return m_pidNexthops;
}
*/

std::vector<PacketIndicator>
YanciHeader::GetPacketsIds() const
{
	return m_packetsIds;
}

bool
YanciHeader::AmINext(const Mac48Address & mac, uint32_t &pid) const
{
	/*
	std::map<Mac48Address, uint32_t>::const_iterator iter = m_pidNexthops.find(mac);
	if(iter == m_pidNexthops.end())
		return false;
	else{
		pid = iter->second;
		return true;
	}*/
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = m_packetsIds.begin(); iter != m_packetsIds.end(); iter++)
	{
		if(iter->address == mac)
		{
			pid = iter->pid;
			return true;
		}
	}
	return false;
}

bool
YanciHeader::IsBroadcast() const
{
	/*
	std::map<Mac48Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{
		if(iter->first.IsBroadcast())
		{
			NS_ASSERT(m_pidNexthops.size() == 1);
			return true;
		}
	}
	return false;
	*/
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = m_packetsIds.begin(); iter != m_packetsIds.end(); iter++)
	{
		if(iter->address.IsBroadcast())
		{
			//NS_ASSERT(m_pidNexthops.size() == 1);
			NS_ASSERT(m_packetsIds.size() == 1);
			return true;
		}
	}
	return false;
}

/*
bool
YanciHeader::AmINext(Ipv4Address & add, uint32_t & pid) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::map<Ipv4Address, uint32_t>::const_iterator iter = m_pidNexthops.find(add);
	if(iter == m_pidNexthops.end())
		return false;
	else{
		pid = iter->second;
		return true;
	}
}
*/

/*
bool
YanciHeader::AmINext(Ipv4Mask & mask, uint32_t & pid) const
{
	NS_LOG_FUNCTION(this<<mask);
	std::map<Ipv4Address, uint32_t>::const_iterator iter;
	for(iter = m_pidNexthops.begin(); iter != m_pidNexthops.end(); iter++)
	{
		if(iter->first.IsBroadcast() || iter->first.IsMulticast() || iter->first.IsLocalMulticast() || iter->first.IsSubnetDirectedBroadcast(mask))
		{
			pid = iter->second;
			return true;
		}
	}
	return false;
}
*/

/*
template <typename StructT, typename AddT>
bool
YanciHeader::Search(const std::vector<StructT> vec, const AddT add) const
{
	typename std::vector<StructT>::const_iterator iter;
	for(iter = vec.begin(); iter != vec.end(); iter++)
	{
		if(iter->address == add)
			return true;
	}
	return false;
}
*/

/*
template<typename StructT, typename AddT>
bool
YanciHeader::Search(const std::vector<StructT> vec, const AddT add, StructT* block) const
{
	NS_LOG_FUNCTION_NOARGS();
	typename std::vector<StructT>::const_iterator iter;
	for(iter = vec.begin(); iter != vec.end(); iter++)
	{
		if(iter->address == add)
		{
			if(block)
				*block = *iter;
			return true;
		}
	}
	return false;
}

template<typename StructT, typename AddT>
bool
YanciHeader::Search(const std::vector<StructT> vec, const std::vector<AddT> add, StructT* block) const
{
	NS_LOG_FUNCTION_NOARGS();
	typename std::vector<StructT>::const_iterator iter;
	typename std::vector<AddT>::const_iterator addIter;
	for(iter = vec.begin(); iter != vec.end(); iter++)
	{
		//if(iter->address == add)
		if( (addIter = find(add.begin(), add.end(), iter->address)) != add.end() )
		{
			if(block)
				*block = *iter;
			return true;
		}
	}
	return false;
}
*/

bool
YanciHeader::Search(const std::vector<RecpReport> reportVec, const Mac48Address & addr, RecpReport* report) const
//YanciHeader::Search(const std::vector<RecpReport> reportVec, const Ipv4Address addr, RecpReport* report) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<RecpReport>::const_iterator iter;
	for(iter = reportVec.begin(); iter!= reportVec.end(); iter++)
	{
		if(iter->address == addr)
		{
			if(report)
				*report = *iter;
			return true;
		}
	}
	return false;
}

/*
bool
YanciHeader::Search(const std::vector<AckBlock> ackVec, const std::vector<Mac48Address> macs, AckBlock* ackblock) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<Mac48Address>::const_iterator macIter;
	std::vector<AckBlock>::const_iterator iter;
	for(iter = ackVec.begin(); iter != ackVec.end(); iter++)
	{
		if ( (macIter = find(macs.begin(), macs.end(), iter->address)) != macs.end())
		{
			if(ackblock)
				*ackblock = *iter;
			return true;
		}
	}
	return false;
}
*/

bool
YanciHeader::Search(const std::vector<AckBlock> ackVec, const Mac48Address & mac, AckBlock* ackblock) const
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<Mac48Address>::const_iterator macIter;
	std::vector<AckBlock>::const_iterator iter;
	for(iter = ackVec.begin(); iter != ackVec.end(); iter++)
	{
		if(iter->address == mac)
		{
			if(ackblock)
				*ackblock = *iter;
			return true;
		}
	}
	return false;
}

/*
bool
YanciHeader::Search(const Mac48Address mac) const
{
	return _Search(m_ackBlocks, mac);
}

bool
YanciHeader::Search(const Ipv4Address ip) const
{
	return _Search(m_receptionReports, ip);
}

AckBlock
YanciHeader::ExtractAck(const Mac48Address mac) const
{
	std::vector<AckBlock>::const_iterator iter;
	for(iter = m_ackBlocks.begin(); iter != m_ackBlocks.end(); iter++)
		if(iter->address == mac)
			return *iter;
}

RecpReport
YanciHeader::ExtractRecp(const Ipv4Address ip) const
{
	std::vector<RecpReport>::const_iterator iter;
	for(iter = m_receptionReports.begin(); iter != m_receptionReports.end(); iter++)
		if(iter->address == ip)
			return *iter;
}
*/

bool
YanciHeader::AddRecpReport(const Mac48Address & address, uint32_t lastPkt, uint8_t bitMap)
//YanciHeader::AddRecpReport(Ipv4Address address, uint32_t lastPkt, uint8_t bitMap)
{
	if(Search(m_receptionReports, address))
		return false;
	RecpReport recpReport;
	recpReport.address = address;
	recpReport.lastPkt = lastPkt;
	recpReport.bitMap = bitMap;
	m_receptionReports.push_back(recpReport);
	m_reportNum++;
	NS_ASSERT(m_reportNum == m_receptionReports.size());
	return true;
}

bool
YanciHeader::AddAckBlock(const Mac48Address & address, uint32_t lastAck, uint8_t ackMap)
{
	if(Search(m_ackBlocks, address))
		return false;
	AckBlock ackBlock;
	ackBlock.address = address;
	ackBlock.lastAck = lastAck;
	ackBlock.ackMap = ackMap;
	m_ackBlocks.push_back(ackBlock);
	m_ackNum++;
	return true;
}

/*
void
YanciHeader::SetLocalPktSeqNum(uint32_t seq)
{
	m_localPktSeqNum = seq;
}
*/

void
YanciHeader::SetReportNum(uint16_t reportNum)
{
	NS_LOG_FUNCTION_NOARGS();
	m_reportNum = reportNum;
}

void
YanciHeader::SetAckNum(uint16_t ackNum)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ackNum = ackNum;
}

void
YanciHeader::AddAckBlock(std::vector<AckBlock> ackBlock)
{
	NS_LOG_FUNCTION_NOARGS();
	m_ackBlocks = ackBlock;
}

uint32_t
YanciHeader::GetPacketSize(uint32_t pid) const
{
	std::vector<PacketIndicator>::const_iterator iter;
	for(iter = m_packetsIds.begin(); iter != m_packetsIds.end(); iter++)
	{
		if(iter->pid == pid)
			return iter->length;
	}
	return 0;
}

/*
void
YanciHeader::SetLocalPktSeqNum(Neighbor neighbor)
{
	SetLocalPktSeqNum(neighbor.GetSeqCounter());
}
*/

}//namespace yanci
}//namespace ns3
