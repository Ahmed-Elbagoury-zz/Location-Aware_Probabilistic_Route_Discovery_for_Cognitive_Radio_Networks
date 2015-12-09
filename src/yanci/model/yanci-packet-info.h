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

#ifndef YANCIPACKETINFO_H
#define YANCIPACKETINFO_H

#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include "yanci-neighbor.h"
#include "yanci-header.h"
#include <deque>
#include <map>

namespace ns3{
namespace yanci{

class PacketInfo
{
public:
	PacketInfo();
	~PacketInfo();

	//typedef std::map<Neighbor, double> Probability; //The neighbor-prabability pair.
	//typedef std::deque<Probability> ProbabilityDeque; //A queue of the pairs above.

	//uint32_t Hash(Ipv4Address address, uint32_t seq_no) const;
	//double GetProbability(uint32_t packetId, Neighbor neighbor) const;
	//double GetProbability(uint32_t packetId, Mac48Address mac) const;
	//double GetProbability(uint32_t packetId, Ipv4Address ip) const;
	/*
	void SetItem(uint32_t packetId, Ipv4Address ip);
	bool GetItem(uint32_t packetId, Ipv4Address ip);
	*/
	void SetItem(uint32_t packetId, const Mac48Address & mac);
	void SetItem(const YanciHeader & header, const Mac48Address & mac);
	bool GetItem(uint32_t packetId, const Mac48Address & mac);
	//void SetProbability(uint32_t packetId, Neighbor neighbor, double probability);
	void Print(std::ostream &os) const;
;

private:
	//std::map<uint32_t, Ipv4Address> m_packetInfo;
	std::map<uint32_t, Mac48Address> m_packetInfo;
	//std::map<uint32_t, Probability> m_packetInfo; 
	//std::map<uint32_t, ProbabilityDeque> m_packetInfo; 

};

}//namespace yanci
}//namespace ns3

#endif
