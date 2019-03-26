//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package plasa.simulations.SwimExtention;

import org.omnetpp.queueing.Classifier;
import org.omnetpp.queueing.Router;
import org.omnetpp.queueing.Source;
import org.omnetpp.queueing.Sink;
import org.omnetpp.queueing.SourceOnce;
import plasa.externalControl.AdaptInterface;
import plasa.managers.monitor.SimProbe;
import plasa.managers.monitor.HPMonitor;
import plasa.model.HPModel;
import plasa.managers.execution.HPExecutionManager;
import plasa.modules.HPLoadBalancer;
import plasa.modules.HPAppServerA;
import plasa.modules.HPAppServerB;
import plasa.modules.HPAppServerC;
import plasa.modules.AppServer;
import plasa.modules.ArrivalMonitor;
import plasa.modules.PredictableSource;
import plasa.modules.PredictableRandomSource;
//
// TODO Auto-generated network
//
network SwimExtention
{
        parameters:
        double bootDelay = default(0);
        double evaluationPeriod = default(10);
        int initialServers = default(1);
        int maxServers = default(1);
        int numberOfBrownoutLevels;
        double dimmerMargin = default(0.0);
        //double responseTimeThreshold @unit(s) = default(1s);
        double responseTimeThreshold = default(0.75);        
        //double maxServiceRate;
        //bool usePredictor = default(false);
        bool useInterArrivalScaleFactorForSlowPlanning = default(false);
        bool useInterArrivalScaleFactorForFastPlanning = default(false);
        bool perfectPrediction = default(false);
        //string pathToDumpProfilingProblems;
        //string pathToProfiledProblems;
        //bool comparePastWorkloadForSimilarity;
        bool problemGenerationMode = default(false);
        bool useIBL = default(false);
       	//int estimatorsCount = default(85);
       	//string pathToTraceEstimatorFile;
       	string pathToLoggingDir;
       	string mode;
       	bool classifierTestMode = default(false);

        //int branchingDepth = default(5);
        //double matchWidthFactor = default(0.1);
        //double bootDelay = default(0);
        //double evaluationPeriod = default(10);
        int initialServersA = default(1);
        int initialServersB = default(0);
        int initialServersC = default(0);
        int maxServersA = default(1);
        int maxServersB = default(1);
        int maxServersC = default(1);
        double penalty = default(-1);
        //int numberOfBrownoutLevels;
        
        double serverA_cost_sec = default(1);
		double serverB_cost_sec = default(16);
		double serverC_cost_sec = default(2);

		double max_arrival_capacity_A = default(40);
		double max_arrival_capacity_low_A = default(100);
		double max_arrival_capacity_B = default(80);
		double max_arrival_capacity_low_B = default(200);
		double max_arrival_capacity_C = default(120);
		double max_arrival_capacity_low_C = default(300);

		double normal_revenue_A = (serverA_cost_sec / max_arrival_capacity_A) * 10;
		double dimmer_revenue_A = (serverA_cost_sec / max_arrival_capacity_low_A) * 3 / 2;
		double normal_revenue_B = (serverB_cost_sec / max_arrival_capacity_B) * 10;
		double dimmer_revenue_B = (serverB_cost_sec / max_arrival_capacity_low_B) * 3 / 2;
		double normal_revenue_C = (serverC_cost_sec / max_arrival_capacity_C) * 10;
		double dimmer_revenue_C = (serverC_cost_sec / max_arrival_capacity_low_C) * 3 / 2;

    submodules:
        sink: Sink {
            @display("p=522,211");
        }
        loadBalancer: HPLoadBalancer {
            @display("p=302,159");
            routingAlgorithm = "probDist";
        }
        arrivalMonitor: ArrivalMonitor {
            @display("p=187,152");
        }
        source: PredictableSource {
            @display("p=54,165");
        }
        classifier: Classifier {
            @display("p=431,165");
        }
        sinkLow: Sink {
            @display("p=522,142");
        }
        adaptInterface: AdaptInterface {
            @display("p=440,32");
        }
        model: HPModel {
            @display("p=127,26");
        }
        monitor: HPMonitor {
            @display("p=210,26");
        }
        executionManager: HPExecutionManager {
            @display("p=302,26");
        }
        //probe: SimProbe {
        //    @display("p=141,94");
        //}
    connections:
        arrivalMonitor.out --> loadBalancer.in++;
        source.out --> arrivalMonitor.in;
        classifier.out++ --> sink.in++;
        classifier.rest --> sinkLow.in++;
        monitor.out --> adaptInterface.probe;
        //probe.out++ --> adaptInterface.probe;
        //probe.out++ --> monitor.probe;
}
