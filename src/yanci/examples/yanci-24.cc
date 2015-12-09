#include "ns3/yanci-module.h"
#include "ns3/yanci-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("YanciTest");

void ReceivePacket (Ptr<Socket> socket)
{
	Ptr<Node> node = socket->GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
	Ipv4Address addr = ipv4->GetAddress(ipv4->GetInterfaceForDevice(ipv4->GetNetDevice(1)), 0).GetLocal();

  NS_LOG_UNCOND("Received one packet at "<<socket<<addr<<"at "<<Simulator::Now().GetSeconds());
}

int main(int argc, char *argv[])
{

	LogComponentEnable("OnOffApplication", LOG_LEVEL_FUNCTION);
	LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_LOGIC);
	LogComponentEnable("UdpSocketImpl", LOG_LEVEL_FUNCTION);
	LogComponentEnable("Socket", LOG_LEVEL_FUNCTION);
	LogComponentEnable("YanciProtocol", LOG_LEVEL_LOGIC);
	LogComponentEnable("YanciDevice", LOG_LEVEL_FUNCTION);
	LogComponentEnable("WifiNetDevice", LOG_LEVEL_FUNCTION);
	LogComponentEnable("RegularWifiMac", LOG_LEVEL_FUNCTION);
	LogComponentEnable("AdhocWifiMac", LOG_LEVEL_FUNCTION);
	LogComponentEnable("DcaTxop", LOG_LEVEL_FUNCTION);
	LogComponentEnable("EdcaTxopN", LOG_LEVEL_FUNCTION);
	LogComponentEnable("MacLow", LOG_LEVEL_FUNCTION);
	LogComponentEnable("YansWifiPhy", LOG_LEVEL_FUNCTION);
	LogComponentEnable("WifiPhy", LOG_LEVEL_FUNCTION);
	LogComponentEnable("YansWifiChannel", LOG_LEVEL_FUNCTION);

	std::string phyMode ("ErpOfdmRate24Mbps");
	double rss = -80;  // -dBm
	uint32_t packetSize = 1500; // bytes
	uint32_t numPackets = 1;
	bool verbose = false;
	Ipv4Address net("10.1.1.0");
	Ipv4Address base("0.0.0.1");
	Ipv4Mask mask("255.255.255.0");
	std::string dataRate("6Mbps");
	double rtTime = 20;
	double retry = 5;
	bool arq = true;
	uint32_t nodeNum = 4;

	//yanci::YanciDevice yanciDevice;
	YanciHelper yanci;

	CommandLine cmd;
	cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue ("rss", "received signal strength", rss);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "number of packets generated", numPackets);
	cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
	cmd.AddValue ("dataRate", "data rate of OnOff app", dataRate);
	cmd.AddValue ("rtTime", "retransmission time of Yanci", rtTime);
	cmd.AddValue ("tryTime", "trySend timer", retry);
	cmd.AddValue ("arq", "whether to use arq", arq);
	cmd.AddValue ("nodeNum", "Number of nodes", nodeNum) ;
	cmd.Parse(argc, argv);

	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("1"));
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

	NodeContainer node_container;
	node_container.Create(nodeNum);

	WifiHelper wifi;
	if (verbose)
	{
		wifi.EnableLogComponents ();  // Turn on all Wifi logging
	}

	wifi.SetStandard(WIFI_PHY_STANDARD_80211g);

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	wifiPhy.Set("RxGain", DoubleValue(0));
	wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	//wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(rss));
	wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
	wifiPhy.SetChannel(wifiChannel.Create());

	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));
	wifiMac.SetType("ns3::AdhocWifiMac");

	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, node_container);
	//NetDeviceContainer yanciDevices = yanci.Install(node_container, arq, rtTime);
	NetDeviceContainer yanciDevices = yanci.Install(node_container, rtTime, retry);
	
	/*-----------Configure YanciDevice-------------*/
	std::string prefix("10.1.1.");
	for(uint32_t i = 0; i < nodeNum; i++)
	{
		Ptr<yanci::YanciDevice> yanciDevice = yanciDevices.Get(i)->GetObject<yanci::YanciDevice>();
		std::stringstream ss;
		ss << prefix << (i+1);
		std::string address = ss.str();
		yanciDevice->SetIP(address.c_str());
		yanciDevice->SetIpv4Mask(mask);
	}
	/*-----------End of Configure YanciDevice-------------*/

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	for(uint32_t i = 0; i<nodeNum; i++)
		positionAlloc->Add (Vector (0.0, 300*i, 0.0));
	mobility.SetPositionAllocator(positionAlloc);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(node_container);

	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);

	InternetStackHelper internet;
	internet.SetRoutingHelper(list);
	internet.Install(node_container);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase(net, mask, base);
	//Ipv4InterfaceContainer i = ipv4.Assign(devices);
	Ipv4InterfaceContainer i = ipv4.Assign(yanciDevices);

	Ptr<OutputStreamWrapper> routingStream2 = Create<OutputStreamWrapper> ("static.routes", std::ios::out);
	staticRouting.PrintRoutingTableAllAt (Seconds(5.0), routingStream2);
	
	/*----Configure static routing ----*/
	std::vector<Ipv4Address> addr_vec;
	for(uint32_t i = 0; i<node_container.GetN(); i++)
	{
		Ptr<Node> node = node_container.Get(i);
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
		Ipv4Address addr = ipv4->GetAddress(ipv4->GetInterfaceForDevice(ipv4->GetNetDevice(1)), 0).GetLocal();
		addr_vec.push_back(addr);
	}
	for(uint32_t i = 0; i<node_container.GetN(); i++)
	{
		Ptr<Node> node = node_container.Get(i);
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
		Ptr<Ipv4StaticRouting> routingMod = staticRouting.GetStaticRouting(ipv4);
		for(uint32_t left = 0; left < i; left++)
		{
			routingMod->AddHostRouteTo(addr_vec[left], addr_vec[i-1], 1);
		}
		for(uint32_t right = i+1; right<node_container.GetN(); right++)
		{
			routingMod->AddHostRouteTo(addr_vec[right], addr_vec[i+1], 1);
		}
	}
	/*----End of static routing configure ----*/

	Address remoteAddress(InetSocketAddress(i.GetAddress(0), 80));
	OnOffHelper oohelper("ns3::UdpSocketFactory", remoteAddress);
	oohelper.SetAttribute("DataRate", StringValue(dataRate));
	oohelper.SetAttribute("MaxBytes", UintegerValue(packetSize*3000));
	oohelper.SetAttribute("PacketSize", UintegerValue(packetSize));

	ApplicationContainer apps = oohelper.Install(node_container.Get(nodeNum-1));
	apps.Start(Seconds(1.0));
	apps.Stop(Seconds(12.0));

	Address remoteAddress2(InetSocketAddress(i.GetAddress(nodeNum-1), 80));
	OnOffHelper oohelper2("ns3::UdpSocketFactory", remoteAddress2);
	oohelper2.SetAttribute("DataRate", StringValue(dataRate));
	oohelper2.SetAttribute("MaxBytes", UintegerValue(packetSize*3000));
	oohelper2.SetAttribute("PacketSize", UintegerValue(packetSize));

	ApplicationContainer apps2 = oohelper2.Install(node_container.Get(0));
	apps2.Start(Seconds(2.0));
	apps2.Stop(Seconds(13.0));

	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	Ptr<Socket> recvSink = Socket::CreateSocket (node_container.Get(0), tid);
	Ptr<Socket> recvSink2 = Socket::CreateSocket (node_container.Get(nodeNum-1), tid);
	
	recvSink->SetAttribute("RcvBufSize", UintegerValue(1310720*2));
	recvSink2->SetAttribute("RcvBufSize", UintegerValue(1310720*2));

	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
	InetSocketAddress local2 = InetSocketAddress(Ipv4Address::GetAny(), 80);

	if(recvSink->Bind(local) == -1)
		NS_LOG_ERROR("Bind failure of node 0");
	if (recvSink2->Bind(local2) == -1)
		NS_LOG_ERROR("Bind failure of node 2");

	recvSink->SetRecvCallback (MakeCallback(&ReceivePacket));
	recvSink2->SetRecvCallback (MakeCallback(&ReceivePacket));

	//wifiPhy.EnablePcap("yanci-test", devices);
	AsciiTraceHelper ascii;
	std::string ascfile("yanci-static-24");
	std::string ascfile_post (".tr");
	std::string ascfile_middle;
	std::stringstream out;
	out<<"-"<<nodeNum<<"-"<<dataRate<<"-"<<packetSize<<"-"<<rtTime<<"-"<<retry;
	ascfile_middle = out.str();
	wifiPhy.EnableAsciiAll(ascii.CreateFileStream(ascfile+ascfile_middle+ascfile_post));
	//wifiPhy.EnableAsciiAll(ascii.CreateFileStream("yanciascii.tr"));

	Simulator::Stop(Seconds(20.0));
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
