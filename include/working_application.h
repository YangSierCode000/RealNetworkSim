// working_application.h
#ifndef WORKING_APPLICATION_H
#define WORKING_APPLICATION_H

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "my_application.h" // Include the header for your MyApplication

using namespace ns3;

// WorkingApplication definition
class WorkingApplication : public Application {
public:
    static TypeId GetTypeId();
    WorkingApplication();
    virtual ~WorkingApplication();

    void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

protected:
    virtual void StartApplication() override;
    virtual void StopApplication() override;

private:
    void ScheduleTx();
    void SendPacket();
    void ScheduleRetransmission();

    Ptr<Socket> m_socket;
    Address m_peer;
    double m_nextDataToSend;
    uint32_t m_packetSize;
    uint32_t m_nPackets;
    DataRate m_dataRate;
    EventId m_sendEvent;
    bool m_running;
    uint32_t m_packetsSent;

    // Instance of MyApplication to process received data
    MyApplication m_myApp;
    bool m_responseReceived;
    void HandleRead(Ptr<Socket> socket); // Handler for incoming packets
};

#endif // WORKING_APPLICATION_H
