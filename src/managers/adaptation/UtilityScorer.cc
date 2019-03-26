/**
 * Copyright (c) 2015 Carnegie Mellon University. All Rights Reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following acknowledgments
 * and disclaimers.

 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.

 * 3. The names "Carnegie Mellon University," "SEI" and/or "Software
 * Engineering Institute" shall not be used to endorse or promote
 * products derived from this software without prior written
 * permission. For written permission, please contact
 * permission@sei.cmu.edu.

 * 4. Products derived from this software may not be called "SEI" nor
 * may "SEI" appear in their names without prior written permission of
 * permission@sei.cmu.edu.

 * 5. Redistributions of any form whatsoever must retain the following
 * acknowledgment:

 * This material is based upon work funded and supported by the
 * Department of Defense under Contract No. FA8721-05-C-0003 with
 * Carnegie Mellon University for the operation of the Software
 * Engineering Institute, a federally funded research and development
 * center.

 * Any opinions, findings and conclusions or recommendations expressed
 * in this material are those of the author(s) and do not necessarily
 * reflect the views of the United States Department of Defense.

 * NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE
 * ENGINEERING INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS"
 * BASIS. CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT
 * LIMITED TO, WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY,
 * EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF THE MATERIAL. CARNEGIE
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH
 * RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
 * INFRINGEMENT.

 * This material has been approved for public release and unlimited
 * distribution.

 * DM-0002494
**/
#include <UtilityScorer.h>
#include <algorithm>
#include <cfloat>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <DebugFileInfo.h>

#if 0
const double WEIGHT_COST = 0.2;
const double WEIGHT_RESPONSE = 0.5;
const double WEIGHT_CONTENT = 0.3;

//const double WEIGHT_COST = 0.4;
//const double WEIGHT_RESPONSE = 0.6;
//const double WEIGHT_CONTENT = 0.0;

//const double UTILITY_COST[] = {1.00, 1.00, 0.90, 0.30, 0.10};
const double UTILITY_COST[] = {1.00, 1.00, 0.90, 0.75, 0.5};

const struct {double r; double u;} UTILITY_RESPONSE[] = {
        {0 , 1.00},
        {0.100, 1.00},
        {0.200, 0.99},
        {0.500, 0.90},
        {1.000, 0.75},
        {1.500, 0.50},
        {2.000, 0.25},
        {4.000, 0.00}
};

double getUtilityCost(unsigned servers) {
    return (servers < (sizeof(UTILITY_COST) / sizeof(*UTILITY_COST))) ? UTILITY_COST[servers] : 0.0;
}

double getUtilityResponse(double responseTime) {
    int i = 0;
    int length = (sizeof(UTILITY_RESPONSE) / sizeof(*UTILITY_RESPONSE));
    if (responseTime <= UTILITY_RESPONSE[0].r) {
        return UTILITY_RESPONSE[0].u;
    }
    if (responseTime >= UTILITY_RESPONSE[length - 1].r) {
        return UTILITY_RESPONSE[length - 1].u;
    }

    while (responseTime >= UTILITY_RESPONSE[i + 1].r) i++;

    double utility = UTILITY_RESPONSE[i].u
            + (UTILITY_RESPONSE[i + 1].u - UTILITY_RESPONSE[i].u)
                    * (responseTime - UTILITY_RESPONSE[i].r)
                    / (UTILITY_RESPONSE[i + 1].r - UTILITY_RESPONSE[i].r);
    return utility;
}

/**
 * returns utility per unit of time;
 */
double UtilityScorer::getUtility(const Configuration& configuration, const Environment& environment, const Observations& observations)
{
    double utilityResponse = getUtilityResponse(observations.avgResponseTime);
    double utilityCost = getUtilityCost(configuration.getServers());
    double utility = WEIGHT_RESPONSE * utilityResponse;
    utility += WEIGHT_CONTENT * std::max(1 - configuration.getBrownOutFactor(), 0.0)
            + WEIGHT_COST * utilityCost;

    if (utilityResponse == 0) {
        utility = 0;
    }

    if (observations.avgResponseTime == DBL_MAX) {
        utility = -1; // if the responseTime makes the system unstable, make the solution infeasible
    }
    return utility;
}
#else
double request_count = 0;

//const double RT_THRESHOLD = 1.0;
//const double SERVER_COST_SEC = 1;
//const double MAX_ARRIVAL_CAPACITY = 54;
//const double NORMAL_REVENUE = (SERVER_COST_SEC / MAX_ARRIVAL_CAPACITY) * 4;
//const double BROWNOUT_REVENUE = NORMAL_REVENUE / 2;

/*
 * The rationale for this utility is the following.
 * For each request Normal service is preferred to brownout service,
 * which is preferred to being late
 * MAX_ARRIVAL_CAPACITY is the max number of requests a single server can
 * process with normal service. We assume that the cost of a server is covered
 * with the revenue of processing half that much.
 * For low service, a server can process 18x more. Since normal service
 * is preferred, the revenue of a low service request is halfed.
 */
const double RT_THRESHOLD = 1.0; // TODO UNCOMMENT NEXT LINE
//const double RT_THRESHOLD = omnetpp::getSimulation()->getSystemModule()->par("responseTimeThreshold").doubleValue();
const double SERVER_COST_SEC = 1;


const double MAX_ARRIVAL_CAPACITY = 53.8;
const double MAX_ARRIVAL_CAPACITY_LOW = 921.8;

double NORMAL_REVENUE = (SERVER_COST_SEC / MAX_ARRIVAL_CAPACITY) * 2;
double BROWNOUT_REVENUE = (SERVER_COST_SEC / MAX_ARRIVAL_CAPACITY_LOW) * 3 / 2;

/*double SERVERA_COST_SEC = simulation.getSystemModule()->par("serverA_cost_sec").doubleValue();
double SERVERB_COST_SEC = simulation.getSystemModule()->par("serverB_cost_sec").doubleValue();
double SERVERC_COST_SEC = simulation.getSystemModule()->par("serverC_cost_sec").doubleValue();

double MAX_ARRIVALA_CAPACITY = simulation.getSystemModule()->par("max_arrival_capacity_A").doubleValue();
double MAX_ARRIVALA_CAPACITY_LOW = simulation.getSystemModule()->par("max_arrival_capacity_low_A").doubleValue();
double MAX_ARRIVALB_CAPACITY = simulation.getSystemModule()->par("max_arrival_capacity_B").doubleValue();
double MAX_ARRIVALB_CAPACITY_LOW = simulation.getSystemModule()->par("max_arrival_capacity_low_B").doubleValue();
double MAX_ARRIVALC_CAPACITY = simulation.getSystemModule()->par("max_arrival_capacity_C").doubleValue();
double MAX_ARRIVALC_CAPACITY_LOW = simulation.getSystemModule()->par("max_arrival_capacity_low_C").doubleValue();

double NORMAL_A_REVENUE = simulation.getSystemModule()->par("normal_revenue_A").doubleValue();
double DIMMER_A_REVENUE = simulation.getSystemModule()->par("dimmer_revenue_A").doubleValue();
double NORMAL_B_REVENUE = simulation.getSystemModule()->par("normal_revenue_B").doubleValue();
double DIMMER_B_REVENUE = simulation.getSystemModule()->par("dimmer_revenue_B").doubleValue();
double NORMAL_C_REVENUE = simulation.getSystemModule()->par("normal_revenue_C").doubleValue();
double DIMMER_C_REVENUE = simulation.getSystemModule()->par("dimmer_revenue_C").doubleValue();

const double HP_DIMMER_REVENUE = DIMMER_A_REVENUE + DIMMER_B_REVENUE + DIMMER_C_REVENUE;
const double HP_NORMAL_REVENUE = NORMAL_A_REVENUE + NORMAL_B_REVENUE + NORMAL_C_REVENUE;*/

void dump_util(double utility) {
    std::string file_name =
            "/home/ashutosp/swimExtension/Experiments/logs/";
    std::ofstream myfile;

    myfile.open(file_name, std::ios::app);
    myfile << utility << "\n";
    myfile.close();
}

/**
 * returns utility for a single request
 */
double UtilityScorer::getRequestUtility(double responseTime, bool lowService) {
    if (responseTime > 0 && responseTime <= RT_THRESHOLD) {
            return (lowService) ? BROWNOUT_REVENUE : NORMAL_REVENUE;
    }

    // Penalty for high response time
    return 0;
}

double UtilityScorer::getRequestUtility(double responseTime, bool lowService,
        MTServerAdvance::ServerType serverType) {
    double utility = omnetpp::getSimulation()->getSystemModule()->par(
            "penalty").doubleValue();
    assert(
            strcmp(omnetpp::getSimulation()->getSystemModule()->getFullName(), "SwimExtention") == 0);

    if (responseTime > 0 && responseTime <= RT_THRESHOLD) {
        double NORMAL_A_REVENUE = omnetpp::getSimulation()->getSystemModule()->par(
                "normal_revenue_A").doubleValue();
        double DIMMER_A_REVENUE = omnetpp::getSimulation()->getSystemModule()->par(
                "dimmer_revenue_A").doubleValue();
        double NORMAL_B_REVENUE = omnetpp::getSimulation()->getSystemModule()->par(
                "normal_revenue_B").doubleValue();
        double DIMMER_B_REVENUE = omnetpp::getSimulation()->getSystemModule()->par(
                "dimmer_revenue_B").doubleValue();
        double NORMAL_C_REVENUE = omnetpp::getSimulation()->getSystemModule()->par(
                "normal_revenue_C").doubleValue();
        double DIMMER_C_REVENUE = omnetpp::getSimulation()->getSystemModule()->par(
                "dimmer_revenue_C").doubleValue();

        double HP_DIMMER_REVENUE = 0;
        double HP_NORMAL_REVENUE = 0;

        switch (serverType) {
        case MTServerAdvance::ServerType::A:
            HP_DIMMER_REVENUE = DIMMER_A_REVENUE;
            HP_NORMAL_REVENUE = NORMAL_A_REVENUE;
            break;
        case MTServerAdvance::ServerType::B:
            HP_DIMMER_REVENUE = DIMMER_B_REVENUE;
            HP_NORMAL_REVENUE = NORMAL_B_REVENUE;
            break;
        case MTServerAdvance::ServerType::C:
            HP_DIMMER_REVENUE = DIMMER_C_REVENUE;
            HP_NORMAL_REVENUE = NORMAL_C_REVENUE;
            break;
        default:
            assert(false);
        }

        utility = (lowService) ? HP_DIMMER_REVENUE : HP_NORMAL_REVENUE;
        //if (a == 0) {
        //std::cout << "UtilityScorer::getRequestUtility = " << lowService << "##"
        //        << HP_DIMMER_REVENUE << "##" << HP_NORMAL_REVENUE << endl;
        //}
        //return (lowService) ? HP_DIMMER_REVENUE : HP_NORMAL_REVENUE;
    }

    //std::cout << "UtilityScorer::getRequestUtility = " << utility << "##" << responseTime << endl;

    //dump_util(utility);
    //++request_count;
    return utility;
}


/**
 * returns utility per unit of time;
 */
/*double UtilityScorer::getUtility(const Configuration& configuration, const Environment& environment, const Observations& observations)
{
    double cost = (configuration.getServers() - 1) * SERVER_COST_SEC;

    double throughput = 1/environment.getArrivalMean();

    double positiveUtility = throughput * (configuration.getBrownOutFactor() * BROWNOUT_REVENUE + (1-configuration.getBrownOutFactor()) * NORMAL_REVENUE);

    double utility = ((observations.avgResponseTime>RT_THRESHOLD || observations.avgResponseTime <= 0) ? 0 : positiveUtility) - cost;

    return utility;
}*/

/**
 * returns utility for a period
 *
 * @param configuration system configuration
 * @param revenue Total revenue for the period
 * @param periodLength length of the period in seconds
 */
/*double UtilityScorer::getPeriodUtility(const Configuration& configuration, double revenue, double periodLength) {
    double cost = (configuration.getServers() - 1) * SERVER_COST_SEC * periodLength;
    return revenue - cost;
}*/

double UtilityScorer::getPeriodUtility(const HPConfiguration& configuration, double revenue, double periodLength) {
    double SERVERA_COST_SEC = omnetpp::getSimulation()->getSystemModule()->par("serverA_cost_sec").doubleValue();
    double SERVERB_COST_SEC = omnetpp::getSimulation()->getSystemModule()->par("serverB_cost_sec").doubleValue();
    double SERVERC_COST_SEC = omnetpp::getSimulation()->getSystemModule()->par("serverC_cost_sec").doubleValue();

    double costA = (configuration.getServers(MTServerAdvance::ServerType::A)) * SERVERA_COST_SEC * periodLength;
    double costB = (configuration.getServers(MTServerAdvance::ServerType::B)) * SERVERB_COST_SEC * periodLength;
    double costC = (configuration.getServers(MTServerAdvance::ServerType::C)) * SERVERC_COST_SEC * periodLength;

    dump_util(revenue - costA - costB - costC);
    //dump_util(request_count);
    //request_count = 0;

    //std::cout << "UtilityScorer::getPeriodUtility = " << revenue << "##"
    //        << costA << "##"  << costB << "##" << costC << "##" << revenue - costA - costB - costC << std::endl;
    return revenue - costA - costB - costC;
}

double UtilityScorer::getPeriodUtility(const HPConfiguration& configuration,
        double revenue, double avgResponseTime, double requestCount, double periodLength) {
    double SERVERA_COST_SEC = omnetpp::getSimulation()->getSystemModule()->par("serverA_cost_sec").doubleValue();
    double SERVERB_COST_SEC = omnetpp::getSimulation()->getSystemModule()->par("serverB_cost_sec").doubleValue();
    double SERVERC_COST_SEC = omnetpp::getSimulation()->getSystemModule()->par("serverC_cost_sec").doubleValue();

    double costA = (configuration.getServers(MTServerAdvance::ServerType::A)) * SERVERA_COST_SEC * periodLength;
    double costB = (configuration.getServers(MTServerAdvance::ServerType::B)) * SERVERB_COST_SEC * periodLength;
    double costC = (configuration.getServers(MTServerAdvance::ServerType::C)) * SERVERC_COST_SEC * periodLength;
    double penalty = omnetpp::getSimulation()->getSystemModule()->par(
            "penalty").doubleValue();
    double earning = (avgResponseTime > RT_THRESHOLD) ? requestCount * penalty : revenue;
    dump_util(earning - costA - costB - costC);

    return earning - costA - costB - costC;
}
#endif
