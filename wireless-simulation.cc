#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    uint32_t numNodes = 10;
    double simulationTime = 100.0; // Increased simulation time to allow for node movement

    NodeContainer nodes;
    nodes.Create(numNodes);

    // Create a monitoring node
    NodeContainer monitoringNode;
    monitoringNode.Create(1);

    MobilityHelper mobility;

    // Set up random walk mobility model for all nodes
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-500, 500, -500, 500)));
    mobility.Install(nodes);

    // Set up constant position mobility model for the monitoring node
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(monitoringNode);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("RxGain", DoubleValue(0));
    wifiPhy.Set("TxGain", DoubleValue(0));
    wifiPhy.Set("RxNoiseFigure", DoubleValue(7));
    wifiPhy.Set("TxPowerStart", DoubleValue(16.0206));
    wifiPhy.Set("TxPowerEnd", DoubleValue(16.0206));
    wifiPhy.Set("CcaEdThreshold", DoubleValue(-62.8));
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    NetDeviceContainer wifiDevices = wifi.Install(wifiPhy, wifiMac, nodes);
    wifiPhy.EnablePcap("wifi-capture", wifiDevices, true);  // Enable pcap tracing for WiFi devices

    // Create point-to-point links between nodes and monitoring node
    PointToPointHelper p2pHelper;
    p2pHelper.EnablePcapAll("wireless-capture", true);  // Enable pcap tracing for point-to-point links
    std::vector<Ptr<PointToPointNetDevice>> monitoringDevices;
    for (uint32_t i = 0; i < numNodes; ++i)
    {
        Ptr<Node> node = nodes.Get(i);
        NetDeviceContainer link = p2pHelper.Install(node, monitoringNode.Get(0));
        Ptr<PointToPointNetDevice> device = DynamicCast<PointToPointNetDevice>(link.Get(1));
        if (device != nullptr)
        {
            monitoringDevices.push_back(device);
            p2pHelper.EnablePcap("monitoring-capture", device, true);  // Enable pcap tracing for monitoring node's point-to-point links
        }
    }

    InternetStackHelper internet;
    internet.Install(nodes);
    internet.Install(monitoringNode); // Install IP stack on monitoring node

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(wifiDevices);

    uint16_t port = 9;

    std::vector<ApplicationContainer> serverApps;
    std::vector<ApplicationContainer> clientApps;

    for (uint32_t i = 0; i < numNodes; ++i)
    {
        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port + i));
        ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(i));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(simulationTime));
        serverApps.push_back(sinkApp);

        for (uint32_t j = 0; j < numNodes; ++j)
        {
            if (i != j)
            {
                OnOffHelper clientHelper("ns3::UdpSocketFactory", InetSocketAddress(interfaces.GetAddress(j), port + j));
                clientHelper.SetConstantRate(DataRate("500kb/s"));
                ApplicationContainer app = clientHelper.Install(nodes.Get(i));
                app.Start(Seconds(1.0));
                app.Stop(Seconds(simulationTime));
                clientApps.push_back(app);
            }
        }
    }

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // NetAnim setup
    AnimationInterface anim("wireless-animation.xml");
    anim.EnablePacketMetadata(true);

    // Packet logging
    PacketSinkHelper monitoringSink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9999));
    ApplicationContainer monitoringApp = monitoringSink.Install(monitoringNode.Get(0));
    monitoringApp.Start(Seconds(0.0));
    monitoringApp.Stop(Seconds(simulationTime + 1));

    AsciiTraceHelper ascii;
    p2pHelper.EnableAsciiAll(ascii.CreateFileStream("wireless-capture.tr"));

    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
