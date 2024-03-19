#include "ModuleSdcard.h"

#define SD_CS 5  // SD card chip select
#define SPI_CLOCK SD_SCK_MHZ(10)

String ModuleSdcard::CSV_HEAD = "time; co2; co2_raw; temperature; humidity; pressure; percent\r\n";
String ModuleSdcard::CSV_FRMT = "%04d-%02d-%02d %02d:%02d:%02d;%s;%s;%s;%s;%s;%s\r\n";

void ModuleSdcard::begin() {
    sd32.begin(SD_CS, SPI_CLOCK);
}

void ModuleSdcard::persistValues(values_all_t* values, int count) {
    file32_def_t fileDef32;
    String dataFileNameLast = "";
    String dataFilePathLast = "";
    File32 csvFile;
    values_all_t value;
    for (int valueIndex = 0; valueIndex < count; valueIndex++) {
        value = values[valueIndex];
        fileDef32 = ModuleTicker::getFile32Def(value.secondstime);  // the file name that shall be written to
        if (fileDef32.name != dataFileNameLast) {
            if (csvFile) {       // file already open -> file change at midnight
                csvFile.sync();  // write anything pending
                csvFile.close();
            }
            if (fileDef32.path != dataFilePathLast) {  // if not only the file name changed, but also the path (a change in month or year, the folders need to be ready)
                buildFolders(fileDef32.path);
                dataFilePathLast = fileDef32.path;
            }
            csvFile.open(fileDef32.name.c_str(), O_RDWR | O_CREAT | O_AT_END);
            if (csvFile.size() == 0) {  // first time this file is being written to -> write csv header
                csvFile.print(ModuleSdcard::CSV_HEAD);
            }
            dataFileNameLast = fileDef32.name;
        }
        csvFile.print(toCsvLine(&value));
    }

    csvFile.sync();
    csvFile.close();
}

String ModuleSdcard::toCsvLine(values_all_t* value) {
    char csvBuffer[128];
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + value->secondstime);
    float deg = SensorScd041::toFloatDeg(value->valuesCo2.deg);
    float hum = SensorScd041::toFloatHum(value->valuesCo2.hum);
    float percent = SensorEnergy::toFloatPercent(value->valuesNrg.percent);
    sprintf(csvBuffer, ModuleSdcard::CSV_FRMT.c_str(), date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(), String(value->valuesCo2.co2), String(value->valuesCo2.co2Raw), String(deg, 1), String(hum, 1), String(value->valuesBme.pressure, 2), String(percent, 2));
    for (int i = 0; i < 128; i++) {
        if (csvBuffer[i] == '.') {
            csvBuffer[i] = ',';
        }
    }
    return csvBuffer;
}

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
