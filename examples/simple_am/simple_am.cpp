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

using namespace std;

const int NUMBER_OF_DIMMER_LEVELS = 3;
const double DIMMER_STEP = 1.0 / (NUMBER_OF_DIMMER_LEVELS - 1);
const double RT_THRESHOLD = 1.00;
const int PERIOD = 60;

std::string getDivertTrafficCommand(int activeServersA, int activeServersB, int activeServersC) {
    std::string cmd = "";

    if (activeServersA && activeServersB && activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_50_25_25;
    } else if (!activeServersA && activeServersB && activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_0_50_50;
    } else if (activeServersA && !activeServersB && activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_50_0_50;
    } else if (activeServersA && activeServersB && !activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_50_50_0;
    } else if (!activeServersA && !activeServersB && activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_0_0_100;
    } else if (!activeServersA && activeServersB && !activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_0_100_0;
    } else if (activeServersA && !activeServersB && !activeServersC) {
        cmd = SwimClient::cmdDivertTraffic_100_0_0;
    }

    return cmd;
}

void handleHighResponseTime(SwimClient& swim) {
    int serversA = swim.getServers(SwimClient::A);
    int serversB = swim.getServers(SwimClient::B);
    int serversC = swim.getServers(SwimClient::C);
    double dimmerFactor = swim.getDimmer();

    int totalServers = serversA + serversB + serversC;

    int activeServersA = swim.getActiveServers(SwimClient::A);
    int activeServersB = swim.getActiveServers(SwimClient::B);
    int activeServersC = swim.getActiveServers(SwimClient::C);

    int totalActiveServers = activeServersA + activeServersB + activeServersC;
    bool isServerBooting = (totalServers > totalActiveServers);
    std::string divertTrafficCmd = getDivertTrafficCommand(activeServersA, activeServersB, activeServersC);
    swim.divertTraffic(divertTrafficCmd);

    if (!isServerBooting) {
        if (serversC < swim.getMaxServers(SwimClient::C)) {
            swim.addServer(SwimClient::C);
        } else if (serversB < swim.getMaxServers(SwimClient::B)) {
            swim.addServer(SwimClient::B);
        } else if (serversA < swim.getMaxServers(SwimClient::A)) {
            swim.addServer(SwimClient::A);
        }
    }

    // Compare probability for a request having an optional content
    // Less dimmer factor implies lesser request are served with optional content
    // Therefore, if probability is greater than 0 then decrease it.
    if (dimmerFactor > 0.0) {
        swim.decreaseDimmer();
    }
}

void handleLowResponseTime(SwimClient& swim) {
    int serversA = swim.getServers(SwimClient::A);
    int serversB = swim.getServers(SwimClient::B);
    int serversC = swim.getServers(SwimClient::C);
    double dimmerFactor = swim.getDimmer();

    int totalServers = serversA + serversB + serversC;

    int activeServersA = swim.getActiveServers(SwimClient::A);
    int activeServersB = swim.getActiveServers(SwimClient::B);
    int activeServersC = swim.getActiveServers(SwimClient::C);

    int totalActiveServers = activeServersA + activeServersB + activeServersC;
    bool isServerBooting = (totalServers > totalActiveServers);

    if (!isServerBooting) {
        if (activeServersA > 0) {
            activeServersA = false;

            std::string divertTrafficCmd = getDivertTrafficCommand(activeServersA, activeServersB, activeServersC);
            swim.divertTraffic(divertTrafficCmd);
            swim.removeServer(SwimClient::A);
        } else if(activeServersB > 0) {
            activeServersB = false;

            std::string divertTrafficCmd = getDivertTrafficCommand(activeServersA, activeServersB, activeServersC);
            swim.divertTraffic(divertTrafficCmd);
            swim.removeServer(SwimClient::B);
        } else if (activeServersC > 0) {
            activeServersC = false;

            std::string divertTrafficCmd = getDivertTrafficCommand(activeServersA, activeServersB, activeServersC);
            swim.divertTraffic(divertTrafficCmd);
            swim.removeServer(SwimClient::C);
        }

    }

    // Compare probability for a request having an optional content
    // More dimmer factor implies more request are served with optional content
    // Therefore, if probability is less than 1 then increase it.
    if (dimmerFactor < 1.0) {
        swim.increaseDimmer();
    }
}

void simpleAdaptationManager(SwimClient& swim) {
    static bool test = true;

    while (swim.isConnected()) {
        if (test) {
            swim.addServer(SwimClient::C);
            swim.decreaseDimmer();
            test = false;
        }

        double responseTime = swim.getAverageResponseTime();

        if (responseTime > RT_THRESHOLD) {
            handleHighResponseTime(swim);
        } else if (responseTime < RT_THRESHOLD) {
            handleLowResponseTime(swim);
        }

        sleep(PERIOD);
    }
}

int main(int argc, char* argv[]) {
    SwimClient swim;

    do {
        try {
            if (argc == 2) {
                swim.connect(argv[1]);
            } else if (argc == 3) {
                swim.connect(argv[1], argv[2]);
            } else {
                cout << "usage: " << argv[0] << " host [port]" << endl;
                return EXIT_FAILURE;
            }
        }
        catch(boost::system::system_error& e) {
            if (e.code() == boost::asio::error::connection_refused) {

                // wait until SWIM is accepting connections
                cout << "Waiting for SWIM..." << endl;
                sleep(3);
            } else {
                std::cout << e.what() << std::endl;
                return EXIT_FAILURE;
            }
        }
    } while(!swim.isConnected());

    cout << "Connected to SWIM" << endl;

    try {
        simpleAdaptationManager(swim);
    }
    catch(boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            cout << "SWIM ended run" << endl;
        } else {
            std::cout << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
