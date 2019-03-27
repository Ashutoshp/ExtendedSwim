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
 * HPExecutionModBase.cpp
 *
 *  Created on: Aug 18, 2015
 *      Author: ashutosp
 */

#include <HPExecutionManagerModBase.h>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <cstdlib>

using namespace std;


const char* HPExecutionManagerModBase::SIG_SERVER_REMOVED = "serverRemoved";
const char* HPExecutionManagerModBase::SIG_SERVER_ADDED = "serverAdded";
const char* HPExecutionManagerModBase::SIG_SERVER_ACTIVATED = "serverActivated";
const char* HPExecutionManagerModBase::SIG_BROWNOUT_SET = "brownoutSet";

HPExecutionManagerModBase::HPExecutionManagerModBase() : serverRemoveInProgress(0), testMsg(0) {
    // TODO Auto-generated constructor stub

}

HPExecutionManagerModBase::~HPExecutionManagerModBase() {
    for (BootCompletes::iterator it = pendingMessages.begin(); it != pendingMessages.end(); it++) {
        cancelAndDelete(*it);
    }
    cancelAndDelete(testMsg);
}

void HPExecutionManagerModBase::initialize() {
    pModel = check_and_cast<HPModel*> (getParentModule()->getSubmodule("model"));
    serverRemovedSignal = registerSignal(SIG_SERVER_REMOVED);
    serverAddedSignal = registerSignal(SIG_SERVER_ADDED);
    serverActivatedSignal = registerSignal(SIG_SERVER_ACTIVATED);
    brownoutSetSignal = registerSignal(SIG_BROWNOUT_SET);
}

void HPExecutionManagerModBase::handleMessage(cMessage* msg) {
    if (msg == testMsg) {
        if (msg->getKind() == 0) {
            addServer();
            msg->setKind(1);
            scheduleAt(simTime() + 31, msg);
        } else {
            removeServer();
        }
        return;
    }

    BootComplete* bootComplete = check_and_cast<BootComplete *>(msg);
    cModule *server = omnetpp::getSimulation()->getModule(bootComplete->getModuleId());
    MTServerAdvance::ServerType serverType = getServerTypeFromName(server->getFullName());
    doAddServerBootComplete(bootComplete);

    //  notify add complete to model
    pModel->serverBecameActive(serverType);
    //emit(serverAddedSignal, true);
    emit(serverActivatedSignal, serverType);

    // remove from pending
    BootCompletes::iterator it = pendingMessages.find(bootComplete);
    ASSERT(it != pendingMessages.end());
    delete *it;
    pendingMessages.erase(it);
}

double HPExecutionManagerModBase::getMeanAndVarianceFromParameter(const cPar& par, double& variance) const {
    if (par.isExpression()) {
        // assume it's a distribution
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        //std::cout << par.getExpression()->str() << std::endl;
        std::string serviceTimeDistribution = par.getExpression()->str();
        tokenizer tokens(serviceTimeDistribution, boost::char_separator<char>("(,)"));
        tokenizer::iterator it = tokens.begin();

        /*while (it != tokens.end()) {
            std::cout << *it << std::endl;
            ++it;
        }
        it = tokens.begin();*/
        ASSERT(it != tokens.end());

        //std::cout << "HPExecutionManagerModBase::getMeanAndVarianceFromParameter = " << *it << std::endl;

        if (*it == "exponential") {
            ASSERT(++it != tokens.end());
            double mean = atof((*it).c_str());
            variance = pow(mean, 2);
            return mean;
        } else if (*it == "normal" || *it == "truncnormal") {
            ASSERT(++it != tokens.end());
            double mean = atof((*it).c_str());
            ASSERT(++it != tokens.end());
            variance = pow(atof((*it).c_str()), 2);
            return mean;
        }
        ASSERT(false); // distribution not supported;
    } else {
        // is a constant
        variance = 0;
        return par.doubleValue();
    }
    return 0;
}

void HPExecutionManagerModBase::addServer(MTServerAdvance::ServerType serverType) {
    addServerLatencyOptional(serverType);
}

void HPExecutionManagerModBase::addServerLatencyOptional(MTServerAdvance::ServerType serverType, bool instantaneous) {
    // This tells simulation kernel that this method can be invoked directly by the methods of other modules.
    Enter_Method("addServer()");
    int serverCount = pModel->getConfiguration().getServers(serverType);
    ASSERT(serverCount < pModel->getMaxServers(serverType));

    BootComplete* bootComplete = doAddServer(serverType, instantaneous);

    double bootDelay = 0;
    if (!instantaneous) {
        bootDelay = pModel->getBootDelay();
    }

    pModel->addServer(bootDelay, serverType);
    //emit(serverAddedSignal, true);
    emit(serverAddedSignal, serverType);

    pendingMessages.insert(bootComplete);
    if (bootDelay == 0) {
        handleMessage(bootComplete);
    } else {
        scheduleAt(simTime() + bootDelay, bootComplete);
    }
}

void HPExecutionManagerModBase::removeServer(MTServerAdvance::ServerType serverType) {
    Enter_Method("removeServer()");
    //int serverCount = pModel->getConfiguration().getServers(serverType);
    ASSERT(pModel->getConfiguration().getTotalActiveServers() > 1);

    ASSERT(serverRemoveInProgress == 0); // removing a server while another is being removed not supported yet

    serverRemoveInProgress++;
    BootComplete* pBootComplete = doRemoveServer(serverType);

    // cancel boot complete event if needed
    for (BootCompletes::iterator it = pendingMessages.begin(); it != pendingMessages.end(); it++) {
        if ((*it)->getModuleId() == pBootComplete->getModuleId()) {
            cancelAndDelete(*it);
            pendingMessages.erase(it);
            break;
        }
    }
    delete pBootComplete;
}

void HPExecutionManagerModBase::setBrownout(double factor) {
    Enter_Method("setBrownout()");
    pModel->setBrownoutFactor(factor);
    doSetBrownout(factor);
    emit(brownoutSetSignal, true);
}

void HPExecutionManagerModBase::notifyRemoveServerCompleted(MTServerAdvance::ServerType serverType) {
   pModel->removeServer(serverType);
   serverRemoveInProgress--;

   // emit signal to notify others (notably iProbe)
   emit(serverRemovedSignal, serverType);
}

void HPExecutionManagerModBase::divertTraffic(HPLoadBalancer::TrafficLoad serverA,
        HPLoadBalancer::TrafficLoad serverB, HPLoadBalancer::TrafficLoad serverC) {
    Enter_Method("divertTraffic()");
    pModel->setTrafficLoad(serverA, serverB, serverC);
}

MTServerAdvance::ServerType HPExecutionManagerModBase::getServerTypeFromName(const char* name) const {
    // Place some sanity checks
    MTServerAdvance::ServerType serverType = MTServerAdvance::NONE;

    if (strncmp(name, "server_A_", 9) == 0) {
        serverType = MTServerAdvance::A;
    } else if (strncmp(name, "server_B_", 9) == 0) {
        serverType = MTServerAdvance::B;
    } else if (strncmp(name, "server_C_", 9) == 0) {
        serverType = MTServerAdvance::C;
    } else {
        assert(false);
    }

    return serverType;
}
