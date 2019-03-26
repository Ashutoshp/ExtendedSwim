//
 // Copyright (c) 2015 Carnegie Mellon University. All Rights Reserved.

 // Redistribution and use in source and binary forms, with or without
 // modification, are permitted provided that the following conditions
 // are met:

 // 1. Redistributions of source code must retain the above copyright
 // notice, this list of conditions and the following acknowledgments
 // and disclaimers.

 // 2. Redistributions in binary form must reproduce the above
 // copyright notice, this list of conditions and the following
 // disclaimer in the documentation and/or other materials provided
 // with the distribution.

 // 3. The names "Carnegie Mellon University," "SEI" and/or "Software
 // Engineering Institute" shall not be used to endorse or promote
 // products derived from this software without prior written
 // permission. For written permission, please contact
 // permission@sei.cmu.edu.

 // 4. Products derived from this software may not be called "SEI" nor
 // may "SEI" appear in their names without prior written permission of
 // permission@sei.cmu.edu.

 // 5. Redistributions of any form whatsoever must retain the following
 // acknowledgment:

 // This material is based upon work funded and supported by the
 // Department of Defense under Contract No. FA8721-05-C-0003 with
 // Carnegie Mellon University for the operation of the Software
 // Engineering Institute, a federally funded research and development
 // center.

 // Any opinions, findings and conclusions or recommendations expressed
 // in this material are those of the author(s) and do not necessarily
 // reflect the views of the United States Department of Defense.

 // NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE
 // ENGINEERING INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS"
 // BASIS. CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND,
 // EITHER EXPRESSED OR IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT
 // LIMITED TO, WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY,
 // EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF THE MATERIAL. CARNEGIE
 // MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH
 // RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
 // INFRINGEMENT.

 // This material has been approved for public release and unlimited
 // distribution.

 // DM-0002494
 //

#include "HPMonitor.h"
#include <sstream>
#include "PassiveQueueDyn.h"
#include <math.h>
#include "PredictableSource.h"
#include "UtilityScorer.h"
#include "ModulePriorities.h"
#include <iostream>
#include <fstream>
#include <DebugFileInfo.h>
#include "HPExecutionManagerModBase.h"

using namespace boost::accumulators;
using namespace std;

Define_Module(HPMonitor);

#define MDL_RESPONSE_TIME 0

HPMonitor::HPMonitor() {
    periodEvent = 0;
    periodPostEvent = 0;
}

void HPMonitor::initialize() {
    numberOfServersSignal = registerSignal("numberOfServers");
    activeServersSignal = registerSignal("activeServers");
    measuredInterarrivalAvg = registerSignal("measuredInterarrivalAvg");
    measuredInterarrivalStdDev = registerSignal("measuredInterarrivalStdDev");
    utilitySignal = registerSignal("utility");
    brownoutFactorSignal = registerSignal("brownoutFactor");

    serverRemovedSignal = registerSignal(HPExecutionManagerModBase::SIG_SERVER_REMOVED);
    serverAddedSignal = registerSignal(HPExecutionManagerModBase::SIG_SERVER_ADDED);
    serverActivatedSignal = registerSignal(HPExecutionManagerModBase::SIG_SERVER_ACTIVATED);
    brownoutSetSignal = registerSignal(HPExecutionManagerModBase::SIG_BROWNOUT_SET);

    jobCompletionSignal = registerSignal("jobCompletion");
    serverActiveSignal = registerSignal("activeServer");
    predictedInterArrival = registerSignal("predictedInterArrival");

    interArrivalSignal = registerSignal("interArrival");
    lifeTimeSignalId = registerSignal("lifeTime");
    serverBusySignalId = registerSignal("busy");

    EV << "subscribing to signals" << endl;
    omnetpp::getSimulation()->getSystemModule()->subscribe(lifeTimeSignalId, this);
    if (omnetpp::getSimulation()->getSystemModule()->isSubscribed(lifeTimeSignalId, this)) {
        EV << "lifeTime subscribed" << endl;
    }

    omnetpp::getSimulation()->getSystemModule()->subscribe(jobCompletionSignal, this);
    if (omnetpp::getSimulation()->getSystemModule()->isSubscribed(jobCompletionSignal, this)) {
        EV << "jobCompletionSignal subscribed" << endl;
    }

    omnetpp::getSimulation()->getSystemModule()->subscribe(serverBusySignalId, this);
    if (omnetpp::getSimulation()->getSystemModule()->isSubscribed(serverBusySignalId, this)) {
        EV << "busy subscribed" << endl;
    }

    omnetpp::getSimulation()->getSystemModule()->subscribe(interArrivalSignal, this);
    if (omnetpp::getSimulation()->getSystemModule()->isSubscribed(interArrivalSignal, this)) {
        EV << "interArrival subscribed" << endl;
    }

    omnetpp::getSimulation()->getSystemModule()->subscribe(serverActiveSignal, this);
    if (omnetpp::getSimulation()->getSystemModule()->isSubscribed(serverActiveSignal, this)) {
        EV << "serverActiveSignal subscribed" << endl;
    }

    responseTimeCumulativePeriod = 0;
    responseTimeCountPeriod = 0;
    requestUtilityCumulative = 0;
    pInterArrivalStats.reset(new InterArrivalStats);

    pModel = check_and_cast<HPModel*>(
            getParentModule()->getSubmodule("model"));

    // Create the event object we'll use for timing -- just any ordinary message.
    periodEvent = new cMessage("periodEvent");
    periodEvent->setSchedulingPriority(MONITOR_PRE_PRIO);
#if HALF_PERIOD
    scheduleAt(pModel->getEvaluationPeriod() / 2.0, periodEvent);
#else
    scheduleAt(pModel->getEvaluationPeriod(), periodEvent);
#endif
    halfPeriod = false;

    periodPostEvent = new cMessage("periodPostEvent");
    periodPostEvent->setSchedulingPriority(MONITOR_POST_PRIO);
    scheduleAt(0, periodPostEvent);
}

void HPMonitor::dump_utility(const double & utility) {
    //if (utility == -60 || utility == -96 || utility == 216) {
    //    cout << "HPMonitor::dump_utility = " << utility << endl;
    //}
    string file_name = DebugFileInfo::getInstance()->GetUtilityFilePath();
            //"/home/ashutosp/Dropbox/regression/HPMonitor_utility";
    std::ofstream myfile;

    myfile.open (file_name, ios::app);
    myfile << utility << "\n";
    myfile.close();

    file_name = DebugFileInfo::getInstance()->GetDebugFilePath();
    //        "/home/ashutosp/Dropbox/regression/HP_triggers_arrival_rate";
    //string file_name = "/home/ashutosp/response_time";
    //std::ofstream myfile;

    myfile.open(file_name, ios::app);
    myfile << utility << "====";
    myfile.close();
}

void HPMonitor::handleMessage(cMessage *msg) {
    /*
     * the monitoring is divided into to parts. One that executes before the adaptation manager
     * and the other that executes after, so that statistics signals can reflect what the
     * adaptation manager just did (like adding a server)
     */
    //printf("HPMonitor::handleMessage %s\n", msg->getName());

    bool debug = false;
    if (simTime().dbl() == 3000) {
        debug = true;
    }
    if (msg == periodEvent) {
#if HALF_PERIOD
        halfPeriod = !halfPeriod;
#endif

        // update interarrival stats
        double measuredMeanInterArrival = mean(*pInterArrivalStats);
        double measuredVariance = boost::accumulators::moment<2>(
                *pInterArrivalStats);
        if (std::isnan(measuredMeanInterArrival)) {
            measuredMeanInterArrival = 0;
            measuredVariance = 0;
        }

        Environment environment;
        environment.setArrivalMean(measuredMeanInterArrival);
        environment.setArrivalVariance(measuredVariance);
        pModel->setEnvironment(environment);

        cout << "Arrival Mean = " << environment.getArrivalMean() << endl;

        if (!halfPeriod) {
            emit(measuredInterarrivalAvg, measuredMeanInterArrival);
            emit(measuredInterarrivalStdDev, sqrt(measuredVariance));

            // TODO
            Observations observations;
            observations.avgResponseTime =
                    (responseTimeCountPeriod > 0) ?
                            responseTimeCumulativePeriod
                                    / responseTimeCountPeriod :
                            0;

            // compute total utilization
            UtilizationTable::iterator it = utilizationTable.begin();
            while (it != utilizationTable.end()) {
                if (omnetpp::getSimulation()->getModule(it->first) == NULL) {
                    // remove it from table
                    utilizationTable.erase(it++);
                } else {
                    observations.utilization += it->second.getUtilization();
                    ++it;
                }
            }

            pModel->setObservations(observations);
            requestCount = responseTimeCountPeriod;


    #if MDL_RESPONSE_TIME
            int activeServerCountForUtil = model.getActiveServerCountIn(-1);
            averageResponseTime = QueueModel::totalTime(activeServerCountForUtil, getServiceRate(lowFidelity ? 1 : 0), meanInterArrival, sqrt(variance));
    #endif
    #if AVERAGED_UTILITY
            double periodUtility = UtilityScorer::getUtility(
                    pModel->getConfiguration(), pModel->getEnvironment(), pModel->getObservations())
                * pModel->getEvaluationPeriod();
    #endif
            double periodUtility = UtilityScorer::getPeriodUtility(
                    pModel->getConfiguration(), requestUtilityCumulative, pModel->getEvaluationPeriod());

            //double periodUtility = UtilityScorer::getPeriodUtility(
            //                    pModel->getConfiguration(), requestUtilityCumulative,
            //                    observations.avgResponseTime, requestCount, pModel->getEvaluationPeriod());
            emit(utilitySignal, periodUtility);
            dump_utility(periodUtility);

            // reset statistics
            responseTimeCumulativePeriod = 0;
            responseTimeCountPeriod = 0;
            requestUtilityCumulative = 0;
        }

        pInterArrivalStats.reset(new InterArrivalStats);

        // schedule the next period
#if HALF_PERIOD
        scheduleAt(simTime() + pModel->getEvaluationPeriod() / 2.0, periodEvent);
#else
        scheduleAt(simTime() + pModel->getEvaluationPeriod(), periodEvent);
#endif
    } else if (msg == periodPostEvent) {

        // reset server utilization stats
        UtilizationTable::iterator it = utilizationTable.begin();
        while (it != utilizationTable.end()) {
            if (omnetpp::getSimulation()->getModule(it->first) == NULL) {
                // remove it from table
                utilizationTable.erase(it++);
            } else {
                it->second.reset();
                ++it;
            }
        }

        emit(numberOfServersSignal,
                pModel->getConfiguration().getServers(MTServerAdvance::ServerType::A)
                + pModel->getConfiguration().getServers(MTServerAdvance::ServerType::B)
                + pModel->getConfiguration().getServers(MTServerAdvance::ServerType::C));

        emit(activeServersSignal,
                pModel->getConfiguration().getActiveServers(MTServerAdvance::ServerType::A)
                + pModel->getConfiguration().getActiveServers(MTServerAdvance::ServerType::B)
                + pModel->getConfiguration().getActiveServers(MTServerAdvance::ServerType::C));

        emit(brownoutFactorSignal,
                pModel->getConfiguration().getBrownOutFactor());

        // schedule the next period
        scheduleAt(simTime() + pModel->getEvaluationPeriod(), periodPostEvent);
    }
}

void HPMonitor::receiveSignal(cComponent *source, simsignal_t signalID,
        const SimTime& t, cObject *details) {
    if (signalID == lifeTimeSignalId) {
        //EV << "received signal from " << source->getName() << "value is " << t << endl;
        latestJobResponseTime = t;
    }
}

void HPMonitor::receiveSignal(cComponent *source, simsignal_t signalID,
        const char* jobName, cObject *details) {
    if (signalID == jobCompletionSignal) {
            //EV << "received signal from " << source->getName() << "value is " << t << endl;
            responseTimeCumulativePeriod += latestJobResponseTime.dbl();
            responseTimeCountPeriod++;
            MTServerAdvance::ServerType serverType = pModel->getJobServerInfo(jobName);
            //double a = requestUtilityCumulative;
            requestUtilityCumulative += UtilityScorer::getRequestUtility(latestJobResponseTime.dbl(),
                    strcmp(source->getName(), "sinkLow") == 0, serverType);
            //cout << "Job Utility = " << requestUtilityCumulative - a << endl;
    //        if (t.dbl() > responseTimeThreshold) {
    //            emit(requestLateSignal, 1);
    //        }
    }
}

/*void HPMonitor::receiveSignal(cComponent *source, simsignal_t signalID,
        const * cObject) {
    if (signalID == lifeTimeSignalId) {
        HPJob *job = check_and_cast<HPJob *>(job);
        //const SimTime t = simTime()- job->getCreationTime();
        //EV << "received signal from " << source->getName() << "value is " << t << endl;
        //responseTimeCumulativePeriod += t.dbl();
        //responseTimeCountPeriod++;
        //MTServerAdvance::ServerType serverType = pModel->getJobServerInfo(job->getName());
        //requestUtilityCumulative += UtilityScorer::getRequestUtility(t.dbl(), strcmp(source->getName(), "sinkLow") == 0, serverType);
    }
}*/

//void HPMonitor::receiveSignal(cComponent *source, simsignal_t signalID,
//        const SimTime& t, ) {

//}

void HPMonitor::receiveSignal(cComponent *source, simsignal_t signalID,
        bool value, cObject *details) {
    if (signalID == serverBusySignalId) {
        if (value) {
            utilizationTable[check_and_cast<cModule*>(source)->getId()].busy();
        } else {
            utilizationTable[check_and_cast<cModule*>(source)->getId()].idle();
        }
    } else if (signalID == serverActiveSignal) {
        emit(activeServersSignal,
                pModel->getConfiguration().getActiveServers(MTServerAdvance::ServerType::A)
                + pModel->getConfiguration().getActiveServers(MTServerAdvance::ServerType::B)
                + pModel->getConfiguration().getActiveServers(MTServerAdvance::ServerType::C));
    }
}

void HPMonitor::receiveSignal(cComponent *source, simsignal_t signalID,
        double value, cObject *details) {
    if (signalID == interArrivalSignal) {
//        EV << "received interArrival signal from " << source->getName() << "value is " << value << endl;
        //EV << "interarrival value " << value << " time " << simTime() << endl;
        pInterArrivalStats->operator ()(value);
    }
}

HPMonitor::~HPMonitor() {
    if (omnetpp::getSimulation()->getSystemModule()->isSubscribed(lifeTimeSignalId, this)) {
        omnetpp::getSimulation()->getSystemModule()->unsubscribe(lifeTimeSignalId, this);
    }
    cancelAndDelete(periodEvent);
    cancelAndDelete(periodPostEvent);
}
