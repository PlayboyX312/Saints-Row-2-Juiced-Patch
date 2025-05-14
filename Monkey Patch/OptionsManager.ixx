module;

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "../GameConfig.h"

export module OptionsManager;

export struct Option {
    std::string appName;
    std::string keyName;
    int* valuePtr;

    Option(const std::string& app, const std::string& key, int* val)
        : appName(app), keyName(key), valuePtr(val) {}
};

export class OptionsManager {
private:

    static std::unordered_map<std::string, std::unordered_map<std::string, Option*>>& getOptionsMap() {
        static std::unordered_map<std::string, std::unordered_map<std::string, Option*>> optionsMap;
        return optionsMap;
    }
    OptionsManager() {}

public:
    static void cleanup() {
        auto& optionsMap = getOptionsMap();
        for (auto& appPair : optionsMap) {
            for (auto& keyPair : appPair.second) {
                delete keyPair.second;
            }
        }
        optionsMap.clear();
    }

    // Register option with app and key, and load the initial value from the INI file
    static void registerOption(const std::string& appName, const std::string& keyName, int* valuePtr, int defaultValue = 0) {
        // Read the value from INI file first
        uint32_t iniValue = GameConfig::GetValue(appName.c_str(), keyName.c_str(), defaultValue);

        *valuePtr = iniValue;
        auto& optionsMap = getOptionsMap();
        if (optionsMap.count(appName) && optionsMap[appName].count(keyName)) {
            optionsMap[appName][keyName]->valuePtr = valuePtr;
        }
        else {
            Option* newOption = new Option(appName, keyName, valuePtr);
            optionsMap[appName][keyName] = newOption;
        }
    }

    static bool setOptionValue(const std::string& appName, const std::string& keyName, int value) {
        Option* opt = getOption(appName, keyName);
        if (opt && opt->valuePtr) {

            *(opt->valuePtr) = value;

            // Sync to INI file
            GameConfig::SetValue(appName.c_str(), keyName.c_str(), value);

            return true;
        }
        return false;
    }

    // Set option value by just key name (finds first match), and sync to INI file
    static bool setOptionValue(const std::string& keyName, int value) {
        auto& optionsMap = getOptionsMap();
        for (auto& appPair : optionsMap) {
            if (appPair.second.count(keyName) && appPair.second[keyName]->valuePtr) {
                Option* opt = appPair.second[keyName];
                *(opt->valuePtr) = value;

                // Sync to INI file
                GameConfig::SetValue(opt->appName.c_str(), opt->keyName.c_str(), value);

                return true;
            }
        }
        return false;
    }

    static Option* getOption(const std::string& appName, const std::string& keyName) {
        auto& optionsMap = getOptionsMap();
        if (optionsMap.count(appName) && optionsMap[appName].count(keyName)) {
            return optionsMap[appName][keyName];
        }
        return nullptr;
    }
    static Option* getOption(const std::string& keyName) {
        auto& optionsMap = getOptionsMap();
        for (auto& appPair : optionsMap) {
            if (appPair.second.count(keyName)) {
                return appPair.second[keyName];
            }
        }
        return nullptr;
    }

    // Get the option value with explicit app and key
    static int getOptionValue(const std::string& appName, const std::string& keyName, int defaultValue = 0) {
        Option* opt = getOption(appName, keyName);
        if (opt && opt->valuePtr) {
            return *(opt->valuePtr);
        }
        else {
            // If the option isn't registered yet, read directly from INI
            return GameConfig::GetValue(appName.c_str(), keyName.c_str(), defaultValue);
        }
    }

    // Get option value by just key name (finds first match)
    static int getOptionValue(const std::string& keyName, int defaultValue = 0) {
        Option* opt = getOption(keyName);
        return (opt && opt->valuePtr) ? *(opt->valuePtr) : defaultValue;
    }

    static bool refreshFromIni(const std::string& appName, const std::string& keyName) {
        Option* opt = getOption(appName, keyName);
        if (opt && opt->valuePtr) {
            // Read value from INI and update the pointed variable
            uint32_t iniValue = GameConfig::GetValue(opt->appName.c_str(), opt->keyName.c_str(), *(opt->valuePtr));
            *(opt->valuePtr) = iniValue;
            return true;
        }
        return false;
    }

    static void refreshAllFromIni() {
        auto& optionsMap = getOptionsMap();
        for (auto& appPair : optionsMap) {
            for (auto& keyPair : appPair.second) {
                Option* opt = keyPair.second;
                if (opt && opt->valuePtr) {
                    uint32_t iniValue = GameConfig::GetValue(opt->appName.c_str(), opt->keyName.c_str(), *(opt->valuePtr));
                    *(opt->valuePtr) = iniValue;
                }
            }
        }
    }

    static void saveAllToIni() {
        auto& optionsMap = getOptionsMap();
        for (auto& appPair : optionsMap) {
            for (auto& keyPair : appPair.second) {
                Option* opt = keyPair.second;
                if (opt && opt->valuePtr) {
                    GameConfig::SetValue(opt->appName.c_str(), opt->keyName.c_str(), *(opt->valuePtr));
                }
            }
        }
    }

    static std::vector<Option*> getAllOptionsForApp(const std::string& appName) {
        std::vector<Option*> result;
        auto& optionsMap = getOptionsMap();
        if (optionsMap.count(appName)) {
            result.reserve(optionsMap[appName].size());
            for (auto& keyPair : optionsMap[appName]) {
                result.push_back(keyPair.second);
            }
        }
        return result;
    }


    static std::vector<Option*> getAllOptions() {
        std::vector<Option*> result;
        auto& optionsMap = getOptionsMap();
        for (auto& appPair : optionsMap) {
            for (auto& keyPair : appPair.second) {
                result.push_back(keyPair.second);
            }
        }
        return result;
    }
};