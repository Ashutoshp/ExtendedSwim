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
#include "SwimClient.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <sstream>

using boost::asio::ip::tcp;
using namespace std;

/*std::string SwimClient::cmdAddServerA = "addServerA_start";
std::string SwimClient::cmdAddServerB = "addServerB_start";
std::string SwimClient::cmdAddServerC = "addServerC_start";*/

/*std::string SwimClient::cmdRemoveServerA = "removeServerA_start";
std::string SwimClient::cmdRemoveServerB = "removeServerB_start";
std::string SwimClient::cmdRemoveServerC = "removeServerC_start";*/

std::string SwimClient::cmdDecDimmer = "dec_dimmer";
std::string SwimClient::cmdIncDimmer = "inc_dimmer";

std::string SwimClient::cmdDivertTraffic_100_0_0 = "divert_100_0_0";
std::string SwimClient::cmdDivertTraffic_75_25_0 = "divert_75_25_0";
std::string SwimClient::cmdDivertTraffic_75_0_25 = "divert_75_0_25";
std::string SwimClient::cmdDivertTraffic_50_50_0 = "divert_50_50_0";
std::string SwimClient::cmdDivertTraffic_50_0_50 = "divert_50_0_50";
std::string SwimClient::cmdDivertTraffic_50_25_25 = "divert_50_25_25";
std::string SwimClient::cmdDivertTraffic_25_75_0 = "divert_25_75_0";
std::string SwimClient::cmdDivertTraffic_25_0_75 = "divert_25_0_75";
std::string SwimClient::cmdDivertTraffic_25_50_25 = "divert_25_50_25";
std::string SwimClient::cmdDivertTraffic_25_25_50 = "divert_25_25_50";
std::string SwimClient::cmdDivertTraffic_0_100_0 = "divert_0_100_0";
std::string SwimClient::cmdDivertTraffic_0_0_100 = "divert_0_0_100";
std::string SwimClient::cmdDivertTraffic_0_75_25 = "divert_0_75_25";
std::string SwimClient::cmdDivertTraffic_0_25_75 = "divert_0_25_75";
std::string SwimClient::cmdDivertTraffic_0_50_50 = "divert_0_50_50";

SwimClient::SwimClient() : socket(ioService) {
}

void SwimClient::connect(const char* host, const char* service) {
    tcp::resolver resolver(ioService);

    tcp::resolver::query query(host, service);
    tcp::resolver::iterator endpointIt = resolver.resolve(query);
    tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpointIt != end)
    {
      socket.close();
      socket.connect(*endpointIt++, error);
    }
    if (error) {
        socket.close();
        throw boost::system::system_error(error);
    }
}

bool SwimClient::isConnected() const {
    return socket.is_open();
}

std::string SwimClient::sendCommand(const char* command) {
    if (!socket.is_open()) {
        throw std::runtime_error("socket is closed");
    }

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(command, strlen(command)),
              boost::asio::transfer_all(), ignored_error);


    vector<char> buffer(128);
    boost::system::error_code error;

    size_t len = socket.read_some(boost::asio::buffer(buffer), error);

    if (error)
        throw boost::system::system_error(error);

    string reply(buffer.data(), len);
    if (reply.compare(0, 6, "error:") == 0) {
        // trim strings
        size_t last = reply.length() - 1;
        while (isspace(reply.at(last))) {
            reply.erase(last--);
        }
        string cmd(command);
        last = cmd.length() - 1;
        while (isspace(cmd.at(last))) {
            cmd.erase(last--);
        }

        throw std::logic_error(reply + "(cmd " + cmd + ')');
    }

    return reply;
}

double SwimClient::probeDouble(const char* command) {
    string reply = sendCommand(command);
    return atof(reply.c_str());
}

int SwimClient::probeInt(const char* command) {
    string reply = sendCommand(command);
    return atoi(reply.c_str());
}

double SwimClient::getDimmer() {
    return probeDouble("get_dimmer\n");
}

int SwimClient::getServers(SwimClient::ServerType serverType) {
    ostringstream cmd;
    cmd << "get_servers " << serverType << '\n';

    return probeInt(cmd.str().c_str());
}

int SwimClient::getActiveServers(SwimClient::ServerType serverType) {
    ostringstream cmd;
    cmd << "get_active_servers " << serverType << '\n';

    return probeInt(cmd.str().c_str());
}

int SwimClient::getMaxServers(SwimClient::ServerType serverType) {
    ostringstream cmd;
    cmd << "get_max_servers " << serverType << '\n';

    return probeInt(cmd.str().c_str());
}

/*
double SwimClient::getUtilization(int serverId) {
    ostringstream cmd;
    cmd << "get_utilization server" << serverId << '\n';
    return probeDouble(cmd.str().c_str());
}


double SwimClient::getOptionalResponseTime() {
    return probeDouble("get_opt_rt\n");
}

double SwimClient::getOptionalThroughput() {
    return probeDouble("get_opt_throughput\n");
}

double SwimClient::getBasicThroughput() {
    return probeDouble("get_basic_throughput\n");
}

*/


double SwimClient::getAverageResponseTime() {
    return probeDouble("get_avg_rt\n");
}

double SwimClient::getArrivalRate() {
    return probeDouble("get_arrival_rate\n");
}

void SwimClient::addServer(SwimClient::ServerType serverType) {
    ostringstream cmd;
    cmd << "add_server " << serverType << '\n';

    sendCommand(cmd.str().c_str());
}

void SwimClient::removeServer(SwimClient::ServerType serverType) {
    ostringstream cmd;
    cmd << "remove_server " << serverType << '\n';

    sendCommand(cmd.str().c_str());
}

/*void SwimClient::setDimmer(double dimmer) {
    ostringstream cmd;
    cmd << "set_dimmer " << dimmer << '\n';
    probeDouble(cmd.str().c_str());
}*/

void SwimClient::increaseDimmer() {
    ostringstream cmd;
    cmd << SwimClient::cmdIncDimmer << " \n";
    sendCommand(cmd.str().c_str());
}

void SwimClient::decreaseDimmer() {
    ostringstream cmd;
    cmd << SwimClient::cmdDecDimmer << " \n";
    sendCommand(cmd.str().c_str());
}

void SwimClient::divertTraffic(std::string divertTrafficCmd) {
    ostringstream cmd;
    cmd << "divert_traffic " << divertTrafficCmd << '\n';
    sendCommand(cmd.str().c_str());
}

void SwimClient::resetRevenue() {
    ostringstream cmd;
    cmd << "reset_revenue " << '\n';
    sendCommand(cmd.str().c_str());
}

double SwimClient::getRevenue() {
    return probeDouble("get_revenue\n");
}

SwimClient::~SwimClient() {
    socket.close();
}

/*
double SwimClient::getTotalUtilization() {
    double utilization = 0;
    int activeServers = getActiveServers();
    for (int s = 1; s <= activeServers; s++) {
        utilization += getUtilization(s);
    }
    return utilization;
}

double SwimClient::getAverageResponseTime() {
    double basicTput = getBasicThroughput();
    double optTput = getOptionalThroughput();

    double avgResponseTime = basicTput * getBasicResponseTime()
            + optTput * getOptionalResponseTime() / (basicTput + optTput);

    return avgResponseTime;
}
*/
