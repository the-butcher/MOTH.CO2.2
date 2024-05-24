#include "ModuleCard.h"

#define SD_CS 5  // SD card chip select
#define SPI_CLOCK SD_SCK_MHZ(10)

SdFat32 ModuleCard::sd32;
bool ModuleCard::hasBegun = false;

void ModuleCard::begin() {
    if (!ModuleCard::hasBegun) {
        ModuleCard::sd32.begin(SD_CS, SPI_CLOCK);
        FsDateTime::setCallback(SensorTime::dateTimeCallback);
        ModuleCard::hasBegun = true;
        // ModuleCard::renameFile32("/2024/05/20240519.dar", "/2024/05/20240519.dap");
    }
}

void ModuleCard::historyValues(config_t& config, values_all_t history[HISTORY_____BUFFER_SIZE]) {

    uint32_t nextMeasureIndex = Values::values->nextMeasureIndex;
    uint32_t secondstimeIncr = config.disp.displayHrsChart * SECONDS_PER_____________HOUR / HISTORY_____BUFFER_SIZE;
    uint32_t secondstimeBase = Values::latest().secondstime - secondstimeIncr * (HISTORY_____BUFFER_SIZE - 1);  // secondstime of the first history measurement being searched for

    int32_t secondstimeDiff;
    uint32_t secondstimeIndx;
    uint32_t secondstimeFile = 0;
    int8_t historyIndexMax = -1;

    // read as much as possible from file
    if (config.disp.displayHrsChart > 1) {

        // file access needed for history
        ModuleCard::begin();

        File32 datFile;
        file32_def_t fileDef32Data;

        String datFileNameLast = "";
        String datFilePathLast = "";

        values_all_t readValue;

        for (uint8_t historyIndex = 0; historyIndex < HISTORY_____BUFFER_SIZE; historyIndex++) {

            // the time being searched for
            secondstimeIndx = secondstimeBase + historyIndex * secondstimeIncr;

            // create empty measurement at that index
            history[historyIndex] = Values::emptyMeasurement(secondstimeIndx);

            // get the file definition for the given time (either a dap or dar file extensions)
            fileDef32Data = SensorTime::getFile32DefData(secondstimeIndx);

            // open when hitting a new file name
            if (fileDef32Data.name != datFileNameLast) {
                if (datFile) {
                    datFile.close();  // close the previous file
                }
                datFile.open(fileDef32Data.name.c_str(), O_READ);
                datFileNameLast = fileDef32Data.name;
            }

            // keep reading from old to young until a time is found that is within search bounds or above search bounds
            while (datFile.available() > 1 && secondstimeFile + 30 <= secondstimeIndx) {
                datFile.read((byte*)&readValue, sizeof(readValue));  // read one measurement from the file
                secondstimeFile = readValue.secondstime;
            }
            secondstimeDiff = secondstimeFile - secondstimeIndx;
            if (abs(secondstimeDiff) <= 30) {
                historyIndexMax = historyIndex;
                history[historyIndex] = readValue;
            }
            if (datFile.available() <= 1) {
                break;
            }
        }
        if (datFile) {
            datFile.close();
        }
    }

    uint32_t measureIndex = nextMeasureIndex;  // first searchable index in data
    uint32_t secondstimeMeas = 0;
    for (uint8_t historyIndex = historyIndexMax + 1; historyIndex < HISTORY_____BUFFER_SIZE; historyIndex++) {

        // the time being searched for
        secondstimeIndx = secondstimeBase + historyIndex * secondstimeIncr;

        // create empty measurement at that index
        history[historyIndex] = Values::emptyMeasurement(secondstimeIndx);

        // keep reading from old to young until a time is found that is within search bounds or above search bounds
        while (measureIndex < nextMeasureIndex + MEASUREMENT_BUFFER_SIZE && secondstimeMeas + 30 <= secondstimeIndx) {  // TODO :: currMeasureIndex + 1 should be measureIndex
            secondstimeMeas = Values::values->measurements[measureIndex % MEASUREMENT_BUFFER_SIZE].secondstime;
            measureIndex++;
        }
        secondstimeDiff = secondstimeMeas - secondstimeIndx;
        if (abs(secondstimeDiff) <= 30) {
            history[historyIndex] = Values::values->measurements[(measureIndex - 1) % MEASUREMENT_BUFFER_SIZE];  // assign (is this a copy or a reference?)
        }
    }
}

void ModuleCard::persistValues() {

    ModuleCard::begin();

    file32_def_t fileDef32Dap;
    String datFileNameLast = "";
    String datFilePathLast = "";
    File32 datFile;
    values_all_t value;
    for (int valueIndex = 0; valueIndex < MEASUREMENT_BUFFER_SIZE; valueIndex++) {
        value = Values::values->measurements[valueIndex];
        fileDef32Dap = SensorTime::getFile32Def(value.secondstime, FILE_FORMAT_DATA_PUBLISHABLE);  // the file name that shall be written to (always in publishable file format)
        if (fileDef32Dap.name != datFileNameLast) {
            if (datFile) {  // file already open -> file change at midnight
                datFile.close();
            }
            if (fileDef32Dap.path != datFileNameLast) {  // if not only the file name changed, but also the path (a change in month or year, the folders need to be ready)
                buildFolders(fileDef32Dap.path);
                datFilePathLast = fileDef32Dap.path;
            }
            datFile.open(fileDef32Dap.name.c_str(), O_RDWR | O_CREAT | O_AT_END);
            datFileNameLast = fileDef32Dap.name;
        }
        datFile.write((byte*)&value, sizeof(value));
    }
    if (datFile) {
        datFile.close();
    }
}

bool ModuleCard::isDataPath(String path) {
    return path.endsWith(FILE_FORMAT_DATA_PUBLISHABLE) || path.endsWith(FILE_FORMAT_DATA____ARCHIVED) || path.endsWith(FILE_FORMAT_DATA);
}

String ModuleCard::toDataPath(String path) {
    if (path.endsWith(FILE_FORMAT_DATA)) {
        String dapFilePath = String(path);
        dapFilePath.replace(FILE_FORMAT_DATA, FILE_FORMAT_DATA_PUBLISHABLE);
        if (ModuleCard::existsPath(dapFilePath)) {
            return dapFilePath;
        } else {
            String darFilePath = String(path);
            darFilePath.replace(FILE_FORMAT_DATA, FILE_FORMAT_DATA____ARCHIVED);
            if (ModuleCard::existsPath(darFilePath)) {
                return darFilePath;
            }
        }
    } else if ((path.endsWith(FILE_FORMAT_DATA_PUBLISHABLE) || path.endsWith(FILE_FORMAT_DATA____ARCHIVED)) && ModuleCard::existsPath(path)) {
        return path;
    }
    return FILE_FORMAT_DATA_____INVALID;
}

bool ModuleCard::buildFolders(String folder) {
    return ModuleCard::sd32.mkdir(folder);
}

bool ModuleCard::removeFolder(String folder) {
    return ModuleCard::sd32.rmdir(folder);
}

bool ModuleCard::existsPath(String path) {
    return ModuleCard::sd32.exists(path);
}

bool ModuleCard::removeFile32(String file) {
    return ModuleCard::sd32.remove(file);
}

bool ModuleCard::renameFile32(String pathCurr, String pathDest) {
    return ModuleCard::sd32.rename(pathCurr, pathDest);
}
