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

#ifndef __PLASASIM_HPMONITOR_H_
#define __PLASASIM_HPMONITOR_H_

#include <omnetpp.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <memory>
#include <map>
#include "model/HPModel.h"
//#include "TimeSeriesPredictor.h"
#include "ServerUtilization.h"
#include "modules/MTServerAdvance.h"

/**
 * TODO - Generated class
 */
class HPMonitor : public cSimpleModule, cListener
{
protected:
    cMessage *periodEvent; // pointer to the event object which we'll use for timing
    cMessage *periodPostEvent; // pointer to the event object which we'll use for timing (post proc)

    /* subscribed signals */
    simsignal_t lifeTimeSignalId;
    simsignal_t jobCompletionSignal;
    simsignal_t interArrivalSignal;
    simsignal_t serverActiveSignal;

    /* emitted signals */
    simsignal_t numberOfServersSignal;
    simsignal_t activeServersSignal;
    simsignal_t requestLateSignal;
    simsignal_t measuredInterarrivalAvg;
    simsignal_t measuredInterarrivalStdDev;
    simsignal_t averageResponseTimeViolationSignal;
    simsignal_t utilitySignal;
    simsignal_t brownoutFactorSignal;
    simsignal_t predictedResponseTime;
    simsignal_t predictedInterArrival;
    simsignal_t serverBusySignalId;

    simsignal_t serverRemovedSignal;
    simsignal_t serverAddedSignal;
    simsignal_t serverActivatedSignal;
    simsignal_t brownoutSetSignal;

    double responseTimeCumulativePeriod; // accumulates individual observed response times for the sampling period
    long responseTimeCountPeriod; // how many observations in the sampling period

    long requestCount; // number of requests in the last period
    double requestUtilityCumulative; // the utility produced by in the last period (not including cost)

    typedef boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean, boost::accumulators::tag::moment<2> > > InterArrivalStats;
    std::auto_ptr<InterArrivalStats> pInterArrivalStats;

    HPModel* pModel;

    typedef std::map<int, ServerUtilization> UtilizationTable;

    UtilizationTable utilizationTable;

    bool halfPeriod;
    SimTime latestJobResponseTime;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void dump_utility(const double & utility);
  public:
    HPMonitor();

    void receiveSignal(cComponent *source, simsignal_t signalID, const SimTime& t, cObject *details);
    void receiveSignal(cComponent *source, simsignal_t signalID, bool value, cObject *details);
    void receiveSignal(cComponent *source, simsignal_t signalID, double value, cObject *details);
    void receiveSignal(cComponent *source, simsignal_t signalID, const char* jobName, cObject *details);
    void resetPeriodRevenue() {requestUtilityCumulative = 0;}
    void periodRevenue() {return requestUtilityCumulative;}

    virtual ~HPMonitor();
};

#endif
