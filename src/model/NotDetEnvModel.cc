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
 * NotDetEnvModel.cc
 *
 *  Created on: Aug 21, 2015
 *      Author: ashutosp
 */

#include <NotDetEnvModel.h>
#include <assert.h>
#include "HPModel.h"

NotDetEnvModel* NotDetEnvModel::mEnvModel = NULL;

#define MINARRIVALRATE 10;
#define BANDWIDTH 10;
#define INITIALMAXARRIVALRATE 100;

bool perfectEnv = false;
double env[43];
int index1 = 0;
int horizon = 7;

NotDetEnvModel::NotDetEnvModel() {
    // TODO Auto-generated constructor stub
    mMinArrivalRate = MINARRIVALRATE;
    mBandWidth = BANDWIDTH;
    mMaxArrivalRate = INITIALMAXARRIVALRATE;
    buildModel();
}

NotDetEnvModel::~NotDetEnvModel() {
    // TODO Auto-generated destructor stub
}

void NotDetEnvModel::buildModel() {
    if (perfectEnv) {
        env[0] = 0.0136644;
        env[1] = 0.0133373;
        env[2] = 0.0119273;
        env[3] = 0.0090703;
        env[4] = 0.00879577;
        env[5] = 0.00854561;
        env[6] = 0.00926523;
        env[7] = 0.00877777;
        env[8] = 0.00871189;
        env[9] = 0.00804859;
        env[10] = 0.00780405;
        env[11] = 0.00792207;
        env[12] = 0.00795251;
        env[13] = 0.00749685;
        env[14] = 0.00674501;
        env[15] = 0.00756305;
        env[16] = 0.00893382;
        env[17] = 0.00998392;
        env[18] = 0.0100628;
        env[19] = 0.0100136;
        env[20] = 0.0103759;
        env[21] = 0.0110962;
        env[22] = 0.0111993;
        env[23] = 0.0123696;
        env[24] = 0.0121001;
        env[25] = 0.0112575;
        env[26] = 0.0103757;
        env[27] = 0.00945316;
        env[28] = 0.0087403;
        env[29] = 0.00876926;
        env[30] = 0.00856684;
        env[31] = 0.00850081;
        env[32] = 0.00866338;
        env[33] = 0.00854196;
        env[34] = 0.0079456;
        env[35] = 0.00756412;
        env[36] = 0.00667872;
        env[37] = 0.00808097;
        env[38] = 0.00966343;
        env[39] = 0.0106719;
        env[40] = 0.0115544;
        env[41] = 0.0114222;
        env[42] = 0.0121236;
    } else {
        mInterArrivalRates.clear();
        mInterArrivalRates.insert(mMinArrivalRate);

        double arrivalRate = mMinArrivalRate;

        while (true) {
            mInterArrivalRates.insert(arrivalRate);
            arrivalRate += mBandWidth;

            if (arrivalRate > mMaxArrivalRate) {
                break;
            }
        }
    }
}

void NotDetEnvModel::updateModel(double arrivalRate) {
    int factor = arrivalRate/mBandWidth + 1;
    mMaxArrivalRate = mBandWidth * factor;

    buildModel();
}

unsigned NotDetEnvModel::getClosestArrivalRateIndex(double envArrivalRate, bool search) const {
    unsigned counter = UINT_MAX;

    if (perfectEnv) {
        //static bool a = true; // Hack due to two calls made to this function
        static int k = 0;
        //return hpModel->getCurrentTime();
        if (search) {
        //    a = true;
        //} else {
            //k = index1;
            while (k < 43) {
                //std::cout << 1 / envArrivalRate << "##" << env[k] << "##"
                //        << 1 / envArrivalRate - env[k] << std::endl;
                if (-0.00001 < (1 / envArrivalRate - env[k])
                        && (1 / envArrivalRate - env[k]) < 0.00001) {
                    break;
                }
                //if (double (1/envArrivalRate - env[k]) == 0) {
                //    break;
                //}
                ++k;
            }
            //a = false;
        }
        counter = k%(horizon+1);
    } else {
        if (envArrivalRate <= mMinArrivalRate) {
            counter = 0;
        } else if (envArrivalRate <= (mMaxArrivalRate + mBandWidth / 2)) {
            EnvModel::iterator itr = mInterArrivalRates.begin();
            counter = 0;
            unsigned arrivalRate = *itr;

            while (itr != mInterArrivalRates.end()) {
                ++itr;
                if (arrivalRate < envArrivalRate) {
                    ++counter;
                    arrivalRate = *itr;
                } else {
                    break;
                }
            }

            if (arrivalRate >= envArrivalRate) {
                counter =
                        ((arrivalRate - mBandWidth / 2) < envArrivalRate) ?
                                counter : counter - 1;
            } else {
                --counter;
            }

            assert(counter <= mInterArrivalRates.size());
        }
    }

    return counter;
}

/*
[tick] true -> 0.10 : (s' = 0)
        + 0.10 : (s' = 1)
        + 0.10: (s' = 2)
        + 0.10: (s' = 3)
        + 0.10 : (s' = 4)
        + 0.10 : (s' = 5)
        + 0.10: (s' = 6)
        + 0.10: (s' = 7)
        + 0.10 : (s' = 8)
        + 0.10 : (s' = 9);
*/

void NotDetEnvModel::getEnvPrismModel(const double envArrivalMean, std::stringstream& env_model, bool probabilistic) const {
    if (perfectEnv) {
        //HPModel* hpmodel = simulation.getModule("model");
        //int horizon = 7;
        env_model << "module environment" << endl;
        env_model << "s : [0.."<< horizon + 1 << "] init 0;" << endl;

        int j = 0;

        while (j < horizon + 1) {
            env_model << "[tick] s = " << j << " -> 1 : (s' = " << j + 1 << ");" << endl;
            ++j;
        }

        env_model << "endmodule" << endl << endl;
        env_model << "formula stateValue = ";
        j = 0;

        while (j <= horizon + 1) {
            if (j == horizon + 1) {
                if (index1 > 42) {
                    env_model << "             (s = " << j << " ? " << env[42] << " : 0);" << endl;
                } else {
                    env_model << "             (s = " << j << " ? " << env[index1] << " : 0);" << endl;
                }
            } else {
                if (index1 > 42) {
                    env_model << "             (s = " << j << " ? " << env[42]
                            << " : 0) + " << endl;
                } else {
                    env_model << "             (s = " << j << " ? "
                            << env[index1] << " : 0) + " << endl;
                }
            }
            ++j;
            ++index1;
        }
        --index1;
    } else {
        env_model << "module environment" << endl;
        env_model << "s : [0.." << mInterArrivalRates.size() - 1 << "] init "
                << getClosestArrivalRateIndex(1 / envArrivalMean) << ";"
                << endl;

        unsigned i = 0;

        if (probabilistic) {
            double probability = double(1.0 / mInterArrivalRates.size());
            assert(mInterArrivalRates.size() != 0);
            while (i < mInterArrivalRates.size()) {
                if (i == 0) {
                    env_model << endl << "[tick] true -> " << probability
                            << " : (s' = " << i << ")";
                } else {
                    env_model << endl << "+ " << probability << ": (s' = " << i
                            << ")";
                }

                if (i == mInterArrivalRates.size() - 1) {
                    env_model << ";" << endl;
                }
                ++i;
            }
        } else {
            while (i < mInterArrivalRates.size()) {
                env_model << "[tick] true -> 1 : (s' = " << i << ");" << endl;
                ++i;
            }
        }

        env_model << "endmodule" << endl << endl;

        EnvModel::iterator itr = mInterArrivalRates.begin();
        env_model << "formula stateValue = ";
        i = 0;

        while (itr != mInterArrivalRates.end()) {
            if (i == mInterArrivalRates.size() - 1) {
                env_model << "             (s = " << i << " ? 1/" << (*itr)
                        << " : 0);" << endl;
            } else {
                env_model << "             (s = " << i << " ? 1/" << (*itr)
                        << " : 0) + " << endl;
            }
            ++itr;
            ++i;
        }
    }
}
