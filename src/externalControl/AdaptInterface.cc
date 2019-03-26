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
#include "AdaptInterface.h"
#include <string>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <managers/execution/HPExecutionManagerMod.h>
#include "HPLoadBalancer.h"

Define_Module(AdaptInterface);

#define DEBUG_ADAPT_INTERFACE 1

using namespace std;

namespace {
    const string UNKNOWN_COMMAND = "error: unknown command\n";
    const string COMMAND_SUCCESS = "OK\n";
    const string INVALID_ARGUMENT = "Invalid command argument\n";
}


AdaptInterface::AdaptInterface() {
    commandHandlers["add_server"] = std::bind(&AdaptInterface::cmdAddServer, this, std::placeholders::_1);
    commandHandlers["remove_server"] = std::bind(&AdaptInterface::cmdRemoveServer, this, std::placeholders::_1);
    //commandHandlers["set_dimmer"] = std::bind(&AdaptInterface::cmdSetDimmer, this, std::placeholders::_1);
    commandHandlers["divert_traffic"] = std::bind(&AdaptInterface::cmdDivertTraffic, this, std::placeholders::_1);
    commandHandlers["inc_dimmer"] = std::bind(&AdaptInterface::cmdIncreaseDimmer, this, std::placeholders::_1);
    commandHandlers["dec_dimmer"] = std::bind(&AdaptInterface::cmdDecreaseDimmer, this, std::placeholders::_1);
    // get commands
    commandHandlers["get_dimmer"] = std::bind(&AdaptInterface::cmdGetDimmer, this, std::placeholders::_1);
    commandHandlers["get_servers"] = std::bind(&AdaptInterface::cmdGetServers, this, std::placeholders::_1);
    commandHandlers["get_active_servers"] = std::bind(&AdaptInterface::cmdGetActiveServers, this, std::placeholders::_1);
    commandHandlers["get_max_servers"] = std::bind(&AdaptInterface::cmdGetMaxServers, this, std::placeholders::_1);
    //commandHandlers["get_utilization"] = std::bind(&AdaptInterface::cmdGetUtilization, this, std::placeholders::_1);
    commandHandlers["get_avg_rt"] = std::bind(&AdaptInterface::cmdGetAvgResponseTime, this, std::placeholders::_1);
    //commandHandlers["get_basic_throughput"] = std::bind(&AdaptInterface::cmdGetBasicThroughput, this, std::placeholders::_1);
    //commandHandlers["get_opt_rt"] = std::bind(&AdaptInterface::cmdGetOptResponseTime, this, std::placeholders::_1);
    //commandHandlers["get_opt_throughput"] = std::bind(&AdaptInterface::cmdGetOptThroughput, this, std::placeholders::_1);
    commandHandlers["get_arrival_rate"] = std::bind(&AdaptInterface::cmdGetArrivalRate, this, std::placeholders::_1);
    commandHandlers["get_traffic"] = std::bind(&AdaptInterface::cmdGetTraffic, this, std::placeholders::_1);

    // dimmer, numServers, numActiveServers, utilization(total or indiv), response time and throughput for mandatory and optional, avg arrival rate
}

AdaptInterface::~AdaptInterface() {
    cancelAndDelete(rtEvent);
}

void AdaptInterface::initialize()
{
    rtEvent = new cMessage("rtEvent");
    rtScheduler = check_and_cast<cSocketRTScheduler *>(getSimulation()->getScheduler());
    rtScheduler->setInterfaceModule(this, rtEvent, recvBuffer, BUFFER_SIZE, &numRecvBytes);
    pModel = check_and_cast<HPModel*> (getParentModule()->getSubmodule("model"));
    //pProbe = check_and_cast<IProbe*> (gate("probe")->getPreviousGate()->getOwnerModule());
}

void AdaptInterface::handleMessage(cMessage *msg)
{
    if (msg == rtEvent) {

        // get data from buffer
        string input = string(recvBuffer, numRecvBytes);
        numRecvBytes = 0;
#if DEBUG_ADAPT_INTERFACE
        EV << "received [" << input << "]" << endl;
#endif
        std::istringstream inputStream(input);

        std::string line;
        while (std::getline(inputStream, line))
        {
            line.erase(line.find_last_not_of("\r\n") + 1);
#if DEBUG_ADAPT_INTERFACE
            cout << "received line is [" << line << "]" << endl;
#endif
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            tokenizer tokens(line, boost::char_separator<char>(" "));
            tokenizer::iterator it = tokens.begin();
            if (it != tokens.end()) {
                string command = *it;
                vector<string> args;
                while (++it != tokens.end()) {
                    args.push_back(*it);
                }

                auto handler = commandHandlers.find(command);
                if (handler == commandHandlers.end()) {
                    rtScheduler->sendBytes(UNKNOWN_COMMAND.c_str(), UNKNOWN_COMMAND.length());
                } else {
                    string reply = commandHandlers[command](args);
#if DEBUG_ADAPT_INTERFACE
                    cout << "command reply is[" << reply << ']' << endl;
#endif
                    rtScheduler->sendBytes(reply.c_str(), reply.length());
                }
            }

        }

    }
}

std::string AdaptInterface::cmdAddServer(const std::vector<std::string>& args) {
    if (args.size() == 0) {
            return "error: missing addServerCmd argument\n";
    }

    //string addServerCmd = args[0];
    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType(atoi(args[0].c_str()));

    /*if (addServerCmd == "addServerA_start") {
        serverType = MTServerAdvance::A;
    } else if(addServerCmd == "addServerB_start") {
        serverType = MTServerAdvance::B;
    } else if(addServerCmd == "addServerC_start") {
        serverType = MTServerAdvance::C;
    } else {
        return INVALID_ARGUMENT;
    }*/

    HPExecutionManagerModBase* pExecMgr = check_and_cast<HPExecutionManagerModBase*> (getParentModule()->getSubmodule("executionManager"));
    pExecMgr->addServer(serverType);

    return COMMAND_SUCCESS;
}

std::string AdaptInterface::cmdRemoveServer(const std::vector<std::string>& args) {
    if (args.size() == 0) {
            return "error: missing removeServerCmd argument\n";
    }

    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType(atoi(args[0].c_str()));

    /*if (removeServerCmd == "removeServerA_start") {
        serverType = MTServerAdvance::A;
    } else if(removeServerCmd == "removeServerB_start") {
        serverType = MTServerAdvance::B;
    } else if(removeServerCmd == "removeServerC_start") {
        serverType = MTServerAdvance::C;
    } else {
        return INVALID_ARGUMENT;
    }*/

    HPExecutionManagerModBase* pExecMgr = check_and_cast<HPExecutionManagerModBase*> (getParentModule()->getSubmodule("executionManager"));
    pExecMgr->removeServer(serverType);

    return COMMAND_SUCCESS;
}

/*std::string AdaptInterface::cmdSetDimmer(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        return "error: missing dimmer argument\n";
    }

    double dimmerFactor = atof(args[0].c_str());
    ExecutionManagerModBase* pExecMgr = check_and_cast<ExecutionManagerModBase*> (getParentModule()->getSubmodule("executionManager"));
    pExecMgr->setBrownout(dimmerFactor - 1);

    return COMMAND_SUCCESS;
}*/

// Refer MTBrownoutServer::generateJobServiceTime
// maximum dimmerLevel => newBrownoutFactor == 1 => No request served with options content.
// minimum dimmerLevel => newBrownoutFactor == 0 => all request served with options content
std::string AdaptInterface::setDimmer(int dimmerLevel) {
    double newBrownoutFactor = 0.0;

    newBrownoutFactor = (dimmerLevel - 1.0)
                                / (pModel->getNumberOfBrownoutLevels() - 1.0);

    HPExecutionManagerModBase* pExecMgr = check_and_cast<HPExecutionManagerModBase*> (getParentModule()->getSubmodule("executionManager"));
    pExecMgr->setBrownout(newBrownoutFactor);

    return COMMAND_SUCCESS;
}

// Less request served with option content
std::string AdaptInterface::cmdDecreaseDimmer(const std::vector<std::string>& args) {
    int dimmerLevel = pModel->getDimmerLevel();
    dimmerLevel++; // As factor increases less option content is served
    return setDimmer(dimmerLevel);
}

// More requests served with optional content
std::string AdaptInterface::cmdIncreaseDimmer(const std::vector<std::string>& args) {
    int dimmerLevel = pModel->getDimmerLevel();
    dimmerLevel--; // As factor decreases more option content is served
    return setDimmer(dimmerLevel);
}

// Ideally, instead of a string this command should get traffic load for each server.
std::string AdaptInterface::cmdDivertTraffic(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        return "error: missing divert_traffic argument\n";
    }

    string divertCmdString = args[0];
    HPLoadBalancer::TrafficLoad serverA = HPLoadBalancer::INVALID;
    HPLoadBalancer::TrafficLoad serverB = HPLoadBalancer::INVALID;
    HPLoadBalancer::TrafficLoad serverC = HPLoadBalancer::INVALID;

    if (divertCmdString == "divert_100_0_0") {
        serverA = HPLoadBalancer::TrafficLoad::HUNDRED;
        serverB = HPLoadBalancer::TrafficLoad::ZERO;
        serverC = HPLoadBalancer::TrafficLoad::ZERO;
    } else if (divertCmdString == "divert_75_25_0") {
        serverA = HPLoadBalancer::TrafficLoad::SEVENTYFIVE;
        serverB = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverC = HPLoadBalancer::TrafficLoad::ZERO;
    } else if (divertCmdString == "divert_75_0_25") {
        serverA = HPLoadBalancer::TrafficLoad::SEVENTYFIVE;
        serverB = HPLoadBalancer::TrafficLoad::ZERO;
        serverC = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
    } else if (divertCmdString == "divert_50_50_0") {
        serverA = HPLoadBalancer::TrafficLoad::FIFTY;
        serverB = HPLoadBalancer::TrafficLoad::FIFTY;
        serverC = HPLoadBalancer::TrafficLoad::ZERO;
    } else if (divertCmdString == "divert_50_0_50") {
        serverA = HPLoadBalancer::TrafficLoad::FIFTY;
        serverB = HPLoadBalancer::TrafficLoad::ZERO;
        serverC = HPLoadBalancer::TrafficLoad::FIFTY;
    } else if (divertCmdString == "divert_50_25_25") {
        serverA = HPLoadBalancer::TrafficLoad::FIFTY;
        serverB = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverC = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
    } else if (divertCmdString == "divert_25_75_0") {
        serverA = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverB = HPLoadBalancer::TrafficLoad::SEVENTYFIVE;
        serverC = HPLoadBalancer::TrafficLoad::ZERO;
    } else if (divertCmdString == "divert_25_0_75") {
        serverA = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverB = HPLoadBalancer::TrafficLoad::ZERO;
        serverC = HPLoadBalancer::TrafficLoad::SEVENTYFIVE;;
    } else if (divertCmdString == "divert_25_50_25") {
        serverA = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverB = HPLoadBalancer::TrafficLoad::FIFTY;
        serverC = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
    } else if (divertCmdString == "divert_25_25_50") {
        serverA = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverB = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverC = HPLoadBalancer::TrafficLoad::FIFTY;
    } else if (divertCmdString == "divert_0_100_0") {
        serverA = HPLoadBalancer::TrafficLoad::ZERO;
        serverB = HPLoadBalancer::TrafficLoad::HUNDRED;
        serverC = HPLoadBalancer::TrafficLoad::ZERO;
    } else if (divertCmdString == "divert_0_0_100") {
        serverA = HPLoadBalancer::TrafficLoad::ZERO;
        serverB = HPLoadBalancer::TrafficLoad::ZERO;
        serverC = HPLoadBalancer::TrafficLoad::HUNDRED;
    } else if (divertCmdString == "divert_0_75_25") {
        serverA = HPLoadBalancer::TrafficLoad::ZERO;
        serverB = HPLoadBalancer::TrafficLoad::SEVENTYFIVE;
        serverC = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
    } else if (divertCmdString == "divert_0_25_75") {
        serverA = HPLoadBalancer::TrafficLoad::ZERO;
        serverB = HPLoadBalancer::TrafficLoad::TWENTYFIVE;
        serverC = HPLoadBalancer::TrafficLoad::SEVENTYFIVE;
    } else if (divertCmdString == "divert_0_50_50") {
        serverA = HPLoadBalancer::TrafficLoad::ZERO;
        serverB = HPLoadBalancer::TrafficLoad::FIFTY;
        serverC = HPLoadBalancer::TrafficLoad::FIFTY;
    } else {
        return INVALID_ARGUMENT;
    }

    HPExecutionManagerModBase* pExecMgr = check_and_cast<HPExecutionManagerModBase*> (getParentModule()->getSubmodule("executionManager"));
    pExecMgr->divertTraffic(serverA, serverB, serverC);

    return COMMAND_SUCCESS;
}

std::string AdaptInterface::cmdGetTraffic(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        return "error: missing get_traffic argument\n";
    }

    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType(atoi(args[0].c_str()));

    ostringstream reply;
    reply << pModel->getConfiguration().getTraffic(serverType) << '\n';

    return reply.str();
}

std::string AdaptInterface::cmdGetDimmer(const std::vector<std::string>& args) {
    ostringstream reply;
    double brownoutFactor = pModel->getBrownoutFactor();
    double dimmer = 1 - brownoutFactor;
    //double dimmer = pModel->getDimmerLevel();
    reply << dimmer << '\n';
    //reply << 1 - brownoutFactor << '\n';

    return reply.str();
}

std::string AdaptInterface::cmdGetServers(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        return "error: missing get_servers argument\n";
    }

    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType(atoi(args[0].c_str()));

    ostringstream reply;
    reply << pModel->getConfiguration().getServers(serverType) << '\n';

    return reply.str();
}


std::string AdaptInterface::cmdGetActiveServers(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        return "error: missing get_active_servers argument\n";
    }

    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType(atoi(args[0].c_str()));

    ostringstream reply;
    reply << pModel->getConfiguration().getActiveServers(serverType) << '\n';

    return reply.str();
}


std::string AdaptInterface::cmdGetMaxServers(const std::vector<std::string>& args) {
    if (args.size() == 0) {
        return "error: missing get_active_servers argument\n";
    }

    int maxServer = 0;
    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType(atoi(args[0].c_str()));

    if (serverType == MTServerAdvance::A) {
        maxServer = omnetpp::getSimulation()->getSystemModule()->par("maxServersA");
    } else if (serverType == MTServerAdvance::B) {
        maxServer = omnetpp::getSimulation()->getSystemModule()->par("maxServersB");
    } else if (serverType == MTServerAdvance::C) {
        maxServer = omnetpp::getSimulation()->getSystemModule()->par("maxServersC");
    } else {
        return INVALID_ARGUMENT;
    }

    ostringstream reply;
    reply << maxServer << '\n';

    return reply.str();
}

/*
std::string AdaptInterface::cmdGetUtilization(const std::vector<std::string>& args) {
    if (args.size() {
        return "error: missing server argument\n";
    }

    ostringstream reply;
    auto utilization = pProbe->getUtilization(args[0]);
    if (utilization < 0) {
        reply << "error: server \'" << args[0] << "\' does no exist";
    } else {
        reply << utilization;
    }
    reply << '\n';

    return reply.str();
}
*/

std::string AdaptInterface::cmdGetAvgResponseTime(
        const std::vector<std::string>& args) {
    ostringstream reply;
    reply << pModel->getAvgResponseTime() << '\n';

    return reply.str();
}

/*
std::string AdaptInterface::cmdGetBasicThroughput(
        const std::vector<std::string>& args) {
    ostringstream reply;
    reply << pProbe->getBasicThroughput() << '\n';

    return reply.str();
}

std::string AdaptInterface::cmdGetOptResponseTime(
        const std::vector<std::string>& args) {
    ostringstream reply;
    reply << pProbe->getOptResponseTime() << '\n';

    return reply.str();
}

std::string AdaptInterface::cmdGetOptThroughput(
        const std::vector<std::string>& args) {
    ostringstream reply;
    reply << pProbe->getOptThroughput() << '\n';

    return reply.str();
}
*/

std::string AdaptInterface::cmdGetArrivalRate(
        const std::vector<std::string>& args) {
    ostringstream reply;
    reply << pModel->getArrivalRate() << '\n';

    return reply.str();
}
