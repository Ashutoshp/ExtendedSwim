/*******************************************************************************
 * Simulator of Web Infrastructure and Management
 * Copyright (c) 2016 Carnegie Mellon University.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS," WITH NO WARRANTIES WHATSOEVER. CARNEGIE
 * MELLON UNIVERSITY EXPRESSLY DISCLAIMS TO THE FULLEST EXTENT PERMITTED BY LAW
 * ALL EXPRESS, IMPLIED, AND STATUTORY WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, AND NON-INFRINGEMENT OF PROPRIETARY RIGHTS.
 *
 * Released under a BSD license, please see license.txt for full terms.
 * DM-0003883
 *******************************************************************************/
#ifndef SWIMCLIENT_H_
#define SWIMCLIENT_H_

#include <stdexcept>
#include <string>
#include <boost/asio.hpp>

class SwimClient {

    boost::asio::io_service ioService;
    boost::asio::ip::tcp::tcp::socket socket;

    std::string sendCommand(const char* command);
    double probeDouble(const char* command);
    int probeInt(const char* command);

public:

    enum ServerType {
        NONE,
        A,
        B,
        C
    };

    enum TrafficLoad {
        ZERO, TWENTYFIVE, FIFTY, SEVENTYFIVE, HUNDRED, INVALID
    };

    /*static std::string cmdAddServerA;
    static std::string cmdAddServerB;
    static std::string cmdAddServerC;

    static std::string cmdRemoveServerA;
    static std::string cmdRemoveServerB;
    static std::string cmdRemoveServerC;*/

    static std::string cmdDecDimmer;
    static std::string cmdIncDimmer;

    static std::string cmdDivertTraffic_100_0_0;
    static std::string cmdDivertTraffic_75_25_0;
    static std::string cmdDivertTraffic_75_0_25;
    static std::string cmdDivertTraffic_50_50_0;
    static std::string cmdDivertTraffic_50_0_50;
    static std::string cmdDivertTraffic_50_25_25;
    static std::string cmdDivertTraffic_25_75_0;
    static std::string cmdDivertTraffic_25_0_75;
    static std::string cmdDivertTraffic_25_50_25;
    static std::string cmdDivertTraffic_25_25_50;
    static std::string cmdDivertTraffic_0_100_0;
    static std::string cmdDivertTraffic_0_0_100;
    static std::string cmdDivertTraffic_0_75_25;
    static std::string cmdDivertTraffic_0_25_75;
    static std::string cmdDivertTraffic_0_50_50;

    SwimClient();
    void connect(const char* host, const char* port = "4242");
    bool isConnected() const;

    // probes
    double getDimmer();
    int getServers(SwimClient::ServerType serverType);
    int getActiveServers(SwimClient::ServerType serverType);
    int getMaxServers(SwimClient::ServerType serverType);
    //double getUtilization(int serverId);
    //double getBasicResponseTime();
    //double getOptionalResponseTime();
    //double getBasicThroughput();
    //double getOptionalThroughput();
    double getArrivalRate();
    void increaseDimmer();
    void decreaseDimmer();


    // effectors
    void addServer(SwimClient::ServerType serverType);
    void removeServer(SwimClient::ServerType serverType);
    //void setDimmer(double dimmer);
    void divertTraffic(std::string cmd);

    // helper methods
    //double getTotalUtilization();
    double getAverageResponseTime();

    virtual ~SwimClient();
};

#endif /* SWIMCLIENT_H_ */
