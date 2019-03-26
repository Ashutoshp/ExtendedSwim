/*
 * Utils.h
 *
 *  Created on: Dec 19, 2017
 *      Author: ashutosp
 */

#ifndef UTIL_UTILS_H_
#define UTIL_UTILS_H_

#include <string>
#include <vector>
#include <model/HPModel.h>
#include <fstream>

using namespace std;


class DumpPlanningProblems {

private:
    static DumpPlanningProblems* mDumpPlanningProblems;
    const string mLocation;
    const string mModelTemplate;
    const string mSpecFileName;
    const string mStatesFileName;
    const string mLabelsFileName;
    const string mAdversaryFileName;
    const string mFastDirName;
    const string mSlowDirName;
    const string mFeaturesFileName;
    const string mTempDirTemplate;

    DumpPlanningProblems(const string& location);
    void writeHeader(ofstream& fout);


public:
    static DumpPlanningProblems* get_instance(const string& location) {
        if (mDumpPlanningProblems == NULL) {
            mDumpPlanningProblems = new DumpPlanningProblems(location);
        }

        return mDumpPlanningProblems;
    }

    void copySampleProblems(
            const string& reactivePlanDir,
            const string& deliberativePlanDir,
            HPModel* hpModel,
            const std::vector<double>& arrivalRates, double classifierLabel, const string& traceName = "");

    void copyFileFromFast(const string& source, const string& destination);
    void copyFileFromSlow(const string& source, const string& destination);
    void writeInitialStateVariables(ofstream& fout, HPModel* hpModel);
    void writeData(const string& destinationDir, const string& reactivePlanDir,
            const string& deliberativePlanDir, HPModel* hpModel,
            const std::vector<double>& arrivalRates, double classifierLabel, const string& traceName);

    ~DumpPlanningProblems();
};

int create_directory(const char* path);
string create_temp_directory(const char* tempDirTemp);
//void copy_files_from_directory(const string& source, const string& destination);
void copy_file(const char* fileNameFrom, const char* fileNameTo);
//void copySampleProblems(const string& reactive_plan_dir,
//        const string& deliberative_plan_dir, const string& location, const std::vector<double>& features);
//void copy_file_from_fast(const string& source, const string& destination);
//void copy_file_from_slow(const string& source, const string& destination);
void test_utils(string location);

#endif /* UTIL_UTILS_H_ */

