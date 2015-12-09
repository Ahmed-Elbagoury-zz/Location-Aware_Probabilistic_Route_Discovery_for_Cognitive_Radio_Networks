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


#include "yanci-device.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/pointer.h"
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("YanciDevice");

namespace ns3{
namespace yanci{

YanciDevice::YanciDevice()
{
	NS_LOG_FUNCTION(this);
}

YanciDevice::~YanciDevice(){}

TypeId
YanciDevice::GetTypeId()
{
	static TypeId tid = TypeId ("ns3::yanci::YanciDevice")
		.SetParent<NetDevice>()
		.AddConstructor<YanciDevice> ()
		.AddAttribute ("Mtu", "The Mac-level Maximum Transimission Unit",
						UintegerValue(0xffff),
						MakeUintegerAccessor (&YanciDevice::SetMtu,
											  &YanciDevice::GetMtu),
						MakeUintegerChecker<uint16_t> () )
		.AddTraceSource("YanciRx",
						"Packet received by NC device. Being forwarded. Non-promisc.",
						MakeTraceSourceAccessor(&YanciDevice::m_yanciRxTrace))
		.AddTraceSource("YanciPromiscRx",
						"Packet received by NC device. Being forwarded. Promisc mode.",
						MakeTraceSourceAccessor(&YanciDevice::m_yanciPromiscRxTrace))
		;
	return tid;
}

void
YanciDevice::StartProtocol(bool arq, double rttime)
{
	NS_LOG_FUNCTION(this<<"About to create YanciProtocol with args: "<<arq<<rttime);
	m_yanci = Create<YanciProtocol>(rttime);
	//m_yanci = Create<YanciProtocol>(arq, rttime);
	m_yanci->SetDevice(this);
}

void
YanciDevice::StartProtocol(double rttime, double retry)
{
	NS_LOG_FUNCTION(this<<"About to create YanciProtocol with args: "<<rttime);
	m_yanci = Create<YanciProtocol>(rttime, retry);
	//m_yanci = Create<YanciProtocol>(arq, rttime);
	m_yanci->SetDevice(this);
}

void
YanciDevice::SetIfIndex (const uint32_t index)
{
	NS_LOG_FUNCTION_NOARGS ();
	m_ifIndex = index;
}

uint32_t
YanciDevice::GetIfIndex () const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_ifIndex;
}

Ptr<Channel>
YanciDevice::GetChannel() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_channel;
}

Address
YanciDevice::GetAddress() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_address;
	//return m_macs[0];
}

/*
std::vector<Mac48Address>
YanciDevice::GetAddresses() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_macs;
}
*/

void
YanciDevice::SetAddress (Address address)
{
	NS_LOG_FUNCTION_NOARGS();
	m_address = Mac48Address::ConvertFrom(address);
	//m_ip = Ipv4Address::ConvertFrom(address);
}

bool
YanciDevice::SetMtu(const uint16_t mtu)
{
	NS_LOG_FUNCTION_NOARGS();
	m_mtu = mtu;
	return true;
}

uint16_t
YanciDevice::GetMtu() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_mtu;
}

bool
YanciDevice::IsLinkUp () const
{
	NS_LOG_FUNCTION_NOARGS();
	return true;
}

void
YanciDevice::AddLinkChangeCallback (Callback<void> callback)
{
	//do nothing
}

bool
YanciDevice::IsBroadcast() const
{
	NS_LOG_FUNCTION_NOARGS();
	return true;
}

Address
YanciDevice::GetBroadcast() const
{
	NS_LOG_FUNCTION_NOARGS();
	return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
YanciDevice::IsMulticast() const
{
	NS_LOG_FUNCTION_NOARGS();
	return true;
}

Address
YanciDevice::GetMulticast (Ipv4Address multicastGroup) const
{
	NS_LOG_FUNCTION(this << multicastGroup);
	Mac48Address multicast = Mac48Address::GetMulticast (multicastGroup);
	return multicast;
}

Address
YanciDevice::GetMulticast(Ipv6Address addr) const
{
	NS_LOG_FUNCTION (this << addr);
	return Mac48Address::GetMulticast(addr);
}

bool
YanciDevice::IsPointToPoint () const
{
	NS_LOG_FUNCTION_NOARGS();
	return false;
}

bool
YanciDevice::IsBridge() const
{
	NS_LOG_FUNCTION_NOARGS();
	return false;
}

bool
YanciDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION(dest);
	const Mac48Address dest_mac = Mac48Address::ConvertFrom(dest);
	if(protocolNumber != Ipv4L3Protocol::PROT_NUMBER)
	{
		m_ifaces[0]->Send(packet, dest, protocolNumber);
		return true;
	}
	else
		return m_yanci->Enqueue(packet, m_address, dest_mac, protocolNumber);
}

bool
YanciDevice::SendFrom(Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION(dest);
	const Mac48Address src_mac = Mac48Address::ConvertFrom(src);
	const Mac48Address dest_mac = Mac48Address::ConvertFrom(dest);
	if(protocolNumber != Ipv4L3Protocol::PROT_NUMBER)
	{
		m_ifaces[0]->Send(packet, dest, protocolNumber);
		return true;
	}
	else
		return m_yanci->Enqueue(packet, src_mac, dest_mac, protocolNumber);
}

void
YanciDevice::ForwardDown(Ptr<Packet> packet, const Mac48Address & dest, uint16_t protocolNumber, uint32_t index)
{
	NS_LOG_FUNCTION(this<<m_ifaces[index]<<dest);
	m_ifaces[index]->Send(packet, dest, protocolNumber);
}

Ptr<Node>
YanciDevice::GetNode() const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_node;
}

void
YanciDevice::SetNode(Ptr<Node> node)
{
	NS_LOG_FUNCTION_NOARGS();
	m_node = node;
}

bool
YanciDevice::NeedsArp() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return true;
}

void
YanciDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
	NS_LOG_FUNCTION_NOARGS();
	m_rxCallback = cb;
}

void
YanciDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
	NS_LOG_FUNCTION_NOARGS();
	m_promiscRxCallback = cb;
}

bool
YanciDevice::SupportsSendFrom() const
{
	NS_LOG_FUNCTION_NOARGS();
	return false;
}

void
YanciDevice::AddInterface (Ptr<NetDevice> iface)
{
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(iface != this);
	if(!Mac48Address::IsMatchingType(iface->GetAddress()))
		NS_FATAL_ERROR ("EUI 48 Address support required.");
	if(m_ifaces.empty())
	{
		m_address = Mac48Address::ConvertFrom(iface->GetAddress());
		//m_macs.push_back(Mac48Address::ConvertFrom(iface->GetAddress()));
	}
	Ptr<WifiNetDevice> wifiNetDevice = iface->GetObject<WifiNetDevice>();
	if(wifiNetDevice == 0)
		NS_FATAL_ERROR("Device is not a Wifi NIC");

	Ptr<RegularWifiMac> wifiMac = wifiNetDevice->GetMac()->GetObject<RegularWifiMac>();
	//Ptr<DcaTxop> txop = wifiMac->GetDcaTxop();
	//txop->SetMaxQueueSize(10);
	PointerValue ptr;
	wifiMac->GetAttribute("DcaTxop", ptr);
	Ptr<DcaTxop> txop = ptr.Get<DcaTxop>();
	Ptr<WifiMacQueue> wifiMacQueue = txop->GetQueue();
	wifiMacQueue->SetMaxSize(10);

	m_node -> RegisterProtocolHandler (MakeCallback (&YanciDevice::ReceiveFromDevice, this), 0x0, iface, true);

	m_ifaces.push_back(iface);
	//m_yanci->Init();
}

uint32_t
YanciDevice::GetNumberOfInterfaces () const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_ifaces.size();
}

Ptr<NetDevice>
YanciDevice::GetInterface (uint32_t id) const
{
	NS_LOG_FUNCTION(this<<id<<m_ifaces.size());
	std::vector< Ptr<NetDevice> >::const_iterator iter;
	for(iter = m_ifaces.begin(); iter != m_ifaces.end(); iter++)
	{
		NS_LOG_DEBUG("IfIndex: "<<(*iter)->GetIfIndex());
		if ((*iter)->GetIfIndex() == id)
			return (*iter);
	}
	NS_FATAL_ERROR("Interface is not found by index");
	return 0;
}

std::vector< Ptr<NetDevice> >
YanciDevice::GetInterfaces() const
{
	return m_ifaces;
}

void
YanciDevice::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& source, const Address& dest, PacketType packetType)
{
	NS_LOG_FUNCTION(this<<protocol);
	packet->Print(std::cout);
	std::cout<<std::endl;
	Ptr<Packet> p = packet->Copy();
	if(protocol != Ipv4L3Protocol::PROT_NUMBER)
		ForwardUp(p, protocol, Mac48Address::ConvertFrom(source), Mac48Address::ConvertFrom(dest), packetType);
	else
		m_yanci->Recv(device, p, protocol, source, dest, packetType);
}

void
YanciDevice::ForwardUp(Ptr<Packet> packet, uint16_t protocol, const Mac48Address & src, const Mac48Address & dest, PacketType packetType)
{
	NS_LOG_FUNCTION_NOARGS();
	std::vector<Mac48Address>::iterator iter;
	enum NetDevice::PacketType type;
	Ptr<Packet> packet_copy = packet->Copy();
	NS_LOG_FUNCTION(this<<src<<dest<<protocol<<packetType);
	if(dest.IsBroadcast())
		type = NetDevice::PACKET_BROADCAST;
	else if (dest.IsGroup())
		type = NetDevice::PACKET_MULTICAST;
	else if (dest == m_address)
		type = NetDevice::PACKET_HOST;
	else
		type = NetDevice::PACKET_OTHERHOST;

	if(type != NetDevice::PACKET_OTHERHOST)
	{
		NotifyRx(packet);
		m_rxCallback(this, packet_copy, protocol, src);
	}
	if(!m_promiscRxCallback.IsNull())
	{
		NotifyPromiscRx(packet);
		m_promiscRxCallback(this, packet_copy, protocol, src, dest, type);
	}
}

void
YanciDevice::NotifyRx(Ptr<const Packet> packet)
{
	m_yanciRxTrace (packet);
}

void
YanciDevice::NotifyPromiscRx(Ptr<const Packet> packet)
{
	m_yanciPromiscRxTrace(packet);
}

void
YanciDevice::SetIP(Ipv4Address ip)
{
	NS_LOG_FUNCTION(this<<ip);
	m_ip = ip;
	m_yanci->SetIP(ip);
}

Ipv4Address
YanciDevice::GetIP() const
{
	return m_ip;
}

void 
YanciDevice::SetIpv4Mask(const Ipv4Mask & mask)
{
	m_mask = mask;
	m_yanci->SetIpv4Mask(mask);
}

void
YanciDevice::SaySth() const
{
	std::cout<<"Say sth!!!\n";
}

Ptr<YanciProtocol>
YanciDevice::GetProtocol() const
{
	return m_yanci;
}

}//namespace yanci
}//namespace ns3
