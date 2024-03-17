// working_application.cc
#include "working_application.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("WorkingApplication");

TypeId WorkingApplication::GetTypeId() {
    static TypeId tid = TypeId("WorkingApplication")
        .SetParent<Application>()
        .SetGroupName("MyApp")
        .AddConstructor<WorkingApplication>();
    return tid;
}

WorkingApplication::WorkingApplication()
    : m_socket(nullptr), m_peer(), m_packetSize(0), m_nPackets(0), m_dataRate(0),
      m_sendEvent(), m_running(false), m_packetsSent(0) {
}

WorkingApplication::~WorkingApplication() {
    m_socket = nullptr;
}

void WorkingApplication::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize,
                               uint32_t nPackets, DataRate dataRate) {
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_nPackets = nPackets;
    m_dataRate = dataRate;
}


void WorkingApplication::StopApplication() {
    m_running = false;
    if (m_sendEvent.IsRunning()) {
        Simulator::Cancel(m_sendEvent);
    }
    if (m_socket) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void WorkingApplication::StartApplication() {
    m_running = true;
    m_packetsSent = 0;
    m_responseReceived = false;

    if (!m_socket) {
        m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        m_socket->Bind();
        m_socket->Connect(m_peer);
    }

    m_socket->SetRecvCallback(MakeCallback(&WorkingApplication::HandleRead, this));
}

void WorkingApplication::StartSendingData(double initialDataValue) {
    m_nextDataToSend = initialDataValue; // Set the initial data to send
    SendPacket();  // Trigger the first packet transmission
}


void WorkingApplication::SendPacket() {
    NS_LOG_UNCOND("Sending data: " << m_nextDataToSend);
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (++m_packetsSent < m_nPackets)
    {
        ScheduleTx();
    }
}


void
WorkingApplication::ScheduleTx()
{
    if (m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &WorkingApplication::SendPacket, this);
    }
}


void WorkingApplication::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    Address from;
    NS_LOG_UNCOND("Ip from " << InetSocketAddress::ConvertFrom(from).GetIpv4());
    while ((packet = socket->RecvFrom(from))) {
        if (packet->GetSize() == 0) {
            break;
        }

        // Data has been received, set the flag
        m_responseReceived = true;

        double receivedData;
        packet->CopyData(reinterpret_cast<uint8_t*>(&receivedData), sizeof(receivedData));
        NS_LOG_UNCOND("Received data: " << receivedData);

        // Process the data using MyApplication
        double result = m_myApp.runPythonCode(receivedData);
        NS_LOG_UNCOND("Processed result: " << result);

        m_nextDataToSend = m_myApp.runPythonCode(receivedData);
        SendPacket();
    }
}
