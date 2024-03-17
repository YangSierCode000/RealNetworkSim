#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "working_application.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MainScript");


int main(int argc, char *argv[]) {
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));

    NodeContainer nodes;
    nodes.Create(2);

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices = pointToPoint.Install(nodes);

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Setting up applications to send and receive data
    uint16_t port = 8080;

    // Setup WorkingApplication on Node 0 to send data to Node 1
    Ptr<WorkingApplication> appSrc = CreateObject<WorkingApplication>();
    Address addrSrc(InetSocketAddress(interfaces.GetAddress(1), port)); // Destination is Node 1
    appSrc->Setup(Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId()), addrSrc, 1024, 1, DataRate("1Mbps"));
    nodes.Get(0)->AddApplication(appSrc);
    appSrc->SetStartTime(Seconds(1.0));
    appSrc->SetStopTime(Seconds(20.0));

    // Setup WorkingApplication on Node 1 to send data to Node 0
    Ptr<WorkingApplication> appDst = CreateObject<WorkingApplication>();
    Address addrDst(InetSocketAddress(interfaces.GetAddress(0), port)); // Destination is Node 0
    appDst->Setup(Socket::CreateSocket(nodes.Get(1), TcpSocketFactory::GetTypeId()), addrDst, 1024, 1, DataRate("1Mbps"));
    nodes.Get(1)->AddApplication(appDst);
    appDst->SetStartTime(Seconds(1.5)); // Slightly delay the start to ensure it receives data before sending
    appDst->SetStopTime(Seconds(20.0));


    Simulator::Stop(Seconds(20));
    Simulator::Run();

    // After the simulation
    flowMonitor->CheckForLostPackets();
    flowMonitor->SerializeToXmlFile("flow-monitor-results.xml", true, true);

    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMonitor->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = flowStats.begin(); iter != flowStats.end(); ++iter) {
        // Correctly cast the classifier
        if (classifier) {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);
            std::cout << "Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress << std::endl;
            std::cout << "Tx Packets = " << iter->second.txPackets << std::endl;
            std::cout << "Rx Packets = " << iter->second.rxPackets << std::endl;
            std::cout << "Total Lost Packets = " << iter->second.lostPackets << std::endl;
            std::cout << "Packet Delivery Ratio = " << iter->second.rxPackets * 100.0 / iter->second.txPackets << "%" << std::endl;
            std::cout << "Packet Loss Ratio = " << iter->second.lostPackets * 100.0 / iter->second.txPackets << "%" << std::endl;
            if (iter->second.rxPackets > 0) {
                std::cout << "Average Latency = " << iter->second.delaySum.GetSeconds() / iter->second.rxPackets << "s" << std::endl;
            }
        }
    }


    Simulator::Destroy();

    return 0;
}
