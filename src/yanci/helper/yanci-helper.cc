/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Yang CHI <chiyg@mail.uc.edu>
 */

#include "yanci-helper.h"

namespace ns3{
YanciHelper::YanciHelper(){}
YanciHelper::~YanciHelper(){}

/*
NetDeviceContainer
YanciHelper::Install(NodeContainer nodes) const
{
	return Install(nodes, true, 50.0);
}
*/

NetDeviceContainer
YanciHelper::Install(NodeContainer nodes, bool arq, double rttime) const
{
	//NS_LOG_FUNCTION(this<<arq<<rttime);
	NetDeviceContainer devices;
	for(NodeContainer::Iterator iter = nodes.Begin(); iter != nodes.End(); iter++)
	{
		Ptr<Node> node = *iter;
		Ptr<yanci::YanciDevice> yanciDevice = CreateObject<yanci::YanciDevice>();
		yanciDevice ->SetNode(node);
		uint32_t ifNum = node->GetNDevices();
		uint32_t iface;
		for(iface = 0; iface<ifNum; iface++)
		{
			yanciDevice->AddInterface(node->GetDevice(iface));
		}
		node->AddDevice(yanciDevice);
		yanciDevice->StartProtocol(arq, rttime);
		devices.Add(yanciDevice);
	}
	return devices;
}

NetDeviceContainer
YanciHelper::Install(NodeContainer nodes, double rttime, double retry) const
{
	NetDeviceContainer devices;
	for(NodeContainer::Iterator iter = nodes.Begin(); iter != nodes.End(); iter++)
	{
		Ptr<Node> node = *iter;
		Ptr<yanci::YanciDevice> yanciDevice = CreateObject<yanci::YanciDevice>();
		yanciDevice ->SetNode(node);
		uint32_t ifNum = node->GetNDevices();
		uint32_t iface;
		for(iface = 0; iface<ifNum; iface++)
		{
			yanciDevice->AddInterface(node->GetDevice(iface));
		}
		node->AddDevice(yanciDevice);
		yanciDevice->StartProtocol(rttime, retry);
		devices.Add(yanciDevice);
	}
	return devices;
}

/*
NetDeviceContainer
YanciHelper::Install(const NetDeviceContainer nics, NodeContainer nodes) const
{
	NetDeviceContainer devices;
	for(NodeContainer::Iterator iter = nodes.Begin(); iter != nodes.End(); iter++)
	{
		Ptr<Node> node = *iter;
		Ptr<yanci::YanciDevice> yanciDevice = CreateObject<yanci::YanciDevice>();
		node->AddDevice(yanciDevice);
		NetDeviceContainer::Iterator nicIter = nics.Begin();
		for(; nicIter != nics.End(); nicIter++)
		{
			node->AddDevice(*nicIter);
			yanciDevice ->AddInterface(*nicIter);
		}
		devices.Add(yanciDevice);
	}
	return devices;
}
*/

/*
NetDeviceContainer
YanciHelper::Install(NodeContainer nodes, Ipv4Address network, Ipv4Mask mask, Ipv4Address base)
{
	NetDeviceContainer devices;

}
*/

/*
void
YanciHelper::SetNumberofInterfaces (uint32_t nInterfaces)
{
	m_nInterfaces = nInterfaces;
}
*/

}//namespace ns3
