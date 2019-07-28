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
#include "HPModel.h"
#include <omnetpp.h>
#include <execution/HPExecutionManagerModBase.h>
#include "modules/PredictableRandomSource.h"
#include <sstream>
#include <math.h>
#include <iostream>
#include <fstream>
//#include <EnvPredictionRecord.h>
#include <DebugFileInfo.h>

using namespace std;

// repeat the root node of the probability tree (i.e., the last observation)
// so that the prediction for the period about to start is the last observation
#define REPEAT_PROB_TREE_ROOT 0

Define_Module(HPModel);

const unsigned PROB_TREE_DEPTH = 4;
const unsigned PROB_TREE_BRANCHING = 2;

HPModel::HPModel() : timeActiveServerCountLast(0.0),
                     lastConfigurationUpdate(0.0), currentPeriod(-1) {

}

HPModel::~HPModel() {
    //if (pArrivalMeanPredictor) delete(pArrivalMeanPredictor);
}


void HPModel::addExpectedChange(double time, ModelChange change) {
    //if (time > simTime().dbl()) { // apandey
        ModelChangeEvent event;
        event.startTime = simTime().dbl();
        event.time = time;
        event.change = change;
        events.insert(event);
    //} else {
    //    std::cout << "addExpectedChange(): skipping for not being in the future"
    //            << std::endl;
    //}
}

void HPModel::removeExpectedChange() {
    if (!events.empty()) {
        events.erase(--events.end());
    } else {
        std::cout << "removeExpectedChange(): activeServerCount "
                << configuration.getTotalActiveServers() << std::endl;
    }
}

// apandey this function is missing
void HPModel::removeExpiredEvents() {
    double currentTime = simTime().dbl();
    ModelChangeEvents::iterator it = events.begin();
    while (it != events.end() && it->time <= currentTime) {
        it++;
    }
    events.erase(events.begin(), it);
}

int HPModel::getActiveServerCountIn(double deltaTime, MTServerAdvance::ServerType serverType) {
    /*
     * We don't keep a past history, but if we need to know what was the active
     * server count an infinitesimal instant before now, we can use a negative delta time
     */
    ServerInfo* serverInfo = const_cast<ServerInfo*>(getServerInfoObj(serverType));

    if (deltaTime < 0) {
        return (timeActiveServerCountLast < simTime().dbl()) ?
                configuration.getActiveServers(serverType) : serverInfo->activeServerCountLast;
    }

    int servers = configuration.getActiveServers(serverType);

    // first erase the events that have already passed
    removeExpiredEvents();

    double currentTime = simTime().dbl();
    ModelChangeEvents::iterator it = events.begin();
    while (it != events.end() && it->time <= currentTime + deltaTime) {
        if (it->change == getOnlineEventCode(serverType)) {
            servers++;
        }
        it++;
    }

    return servers;
}

const HPModel::ServerInfo* HPModel::getServerInfoObj(MTServerAdvance::ServerType serverType) const {
    const ServerInfo* serverInfo = NULL;

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        serverInfo = &serverA;
        break;
    case MTServerAdvance::ServerType::B:
        serverInfo = &serverB;
        break;
    case MTServerAdvance::ServerType::C:
        serverInfo = &serverC;
        break;
    case MTServerAdvance::ServerType::NONE:
        assert(false);
    }

    return serverInfo;
}

// apandey boot delay not set
void HPModel::addServer(double bootDelay, MTServerAdvance::ServerType serverType) {
    std::cout << "HPModel::addServer " << serverType << std::endl;

    // only one add server tactic at a time
    //ASSERT(configuration.getBootRemain() == 0);
    ASSERT(!isServerBooting());

    //if (bootDelay > 0) {
    addExpectedChange(simTime().dbl() + bootDelay, getOnlineEventCode(serverType));
    std::cout << "HPModel::addServer configuration.setBootRemain(ceil(bootDelay / evaluationPeriod), serverType);" << std::endl;
    configuration.setBootRemain(ceil(bootDelay / evaluationPeriod), serverType);
    //}
    lastConfigurationUpdate = simTime();
    //removeExpiredEvents();

#if LOCDEBUG
    std::cout << simTime().dbl() << ": " << "addServer(" << bootDelay << "): " << "expected = " << events.size() << std::endl;
#endif
}

// apandey erease online event from events
void HPModel::serverBecameActive(MTServerAdvance::ServerType serverType) {
    std::cout << "HPModel::serverBecameActive " << serverType << std::endl;

    ServerInfo* serverInfo = const_cast<ServerInfo* >(getServerInfoObj(serverType));
    serverInfo->activeServerCountLast = configuration.getActiveServers(serverType);
    timeActiveServerCountLast = simTime().dbl();

    /* remove expected change...assume it is the first SERVER_ONLINE */
    HPModel::ModelChange serverBootupEventCode = getOnlineEventCode(serverType);
    ModelChangeEvents::iterator it = events.begin();

    while (it != events.end() && it->change != serverBootupEventCode) {
        it++;
    }

    assert(it != events.end()); // there must be an expected change for this
    events.erase(it);

    configuration.setActiveServers(serverInfo->activeServerCountLast + 1, serverType);
    std::cout << "HPModel::serverBecameActive configuration.setBootRemain(0);" << std::endl;
    configuration.setBootRemain(0);
    lastConfigurationUpdate = simTime();

    //removeExpiredEvents();
    std::cout << "serverBecameActive " << serverType << std::endl;
#if LOCDEBUG
    std::cout << simTime().dbl() << ": " << "serverBecameActive(): serverCount=" << serverCount << " active=" << activeServerCount << " expected=" << events.size() << std::endl;
#endif

}

// apandey
void HPModel::removeServer(MTServerAdvance::ServerType serverType) {
    std::cout << "HPModel::removeServer " << serverType << std::endl;

    // Assume only one server is booting at a time.
    if (isServerBooting()) {
        /* the server we're removing is not active yet */
        removeExpectedChange();
        std::cout << "configuration.setBootRemain(0, serverType)" << std::endl;
        configuration.setBootRemain(0);
    } else {
        ServerInfo* serverInfo = const_cast<ServerInfo*>(getServerInfoObj(serverType));
        serverInfo->activeServerCountLast = configuration.getActiveServers(serverType);
        timeActiveServerCountLast = simTime().dbl();

        configuration.setActiveServers(serverInfo->activeServerCountLast - 1, serverType);
    }

    lastConfigurationUpdate = simTime();

    std::cout << "Server removed" << serverType << std::endl;

#if LOCDEBUG
    std::cout << simTime().dbl() << ": " << "removeServer(): serverCount=" << serverCount << " active=" << activeServerCount << " expected=" << events.size() << std::endl;
#endif
}

void HPModel::setBrownoutFactor(double factor) {
    std::cout << "setBrownoutFactor " << factor << std::endl;

    configuration.setBrownOutFactor(factor);
}

double HPModel::getBrownoutFactor() const {
    //std::cout << "getBrownoutFactor " << factor << std::endl;

    return configuration.getBrownOutFactor();
}

/*ScenarioTree* HPModel::getEnvironmentProbabilityTree() const {
    ScenarioTree* pTree = 0;

    if (perfectPrediction) {
        PredictableSource *source = check_and_cast<PredictableSource *> (getParentModule()->getSubmodule("source"));

        pTree = new ScenarioTree;
        pTree->getRoot().value = environment.getArrivalMean();

        ScenarioTree::Node* pNode = &pTree->getRoot();
        for(int t = 0; t < horizon; t++) {
            double mean = source->getPrediction(evaluationPeriod * t, evaluationPeriod, 0);
            pNode->edges.push_back(
                    ScenarioTree::Edge(1,
                            ScenarioTree::Node(mean)));
            pNode = &pNode->edges.back().child;
        }
    } else {
        //unsigned branchingDepth = par("probabilityTree").boolValue() ? 2 : 0;
        //unsigned branchingDepth = simulation.getSystemModule()->par("usePredictor").doubleValue();
        unsigned branchingDepth = horizon -1; // Because t=horizon is only used to calculate utility.
        PredictableRandomSource *source = dynamic_cast<PredictableRandomSource *> (getParentModule()->getSubmodule("source"));
        if (source) {

            // if we can get bounds for the arrival rate, we use them
            pTree = pArrivalMeanPredictor->createScenarioTree(1 / source->getMaxRate(),
                    1 / source->getMinRate(), branchingDepth,
                    max(branchingDepth, (unsigned) horizon));
        } else {
            pTree = pArrivalMeanPredictor->createScenarioTree(0, DBL_MAX,
                    branchingDepth, max(branchingDepth, (unsigned) horizon));
        }
    }

#if REPEAT_PROB_TREE_ROOT
    ScenarioTree* pOldTree = pTree;
    pTree = pOldTree->cloneWithNewRoot();
    pTree->getRoot().value = pOldTree->getRoot().value;
    delete pOldTree;
#endif

    pTree->updateDepths();
    return pTree;
}*/

void HPModel::initialize(int stage) {
    if (stage == 0) {
        // get parameters
        serverA.maxServers = omnetpp::getSimulation()->getSystemModule()->par("maxServersA");
        serverB.maxServers = omnetpp::getSimulation()->getSystemModule()->par("maxServersB");
        serverC.maxServers = omnetpp::getSimulation()->getSystemModule()->par("maxServersC");

        int maxServers = serverA.maxServers + serverB.maxServers + serverC.maxServers;

        evaluationPeriod =
                omnetpp::getSimulation()->getSystemModule()->par("evaluationPeriod").doubleValue();
        bootDelay = omnetpp::getSimulation()->getSystemModule()->par("bootDelay");
        horizon = max(5.0,
                ceil(bootDelay / evaluationPeriod) * (maxServers - 1) + 1);
        //horizon = 5;
        numberOfBrownoutLevels = omnetpp::getSimulation()->getSystemModule()->par(
                "numberOfBrownoutLevels");

        /*std::vector<std::string> modelArgs;

        modelArgs.push_back("LES");
        modelArgs.push_back("0.8");
        modelArgs.push_back("0.1");

        if (omnetpp::getSimulation()->getSystemModule()->par("usePredictor").boolValue()) {
            // Initialize predictor data
            double maxMatchWidthFactor = omnetpp::getSimulation()->getSystemModule()->par("matchWidthFactor").doubleValue();
            EnvPredictionRecord::get_instance(horizon, maxMatchWidthFactor);
        }
        //double trainingLength = simulation.getSystemModule()->par(
        //                "trainingLength").doubleValue();

        perfectPrediction = omnetpp::getSimulation()->getSystemModule()->par("perfectPrediction");

        // initialize predictor
        // set the number of training samples equal to the number of warmup periods (no less than 10, though)
        //vector<string> params = cStringTokenizer(
        //        par("predictorConfig").stringValue()).asVector();
        int trainingLength = max(10,
                int(
                        ceil(
                                omnetpp::getSimulation()->getWarmupPeriod().dbl()
                                        / evaluationPeriod)));
        cout << "Training samples=" << trainingLength << endl;
        //pArrivalMeanPredictor = TimeSeriesPredictor::getInstance(
        //        par("predictorConfig").stringValue(), trainingLength, horizon);

        pArrivalMeanPredictor = TimeSeriesPredictor::getInstance(modelArgs, trainingLength, horizon);*/
    } else {
        // start servers
        HPExecutionManagerModBase* pExecMgr = check_and_cast<
                HPExecutionManagerModBase*>(
                getParentModule()->getSubmodule("executionManager"));

        int initialServers = omnetpp::getSimulation()->getSystemModule()->par(
                "initialServersA");

        while (initialServers > 0) {
            pExecMgr->addServerLatencyOptional(MTServerAdvance::ServerType::A, true);
            initialServers--;
        }

        initialServers = omnetpp::getSimulation()->getSystemModule()->par(
                                "initialServersB");

        while (initialServers > 0) {
            pExecMgr->addServerLatencyOptional(MTServerAdvance::ServerType::B, true);
            initialServers--;
        }

        initialServers = omnetpp::getSimulation()->getSystemModule()->par(
                                "initialServersC");

        while (initialServers > 0) {
            pExecMgr->addServerLatencyOptional(MTServerAdvance::ServerType::C, true);
            initialServers--;
        }

        // TODO also set initial diversion
    }
}

bool HPModel::isServerBooting(MTServerAdvance::ServerType serverType) const {
    bool isBooting = false;

    if (!events.empty()) {
        // find if a server is booting. TODO Assume only one can be booting
        ModelChangeEvents::const_iterator eventIt = events.begin();
        if (eventIt != events.end()) {
            ModelChange changeEventCode = getOnlineEventCode(serverType);
            ASSERT(eventIt->change == changeEventCode);
            isBooting = true;
            eventIt++;
            ASSERT(eventIt == events.end()); // only one tactic should be active
        }
    }

    return isBooting;
}

// apandey
bool HPModel::isServerBooting() const {
    bool isBooting = false;

    if (!events.empty()) {

        /* find if any server is booting. Assume only one can be booting */
        ModelChangeEvents::const_iterator eventIt = events.begin();
        if (eventIt != events.end()) {
            ModelChange changeEventCodeServerA = getOnlineEventCode(MTServerAdvance::ServerType::A);
            ModelChange changeEventCodeServerB = getOnlineEventCode(MTServerAdvance::ServerType::B);
            ModelChange changeEventCodeServerC = getOnlineEventCode(MTServerAdvance::ServerType::C);

            ASSERT(eventIt->change == changeEventCodeServerA
                    || eventIt->change == changeEventCodeServerB
                    || eventIt->change == changeEventCodeServerC);

            isBooting = true;
            eventIt++;
            ASSERT(eventIt == events.end()); // only one tactic should be active
        }
    }

    return isBooting;
}

// apandey
const HPConfiguration& HPModel::getConfiguration() {
    // configuration is kept mostly up to date, but the bootRemain member may need to be updated

    if (events.empty()) {
        configuration.setBootRemain(0);
    } else {//(lastConfigurationUpdate < simTime() && !events.empty()) {
        ModelChangeEvents::const_iterator eventIt = events.begin();
        if (eventIt != events.end()) {
            int bootRemain = ceil(
                    (eventIt->time - simTime().dbl()) / evaluationPeriod);

            /*
             * we never set boot remain to 0 here because the server could
             * still be booting (if we allowed random boot times)
             * so, we keep it > 0, and only serverBecameActive() can set it to 0
             */
            //std::cout << "HPModel::getConfiguration configuration.setBootRemain(std::max(1, bootRemain)) " << std::endl;
            configuration.setBootRemain(std::max(1, bootRemain));
            lastConfigurationUpdate = simTime();
            eventIt++;
            ASSERT(eventIt == events.end()); // only one tactic should be active
        }
    }

    return configuration;
}

const Environment& HPModel::getEnvironment() const {
    return environment;
}

double HPModel::getAvgResponseTime() const {
    return observations.avgResponseTime;
}

void dump_arrival_rate(const double & arrival_rate) {
    string file_name = DebugFileInfo::getInstance()->GetRequestArrivalFilePath();
    //        "/home/ashutosp/Dropbox/regression/HPModel_arrival_rate";
    std::ofstream myfile;
    std::cout << "arrivaRate = " << 1/arrival_rate
            << " " << arrival_rate << std::endl;
    myfile.open(file_name, ios::app);
    myfile << 1/arrival_rate << "\n";
    myfile.close();
}

void HPModel::getPastInterarrivalRates(std::vector<double>& interarrival_means, unsigned long history_length) {
    unsigned long index = 0;
    unsigned long start_index = envRecord.size() - history_length;
    assert(start_index >= 0 && start_index <= envRecord.size());

    EnvRecord::const_iterator itr = envRecord.begin();

    while (itr != envRecord.end()) {
        if (index >= start_index) {
            //std::cout << "HPModel::getPastInterarrivalRates Pushing Predicted Value = "
            //        << 1/((*itr).getArrivalMean()) << std::endl;
            interarrival_means.push_back(1/(*itr).getArrivalMean());
        }

        ++itr;
        ++index;
    }

    return;
}


void HPModel::setEnvironment(const Environment& environment) {
    this->environment = environment;
    envRecord.push_back(environment);

    // update predictors
    //arrivalMeanPredictor.observe(environment.getArrivalMean());
    dump_arrival_rate(environment.getArrivalMean());
//#if USE_VARIANCE_PREDICTOR
//    arrivalVariancePredictor.observe(environment.getArrivalVariance());
//#endif

    // update predictors
    //pArrivalMeanPredictor->observe(environment.getArrivalMean());

}

double HPModel::getArrivalRate() const {
    return environment.getArrivalMean();
}


int HPModel::getMaxServers(MTServerAdvance::ServerType serverType) const {
    return getServerInfoObj(serverType)->maxServers;
}

double HPModel::getEvaluationPeriod() const {
    return evaluationPeriod;
}

const Observations& HPModel::getObservations() const {
    return observations;
}

void dump_response_time(const Observations& observations) {
    static bool dump = true;
    std::cout << "HPModel::dump_response_time =" << observations.avgResponseTime << "\n";

    if (dump) {
        string file_name = DebugFileInfo::getInstance()->GetResponseTimeFilePath();
        //        "/home/ashutosp/Dropbox/regression/HPModel_response_time";
        //string file_name = "/home/ashutosp/response_time";
        std::ofstream myfile;

        myfile.open(file_name, ios::app);
        myfile << observations.avgResponseTime << "\n";
        myfile.close();

        file_name = DebugFileInfo::getInstance()->GetDebugFilePath();
        //        "/home/ashutosp/Dropbox/regression/HP_triggers_arrival_rate";
        //string file_name = "/home/ashutosp/response_time";
        //std::ofstream myfile;

        myfile.open(file_name, ios::app);
        myfile << observations.avgResponseTime << "====";
        myfile.close();
        //dump = false;
    } else {
        //dump = true;
    }
}

void HPModel::setObservations(const Observations& observations) {
    this->observations = observations;
    dump_response_time(observations);
}

/*
EnvironmentVector HPModel::getEnvironmentPrediction(int steps) {
    EnvironmentVector predictionsVectorPerfect(steps);
    EnvironmentVector predictionsVector(steps);

    if (perfectPrediction) {
        PredictableSource *source = check_and_cast<PredictableSource *> (getParentModule()->getSubmodule("source"));

        for(int t = 0;t < steps; t++){
            double variance = 0;
            double mean = source->getPrediction(evaluationPeriod * t, evaluationPeriod, &variance);
            predictionsVectorPerfect[t].setArrivalMean(mean);
            predictionsVectorPerfect[t].setArrivalVariance(variance);
//            predictionsVector[t].setArrivalMean(mean);
//            predictionsVector[t].setArrivalVariance(variance);
        }
    } else {
#if HALF_PERIOD
        int factor = 2;
#else
        int factor = 1;
#endif
        double predictions[steps * factor];

        // predictions
        PredictableRandomSource *source = dynamic_cast<PredictableRandomSource *> (getParentModule()->getSubmodule("source"));
        if (source) {
            pArrivalMeanPredictor->predict(1 / source->getMaxRate(), 1 / source->getMinRate(),  steps * factor, predictions);
        } else {
            pArrivalMeanPredictor->predict(0, DBL_MAX, steps * factor, predictions);
        }

        for (int i = 0; i < steps; i++) {

#if HALF_PERIOD
            predictionsVector[i].setArrivalMean((predictions[i * 2] + predictions[i * 2 + 1]) / 2.0);
            predictionsVector[i].setArrivalVariance(pow(predictionsVector[i].getArrivalMean(), 2));
#else
            predictionsVector[i].setArrivalMean(predictions[i]);
            predictionsVector[i].setArrivalVariance(pow(predictions[i], 2));
#endif
        }
    }

    emit(registerSignal("prediction"), predictionsVector[0].getArrivalMean());
    if (perfectPrediction) {
        emit(registerSignal("perfect"), predictionsVectorPerfect[0].getArrivalMean());
        return predictionsVectorPerfect;
    } else {
        return predictionsVector;
    }
}*/

/*
EnvironmentVector HPModel::getEnvironmentPrediction(int steps) const{
    EnvironmentVector predictionsVectorPerfect(steps);
    EnvironmentVector predictionsVector(steps);

#if PERFECT_PREDICTION
    {
        PredictableSource *source = check_and_cast<PredictableSource *> (getParentModule()->getSubmodule("source"));

        for(int t = 0;t < steps; t++) {
            double variance = 0;
            double mean = source->getPrediction(evaluationPeriod * t, evaluationPeriod, &variance);
            predictionsVectorPerfect[t].setArrivalMean(mean);
            predictionsVectorPerfect[t].setArrivalVariance(variance);
//            predictionsVector[t].setArrivalMean(mean);
//            predictionsVector[t].setArrivalVariance(variance);
        }
    }
#else
#if HALF_PERIOD
    int factor = 2;
#else
    int factor = 1;
#endif
    double predictions[steps * factor];
#if USE_VARIANCE_PREDICTOR
    double variances[steps * factor];
#endif

    // predictions
    PredictableRandomSource *source =
            dynamic_cast<PredictableRandomSource *>(getParentModule()->getSubmodule(
                    "source"));
    if (source) {
        arrivalMeanPredictor.predict(1 / source->getMaxRate(),
                1 / source->getMinRate(), steps * factor, predictions);
    } else {
        arrivalMeanPredictor.predict(0, DBL_MAX, steps * factor, predictions);
    }
#if USE_VARIANCE_PREDICTOR
    arrivalVariancePredictor.predict(0, DBL_MAX, steps * factor, variances);
#endif

    for (int i = 0; i < steps; i++) {

#if HALF_PERIOD
        predictionsVector[i].setArrivalMean((predictions[i * 2] + predictions[i * 2 + 1]) / 2.0);

#if USE_VARIANCE_PREDICTOR
        // the predictor can produce negative variances, so we truncate it
        predictionsVector[i].setArrivalVariance(max(0.0, (variances[i * 2] + variances[i * 2 + 1]) / 2.0));
#else
        predictionsVector[i].setArrivalVariance(pow(predictionsVector[i].getArrivalMean(), 2));
#endif
#else
        predictionsVector[i].setArrivalMean(predictions[i]);

#if USE_VARIANCE_PREDICTOR
        // the predictor can produce negative variances, so we truncate it
        predictionsVector[i].setArrivalVariance(max(0.0, variances[i]));
#else
        predictionsVector[i].setArrivalVariance(pow(predictions[i], 2));
#endif

#endif
    }
#endif

#if PERFECT_PREDICTION
    emit(registerSignal("perfect"), predictionsVectorPerfect[0].getArrivalMean());
#endif
    emit(registerSignal("prediction"), predictionsVector[0].getArrivalMean());

#if PERFECT_PREDICTION
    return predictionsVectorPerfect;
#else
    return predictionsVector;
#endif
}*/

double HPModel::getBootDelay() const {
    return bootDelay;
}

int HPModel::getHorizon() const {
    return horizon;
}

int HPModel::getNumberOfBrownoutLevels() const {
    return numberOfBrownoutLevels;
}

double HPModel::getLowFidelityServiceTime(MTServerAdvance::ServerType serverType) const {
    ServerInfo serverInfo;

    return getServerInfoObj(serverType)->lowFidelityServiceTime;
}

void HPModel::setLowFidelityServiceTime(double lowFidelityServiceTimeMean,
        double lowFidelityServiceTimeVariance, MTServerAdvance::ServerType serverType) {
    ServerInfo* serverInfo = const_cast<ServerInfo*>(getServerInfoObj(serverType));

    serverInfo->lowFidelityServiceTime = lowFidelityServiceTimeMean;
    serverInfo->lowFidelityServiceTimeVariance = lowFidelityServiceTimeVariance;
}

int HPModel::getServerThreads() const {
    return serverThreads;
}

void HPModel::setServerThreads(int serverThreads) {
    this->serverThreads = serverThreads;
}

double HPModel::getServiceTime(MTServerAdvance::ServerType serverType) const {
    return getServerInfoObj(serverType)->serviceTime;
}

void HPModel::setServiceTime(double serviceTimeMean, double serviceTimeVariance, MTServerAdvance::ServerType serverType) {
    ServerInfo* serverInfo = const_cast<ServerInfo*>(getServerInfoObj(serverType));
    serverInfo->serviceTime = serviceTimeMean;
    serverInfo->serviceTimeVariance = serviceTimeVariance;
}

double HPModel::getLowFidelityServiceTimeVariance(MTServerAdvance::ServerType serverType) const {
    return getServerInfoObj(serverType)->lowFidelityServiceTimeVariance;
}

double HPModel::getServiceTimeVariance(MTServerAdvance::ServerType serverType) const {
    return getServerInfoObj(serverType)->serviceTimeVariance;
}

void HPModel::setCurrentTime(int period) {
    currentPeriod = period;
}

void HPModel::resetCurrentTime() {
    currentPeriod = 0;
}

int HPModel::getCurrentTime() const {
    return currentPeriod;
}

int HPModel::getDimmerLevel() const {
    return 1 + (numberOfBrownoutLevels - 1) * configuration.getBrownOutFactor();
}

void HPModel::setTrafficLoad(HPLoadBalancer::TrafficLoad serverA,
              HPLoadBalancer::TrafficLoad serverB, HPLoadBalancer::TrafficLoad serverC) {
    this->configuration.setTraffic(MTServerAdvance::ServerType::A, serverA);
    this->configuration.setTraffic(MTServerAdvance::ServerType::B, serverB);
    this->configuration.setTraffic(MTServerAdvance::ServerType::C, serverC);
}

HPModel::ModelChange HPModel::getOnlineEventCode(MTServerAdvance::ServerType serverType) const {
    ModelChange event = INVALID;

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        event = SERVERA_ONLINE;
        break;
    case MTServerAdvance::ServerType::B:
        event = SERVERB_ONLINE;
        break;
    case MTServerAdvance::ServerType::C:
        event = SERVERC_ONLINE;
        break;
    case MTServerAdvance::ServerType::NONE:
        assert(false);
    }

    return event;
}


void HPModel::setJobServerInfo(string jobName, MTServerAdvance::ServerType serverType) {
    JobServeInfo::iterator itr = jobServeInfo.find(jobName);

    assert(itr == jobServeInfo.end());
    jobServeInfo[jobName] = serverType;
    //cout << "JobName = " << jobName << endl;
}

MTServerAdvance::ServerType HPModel::getJobServerInfo(string jobName) {
    JobServeInfo::iterator itr = jobServeInfo.find(jobName);
    MTServerAdvance::ServerType serverType = MTServerAdvance::ServerType::NONE;

    if (itr != jobServeInfo.end()) {
        serverType = itr->second;
    }

    return serverType;
}
