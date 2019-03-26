/*
 * Utils.cpp
 *
 *  Created on: Dec 19, 2017
 *      Author: ashutosp
 */

#include <Utils.h>
//#include <boost/filesystem.hpp>
//#include <experimental/filesystem>
#include <sys/stat.h>
//#include <errno.h>
#include <string.h>
#include <stdexcept>

DumpPlanningProblems* DumpPlanningProblems::mDumpPlanningProblems = NULL;

// Generate fast plan all the time
// Generate features

int createDirectory(const char* path) {
    const int dirErr = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (dirErr == -1) {
        throw runtime_error("error createDirectory");
    }

    return dirErr;
}

string createTempDirectory(const char* tempDirTemp) {
    char tempDirTemplate[2048];
    strcpy(tempDirTemplate, tempDirTemp);

    // create temp directory
    char* tempDir = mkdtemp(tempDirTemplate);

    if (!tempDir) {
        // TODO improve error handling
        throw runtime_error("error create_temp_directory mkdtemp");
    }

    return tempDir;
}

void copyFile(string fileNameFrom, string fileNameTo) {
    //printf("%s %s\n", fileNameFrom, fileNameTo);

    std::ifstream in (fileNameFrom.c_str());
    std::ofstream out (fileNameTo.c_str());
    out << in.rdbuf();
    out.close();
    in.close();
}


//void copy_files_from_directory(const string& source, const string& destination) {

//}
DumpPlanningProblems::DumpPlanningProblems(const string& location) : mLocation(location),
                                               mModelTemplate("modelXXXXXX"),
                                               mSpecFileName("ttimemodel.prism"),
                                               mStatesFileName("result.sta"),
                                               mLabelsFileName("result.lab"),
                                               mAdversaryFileName("result.adv"),
                                               mFastDirName("fast"),
                                               mSlowDirName("slow"),
                                               mFeaturesFileName("ClassifierTest.csv"),
                                               mTempDirTemplate(mLocation +  "/" + mModelTemplate) {

}

DumpPlanningProblems::~DumpPlanningProblems() {


}

void DumpPlanningProblems::writeHeader(ofstream& fout) {
    fout << "TraceName" << "," << "Profile Dir"
            << "," << "Reactive Dir"
            << "," << "Deliberative Dir"
            << "," << "Dimmer"
            << "," << "ServerA Count"
            << "," << "ServerB Count"
            << "," << "ServerC Count"
            << "," << "ServerA State"
            << "," << "ServerB State"
            << "," << "ServerC State"
            << "," << "ServerA Load"
            << "," << "ServerB Load"
            << "," << "ServerC Load"
            << "," << "Response Time"
            << "," << "RA 1"
            << "," << "RA 2"
            << "," << "RA 3"
            << "," << "RA 4"
            << "," << "RA 5"
            << "," << "RA 6"
            << "," << "RA 7"
            << "," << "RA 8"
            << "," << "RA 9"
            << "," << "RA 10"
            << "," << "RA 11"
            << "," << "RA 12"
            << "," << "RA 13"
            << "," << "RA 14"
            << "," << "RA 15"
            << "," << "RA 16"
            << "," << "RA 17"
            << "," << "RA 18"
            << "," << "RA 19"
            << "," << "RA 20"
            << "," << "Predicted"
            << "," << "Use Reactive"
            << "\n";
}

void DumpPlanningProblems::copyFileFromFast(const string& source, const string& destination) {
    string srcModelPath = source + "/" + mSpecFileName;
    string srcAdversaryPath = source + "/" + mAdversaryFileName;
    string srcStatesPath = source + "/" + mStatesFileName;
    string srcLabelsPath = source + "/" + mLabelsFileName;

    string destModelPath = destination + "/" + mSpecFileName;
    string destAdversaryPath = destination + "/" + mAdversaryFileName;
    string destStatesPath = destination + "/" + mStatesFileName;
    string destLabelsPath = destination + "/" + mLabelsFileName;

    // specification file .... though not required
    copyFile(srcModelPath, destModelPath);

    // label file
    copyFile(srcAdversaryPath, destAdversaryPath);

    // adversary file
    copyFile(srcStatesPath, destStatesPath);

    // states file
    copyFile(srcLabelsPath, destLabelsPath);
}

void DumpPlanningProblems::copyFileFromSlow(const string& source, const string& destination) {
    // spec file
    string srcModelPath = source + "/" + mSpecFileName;
    string destModelPath = destination + "/" + mSpecFileName;

    copyFile(srcModelPath, destModelPath);
}

/*void testUtils(string location) {
    string tempDirTemplate = location + "modelXXXXXX";
    string path = create_temp_directory(tempDirTemplate.c_str());

}*/

void DumpPlanningProblems::writeInitialStateVariables(ofstream& fout, HPModel* hpModel) {
    // Write Server info
    //initialState << "const int ini_dimmer = ";
    int discretizedBrownoutFactor = 1
            + (hpModel->getNumberOfBrownoutLevels() - 1)
                * hpModel->getConfiguration().getBrownOutFactor();
    fout << "," << discretizedBrownoutFactor;

    fout << "," << hpModel->getConfiguration().getActiveServers(
                        MTServerAdvance::ServerType::A);
    fout << "," << hpModel->getConfiguration().getActiveServers(
                            MTServerAdvance::ServerType::B);
    fout << "," << hpModel->getConfiguration().getActiveServers(
                            MTServerAdvance::ServerType::C);


    int addServerAState = 0;
    int addServerBState = 0;
    int addServerCState = 0;

    int addServerState = 0;

    if (hpModel->getConfiguration().getBootRemain() > 0) {
        int bootPeriods = hpModel->getBootDelay()
                / hpModel->getEvaluationPeriod();

        addServerState = min(
                bootPeriods - hpModel->getConfiguration().getBootRemain(),
                bootPeriods);

        MTServerAdvance::ServerType bootType =
                hpModel->getConfiguration().getBootType();

        switch (bootType) {
            case MTServerAdvance::ServerType::A:
                addServerAState = addServerState;
                break;
            case MTServerAdvance::ServerType::B:
                addServerBState = addServerState;
                break;
            case MTServerAdvance::ServerType::C:
                addServerCState = addServerState;
                break;
            case MTServerAdvance::ServerType::NONE:
                assert(false);
        }
    }

    fout << "," << addServerAState;
    fout << "," << addServerBState;
    fout << "," << addServerCState;

    // Write workload distribution
    fout << "," << hpModel->getConfiguration().getTraffic(MTServerAdvance::ServerType::A);
    fout << "," << hpModel->getConfiguration().getTraffic(MTServerAdvance::ServerType::B);
    fout << "," << hpModel->getConfiguration().getTraffic(MTServerAdvance::ServerType::C);

    // Write current average response time
    fout << "," << (hpModel->getObservations()).avgResponseTime;
}

void DumpPlanningProblems::writeData(const string& destinationDir,
        const string& reactivePlanDir,
        const string& deliberativePlanDir,
        HPModel* hpModel,
        const std::vector<double>& arrivalRates,
        double classifierLabel,
        const string& traceName) {

    static bool headerWritten = false;

    // Open file
    string filePath = mLocation + "/" + mFeaturesFileName;

    // Append to the file
    ofstream fout(filePath.c_str(), std::ofstream::out | std::ofstream::app);

    if (!headerWritten) {
        writeHeader(fout);
        headerWritten = true;
    }

    fout << traceName << "," << destinationDir
            << "," << reactivePlanDir
            << "," << deliberativePlanDir;

    // Add initial state variables
    writeInitialStateVariables(fout, hpModel);

    // Now add arrival rates
    std::vector<double>::const_iterator itr = arrivalRates.begin();

    while (itr != arrivalRates.end()) {
        fout << "," << *itr;
        ++itr;
    }

    fout << "," << classifierLabel;
    fout << endl;

    // Close file
    fout.close();
}

void DumpPlanningProblems::copySampleProblems(
        const string& reactivePlanDir,
        const string& deliberativePlanDir,
        HPModel* hpModel,
        const std::vector<double>& arrivalRates,
        double classifierLabel, const string& traceName) {
    // Create parent directory
    //char tempDirTemplate[] = "modelXXXXXX";
    //string tempDirTemplate = location +  "/" + mModelTemplate;
    string path = createTempDirectory(mTempDirTemplate.c_str());

    string fastPath = path +  "/" + mFastDirName;
    string slowPath = path + "/" + mSlowDirName;
    //string features_path = path + "/features";

    // Create fast directory
    createDirectory(fastPath.c_str());

    // Create slow directory
    createDirectory(slowPath.c_str());

    // Create features directory
    //create_directory(features_path.c_str());

    // Copy fast
    copyFileFromFast(reactivePlanDir, fastPath);

    // Copy slow
    copyFileFromSlow(deliberativePlanDir, slowPath);

    // Write features
    writeData(path, reactivePlanDir, deliberativePlanDir, hpModel, arrivalRates, classifierLabel, traceName);
}




