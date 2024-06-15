#include "ModuleCard.h"

void ModuleCard::begin() {
    LittleFS.begin(true);
}

bool ModuleCard::removeFile(String file) {
    return LittleFS.remove(file);
}

bool ModuleCard::removeFolder(String folder) {
    return LittleFS.remove(folder);
}

bool ModuleCard::buildFolders(String folder) {
    return LittleFS.mkdir(folder);
}

bool ModuleCard::existsPath(String path) {
    return LittleFS.exists(path);
}
