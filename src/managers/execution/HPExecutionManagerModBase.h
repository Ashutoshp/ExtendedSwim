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
 * HPExecutionModBase.h
 *
 *  Created on: Aug 18, 2015
 *      Author: ashutosp
 */

#ifndef HPEXECUTIONMODBASE_H_
#define HPEXECUTIONMODBASE_H_

//#include <csimplemodule.h>
#include <set>
#include "BootComplete_m.h"
#include <model/HPModel.h>
#include "ExecutionManager.h"
#include "AllTactics.h"
#include "assert.h"

class HPExecutionManagerModBase : public cSimpleModule, public ExecutionManager {
    int serverRemoveInProgress;
    omnetpp::simsignal_t serverRemovedSignal;
    omnetpp::simsignal_t serverAddedSignal;
    omnetpp::simsignal_t serverActivatedSignal;
    omnetpp::simsignal_t brownoutSetSignal;

  protected:
    typedef std::set<BootComplete*> BootCompletes;
    BootCompletes pendingMessages;

    HPModel* pModel;
    cMessage* testMsg;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    double getMeanAndVarianceFromParameter(const cPar& par, double& variance) const;

    /**
     * When a remove server completes, the implementation must call this function
     *
     * This is used to, for example, wait for a server to complete ongoing requests
     * doRemoveServer() can disconnect the server from the load balancer
     * and when all requests in the server have been processed, call this method,
     * so that the server is removed from the model
     * TODO there should be a state in the model to mark servers being shutdown
     */
    void notifyRemoveServerCompleted(MTServerAdvance::ServerType serverType);


    // target-specific methods (e.g., actual servers, sim servers, etc.)

    /**
     * @return BootComplete* to be handled later by doAddServerBootComplete()
     */
    virtual BootComplete* doAddServer(MTServerAdvance::ServerType serverType, bool instantaneous = false) = 0;
    virtual void doAddServerBootComplete(BootComplete* bootComplete) = 0;

    /**
     * @return BootComplete* identical in content (not the pointer itself) to
     *   what doAddServer() would have returned for this server
     */
    virtual BootComplete* doRemoveServer(MTServerAdvance::ServerType serverType) = 0;
    virtual void doSetBrownout(double factor) = 0;
    //virtual void doDivertTraffic(HPLoadBalancer::TrafficLoad serverA, HPLoadBalancer::TrafficLoad serverB, HPLoadBalancer::TrafficLoad serverC) = 0;
    virtual MTServerAdvance::ServerType getServerTypeFromName(const char* name) const;

  public:
    static const char* SIG_SERVER_REMOVED;
    static const char* SIG_SERVER_ADDED;
    static const char* SIG_SERVER_ACTIVATED;
    static const char* SIG_BROWNOUT_SET;

    HPExecutionManagerModBase();
    virtual ~HPExecutionManagerModBase();
    virtual void addServerLatencyOptional(MTServerAdvance::ServerType serverType, bool instantaneous = false);

    // ExecutionManager interface
    virtual void addServer() {assert(false);} // Hack
    virtual void removeServer() {assert(false);} // Hack

    virtual void addServer(MTServerAdvance::ServerType serverType);
    virtual void removeServer(MTServerAdvance::ServerType serverType);
    virtual void setBrownout(double factor);
    virtual void divertTraffic(HPLoadBalancer::TrafficLoad serverA, HPLoadBalancer::TrafficLoad serverB, HPLoadBalancer::TrafficLoad serverC);
};

#endif /* HPEXECUTIONMODBASE_H_ */
