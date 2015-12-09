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

#include "yanci-hash.h"

namespace ns3{
namespace yanci{

uint32_t Hash(const Ipv4Address & address, uint32_t seq_no) 
{
	uint32_t addressInt = address.Get();
	uint64_t key = addressInt;
	key = (key << 32) + seq_no;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}

uint32_t Hash(const Mac48Address & address, uint16_t seq_no) 
{
	uint8_t buffer[6];
	uint32_t addressInt = 0;
	uint32_t seq = seq_no;
	//uint32_t addressInt = address.Get();
	address.CopyTo(buffer);
	for(int i = 0; i<4; i++)
		addressInt = (addressInt<<8) + buffer[i];
	for(int i = 4; i<6; i++)
		seq = (seq<<8) + buffer[i];
	uint64_t key = addressInt;
	key = (key << 32) + seq;
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}

}
}
