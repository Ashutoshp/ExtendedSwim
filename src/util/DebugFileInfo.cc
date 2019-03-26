/*
 * DebugFileInfo.cc
 *
 *  Created on: Jul 18, 2018
 *      Author: ashutosp
 */

#include <DebugFileInfo.h>
#include <string>
//#include <omnetpp.h>
#include <iostream>
#include <assert.h>
#include <string.h>

DebugFileInfo* DebugFileInfo::mDebugFileInfo = NULL;

DebugFileInfo::DebugFileInfo(bool standardFilePaths, string trace, string parentDirectory, string mode)
        : mStandardFilePaths(standardFilePaths),
          mTrace(trace),
          mParentDirectory(parentDirectory),
          mMode(mode),
          mUtilityFile("utility"),
          mRequestArrivalFile("arrivalRate"),
          mResponseTimeFile("responseTime"),
          mDetailedDebugFile("debug") {
    // TODO Auto-generated constructor stub

    SetDayFromTrace();
}

DebugFileInfo::~DebugFileInfo() {
    // TODO Auto-generated destructor stub
}

void DebugFileInfo::SetDayFromTrace() {
    //wc_day_6

    std::size_t found = mTrace.rfind("wc_day_");
    assert(found!=std::string::npos);

    std::size_t len = strlen("wc_day_");
    //std::cout << "mTrace[found  + len + 1] = " << mTrace[found  + len + 1] << endl;

    if (mTrace[found  + len + 1] != '_') {
        mDay = mTrace.substr(found  + len, 2);
    } else {
        mDay = mTrace.substr(found  + len, 1);
    }
}

string DebugFileInfo::GetUtilityFilePath() const {
    string path = "";

    if (mStandardFilePaths) {
        path = mParentDirectory + "/" + mUtilityFile;
    } else {
        path = mParentDirectory + "/Day" + mDay + "/" + mMode + "/" + mUtilityFile;
    }

    return path;
}

string DebugFileInfo::GetRequestArrivalFilePath() const {
    string path = "";

    if (mStandardFilePaths) {
        path = mParentDirectory + "/" + mRequestArrivalFile;
    } else {
        path = mParentDirectory + "/Day" + mDay + "/" + mMode + "/" + mRequestArrivalFile;
    }

    return path;
}

string DebugFileInfo::GetResponseTimeFilePath() const {
    string path = "";

    if (mStandardFilePaths) {
        path = mParentDirectory + "/" + mResponseTimeFile;
    } else {
        path = mParentDirectory + "/Day" + mDay + "/" + mMode + "/" + mResponseTimeFile;
    }

    return path;
}

string DebugFileInfo::GetDebugFilePath() const {
    string path = "";

    if (mStandardFilePaths) {
        path = mParentDirectory + "/" + mDetailedDebugFile;
    } else {
        path = mParentDirectory + "/Day" + mDay + "/" + mMode + "/" + mDetailedDebugFile;
    }

    return path;
}



const string mUtilityFile;
const string mRequestArrivalFile;
const string mResponseTimeFile;
const string mDetailedDebugFile;
