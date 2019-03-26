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

#include "PredictableSource.h"
#include "Job.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include "string"
//#include <ClassifyProblemInstance.h>
#include <DebugFileInfo.h>

Define_Module(PredictableSource);

using namespace boost::accumulators;
using namespace std;

void cleanLogs() {
    char command[4096];

    /*std::string dir_path = "/home/ashutosp/Dropbox/regression/";
    std::string MC_triggers_arrival_rate = dir_path + "MC_triggers_arrival_rate";
    std::string HP_triggers_arrival_rate = dir_path + "HP_triggers_arrival_rate";
    std::string HPMonitor_utility = dir_path + "HPMonitor_utility";
    std::string HPModel_arrival_rate = dir_path + "HPModel_arrival_rate";
    std::string HPModel_response_time = dir_path + "HPModel_response_time";
    std::string HP_cycle_response_time = dir_path + "HP_cycle_response_time";
    std::string model_arrival_rate = dir_path + "model_arrival_rate";*/


    sprintf(command, "rm -rf %s %s %s %s", DebugFileInfo::getInstance()->GetDebugFilePath().c_str(),
            DebugFileInfo::getInstance()->GetUtilityFilePath().c_str(),
            DebugFileInfo::getInstance()->GetResponseTimeFilePath().c_str(),
            DebugFileInfo::getInstance()->GetRequestArrivalFilePath().c_str());
            //"/home/ashutosp/Sandbox/plasasim/simulations/brownout/model*");
    printf("%s\n", command);
    int res = system(command);

    assert(res!=-1);
}

void BuildClassifier(string sourceFile) {
    // TODO where to call destructor.
    int maxServersA = omnetpp::getSimulation()->getSystemModule()->par("maxServersA");
    int maxServersB = omnetpp::getSimulation()->getSystemModule()->par("maxServersB");
    int maxServersC = omnetpp::getSimulation()->getSystemModule()->par("maxServersC");

    int maxServers = maxServersA + maxServersB + maxServersC;

    double evaluationPeriod =
                omnetpp::getSimulation()->getSystemModule()->par("evaluationPeriod").doubleValue();
    double bootDelay = omnetpp::getSimulation()->getSystemModule()->par("bootDelay");
    unsigned horizon = max(5.0, ceil(bootDelay / evaluationPeriod) * (maxServers - 1) + 1);

    unsigned warmup_period = max(10, int(ceil(omnetpp::getSimulation()->getWarmupPeriod().dbl() / evaluationPeriod)));
    //string sourceFile = par("interArrivalsFile").stringValue();

    //unsigned time_series_length_for_comparision = (omnetpp::getSimulation()->getSystemModule()
    //        ->par("comparePastWorkloadForSimilarity").boolValue()) ?
    //                horizon + warmup_period + 1 : horizon + 1;

    /*std::cout << "########## Building Classifier ########" << std::endl;
    string problem_db_file = omnetpp::getSimulation()->getSystemModule()->par(
                        "pathToProfiledProblems").stdstringValue();
    ClassifyProblemInstance::getInstance(problem_db_file, sourceFile);
    std::cout << "########## Classifier Ready ###########" << std::endl;*/

    //ProblemDB::getInstance()->getInstance()->populate_db();
}

void PredictableSource::preload() {
    sourceFile = par("interArrivalsFile").stringValue();

    string logParentDirectory = omnetpp::getSimulation()->getSystemModule()->par(
            "pathToLoggingDir").stdstringValue();
    string mode = omnetpp::getSimulation()->getSystemModule()->par(
            "mode").stdstringValue();

    // TODO Remove hack about classifierMode later.
    if (!omnetpp::getSimulation()->getSystemModule()->par("classifierTestMode").boolValue()
            && !omnetpp::getSimulation()->getSystemModule()->par("problemGenerationMode").boolValue()) {
        DebugFileInfo::getInstance(false, sourceFile, logParentDirectory, mode);
    } else {
        DebugFileInfo::getInstance(true, sourceFile, logParentDirectory, mode);
    }

    cleanLogs();

    //std::string file_name = "/home/ashutosp/Dropbox/regression/HP_triggers_arrival_rate";
    std::string file_name = DebugFileInfo::getInstance()->GetDebugFilePath();
    std::ofstream myfile;
    myfile.open(file_name, std::ios::app);
    myfile << sourceFile << std::endl;
    myfile.close();

    if (omnetpp::getSimulation()->getSystemModule()->par("useIBL").boolValue()
            || omnetpp::getSimulation()->getSystemModule()->par("classifierTestMode").boolValue()) {
        BuildClassifier(sourceFile);
    }

    double arrivalTime = 0;
    //sourceFile = filePath; // Replace filePath with sourceFile
    ifstream fin(sourceFile.c_str());
    if (!fin) {
        error("PredictableSource %s could not read input file '%s'", this->getFullName(), sourceFile);
    } else {
        double timeValue;
        while (fin >> timeValue) {
            timeValue *= scale;
            arrivalTime += timeValue;
            arrivalTimes.push_back(arrivalTime);
            interArrivalTimes.push_back(timeValue);
        }
        fin.close();
        EV << "read " << arrivalTimes.size() << " elements from " << sourceFile << endl;
    }
}

bool PredictableSource::generateArrival() {
    return false;
}

void PredictableSource::initialize()
{
    SourceBase::initialize();
    scale = par("scale").doubleValue();
    sourceFile = "";

    nextArrivalIndex = 0;
    preload();

    // schedule the first message timer, if there is one
    if (interArrivalTimes.size() > 0) {
        scheduleAt(interArrivalTimes[nextArrivalIndex++], new cMessage("newJobTimer"));
    }
}

void PredictableSource::handleMessage(cMessage *msg)
{
    ASSERT(msg->isSelfMessage());

    if (nextArrivalIndex < interArrivalTimes.size() || generateArrival())
    {
        // reschedule the timer for the next message
        scheduleAt(simTime() + interArrivalTimes[nextArrivalIndex++], msg);

        queueing::Job *job = createJob();
        send(job, "out");
    }
    else
    {
        // finished
        delete msg;
    }

}

double PredictableSource::getPrediction(double startDelta, double windowDuration, double* pVariance, bool debug) {
    double average = 0;
    double variance = 0;

    // skip until beginning of window
    simtime_t start = simTime() + startDelta;
    unsigned index = nextArrivalIndex;
    if (index == 0) {
        index++; // the first one is not really valid because there was no previous arrival
    } else if (index > 1) {
        index--; // because nextArrivalIndex points to the next arrival to be scheduled, which means that nextArrivalIndex-1 points to the one that has been scheduled and not happened yet
    }
    while (index < arrivalTimes.size() && arrivalTimes[index] < start.dbl()) {
        index++;
    }


    if (index < arrivalTimes.size()) {
        accumulator_set<double, stats<tag::mean, tag::moment<2> > > interArrivalStats;

        double windowEnd = start.dbl() + windowDuration;
        while (index < arrivalTimes.size() && arrivalTimes[index] <= windowEnd) {
            if (debug) {
                EV << "dbginterarrival value " << interArrivalTimes[index] << " time " << arrivalTimes[index] << endl;
            }
            interArrivalStats(interArrivalTimes[index]);
            index++;
        }

        average = mean(interArrivalStats);
        variance = moment<2>(interArrivalStats);
        if (std::isnan(average)) {
            average = 0;
            variance = 0;
        }
        if (debug) {
            EV << "dbginterarrival mean " << average << endl;
        }
    }

    if (pVariance) {
        *pVariance = variance;
    }
    return average;
}

