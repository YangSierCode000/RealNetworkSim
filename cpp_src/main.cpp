// Includes for the necessary ns-3 modules
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "working_application.h"
#include <fstream>

using namespace ns3;

// Define a log component for this script
NS_LOG_COMPONENT_DEFINE("MainScript");

int main(int argc, char *argv[]) {
    // Setup command line arguments
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Set default TCP properties
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));

    // Create two nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Initialize the flow monitor
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Configure the point-to-point link between nodes
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Install devices on nodes based on the point-to-point link
    NetDeviceContainer devices = pointToPoint.Install(nodes);

    // Set an error model to simulate packet loss
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    // Install the internet stack (TCP/IP protocol stack) on nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses to device interfaces
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Configure applications
    uint16_t port = 8080;

    // Setup the WorkingApplication on Node 0 to send data to Node 1
    Ptr<WorkingApplication> appSrc = CreateObject<WorkingApplication>();
    Address addrSrc(InetSocketAddress(interfaces.GetAddress(1), port)); // Destination is Node 1
    appSrc->Setup(Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId()), addrSrc, 1024, 1, DataRate("1Mbps"));
    nodes.Get(0)->AddApplication(appSrc);
    appSrc->SetStartTime(Seconds(0));
    appSrc->SetStopTime(Seconds(20.0));

    // Setup the WorkingApplication on Node 1 to send data to Node 0
    Ptr<WorkingApplication> appDst = CreateObject<WorkingApplication>();
    Address addrDst(InetSocketAddress(interfaces.GetAddress(0), port)); // Destination is Node 0
    appDst->Setup(Socket::CreateSocket(nodes.Get(1), TcpSocketFactory::GetTypeId()), addrDst, 1024, 1, DataRate("1Mbps"));
    nodes.Get(1)->AddApplication(appDst);
    appDst->SetStartTime(Seconds(5)); // Slightly delay to ensure it receives data before sending
    appDst->SetStopTime(Seconds(20.0));

    appSrc->StartSendingData(9);  // Start the data transmission process

    // Run the simulation
    Simulator::Stop(Seconds(30));
    Simulator::Run();

    // Cleanup and generate flow monitor report
    Simulator::Destroy();

    return 0;
}
