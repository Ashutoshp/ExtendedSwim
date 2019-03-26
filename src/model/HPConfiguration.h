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

#ifndef HPCONFIGURATION_H_
#define HPCONFIGURATION_H_

#include <modules/MTServerAdvance.h>
#include <modules/HPLoadBalancer.h>

class HPConfiguration {
protected:
    int mServersA; // number of active servers of type A (there is one more powered up if bootRemain > 0
    int mServersB;
    int mServersC;

    int mBootRemain; // how many periods until we have one more server. If 0, no server is booting
    MTServerAdvance::ServerType mBootServerType;

    double mBrownoutFactor;

    HPLoadBalancer::TrafficLoad mTrafficA;
    HPLoadBalancer::TrafficLoad mTrafficB;
    HPLoadBalancer::TrafficLoad mTrafficC;

public:
    HPConfiguration();
    HPConfiguration(int serverA, int serverB, int serverC,
            int bootRemain, MTServerAdvance::ServerType serverType,
            double brownoutFactor,
            HPLoadBalancer::TrafficLoad trafficA = HPLoadBalancer::TrafficLoad::HUNDRED,
            HPLoadBalancer::TrafficLoad trafficB = HPLoadBalancer::TrafficLoad::ZERO,
            HPLoadBalancer::TrafficLoad trafficC = HPLoadBalancer::TrafficLoad::ZERO);
    virtual ~HPConfiguration();

    int getBootRemain() const;
    MTServerAdvance::ServerType getBootType() const;
    void setBootRemain(int bootRemain, MTServerAdvance::ServerType = MTServerAdvance::ServerType::NONE);

    double getBrownOutFactor() const;
    void setBrownOutFactor(double brownOutFactor);

    int getActiveServers(MTServerAdvance::ServerType) const;
    void setActiveServers(int servers, MTServerAdvance::ServerType);
    int getServers(MTServerAdvance::ServerType serverType) const;

    HPLoadBalancer::TrafficLoad getTraffic(MTServerAdvance::ServerType serverType) const;
    void setTraffic(MTServerAdvance::ServerType serverType, HPLoadBalancer::TrafficLoad trafficLoad);
    int getTotalActiveServers() const;
};

#endif /* HPCONFIGURATION_H_ */
