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
/*
 * HPExecutionManagerMod.cpp
 *
 *  Created on: Aug 18, 2015
 *      Author: ashutosp
 */

#include <HPExecutionManagerMod.h>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include "PassiveQueue.h"
#include "MTServer.h"

using namespace std;

Define_Module(HPExecutionManagerMod);

const char* HP_SERVER_MODULE_NAME = "server";
const char* HP_LOAD_BALANCER_MODULE_NAME = "loadBalancer";
const char* HP_INTERNAL_SERVER_MODULE_NAME = "server";
const char* HP_INTERNAL_QUEUE_MODULE_NAME = "queue";
const char* HP_SINK_MODULE_NAME = "classifier";

HPExecutionManagerMod::HPExecutionManagerMod()
: serverBeingRemovedModuleId(-1), completeRemoveMsg(0) {
    serverBusySignalId = registerSignal("busy");
}

HPExecutionManagerMod::~HPExecutionManagerMod() {
    // TODO Auto-generated destructor stub
}

void HPExecutionManagerMod::initialize() {
    HPExecutionManagerModBase::initialize();
    omnetpp::getSimulation()->getSystemModule()->subscribe(serverBusySignalId, this);
}

void HPExecutionManagerMod::handleMessage(cMessage *msg) {
    if (msg == completeRemoveMsg) {
        cModule* module = omnetpp::getSimulation()->getModule(serverBeingRemovedModuleId);
        MTServerAdvance::ServerType serverType = getServerTypeFromName(module->getFullName());
        module->gate("out")->disconnect();
        module->deleteModule();

        // TODO send server name instead of type to support multiple servers of same type.
        notifyRemoveServerCompleted(serverType);

        serverBeingRemovedModuleId = -1;

        cancelAndDelete(msg);
        completeRemoveMsg = 0;
    } else {
        HPExecutionManagerModBase::handleMessage(msg);
    }
}

string HPExecutionManagerMod::getServerString(MTServerAdvance::ServerType serverType, bool internal) const {
    string name_str = HP_SERVER_MODULE_NAME;

    if (internal) {
        name_str = HP_INTERNAL_SERVER_MODULE_NAME;
    }

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        name_str += string("_A_");
        break;
    case MTServerAdvance::ServerType::B:
        name_str += string("_B_");
        break;
    case MTServerAdvance::ServerType::C:
        name_str += string("_C_");
        break;
    case MTServerAdvance::ServerType::NONE:
        name_str = "";
    }

    return name_str;
}



string HPExecutionManagerMod::getModuleStr(MTServerAdvance::ServerType serverType) const {
    string module_str = "";

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        module_str = "plasa.modules.HPAppServerA";
        break;
    case MTServerAdvance::ServerType::B:
        module_str = "plasa.modules.HPAppServerB";
        break;
    case MTServerAdvance::ServerType::C:
        module_str = "plasa.modules.HPAppServerC";
        break;
    case MTServerAdvance::ServerType::NONE:
        module_str = "";
    }

    return module_str;
}

// This function connects the server input and output
void HPExecutionManagerMod::doAddServerBootComplete(BootComplete* bootComplete) {
    cModule *server = omnetpp::getSimulation()->getModule(bootComplete->getModuleId());
    cModule* loadBalancer = getParentModule()->getSubmodule(
            HP_LOAD_BALANCER_MODULE_NAME);
    cModule* sink = getParentModule()->getSubmodule(HP_SINK_MODULE_NAME);
    // connect gates
    loadBalancer->getOrCreateFirstUnconnectedGate("out", 0, false, true)->connectTo(
            server->gate("in"));
    server->gate("out")->connectTo(
            sink->getOrCreateFirstUnconnectedGate("in", 0, false, true));

    //std::cout << "HPExecutionManagerMod::doAddServerBootComplete " << std::endl;
}

// This functions creates a server module with an initial configuration.
// Return boot complete message to be used when bootup is complete.
BootComplete* HPExecutionManagerMod::doAddServer(MTServerAdvance::ServerType serverType, bool instantaneous) {
    // find factory object
    cModuleType *moduleType = cModuleType::get(getModuleStr(serverType).c_str());
    int serverCount = pModel->getConfiguration().getServers(serverType);

    stringstream name;
    name << getServerString(serverType);
    name << serverCount + 1;
    cModule *module = moduleType->create(name.str().c_str(), getParentModule());

    // setup parameters
    module->finalizeParameters();
    module->buildInside();

    // copy all params of the server inside the appserver module from the template
    cModule* pNewSubmodule = module->getSubmodule(HP_INTERNAL_SERVER_MODULE_NAME);
    pModel->setServerThreads(pNewSubmodule->par("threads"));
    double variance = 0.0;
    double mean = getMeanAndVarianceFromParameter(
            pNewSubmodule->par("serviceTime"), variance);
    pModel->setServiceTime(mean, variance, serverType);
    mean = getMeanAndVarianceFromParameter(
            pNewSubmodule->par("lowFidelityServiceTime"), variance);
    pModel->setLowFidelityServiceTime(mean, variance, serverType);
    //pModel->setBrownoutFactor(pNewSubmodule->par("brownoutFactor"));
    if (pModel->getConfiguration().getTotalActiveServers() >= 1) {
        pNewSubmodule->par("brownoutFactor").setDoubleValue(pModel->getConfiguration().getBrownOutFactor());
    } else {
        pModel->setBrownoutFactor(pNewSubmodule->par("brownoutFactor"));
    }

    // create activation message
    module->scheduleStart(simTime());
    module->callInitialize();

    //std::cout << "HPExecutionManagerMod::doAddServer " << serverType << std::endl;

    BootComplete* bootComplete = new BootComplete;
    bootComplete->setModuleId(module->getId());
    return bootComplete;
}

BootComplete*  HPExecutionManagerMod::doRemoveServer(MTServerAdvance::ServerType serverType) {
    int serverCount = pModel->getConfiguration().getServers(serverType);

    stringstream name;
    name << getServerString(serverType);
    name << serverCount;
    cModule *module = getParentModule()->getSubmodule(name.str().c_str());

    // disconnect module from load balancer
    cGate* pInGate = module->gate("in");
    if (pInGate->isConnected()) {
        cGate *otherEnd = pInGate->getPathStartGate();
        otherEnd->disconnect();
        //cout << "2. otherEnd->getIndex() = " << otherEnd->getIndex() << endl;
        //cout << "2. otherEnd->getVectorSize() -1 = " << otherEnd->getVectorSize() -1 << endl;

        if (strcmp(omnetpp::getSimulation()->getSystemModule()->getFullName(), "SwimExtention") == 0) {
            // NOTE: Vector size is still the same i.e. Ideally it should be reduced by 1.

        } else {
            // TODO this code is not used in case of multiple servers.
            ASSERT(pModel->getConfiguration().getTotalActiveServers() == otherEnd->getVectorSize() - 1);

            // reduce the size of the out gate in the queue module
            otherEnd->getOwnerModule()->setGateSize(otherEnd->getName(), otherEnd->getVectorSize() - 1);
            //TODO this is probably leaking memory because the gate may not be being deleted
        }
    }

    serverBeingRemovedModuleId = module->getId();
    //std::cout << "HPExecutionManagerMod::doRemoveServer " << serverType << std::endl;

    // check to see if we can delete the server immediately (or if it's busy)
    if (isServerBeingRemoveEmpty()) {
        completeServerRemoval();
    }

    BootComplete* bootComplete = new BootComplete;
    bootComplete->setModuleId(module->getId());
    return bootComplete;
}

void HPExecutionManagerMod::doSetBrownout(MTServerAdvance::ServerType serverType, double factor) {
    int serverCount = pModel->getConfiguration().getServers(serverType);

    for (int s = 1; s <= serverCount; s++) {
        stringstream name;
        name << getServerString(serverType);
        name << s;

        //std::cout << "HPExecutionManagerMod::doSetBrownout" << serverType << " " << name.str().c_str() << std::endl;

        cModule* module = getParentModule()->getSubmodule(name.str().c_str());
        module->getSubmodule("server")->par("brownoutFactor").setDoubleValue(factor);
    }
}

void HPExecutionManagerMod::doSetBrownout(double factor) {
    doSetBrownout(MTServerAdvance::ServerType::A, factor);
    doSetBrownout(MTServerAdvance::ServerType::B, factor);
    doSetBrownout(MTServerAdvance::ServerType::C, factor);
}

void HPExecutionManagerMod::completeServerRemoval() {
    Enter_Method("sendMe()");
    completeRemoveMsg = new cMessage("completeRemove");
    cout << "scheduled complete remove at " << simTime() << endl;
    scheduleAt(simTime(), completeRemoveMsg);
}

bool HPExecutionManagerMod::isServerBeingRemoveEmpty() {
    bool isEmpty = false;
    cModule* module = omnetpp::getSimulation()->getModule(serverBeingRemovedModuleId);
    MTServer* internalServer = check_and_cast<MTServerAdvance*> (module->getSubmodule(HP_INTERNAL_SERVER_MODULE_NAME));
    if (internalServer->isEmpty()) {
        queueing::PassiveQueue* queue = check_and_cast<queueing::PassiveQueue*> (module->getSubmodule(HP_INTERNAL_QUEUE_MODULE_NAME));
        if (queue->length() == 0) {
            isEmpty = true;
        }
    }
    return isEmpty;
}

void HPExecutionManagerMod::receiveSignal(cComponent *source, simsignal_t signalID, bool value, cObject *details) {
    if (signalID == serverBusySignalId && source->getParentModule()->getId() == serverBeingRemovedModuleId) {
        if (value == 0 && isServerBeingRemoveEmpty()) {
            completeServerRemoval();
        }
    }
}


