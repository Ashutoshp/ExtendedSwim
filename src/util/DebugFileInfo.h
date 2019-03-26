/*
 * DebugFileInfo.h
 *
 *  Created on: Jul 18, 2018
 *      Author: ashutosp
 */

#ifndef UTIL_DEBUGFILEINFO_H_
#define UTIL_DEBUGFILEINFO_H_

using namespace std;

#include <stdio.h>
#include <iostream>
#include <string>

class DebugFileInfo {
private:
    const bool mStandardFilePaths;
    const string mTrace;
    const string mParentDirectory;
    const string mMode;
    const string mUtilityFile;
    const string mRequestArrivalFile;
    const string mResponseTimeFile;
    const string mDetailedDebugFile;
    string mDay;

    static DebugFileInfo* mDebugFileInfo;
    DebugFileInfo(bool standardFilePaths, string trace, string parentDirectory, string mode);
    void SetDayFromTrace();

public:
    static DebugFileInfo* getInstance(bool standardFilePaths = true, string traceName = "", string parentDir = "", string mode = "") {
        if (mDebugFileInfo == NULL) {
            mDebugFileInfo = new DebugFileInfo(standardFilePaths, traceName, parentDir, mode);
        }

        return mDebugFileInfo;
    }

    string GetUtilityFilePath() const;
    string GetRequestArrivalFilePath() const;
    string GetResponseTimeFilePath() const;
    string GetDebugFilePath() const;
    //string GetParentDirectory() const;

    virtual ~DebugFileInfo();
};

#endif /* UTIL_DEBUGFILEINFO_H_ */
