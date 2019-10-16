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

#include <HPConfiguration.h>
#include <assert.h>

HPConfiguration::HPConfiguration() : mServersA(0), mServersB(0), mServersC(0),
        mBootRemain(0), mBootServerType(MTServerAdvance::NONE), mBrownoutFactor(0), mTrafficA(HPLoadBalancer::TrafficLoad::HUNDRED),
        mTrafficB(HPLoadBalancer::TrafficLoad::ZERO), mTrafficC(HPLoadBalancer::TrafficLoad::ZERO) {
    //std::cout << "Hi" << endl;
}

HPConfiguration::HPConfiguration(int serverA, int serverB, int serverC,
        int bootRemain, MTServerAdvance::ServerType serverType,
        double brownoutFactor, HPLoadBalancer::TrafficLoad trafficA, HPLoadBalancer::TrafficLoad trafficB, HPLoadBalancer::TrafficLoad trafficC) :
        mServersA(serverA), mServersB(serverB), mServersC(serverC),
                mBootRemain(bootRemain), mBootServerType(serverType),
                mBrownoutFactor(brownoutFactor), mTrafficA(trafficA),
                mTrafficB(trafficB), mTrafficC(trafficC) {
    //std::cout << "Hello" << endl;
}

HPConfiguration::~HPConfiguration() {
    // TODO Auto-generated destructor stub
}

int HPConfiguration::getBootRemain() const {
    //std::cout << "HPConfiguration::getBootRemain = " << mBootRemain << std::endl;

    return mBootRemain;
}

MTServerAdvance::ServerType HPConfiguration::getBootType() const {
    return mBootServerType;
}

void HPConfiguration::setBootRemain(int bootRemain, MTServerAdvance::ServerType serverType) {
    //std::cout << "HPConfiguration::setBootRemain = " << bootRemain << " for Type = " << serverType << std::endl;

    this->mBootRemain = bootRemain;

    if (this->mBootRemain == 0) {
        this->mBootServerType = MTServerAdvance::ServerType::NONE;
    } else if (serverType != MTServerAdvance::ServerType::NONE) {
        this->mBootServerType = serverType;
    }
}

double HPConfiguration::getBrownOutFactor() const {
    return mBrownoutFactor;
}

void HPConfiguration::setBrownOutFactor(double brownOutFactor) {
    this->mBrownoutFactor = brownOutFactor;
}

int HPConfiguration::getServers(MTServerAdvance::ServerType type) const {
    int serverCount;

    switch (type) {
    case MTServerAdvance::ServerType::A:
        serverCount = this->mServersA + ((mBootServerType == MTServerAdvance::ServerType::A) ? 1 : 0);
        break;
    case MTServerAdvance::ServerType::B:
        serverCount = this->mServersB + ((mBootServerType == MTServerAdvance::ServerType::B) ? 1 : 0);
        break;
    case MTServerAdvance::ServerType::C:
        serverCount = this->mServersC + ((mBootServerType == MTServerAdvance::ServerType::C) ? 1 : 0);
        break;
    case MTServerAdvance::ServerType::NONE:
        serverCount = 0;
    }

    return serverCount;
}

int HPConfiguration::getActiveServers(MTServerAdvance::ServerType serverType) const {
    int serverCount;

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        serverCount = this->mServersA;
        break;
    case MTServerAdvance::ServerType::B:
        serverCount = this->mServersB;
        break;
    case MTServerAdvance::ServerType::C:
        serverCount = this->mServersC;
        break;
    case MTServerAdvance::ServerType::NONE:
        serverCount = 0;
    }

    return serverCount;
}

void HPConfiguration::setActiveServers(int servers, MTServerAdvance::ServerType serverType) {
    //std::cout << "HPConfiguration::setActiveServers = " << serverType << std::endl;

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        this->mServersA = servers;
        break;
    case MTServerAdvance::ServerType::B:
        this->mServersB = servers;
        break;
    case MTServerAdvance::ServerType::C:
        this->mServersC = servers;
        break;
    case MTServerAdvance::ServerType::NONE:
        assert(false);
    }

    this->mBootRemain = 0;
    this->mBootServerType = MTServerAdvance::ServerType::NONE;
}

HPLoadBalancer::TrafficLoad HPConfiguration::getTraffic(MTServerAdvance::ServerType serverType) const {
    HPLoadBalancer::TrafficLoad traffic = HPLoadBalancer::TrafficLoad::INVALID;

    switch (serverType) {
    case MTServerAdvance::ServerType::A:
        traffic = this->mTrafficA;
        break;
    case MTServerAdvance::ServerType::B:
        traffic = this->mTrafficB;
        break;
    case MTServerAdvance::ServerType::C:
        traffic = this->mTrafficC;
        break;
    case MTServerAdvance::ServerType::NONE:
        assert(false);
    }

    return traffic;
}

void HPConfiguration::setTraffic(MTServerAdvance::ServerType serverType, HPLoadBalancer::TrafficLoad trafficLoad) {
    switch (serverType) {
        case MTServerAdvance::ServerType::A:
            this->mTrafficA = trafficLoad;
            break;
        case MTServerAdvance::ServerType::B:
            this->mTrafficB = trafficLoad;
            break;
        case MTServerAdvance::ServerType::C:
            this->mTrafficC = trafficLoad;
            break;
        case MTServerAdvance::ServerType::NONE:
            assert(false);
        }
}

int HPConfiguration::getTotalActiveServers() const {
    return this->mServersA + this->mServersB + this->mServersC;
}
