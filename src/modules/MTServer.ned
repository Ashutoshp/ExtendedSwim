//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package plasa.modules;
import org.omnetpp.queueing.Server;
//
// Queue server. It serves multiple input queues (PassiveQueue), using a preset
// algorithm. Inputs must be connected to Passive Queues (PassiveQueue)
//
simple MTServer extends Server
{
    parameters:
		int threads = default(1);
		double timeout @unit(s) = default(0.0); // if an arriving job has spent this amount of time or more queueing, it is just passed without being serviced
	
	@class(MTServer);
}


