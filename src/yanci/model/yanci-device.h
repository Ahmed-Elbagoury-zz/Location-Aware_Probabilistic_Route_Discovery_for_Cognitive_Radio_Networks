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

#ifndef YANCIDEVICE_H
#define YANCIDEVICE_H

#include "ns3/net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/wifi-net-device.h"
#include "ns3/ipv4-address.h"
#include "ns3/timer.h"
#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/wifi-net-device.h"
#include "ns3/dca-txop.h"
#include "ns3/ipv4-l3-protocol.h"

#include "yanci-protocol.h"
#include "yanci-header.h"
#include "yanci-packet-info.h"
#include "yanci-queue.h"
#include "yanci-neighbor.h"
#include "yanci-packet-pool.h"

#include <vector>
#include <set>
#include <map>
#include <cstdlib>

namespace ns3{
namespace yanci{

class YanciDevice : public NetDevice
{
public:
	static TypeId GetTypeId();
	YanciDevice();
	virtual ~YanciDevice();

	virtual void SetIfIndex (const uint32_t index);
	virtual uint32_t GetIfIndex () const;
	virtual Ptr<Channel> GetChannel () const;
	virtual Address GetAddress () const;
	virtual void SetAddress (Address address);
	virtual bool SetMtu (const uint16_t mtu);
	virtual uint16_t GetMtu () const;
	virtual bool IsLinkUp () const;

	/* Add a callback invoked whenever the link 
	 * status changes to UP. This callback is typically used
	 * by the IP/ARP layer to flush the ARP cache and by IPv6 stack
	 * to flush NDISC cache whenever the link goes up.
	 */
	virtual void AddLinkChangeCallback (Callback<void> callback);

	/**
	 * \return true if this interface supports a broadcast address,
	 *         false otherwise.
	 */
	virtual bool IsBroadcast () const;
	virtual Address GetBroadcast () const;
	virtual bool IsMulticast () const;
	virtual Address GetMulticast (Ipv4Address multicastGroup) const;
	virtual bool IsPointToPoint () const;
	virtual bool IsBridge () const;

	/**
	 * \param packet packet sent from above down to Network Device
	 * \param dest mac address of the destination (already resolved)
	 * \param protocolNumber identifies the type of payload contained in
	 *        this packet. Used to call the right L3Protocol when the packet
	 *        is received.
	 * 
	 *  Called from higher layer to send packet into Network Device
	 *  to the specified destination Address
	 * 
	 * \return whether the Send operation succeeded 
	 */
	virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

	/**
	 * \param packet packet sent from above down to Network Device
	 * \param source source mac address (so called "MAC spoofing")
	 * \param dest mac address of the destination (already resolved)
	 * \param protocolNumber identifies the type of payload contained in
	 *        this packet. Used to call the right L3Protocol when the packet
	 *        is received.
	 * 
	 *  Called from higher layer to send packet into Network Device
	 *  with the specified source and destination Addresses.
	 * 
	 * \return whether the Send operation succeeded 
	 */
	virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
	virtual Ptr<Node> GetNode () const;
	virtual void SetNode (Ptr<Node> node);
	virtual bool NeedsArp () const;

	virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
	virtual void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);
	virtual bool SupportsSendFrom () const;
	virtual Address GetMulticast (Ipv6Address addr) const;

	//not from net-device:
	//std::vector<Mac48Address> GetAddresses() const;
	void StartProtocol(bool arq, double rttime);
	void StartProtocol(double rttime, double retry);
	void AddInterface (Ptr<NetDevice> iface);
	uint32_t GetNumberOfInterfaces () const;
	Ptr<NetDevice> GetInterface (uint32_t id) const;
	Ptr<NetDevice> GetInterface() const;
	std::vector< Ptr<NetDevice> > GetInterfaces() const;
	void SetIP(Ipv4Address ip);
	Ipv4Address GetIP() const;
	void NotifyRx(Ptr<const Packet> packet);
	void NotifyPromiscRx(Ptr<const Packet> packet);
	void ForwardUp(Ptr<Packet> packet, uint16_t protocol, const Mac48Address & src, const Mac48Address & dest, PacketType packetType);
	void ForwardDown(Ptr<Packet> packet, const Mac48Address & dest, uint16_t protocolNumber, uint32_t index = 0);

	void SetIpv4Mask(const Ipv4Mask & mask);
	void SaySth() const;
	Ptr<YanciProtocol> GetProtocol() const;
	//bool IsUnicast(const Ipv4Address ipaddr, const Ipv4Mask mask) const;
	//bool IsUnicast(const Ipv4Address ipaddr) const;
	//bool Encode(QueueEntry entry, Ptr<Packet> & packet, YanciHeader & yanciHeader);

	//Ptr<Packet> XOR(Ptr<const Packet> p1, Ptr<const Packet> p2);
	//void DoSend(); //this was Send() in YanciProtocol
	//void TrySend();

	/*
	 * The following explanation is COPIED from PromiscCallBack in src/node/net-device.h!!!
	 * 
	 * \param device a pointer to the net device which is calling this callback
	 * \param packet the packet received
	 * \param protocol the 16 bit protocol number associated with this packet.
	 *        This protocol number is expected to be the same protocol number
	 *        given to the Send method by the user on the sender side.
	 * \param sender the address of the sender
	 * \param receiver the address of the receiver
	 * \param packetType type of packet received (broadcast/multicast/unicast/otherhost)
	 * \returns true if the callback could handle the packet successfully, false
	 *          otherwise.
	 *
	 * typedef Callback< bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address &, const Address &, enum PacketType > PromiscReceiveCallback;
	 */
	//bool Recv(Ptr<NetDevice> netDevice, Ptr<const Packet> pkt, uint16_t protocol, const Address & sender, const Address & receiver, NetDevice::PacketType packetType);
	//void Retransmit();
	/*
	 * \returns -1 if we need more packets to decode it. 0 if we have every packet. a positive pid if it's decodable and it's decoded.
	 */
	//int Decode(const YanciHeader & header, Ptr<Packet> packet);
	//bool Enqueue(Ptr<Packet> packet, const Mac48Address& src, const Mac48Address& dest, uint16_t protocolNumber);

private:
	void ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& source, const Address& dest, PacketType packetType);

private:
	uint32_t m_ifIndex;
	Ptr<Channel> m_channel;
	Mac48Address m_address;
	//std::vector<Mac48Address> m_macs;
	Ipv4Address m_ip;
	uint16_t m_mtu;
	//Ptr<YanciProtocol> m_yanciProtocol;
	Ptr<Node> m_node;
	NetDevice::ReceiveCallback m_rxCallback;
	NetDevice::PromiscReceiveCallback m_promiscRxCallback;
	std::vector< Ptr<NetDevice> > m_ifaces;
	TracedCallback<Ptr<const Packet> > m_yanciRxTrace;
	TracedCallback<Ptr<const Packet> > m_yanciPromiscRxTrace;
	Ptr<YanciProtocol> m_yanci;

	//Timer m_timer;
	//Time m_rtimeout;
	
	//Neighbors m_neighbors;
	//Queue m_queue; //Output queue
	//Queue m_rtqueue; //Retransmission queue
	//PacketPool m_pool;
	//Ipv4Address m_ip;
	//PacketInfo m_packetInfo;
	//std::map<Mac48Address, uint32_t> m_ackList;
	//std::vector<AckBlock> m_ackBlockList;
	//static const double m_thresholdG = 0.8;
	Ipv4Mask m_mask;
	//static const uint32_t m_ls_threshold = 100;
	//bool m_isSending;

};//class YanciDevice
}//namespace yanci
}//namespace ns3

#endif
