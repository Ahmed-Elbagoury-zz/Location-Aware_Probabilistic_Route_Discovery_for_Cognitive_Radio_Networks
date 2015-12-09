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

#include "yanci-protocol.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/wifi-mac-queue.h"
#include <stdlib.h>
#include <stdio.h>

NS_LOG_COMPONENT_DEFINE ("YanciProtocol");

namespace ns3{
namespace yanci{

YanciProtocol::YanciProtocol() :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try(Timer::CANCEL_ON_DESTROY)
{
	NS_LOG_FUNCTION(this<<"default constructor");
	m_rtimeout = MilliSeconds(25.0);
	//m_arq = true;
	Init();
}

YanciProtocol::YanciProtocol(double rttime) :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try (Timer::CANCEL_ON_DESTROY)
{
	NS_LOG_FUNCTION(this<<"Constructor with 1 arg: "<<rttime);
	m_rtimeout = MilliSeconds(rttime);
	//m_arq = true;
	Init();
}

YanciProtocol::YanciProtocol(double rttime, double retry) :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try (Timer::CANCEL_ON_DESTROY)
{
	NS_LOG_FUNCTION(this<<"Constructor with 1 arg: "<<rttime);
	m_rtimeout = MilliSeconds(rttime);
	m_ttimeout = Seconds(retry);
	Init();
}

/*
YanciProtocol::YanciProtocol(bool arq) :
	m_timer(Timer::CANCEL_ON_DESTROY)
{
	m_rtimeout = MilliSeconds(25.0);
	m_arq = arq;
	Init();
}*/

/*
YanciProtocol::YanciProtocol(bool arq, double rttime) :
	m_timer(Timer::CANCEL_ON_DESTROY), m_try (Timer::CANCEL_ON_DESTROY)
{
	NS_LOG_FUNCTION(this<<"Constructor with 2 args: "<<arq<<rttime);
	m_rtimeout = MilliSeconds(rttime);
	//m_arq = arq;
	Init();
}
*/

YanciProtocol::~YanciProtocol()
{
	NS_LOG_FUNCTION(this<<"Protocol object destroyed");
}

TypeId
YanciProtocol::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::yanci::YanciProtocol")
		.SetParent<Object> ();
	return tid;
}

void
YanciProtocol::Init()
{
	NS_LOG_FUNCTION_NOARGS();
	m_isSending = false;
	//m_ttimeout = MilliSeconds(m_rtimeout.GetMilliSeconds()/2);

	/*
	Ptr<WifiNetDevice> wifiDevice = m_device->GetInterface(0)->GetObject<WifiNetDevice>();
	Ptr<RegularWifiMac> wifiMac = wifiDevice->GetMac()->GetObject<RegularWifiMac>();
	Ptr<DcaTxop> txop = wifiMac->GetDcaTxop();
	txop->SetMaxQueueSize(10);
	*/

	//if(m_arq)
	{
		m_timer.SetDelay(m_rtimeout);
		m_timer.SetFunction(&YanciProtocol::Retransmit, this);
		m_try.SetDelay(m_ttimeout);
		m_try.SetFunction(&YanciProtocol::TrySend, this);
		m_try.Schedule();
	}

}

bool
YanciProtocol::Enqueue(Ptr<Packet> packet, const Mac48Address& src, const Mac48Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION(this<<src<<dest<<m_queue.Size());
	//uint32_t m_sequence;
	uint16_t sequence;
	QueueEntry entry(packet);
	Ipv4Header ipHeader;
	if(!packet->PeekHeader(ipHeader))
		return false;
	entry.SetIpHeader(ipHeader);
	entry.SetNexthop();
	entry.SetSrcMac(src);
	entry.SetDestMac(dest);
	entry.SetProtocolNumber(protocolNumber);
	entry.SetIPSrc(ipHeader.GetSource());
	//m_sequence = (ipHeader.GetIdentification() << 16) + ipHeader.GetFragmentOffset();
	sequence = ipHeader.GetIdentification();
	entry.SetSequence(sequence);
	entry.SetPacketId(Hash(ipHeader.GetSource(), sequence));
	//entry.SetPacketId(Hash(src, m_sequence));
	NS_LOG_DEBUG("A packet with id: "<<entry.GetPacketId()<<" and seq:"<<entry.GetSequence()<<" is enqueued.");
	if (m_queue.EnqueueBack(entry))
	{
		NS_ASSERT(entry.GetPacketId() == (*(m_queue.LastPosition())).GetPacketId());

		Neighbors::NeighborIterator neighborIter;
		if(!dest.IsBroadcast())
		{
			int32_t neighborPos = m_neighbors.SearchNeighbor(dest);
			if(neighborPos < 0)
			{
				Neighbor tmpNeighbor(dest);
				tmpNeighbor.AddVirtualQueueEntry(m_queue.LastPosition());
				m_neighbors.AddNeighbor(tmpNeighbor);
			}
			else
			{
				neighborIter = m_neighbors.At(neighborPos);
				neighborIter->AddVirtualQueueEntry(m_queue.LastPosition());
			}
		}
		m_pool.AddToPool(entry.GetPacketId(), packet, sequence);
		//m_pool.AddToPool(entry.GetPacketId(), packet);
	}
	TrySend();
	return true;
}

void
YanciProtocol::TrySend()
{
	NS_LOG_FUNCTION(this<<m_ip<<m_queue.Size());
	Ptr<WifiNetDevice> wifiDevice = m_device->GetInterface(0)->GetObject<WifiNetDevice>();
	/*
	Ptr<NetDevice> netDevice = m_device->GetInterface(0);
	NS_LOG_DEBUG("Get NetDevice");
	Ptr<WifiNetDevice> wifiDevice = netDevice->GetObject<WifiNetDevice>();
	NS_LOG_DEBUG("Get Wifi Device");
	*/

	Ptr<RegularWifiMac> wifiMac = wifiDevice->GetMac()->GetObject<RegularWifiMac>();
	//Ptr<DcaTxop> txop = wifiMac->GetDcaTxop();
	PointerValue ptr;
	wifiMac->GetAttribute("DcaTxop", ptr);
	Ptr<DcaTxop> txop = ptr.Get<DcaTxop>();
	Ptr<WifiMacQueue> wifiMacQueue = txop->GetQueue();

	if(wifiMacQueue->GetSize() == wifiMacQueue->GetMaxSize())
	//if (txop->IsQueueFull())
	{
		NS_LOG_FUNCTION(this<<m_ip<<"MAC queue is full.");
		/*
		//if(m_try.GetDelayLeft().IsZero() || !m_try.IsRunning())
		{
			NS_LOG_LOGIC("Reschedule");
			m_try.Cancel();
			m_try.Schedule();
		}
		*/
		return TrySendSchedule();
	}
	else
	{
		/*
		NS_LOG_FUNCTION("Queue not full: "<<txop->GetQueueSize()<<" and out queue: "<<m_queue.Size());
		if (txop->GetQueueSize() == 0 && m_queue.Size() > 0)
			NS_LOG_FUNCTION(this<<"Warning: losing bandwidth");
		if(txop->GetQueueSize() == 0 && m_queue.Size() > 1)
			NS_LOG_FUNCTION(this<<"Warning: losing serious bandwidth");
			*/
	}
	//if(!m_isSending && (m_queue.Size() || m_ackBlockList.size()))
	if(!m_isSending && m_queue.Size())
		DoSend();
	//m_try.Schedule();
	TrySendSchedule();
}

void
YanciProtocol::TrySendSchedule()
{
	m_try.Cancel();
	m_try.Schedule();
}

void
YanciProtocol::DoSend()
{
	NS_LOG_FUNCTION(this<<m_ip<<m_queue.Size()<<m_rtqueue.Size()<<m_ackBlockList.size());
	m_isSending = true;
	Ptr<Packet> packet = Create<Packet>();
	YanciHeader header;
	Neighbors::NeighborIterator neighborIter;
	QueueEntry entry;
	bool hasPayload = m_queue.Size();
	//Mac48Address destMac;

	if(hasPayload)
	{
		//Dequeue Head of the output queue
		entry = m_queue.Front();
		//header.AddIdNexthop(entry.GetDestMac(), entry.GetPacketId());
		header.AddPacketIndicator(entry.GetDestMac(), entry.GetPacketId(), entry.Size());

		if(!entry.GetDestMac().IsBroadcast())
		{
			int32_t neighborPos = m_neighbors.SearchNeighbor(entry.GetDestMac());
			NS_ASSERT(neighborPos > -1);
			if (! m_neighbors.At(neighborPos)->RemoveVirtualQueueEntry(entry.GetPacketId()))
				NS_FATAL_ERROR("Removed failed");
		}

		m_queue.Dequeue();

		bool encoded = false;
		if(m_queue.Size() > 0 && !entry.GetDestMac().IsBroadcast())
		{
			encoded = Encode(entry, packet, header);
			header.Print(std::cout);
			fflush(stdout);
		}
		if(!encoded)
			packet = entry.GetPacket()->Copy();

		header.SetLocalPktSeqNum(entry.GetSequence());
	}
	else
		header.SetLocalPktSeqNum(0);

	//Add reception reports
	uint16_t counter = 0;
	uint32_t neighborSize = m_neighbors.Size();
	for(uint32_t i = 0; i<neighborSize; i++)
	{
		neighborIter = m_neighbors.At(i);
		if (neighborIter->IsUpdated())
		{
			//counter++;
			if(header.AddRecpReport(neighborIter->GetMac(), neighborIter->GetLastPkt(), neighborIter->GetRecpMap()))
				//if(header.AddRecpReport(neighborIter->GetIp(), neighborIter->GetLastPkt(), neighborIter->GetRecpMap()))
			{
				counter++;
				neighborIter->ResetUpdate();
			}
			else
			{
				NS_LOG_FUNCTION(this<<"RecpReport wasn't added."<<neighborIter->GetIp());
				header.Print(std::cout);
				fflush(stdout);
			}
		}
	}
	header.SetReportNum(counter);

	//Add acks to header
	//if(m_arq)
	{
		NS_LOG_LOGIC("Add "<<m_ackBlockList.size()<<" acks to header");
		header.SetAckNum(m_ackBlockList.size());
		if(m_ackBlockList.size())
		{
			NS_LOG_FUNCTION(this<<"add ack blocks"<<m_ackBlockList.size());
			header.AddAckBlock(m_ackBlockList);
			NS_LOG_FUNCTION(this<<"added ack blocks");
			m_ackBlockList.clear();
		}
	}

	//forward down
	packet->AddHeader(header);
	NS_LOG_FUNCTION(this<<"WifiNetDevice about to send:");
	packet->Print(std::cout);
	fflush(stdout);
	if(hasPayload)
		m_device->ForwardDown(packet, entry.GetDestMac(), entry.GetProtocolNumber());
	else 
	{
		//if(m_arq)
	//		m_device->ForwardDown(packet, destMac, Ipv4L3Protocol::PROT_NUMBER);
			/*
		else
		{
			m_isSending = false;
			return;
		}
		*/
	}

	//if(m_arq)
	{
		if(m_timer.GetDelayLeft().IsZero() || !m_timer.IsRunning())
		{
			NS_LOG_FUNCTION(this<<"Delay left is zero");
			m_timer.Schedule();
			NS_LOG_FUNCTION(m_timer.IsRunning());
		}
	}
	m_isSending = false;
	//TrySend();
}

void
YanciProtocol::Recv(Ptr<NetDevice> netDevice, Ptr<const Packet> pkt, uint16_t protocol, const Address & sender, const Address & receiver, NetDevice::PacketType packetType)
{
	NS_LOG_FUNCTION(this<<m_ip);
	uint32_t pid;
	uint16_t sequence;
	//uint32_t sequence;
	Mac48Address sMac = Mac48Address::ConvertFrom(sender);
	Mac48Address destMac = Mac48Address::ConvertFrom(receiver);
	Ipv4Header ipHeader;
	YanciHeader header;
	Ipv4Address ipAddr;

	Ptr<Packet> packet = pkt->Copy();
	packet->RemoveHeader(header);
	uint32_t encodedNum = header.GetEncodedNum();
	NS_LOG_FUNCTION(this<<"ENCODED NUM "<<encodedNum);
	int64_t isDecodable;

	if (encodedNum)
	{
		if(encodedNum > 1 )
		{
			isDecodable = Decode(header, packet, sequence);
			if(isDecodable >= 0 )
			{
				if(header.GetPacketSize(isDecodable))
				{
					packet = Shrink(packet, header.GetPacketSize(isDecodable));
				}
				else
					NS_FATAL_ERROR("Can't find the packet with pid: "<<isDecodable);
				//packet->PeekHeader(ipHeader);
				packet->RemoveHeader(ipHeader);
				ipHeader.SetTtl(64);
				packet->AddHeader(ipHeader);
				sequence = ipHeader.GetIdentification();
				ipAddr = ipHeader.GetSource();
				pid = Hash(ipAddr, header.GetLocalPktSeqNum()); 
			}
		}
		else 
		{
			packet->PeekHeader(ipHeader);
			sequence = ipHeader.GetIdentification();
			ipAddr = ipHeader.GetSource();
			pid = Hash(ipAddr, header.GetLocalPktSeqNum()); 
		}
	}

	Neighbor sNeighbor(sMac);

	int32_t neighborPos = 0;
	neighborPos = m_neighbors.SearchNeighbor(sMac);
	if(neighborPos < 0)
	{
		m_neighbors.AddNeighbor(sNeighbor);
	}
	m_neighbors.Print(std::cout);
	fflush(stdout);

	//acks to me
	//if(m_arq)
	{
		AckBlock ackBlock;
		NS_LOG_FUNCTION(this<<"Before update acks. the size of rt queue is "<<m_rtqueue.Size());
		if(header.Search(header.GetAckBlocks(), Mac48Address::ConvertFrom(m_device->GetAddress()), &ackBlock))
		{
			neighborPos = m_neighbors.SearchNeighbor(sMac);
			NS_ASSERT(neighborPos >= 0);
			Neighbors::NeighborIterator neighborIter = m_neighbors.At(neighborPos);
			/*
			   uint32_t last = neighborIter->SetLastAckFromN(ackBlock.lastAck);
			   uint8_t map = neighborIter->SetAckMapFromN(ackBlock.ackMap);
			   */
			neighborIter->SetLastAckFromN(ackBlock.lastAck, ackBlock.ackMap);
			uint32_t last = neighborIter->GetLastAckFromN();
			uint8_t map = neighborIter->GetAckMapFromN();
			NS_LOG_FUNCTION(this<<"Newly acked: "<<ackBlock.lastAck<<(int)ackBlock.ackMap);
			uint8_t i = 1;
			while(map)
			{
				if(map%2)
				{
					if(encodedNum)
						m_rtqueue.Erase(ipAddr, sMac, last-i);
					else 
						m_rtqueue.Erase(sMac, last-i);
				}
				map >>= 1;
				i++;
			}

			m_rtqueue.Clean(sMac, last-i+1);

			if(encodedNum)
				m_rtqueue.Erase(ipAddr, sMac, last);
			else
				m_rtqueue.Erase(sMac, last);
		}
		NS_LOG_FUNCTION(this<<"after update acks. the size of rt queue is "<<m_rtqueue.Size());
	}

	//Update PacketInfo based on all acks in this header:
	std::vector<AckBlock> allAcks = header.GetAckBlocks();
	std::vector<AckBlock>::const_iterator allAckIter;
	for(allAckIter = allAcks.begin(); allAckIter != allAcks.end(); allAckIter++)
	{
		uint32_t last = allAckIter->lastAck;
		uint8_t map = allAckIter->ackMap;
		m_packetInfo.SetItem(last, sMac);
		uint8_t i = 1;
		while(map)
		{
			if(map%2)
			{
				m_packetInfo.SetItem(last-i, sMac);
			}
			map>>=1;
			i++;
		}
	}

	//Extract Reception Reports. Update Neighbor's State
	uint16_t reportNum = header.GetReportNum();
	if(reportNum)
	{
		std::vector<RecpReport> recpReports = header.GetRecpReports();
		NS_ASSERT(reportNum == recpReports.size());
		std::vector<RecpReport>::const_iterator iter;
		for(iter = recpReports.begin(); iter != recpReports.end(); iter++)
		{
			//if(iter->address == m_ip)
			//	continue;
			pid = Hash((*iter).address, (*iter).lastPkt);
			m_packetInfo.SetItem(pid, sMac);
			uint8_t map = (*iter).bitMap;
			uint8_t i = 1;
			while(map)
			{
				if(map%2)
				{
					pid = Hash((*iter).address, (*iter).lastPkt - i);
					m_packetInfo.SetItem(pid, sMac);
				}
				map >>= 1;
				i++;
			}
		}
	}

	if(encodedNum)
	{
		if(encodedNum > 1)
		{
			if(isDecodable >= 0 )
			{
				if (header.IsBroadcast())
				{
					NS_LOG_FUNCTION(this<<"Decoded. broadcast");
					m_device->ForwardUp(packet, protocol, sMac, destMac, packetType);
				}
				else
				{
					Neighbors::NeighborIterator iter = m_neighbors.At(m_neighbors.SearchNeighbor(sMac));
					if(header.AmINext(Mac48Address::ConvertFrom(m_device->GetAddress()), pid))
					{
						NS_LOG_FUNCTION(this<<"Decoded. for me");
						NS_ASSERT((isDecodable - pid) == 0);
						iter->SetLastAckToN(sequence);
						AckBlock ackBlock;
						ackBlock.address = sMac;
						ackBlock.lastAck = iter->GetLastAckToN();
						ackBlock.ackMap = iter->GetAckMapToN();
						AddAck(ackBlock);
						m_device->ForwardUp(packet, protocol, sMac, destMac, packetType);
					}
					else
					{
						NS_LOG_LOGIC("I'm not nexthop in either case.");
						pid = Hash(ipAddr, header.GetLocalPktSeqNum()); 
					}
					iter->DoUpdate(sequence);
					m_pool.AddToPool(pid, packet, sequence);
					m_packetInfo.SetItem(header, sMac);
				}
			}
		}
		else
		{
			AckBlock ackBlock;
			if (header.IsBroadcast())
			{
				NS_LOG_FUNCTION(this<<"Not encoded. broadcast");
				m_device->ForwardUp(packet, protocol, sMac, destMac, packetType);
			}
			else
			{
				if(header.AmINext(Mac48Address::ConvertFrom(m_device->GetAddress()), pid))
				{
					NS_LOG_FUNCTION(this<<"Not encoded. for me");
					m_device->ForwardUp(packet, protocol, sMac, destMac, packetType);
				}
				else
				{
					NS_LOG_LOGIC("No, i'm not");
					pid = Hash(ipAddr, header.GetLocalPktSeqNum()); 
				}
				Neighbors::NeighborIterator iter = m_neighbors.At(m_neighbors.SearchNeighbor(sMac));
				iter->DoUpdate(sequence);
				m_pool.AddToPool(pid, packet, sequence);
				m_packetInfo.SetItem(pid, sMac);
			}
		}
	}
	TrySend();
	return;
}

/*
 * \returns -1 if we need more packets to decode it. 0 if we have every packet. a positive pid if it's decodable and it's decoded.
 */
int64_t
YanciProtocol::Decode(const YanciHeader & header, Ptr<Packet> & packet, uint16_t & sequence)
//YanciProtocol::Decode(const YanciHeader & header, Ptr<Packet> & packet)
{
	NS_LOG_FUNCTION(this);
	uint32_t pid = 0;
	int32_t tempSeq;
	uint16_t seqFromHeader = header.GetLocalPktSeqNum();

	//std::map<Mac48Address, uint32_t> encodedInfo = header.GetIdNexthops();
	//std::map<Mac48Address, uint32_t>::const_iterator iter;
	std::vector<PacketIndicator> packetIds = header.GetPacketsIds();
	std::vector<PacketIndicator>::const_iterator iter;
	uint8_t missing = 0;
	uint8_t found = 0;
	//for(iter = encodedInfo.begin(); iter != encodedInfo.end(); iter++)
	for(iter = packetIds.begin(); iter != packetIds.end(); iter++)
	{
		Ptr<Packet> foundPkt;
		//tempSeq = m_pool.Find(iter->second, foundPkt);
		tempSeq = m_pool.Find(iter->pid, foundPkt);
		//if(!m_pool.Find(iter->second, foundPkt))
		if(tempSeq == -1)
		{
			if(++missing > 1)
			{
				NS_LOG_FUNCTION(this<<"Decoding failed: more than one pkt missing");
				sequence = 0;
				return -1;
			}
			//pid = iter->second;
			pid = iter->pid;
		}
		else
		{
			//NS_LOG_FUNCTION(this<<"found in pool: "<<iter->second<<foundPkt->GetSize());
			NS_LOG_FUNCTION(this<<"found in pool: "<<iter->pid<<foundPkt->GetSize());
			//if(++found < encodedInfo.size())
			if(++found < packetIds.size())
			{
				packet = XOR(foundPkt, packet);
				sequence = (uint16_t)tempSeq ^ seqFromHeader;
			}
			else
				break;
		}
	}
	//if(missing == 0 && found == encodedInfo.size())
	if(missing == 0 && found == packetIds.size())
	{
		NS_LOG_FUNCTION(this<<"Decoding failed: all pkts found");
		return -2;
	}
	//else if(missing == 1 && found == (encodedInfo.size() -1))
	else if(missing == 1 && found == (packetIds.size() -1))
	{
		NS_LOG_FUNCTION(this<<"Decoding succeeded."<<pid<<"size: "<<packet->GetSize());
		return pid;
	}
	else
		NS_FATAL_ERROR("Impossible decoding result");
}

Ptr<Packet>
YanciProtocol::Shrink(Ptr<Packet> packet, uint32_t size)
{
	NS_LOG_FUNCTION_NOARGS();
	uint8_t *buffer = new uint8_t[size];
	packet->CopyData(buffer, size);
	Ptr<Packet> newPacket = Create<Packet>(buffer, size);
	delete [] buffer;
	return newPacket;
}

void
YanciProtocol::Retransmit()
{
	if(!m_rtqueue.Size())
		return;
	NS_LOG_FUNCTION(this<<m_ip<<m_rtqueue.Size());
	QueueEntry entry = m_rtqueue.Dequeue();
	if (m_queue.EnqueueFront(entry))
	{
		NS_ASSERT((*(m_queue.FirstPosition())).GetPacketId() == entry.GetPacketId());
		int32_t neighborPos = m_neighbors.SearchNeighbor(entry.GetDestMac());
		NS_ASSERT(neighborPos >= 0);
		Neighbors::NeighborIterator neighborIter = m_neighbors.At(neighborPos);
		neighborIter->AddVirtualQueueEntryFront(m_queue.FirstPosition());
	}
	m_timer.Schedule();
	NS_LOG_LOGIC("Entering TrySend from Retransmit");
	TrySend();
}

bool
YanciProtocol::Encode(QueueEntry & entry, Ptr<Packet> & packet, YanciHeader & yanciHeader)
{
	NS_LOG_FUNCTION(this<<m_ip<<m_queue.Size());
	QueueEntry newEntry = entry;
	//std::set<Ipv4Address> m_nexthops;
	std::set<Mac48Address> m_nexthops;
	std::set<uint32_t> m_natives;
	Neighbors::NeighborIterator neighborIter;
	bool isEncoded = false;
	QueueEntry* virtualQueueEntry;
	uint32_t packetId = entry.GetPacketId();
	bool capable = true;

	m_nexthops.insert(entry.GetDestMac());
	m_natives.insert(packetId);
	
	uint32_t neighborSize = m_neighbors.Size();
	for(uint32_t i= 0; i<neighborSize; i++)
	{
		neighborIter = m_neighbors.At(i);
		capable = true;
		NS_LOG_FUNCTION(this<<m_ip<<"Actually in the loop");
		std::cout<<neighborIter->GetMac()<<std::endl;
		if(m_nexthops.find(neighborIter->GetMac()) != m_nexthops.end())
			continue;
		virtualQueueEntry = neighborIter->GetVirtualQueueEntry();

		if(!virtualQueueEntry)
			continue;
		NS_LOG_DEBUG("packet id retrived through virtual queue entry: "<<virtualQueueEntry->GetPacketId());

		std::set<Mac48Address>::const_iterator iterNexthops;
		for(iterNexthops = m_nexthops.begin(); iterNexthops != m_nexthops.end(); iterNexthops++)
		{
			NS_LOG_FUNCTION(this<<"In the first inner loop to check if nexthops have the new pkt");
			if(!m_packetInfo.GetItem(virtualQueueEntry->GetPacketId(), *iterNexthops))
			{
				capable = false;
				break;
			}
		}
		std::set<uint32_t>::iterator id_iter;
		for(id_iter = m_natives.begin(); id_iter !=m_natives.end(); id_iter++)
		{
			NS_LOG_FUNCTION(this<<"In the second inner loop to check if this neighbor has all natives");
			if(!m_packetInfo.GetItem(*id_iter, neighborIter->GetMac()))
			{
				capable = false;
				break;
			}
		}
		if(!capable)
			continue;
		NS_LOG_DEBUG("Before enter XOR:");
		newEntry.SetPacket(XOR(entry.GetPacket(), (*virtualQueueEntry).GetPacket()));
		newEntry.SetSequence(entry.GetSequence() ^ virtualQueueEntry->GetSequence());
		
		m_nexthops.insert(neighborIter->GetMac());
		m_natives.insert(virtualQueueEntry->GetPacketId());
		QueueEntry rte = *virtualQueueEntry;
		//if(m_arq)
			m_rtqueue.EnqueueBack(rte);
		isEncoded = true;
		NS_LOG_FUNCTION(this<<"ENCODED!!"<<Simulator::Now().GetSeconds());
		//if (!yanciHeader.AddIdNexthop(virtualQueueEntry->GetDestMac(), virtualQueueEntry->GetPacketId()))
		if(!yanciHeader.AddPacketIndicator(virtualQueueEntry->GetDestMac(), virtualQueueEntry->GetPacketId(), virtualQueueEntry->Size()))
			NS_FATAL_ERROR("IdNexthop not added "<<virtualQueueEntry->GetDestMac());
		//remove the native packet from the output queue
		if (!m_queue.Erase(virtualQueueEntry->GetPacketId()))
			NS_FATAL_ERROR("Failed to erase from queue");
		neighborIter->RemoveVirtualQueueEntry();

		capable = true;
	}
	packet = newEntry.GetPacket()->Copy();
	
	if(isEncoded)
	{
		NS_LOG_FUNCTION(this<<"isEncoded? "<<isEncoded);
		//if(m_arq)
			m_rtqueue.EnqueueBack(entry);
	}
	entry = newEntry;
	return isEncoded;
}

Ptr<Packet>
YanciProtocol::XOR(Ptr<const Packet> p1, Ptr<const Packet> p2)
{
	NS_LOG_FUNCTION(this<<p1->GetSize()<<p2->GetSize());
	uint32_t len1 = p1->GetSize(), len2 = p2->GetSize() ;
	uint32_t big, small;

	big = (len1 > len2) ? len1 : len2;
	small = (len1 > len2) ? len2: len1;
	
	uint8_t *buffer = new uint8_t[big];
	uint8_t *buf1 = new uint8_t[len1];
	uint8_t *buf2 = new uint8_t[len2];

	memset(buffer, 0, big);
	memset(buf1, 0, len1);
	memset(buf2, 0, len2);

	NS_LOG_DEBUG("Before copy");
	p1->CopyData(buf1, len1);
	p2->CopyData(buf2, len2);

	for(uint32_t i = 0; i<small; i++)
		buffer[i] = buf1[i]^buf2[i];
	//for(uint32_t i = small; i<big; i++)
	//	buffer[i] = buf1[i]^0;
	for(uint32_t i = small; i<big; i++)
	{
		if(len1 > len2)
			buffer[i] = buf1[i]^0;
		else 
			buffer[i] = buf2[i]^0;
	}

	NS_LOG_DEBUG("Before XOR");
	uint32_t true_len= big-1;
	NS_LOG_DEBUG(true_len);
	while(!buffer[true_len])
		true_len--;
	Ptr<Packet> packet = Create<Packet>(buffer, true_len+1);
	//Ptr<Packet> packet = Create<Packet>(buffer, big);
	delete [] buffer;
	delete [] buf1;
	delete [] buf2;

	return packet;
}

void
YanciProtocol::SetDevice(Ptr<YanciDevice> device)
{
	m_device = device;
}

Ptr<YanciDevice>
YanciProtocol::GetDevice() const
{
	return m_device;
}

void 
YanciProtocol::SetIpv4Mask(const Ipv4Mask & mask)
{
	m_mask = mask;
}

bool
YanciProtocol::IsUnicast(const Ipv4Address & ipaddr, const Ipv4Mask & mask) const
{
	NS_LOG_FUNCTION_NOARGS();
	if(!ipaddr.IsMulticast() && !ipaddr.IsBroadcast() && !ipaddr.IsLocalMulticast() && !ipaddr.IsSubnetDirectedBroadcast(mask))
		return true;
	else
		return false;

}

bool
YanciProtocol::IsUnicast(const Ipv4Address & ipaddr) const
{
	return IsUnicast(ipaddr, m_mask);
}

void
YanciProtocol::SetIP(const Ipv4Address & ip)
{
	NS_LOG_FUNCTION(this<<ip);
	m_ip = ip;
}

Ipv4Address
YanciProtocol::GetIP() const
{
	return m_ip;
}

void
YanciProtocol::AddAck(AckBlock ack)
{
	//if(!m_arq)
	//	return;
	bool found = false;
	std::vector<AckBlock>::iterator iter;
	for(iter = m_ackBlockList.begin(); iter != m_ackBlockList.end(); iter++)
	{
		if(iter->address == ack.address)
		{
			NS_ASSERT(!found);
			found = true;
			if(iter->lastAck <= ack.lastAck)
			{
				iter->lastAck = ack.lastAck;
				iter->ackMap = ack.ackMap;
			}
		}
	}
	if(!found)
		m_ackBlockList.push_back(ack);
}

void
YanciProtocol::SaySth() const
{
	std::cout<<"Say sth!!!\n";
}

}//namespace yanci
}//namespace ns3
