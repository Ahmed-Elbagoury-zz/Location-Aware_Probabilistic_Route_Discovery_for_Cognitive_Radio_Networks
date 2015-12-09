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

#ifndef YANCI_HELPER_H
#define YANCI_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/yanci-device.h"

namespace ns3{

class YanciHelper
{
public:
	YanciHelper();
	~YanciHelper();
	//NetDeviceContainer Install(const WifiPhyHelper &phyHelper, NodeContainer c) const;
	//NetDeviceContainer Install(const NetDeviceContainer nics, NodeContainer nodes) const;
	NetDeviceContainer Install(NodeContainer nodes, bool arq, double rttime) const;
	NetDeviceContainer Install(NodeContainer nodes, double rtTime, double retry) const;
	//NetDeviceContainer Install(NodeContainer nodes) const;

	//NetDeviceContainer Install(NodeContainer nodes, Ipv4Address network, Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

	//void SetNumberOfInterfaces (uint32_t nInterfaces);

private:
	uint32_t m_nInterfaces;
};

}//namespace ns3
#endif
