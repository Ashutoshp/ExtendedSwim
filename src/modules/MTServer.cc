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

#include <MTServer.h>
#include "Job.h"
#include "SelectionStrategies.h"
#include "IPassiveQueue.h"

Define_Module(MTServer);

using namespace queueing;

MTServer::MTServer() : endExecutionMsg(NULL), selectionStrategy(NULL), maxThreads(0) {}

void MTServer::initialize() {
    busySignal = registerSignal("busy");
    emit(busySignal, false);
    maxThreads = par("threads");
    endExecutionMsg = new cMessage("end-execution");
    selectionStrategy = SelectionStrategy::create(par("fetchingAlgorithm"), this, true);
    if (!selectionStrategy)
        error("invalid selection strategy");
    timeout = par("timeout");
}

MTServer::~MTServer() {
    cancelAndDelete(endExecutionMsg);
    delete selectionStrategy;
    for (RunningJobs::iterator it = runningJobs.begin(); it != runningJobs.end(); it++) {
        delete it->pJob;
    }
}

void MTServer::handleMessage(cMessage* msg) {
    //printf("MTServer::handleMessage msg = %s\n", msg->getName());
    if (msg == endExecutionMsg)
    {
        ASSERT(!runningJobs.empty());
        updateJobTimes(); // Why needed here. no effect, Ashutosh d=0 see function

        // send out all jobs that completed
        RunningJobs::iterator first = runningJobs.begin();
        while (first != runningJobs.end() && first->remainingServiceTime < 1e-10) {
            send(first->pJob, "out");
            runningJobs.erase(first);
            first = runningJobs.begin();
        };

        if (!runningJobs.empty()) {
            scheduleNextCompletion(); // Why executing even after end message
        } else {
            emit(busySignal, false); // Ashutosh who is waiting for this busySignal
            if (hasGUI()) getDisplayString().setTagArg("i",1,"");
        }
    }
    else
    {
//        if (!isIdle())
//            error("job arrived while already full");

        ScheduledJob job;
        job.pJob = check_and_cast<Job *>(msg);
        if (timeout > 0 && job.pJob->getTotalQueueingTime() >= timeout) {
            // don't serve this job, just send it out
            // Ashutosh What is difference? Not updating the time etc.?
            //std::cout << "########## TIME OUT ####" << job.pJob->getName() << std::endl;
            send(job.pJob, "out");
        } else {
            job.remainingServiceTime = generateJobServiceTime(job.pJob).dbl();

            // these two are nops if there was no job running
            updateJobTimes();
            cancelEvent(endExecutionMsg);

            runningJobs.push_back(job);
            runningJobs.sort(); // Ashutosh Why needed?
            scheduleNextCompletion();

            if (runningJobs.size() == 1) { // going from idle to busy
                emit(busySignal, true); //Ashutosh Not clear
                if (hasGUI()) getDisplayString().setTagArg("i",1,"cyan");
            }
        }
    }


    if (runningJobs.size() < maxThreads) {

        // examine all input queues, and request a new job from a non empty queue
        int k = selectionStrategy->select();
        if (k >= 0)
        {
            EV << "requesting job from queue " << k << endl;
            cGate *gate = selectionStrategy->selectableGate(k); //Ashutosh
            check_and_cast<IPassiveQueue *>(gate->getOwnerModule())->request(gate->getIndex());
        }
    }
}

void MTServer::scheduleNextCompletion() {
    // schedule next completion event
    RunningJobs::iterator first = runningJobs.begin();
    //Ashutosh Does it overwrite previous ones
    scheduleAt(simTime() + first->remainingServiceTime * runningJobs.size(), endExecutionMsg); // TODO
}

void MTServer::updateJobTimes() {
    simtime_t d = simTime() - endExecutionMsg->getSendingTime();

    // update service time and remaining time of all jobs
    for (RunningJobs::iterator it = runningJobs.begin(); it != runningJobs.end(); it++) {
        it->remainingServiceTime -= d.dbl() / runningJobs.size(); // Ashutosh Why equal
        it->pJob->setTotalServiceTime(it->pJob->getTotalServiceTime() + d);// Why +d
    }
}

void MTServer::finish() {
}

simtime_t MTServer::generateJobServiceTime(queueing::Job*) {
    simtime_t serviceTime = par("serviceTime");
    if (serviceTime <= 0.0) {
        serviceTime = 0.000001; // make it a very short job
    }
    return serviceTime;
}

bool MTServer::isIdle() {
    //std::cout << "runningJobs.size() = " << runningJobs.size() << endl;
    return runningJobs.size() < maxThreads;
}

bool MTServer::isEmpty() {
    return runningJobs.size() == 0;
}
