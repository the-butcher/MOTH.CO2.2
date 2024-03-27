#include "ModuleSdcard.h"

#define SD_CS 5  // SD card chip select
#define SPI_CLOCK SD_SCK_MHZ(10)

SdFat32 ModuleSdcard::sd32;
bool ModuleSdcard::hasBegun = false;

void ModuleSdcard::begin() {
    if (!ModuleSdcard::hasBegun) {
        ModuleSdcard::sd32.begin(SD_CS, SPI_CLOCK);
        ModuleSdcard::hasBegun = true;
    }
}

static values_all_t emptyMeasurement(uint32_t secondstime) {
    return {
        secondstime,                                                           // secondstime of history measurement
        {0, SensorScd041::toShortDeg(0.0), SensorScd041::toShortHum(0.0), 0},  // co2 measurement
        {0.0f},                                                                // bme measurement
        {SensorEnergy::toShortPercent(0.0)}                                    // nrg measurement
    };
}

void ModuleSdcard::historyValues(values_t* values, config_t* config, values_all_t history[HISTORY_____BUFFER_SIZE]) {
    // setup search seconds
    uint32_t currMeasureIndex = values->nextMeasureIndex - 1;
    uint32_t secondstimeIncr = config->disp.displayHrsChart * SECONDS_PER_____________HOUR / HISTORY_____BUFFER_SIZE;
    uint32_t secondstimeBase = values->measurements[currMeasureIndex % MEASUREMENT_BUFFER_SIZE].secondstime - secondstimeIncr * (HISTORY_____BUFFER_SIZE - 1);  // the secondstime

    int32_t secondstimeDiff;
    uint32_t secondstimeIndx;
    uint32_t secondstimeFile = 0;
    int8_t historyIndexMax = -1;

    // file related stuff
    if (config->disp.displayHrsChart > 1) {
        File32 datFile;
        file32_def_t fileDef32;

        String datFileNameLast = "";
        String datFilePathLast = "";

        values_all_t readValue;

        for (uint8_t historyIndex = 0; historyIndex < HISTORY_____BUFFER_SIZE; historyIndex++) {
            secondstimeIndx = secondstimeBase + historyIndex * secondstimeIncr;
            history[historyIndex] = emptyMeasurement(secondstimeIndx);
            fileDef32 = SensorTime::getFile32Def(secondstimeIndx, "dat");
            // when a new (or actually first) file is encountered -> open it
            if (fileDef32.name != datFileNameLast) {
                if (datFile) {
                    datFile.close();  // close the previous file
                }
                datFile.open(fileDef32.name.c_str(), O_READ);
                datFileNameLast = fileDef32.name;
            }
            // keep reading from old to young until a date larger than search date - 30s is found
            while (datFile.available() > 1 && secondstimeFile + 30 < secondstimeIndx) {
                datFile.read((byte*)&readValue, sizeof(readValue));  // read the next value into readValue
                secondstimeFile = readValue.secondstime;
            }
            secondstimeDiff = secondstimeFile - secondstimeIndx;
            if (abs(secondstimeDiff) <= 30) {
#ifdef USE___SERIAL
                Serial.print(historyIndex);
                Serial.print(" => ");
                Serial.print(secondstimeDiff);
                Serial.print(" OK ");
                Serial.print(SensorTime::getDateTimeDisplayString(secondstimeIndx));
                Serial.print(" :: ");
                Serial.println(SensorTime::getDateTimeDisplayString(secondstimeFile));
#endif
                historyIndexMax = historyIndex;
                history[historyIndex] = readValue;
            } else {
#ifdef USE___SERIAL
                Serial.print(historyIndex);
                Serial.print(" => ");
                Serial.print(secondstimeDiff);
                Serial.print(" !! ");
                Serial.print(SensorTime::getDateTimeDisplayString(secondstimeIndx));
                Serial.print(" :: ");
                Serial.println(SensorTime::getDateTimeDisplayString(secondstimeFile));
#endif
            }
            if (datFile.available() <= 1) {
                break;
            }
        }
        if (datFile) {
            datFile.close();
        }
    }

#ifdef USE___SERIAL
    Serial.println("------------------------------------------------------");
#endif
    uint8_t measureIndex = currMeasureIndex + 1;
    uint32_t secondstimeMeas = 0;
    for (uint8_t historyIndex = historyIndexMax + 1; historyIndex < HISTORY_____BUFFER_SIZE; historyIndex++) {
        secondstimeIndx = secondstimeBase + historyIndex * secondstimeIncr;
        history[historyIndex] = emptyMeasurement(secondstimeIndx);
        while (measureIndex < currMeasureIndex + 1 + MEASUREMENT_BUFFER_SIZE && secondstimeMeas + 30 < secondstimeIndx) {
            secondstimeMeas = values->measurements[measureIndex % MEASUREMENT_BUFFER_SIZE].secondstime;
            // Serial.print(measureIndex);
            // Serial.print(" ## ");
            // Serial.println(SensorTime::getDateTimeDisplayString(secondstimeMeas));
            measureIndex++;
        }
        secondstimeDiff = secondstimeMeas - secondstimeIndx;
        if (abs(secondstimeDiff) <= 30) {
#ifdef USE___SERIAL
            Serial.print(historyIndex);
            Serial.print(" => ");
            Serial.print(secondstimeDiff);
            Serial.print(" OK ");
            Serial.print(SensorTime::getDateTimeDisplayString(secondstimeIndx));
            Serial.print(" :: ");
            Serial.println(SensorTime::getDateTimeDisplayString(secondstimeMeas));
#endif
            history[historyIndex] = values->measurements[(measureIndex - 1) % MEASUREMENT_BUFFER_SIZE];  // assign (is this a copy or a reference?)
        }
    }
#ifdef USE___SERIAL
    Serial.println("======================================================");
#endif
}

void ModuleSdcard::persistValues(values_t* values) {
    file32_def_t fileDef32;
    String datFileNameLast = "";
    String datFilePathLast = "";
    File32 datFile;
    values_all_t value;
    for (int valueIndex = 0; valueIndex < MEASUREMENT_BUFFER_SIZE; valueIndex++) {
        value = values->measurements[valueIndex];
        fileDef32 = SensorTime::getFile32Def(value.secondstime, "dat");  // the file name that shall be written to
        if (fileDef32.name != datFileNameLast) {
            if (datFile) {       // file already open -> file change at midnight
                datFile.sync();  // write anything pending
                datFile.close();
            }
            if (fileDef32.path != datFileNameLast) {  // if not only the file name changed, but also the path (a change in month or year, the folders need to be ready)
                buildFolders(fileDef32.path);
                datFilePathLast = fileDef32.path;
            }
            datFile.open(fileDef32.name.c_str(), O_RDWR | O_CREAT | O_AT_END);
            datFileNameLast = fileDef32.name;
        }
        datFile.write((byte*)&value, sizeof(value));
    }
    datFile.sync();
    datFile.close();
}

// void ModuleSdcard::persistValues(values_all_t values[MEASUREMENT_BUFFER_SIZE]) {
//     file32_def_t fileDef32;
//     String dataFileNameLast = "";
//     String dataFilePathLast = "";
//     File32 csvFile;
//     values_all_t value;
//     for (int valueIndex = 0; valueIndex < MEASUREMENT_BUFFER_SIZE; valueIndex++) {
//         value = values[valueIndex];
//         fileDef32 = SensorTime::getFile32Def(value.secondstime, "csv");  // the file name that shall be written to
//         if (fileDef32.name != dataFileNameLast) {
//             if (csvFile) {       // file already open -> file change at midnight
//                 csvFile.sync();  // write anything pending
//                 csvFile.close();
//             }
//             if (fileDef32.path != dataFilePathLast) {  // if not only the file name changed, but also the path (a change in month or year, the folders need to be ready)
//                 buildFolders(fileDef32.path);
//                 dataFilePathLast = fileDef32.path;
//             }
//             csvFile.open(fileDef32.name.c_str(), O_RDWR | O_CREAT | O_AT_END);
//             if (csvFile.size() == 0) {  // first time this file is being written to -> write csv header
//                 csvFile.print(ModuleSdcard::CSV_HEAD);
//             }
//             dataFileNameLast = fileDef32.name;
//         }
//         csvFile.print(toCsvLine(&value));
//     }
//     csvFile.sync();
//     csvFile.close();
// }

// String ModuleSdcard::toCsvLine(values_all_t* value) {
//     char csvBuffer[128];
//     DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + value->secondstime);
//     float deg = SensorScd041::toFloatDeg(value->valuesCo2.deg);
//     float hum = SensorScd041::toFloatHum(value->valuesCo2.hum);
//     float percent = SensorEnergy::toFloatPercent(value->valuesNrg.percent);
//     sprintf(csvBuffer, ModuleSdcard::CSV_FRMT.c_str(), date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(), String(value->valuesCo2.co2), String(value->valuesCo2.co2Raw), String(deg, 1), String(hum, 1), String(value->valuesBme.pressure, 2), String(percent, 2));
//     for (int i = 0; i < 128; i++) {
//         if (csvBuffer[i] == '.') {
//             csvBuffer[i] = ',';
//         }
//     }
//     return csvBuffer;
// }

bool ModuleSdcard::buildFolders(String folder) {
    return ModuleSdcard::sd32.mkdir(folder);
}

bool ModuleSdcard::removeFolder(String folder) {
    return ModuleSdcard::sd32.rmdir(folder);
}

bool ModuleSdcard::existsPath(String path) {
    return ModuleSdcard::sd32.exists(path);
}

bool ModuleSdcard::removeFile32(String file) {
    return ModuleSdcard::sd32.remove(file);
}
