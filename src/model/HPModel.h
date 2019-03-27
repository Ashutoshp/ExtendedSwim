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

#ifndef __PLASASIM_HPMODEL_H_
#define __PLASASIM_HPMODEL_H_

#include <omnetpp.h>
#include <set>
#include "HPConfiguration.h"
#include "Environment.h"
#include "Observations.h"
#include "modules/MTServerAdvance.h"
//#include <NotDetEnvModel.h>
//#include <TimeSeriesPredictor.h>

/**
 * TODO - Generated class
 */
class HPModel : public omnetpp::cSimpleModule
{
  public:
      enum ModelChange { SERVERA_ONLINE, SERVERB_ONLINE, SERVERC_ONLINE, INVALID};

      struct ModelChangeEvent {
          double startTime; // when the event was created
          double time; // when the event will happen
          ModelChange change;
      };

      struct ModelChangeEventComp {
        bool operator() (const ModelChangeEvent& lhs, const ModelChangeEvent& rhs) const {
            return lhs.time < rhs.time;
        }
      };
      typedef std::multiset<ModelChangeEvent, ModelChangeEventComp> ModelChangeEvents;
      typedef std::map<std::string, MTServerAdvance::ServerType> JobServeInfo;
      typedef std::vector<Environment> EnvRecord;

  protected:
      virtual int numInitStages() const {return 2;}
      virtual void initialize(int stage);
      virtual ModelChange getOnlineEventCode(MTServerAdvance::ServerType serverType) const;
      struct ServerInfo {
          int activeServerCountLast;
          int maxServers;
          double serviceTime;
          double serviceTimeVariance;
          double lowFidelityServiceTime;
          double lowFidelityServiceTimeVariance;

          ServerInfo() : activeServerCountLast(0) {}
      };

      ModelChangeEvents events;
      JobServeInfo jobServeInfo; // Which server served the job

      // these are used so that we can query the model for what happened an instant earlier
      // TODO save the configuration using a timestamp
      double timeActiveServerCountLast;

      double evaluationPeriod;

      // these hold the current configuration
      HPConfiguration configuration;
      Environment environment;
      Observations observations;
      EnvironmentVector environmentPrediction;
      EnvRecord envRecord;

      double bootDelay;
      int horizon;
      int serverThreads;

      ServerInfo serverA;
      ServerInfo serverB;
      ServerInfo serverC;

      int numberOfBrownoutLevels;

      //TimeSeriesPredictor* pArrivalMeanPredictor;

  //#if USE_VARIANCE_PREDICTOR
      //TimeSeriesPredictor arrivalVariancePredictor;
  //#endif

      bool perfectPrediction;
      omnetpp::simtime_t lastConfigurationUpdate;
      int currentPeriod;

      void addExpectedChange(double time, ModelChange change);
      /**
       * This method removes the last expected change (scheduled farthest in the future)
       * For generality, addExpectedChange() should return some id that could be
       * then used to remove the event.
       */
      void removeExpectedChange();

      void removeExpiredEvents();
      const ServerInfo* getServerInfoObj(MTServerAdvance::ServerType serverType) const;

  public:
      HPModel();
      ~HPModel();

      /* the following methods are less general */

      /**
       * Returns the expected number of active servers at a time in the future
       */
      bool isServerBooting(MTServerAdvance::ServerType serverType) const;
      bool isServerBooting() const;
      int getActiveServerCountIn(double deltaTime, MTServerAdvance::ServerType type);
      void setBrownoutFactor(double factor);
      double getBrownoutFactor() const;

      void addServer(double bootDelay, MTServerAdvance::ServerType type);
      void serverBecameActive(MTServerAdvance::ServerType type);
      void removeServer(MTServerAdvance::ServerType type);

      const HPConfiguration& getConfiguration();
      const Environment& getEnvironment() const;
      double getAvgResponseTime() const;
      void setEnvironment(const Environment& environment);
      const Observations& getObservations() const;
      void setObservations(const Observations& observations);

      int getMaxServers(MTServerAdvance::ServerType type) const;

      double getEvaluationPeriod() const;
      double getBootDelay() const;
      int getHorizon() const;
      int getNumberOfBrownoutLevels() const;

      EnvironmentVector getEnvironmentPrediction(int steps) /*const*/;
      //ScenarioTree* getEnvironmentProbabilityTree() const;

      double getLowFidelityServiceTime(MTServerAdvance::ServerType type) const;
      void setLowFidelityServiceTime(double lowFidelityServiceTimeMean,
              double lowFidelityServiceTimeVariance, MTServerAdvance::ServerType type);

      int getServerThreads() const;
      void setServerThreads(int serverThreads);

      double getServiceTime(MTServerAdvance::ServerType type) const;
      void setServiceTime(double serviceTimeMean, double serviceTimeVariance, MTServerAdvance::ServerType type);

      double getLowFidelityServiceTimeVariance(MTServerAdvance::ServerType type) const;
      double getServiceTimeVariance(MTServerAdvance::ServerType type) const;

      void setCurrentTime(int time);
      void resetCurrentTime();
      int getCurrentTime() const;
      int getDimmerLevel() const;
      void setTrafficLoad(HPLoadBalancer::TrafficLoad serverA,
              HPLoadBalancer::TrafficLoad serverB, HPLoadBalancer::TrafficLoad serverC);
      // Not horizon >= currentPeriod because at last time step we just calculate the utility.
      // We only get best actions 1 step before the horizon. Therefore we execute plan till horizon -1
      bool isValidTime() const {return (horizon > currentPeriod);}
      void getPastInterarrivalRates(std::vector<double>& interarrival_means, unsigned long history_length);
      double getArrivalRate() const;

      void setJobServerInfo(std::string jobName, MTServerAdvance::ServerType serverType);
      MTServerAdvance::ServerType getJobServerInfo(std::string);
};

#endif
