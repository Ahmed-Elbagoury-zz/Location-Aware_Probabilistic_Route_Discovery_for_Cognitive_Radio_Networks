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
#include "yanci-packet-info.h"
#include "yanci-queue.h"
#include "yanci-neighbor.h"
#include "yanci-packet-pool.h"
#include "yanci-device.h"
#include <set>
#include <vector>
#include <map>
#include "ns3/mac48-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include <cstdlib>

#ifndef YANCIPROTOCOL_H
#define YANCIPROTOCOL_H

namespace ns3{
namespace yanci{

class YanciDevice;

class YanciProtocol : public Object
{
public:
	static TypeId GetTypeId();
	YanciProtocol();
	YanciProtocol(double rttime); 
	YanciProtocol(double rttime, double retry);
	YanciProtocol(bool arq);
	YanciProtocol(bool arq, double rttime);
	~YanciProtocol();
	bool Encode(QueueEntry & entry, Ptr<Packet> & packet, YanciHeader & yanciHeader);
	Ptr<Packet> XOR(Ptr<const Packet> p1, Ptr<const Packet> p2);
	//int64_t Decode(const YanciHeader & header, Ptr<Packet> & packet);
	int64_t Decode(const YanciHeader & header, Ptr<Packet> & packet, uint16_t & sequence);
	void Retransmit();
	bool Enqueue(Ptr<Packet> packet, const Mac48Address& src, const Mac48Address& dest, uint16_t protocolNumber);

	void SetDevice(Ptr<YanciDevice> device);
	Ptr<YanciDevice> GetDevice () const;
	void Recv(Ptr<NetDevice> netDevice, Ptr<const Packet> pkt, uint16_t protocol, const Address & sender, const Address & receiver, NetDevice::PacketType packetType);
	
	void SetIpv4Mask(const Ipv4Mask & mask);
	bool IsUnicast(const Ipv4Address & ipaddr, const Ipv4Mask & mask) const;
	bool IsUnicast(const Ipv4Address & ipaddr) const;
	void SetIP(const Ipv4Address & ip);
	Ipv4Address GetIP() const;
	void DoSend(); //this was Send() in YanciProtocol
	void TrySend();
	void TrySendSchedule();
	void Init();
	void AddAck(AckBlock ack);
	void SaySth() const;

private:
	Ptr<Packet> Shrink(Ptr<Packet> packet, uint32_t size);

private:
	Ptr<YanciDevice> m_device;
	Ipv4Address m_ip;
	Neighbors m_neighbors;
	Queue m_queue; //Output queue
	Queue m_rtqueue; //Retransmission queue
	Timer m_timer;
	Time m_rtimeout;
	Timer m_try;
	Time m_ttimeout;
	PacketInfo m_packetInfo;
	PacketPool m_pool;
	std::vector<AckBlock> m_ackBlockList;
	uint16_t m_sequence;
	bool m_isSending;
	Ipv4Mask m_mask;
	//bool m_arq;
	double m_rttime;
};


}//namespace yanci
}//namesapce ns3

#endif
