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
 * HPExecutionManagerMod.h
 *
 *  Created on: Aug 18, 2015
 *      Author: ashutosp
 */

#ifndef HPEXECUTIONMANAGERMOD_H_
#define HPEXECUTIONMANAGERMOD_H_

//#include <csimplemodule.h>
#include <set>
#include "BootComplete_m.h"
#include <model/HPModel.h>
#include "HPExecutionManagerModBase.h"
#include <string>

using namespace std;

class HPExecutionManagerMod : public HPExecutionManagerModBase, cListener  {
    simsignal_t serverBusySignalId;
    int serverBeingRemovedModuleId;
    cMessage* completeRemoveMsg;

    /**
     * Sends a message so that when received (immediatly) will complete the removal
     * This is used so that the signal handler can do it
     */
    //void completeServerRemoval(MTServerAdvance::ServerType serverType);
    void completeServerRemoval();

    bool isServerBeingRemoveEmpty();
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    // target-specific methods (e.g., actual servers, sim servers, etc.)

    /**
     * @return BootComplete* to be handled later by doAddServerBootComplete()
     */
    virtual BootComplete* doAddServer(MTServerAdvance::ServerType serverType, bool instantaneous = false);
    virtual void doAddServerBootComplete(BootComplete* bootComplete);

    /**
     * @return BootComplete* identical in content (not the pointer itself) to
     *   what doAddServer() would have return for this server
     */
    virtual BootComplete* doRemoveServer(MTServerAdvance::ServerType serverType);
    virtual void doSetBrownout(MTServerAdvance::ServerType serverType, double factor);
    virtual void doSetBrownout(double factor);
    virtual string getModuleStr(MTServerAdvance::ServerType serverType) const;
  public:
    HPExecutionManagerMod();
    virtual ~HPExecutionManagerMod();

    virtual string getServerString(MTServerAdvance::ServerType serverType, bool internal=false) const;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool value, cObject *details);
};

#endif /* HPEXECUTIONMANAGERMOD_H_ */
