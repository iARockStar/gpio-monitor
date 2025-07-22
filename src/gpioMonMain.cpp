/**
 * Copyright © 2019 Facebook
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gpioMon.hpp"

#include <CLI/CLI.hpp>
#include <boost/asio/io_context.hpp>
#include <nlohmann/json.hpp>
#include <phosphor-logging/lg2.hpp>

#include <fstream>

namespace phosphor
{
namespace gpio
{

std::map<std::string, int> polarityMap = {
    {"FALLING", GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE},
    {"RISING", GPIOD_LINE_REQUEST_EVENT_RISING_EDGE},
    {"BOTH", GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES}};

}
} // namespace phosphor

int main(int argc, char** argv)
{
    boost::asio::io_context io;

    CLI::App app{"Monitor GPIO line for requested state change"};

    std::string gpioFileName;

    app.add_option("-c,--config", gpioFileName, "Name of config json file")
        ->required()
        ->check(CLI::ExistingFile);

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::Error& e)
    {
        return app.exit(e);
    }

    std::ifstream file(gpioFileName);
    if (!file)
    {
        lg2::error("GPIO monitor config file not found: {FILE}", "FILE",
                   gpioFileName);
        return -1;
    }

    nlohmann::json gpioMonObj;
    file >> gpioMonObj;
    file.close();

    std::vector<std::unique_ptr<phosphor::gpio::GpioMonitor>> gpios;

    for (auto& obj : gpioMonObj)
    {
        std::string lineMsg = "GPIO Line ";
        gpiod_line* line = NULL;
        std::string errMsg;
        struct gpiod_line_request_config config{
            "gpio_monitor", GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES, 0};
        bool flag = false;
        std::string target;
        std::string ledTargetName; // Add variable to store the LED target name
        std::map<std::string, std::vector<std::string>> targets;

        if (obj.find("LineName") == obj.end())
        {
            if (obj.find("GpioNum") == obj.end() ||
                obj.find("ChipId") == obj.end())
            {
                lg2::error("Failed to find line name or gpio number: {FILE}",
                           "FILE", gpioFileName);
                return -1;
            }

            std::string chipIdStr = obj["ChipId"];
            int gpioNum = obj["GpioNum"];

            lineMsg += std::to_string(gpioNum);
            line = gpiod_line_get(chipIdStr.c_str(), gpioNum);
        }
        else
        {
            std::string lineName = obj["LineName"];
            lineMsg += lineName;
            line = gpiod_line_find(lineName.c_str());
        }

        if (line == NULL)
        {
            lg2::error("Failed to find the {GPIO}", "GPIO", errMsg);
            return -1;
        }

        if (obj.find("EventMon") != obj.end())
        {
            std::string eventStr = obj["EventMon"];
            auto findEvent = phosphor::gpio::polarityMap.find(eventStr);
            if (findEvent == phosphor::gpio::polarityMap.end())
            {
                lg2::error("{GPIO}: event missing: {EVENT}", "GPIO", lineMsg,
                           "EVENT", eventStr);
                return -1;
            }

            config.request_type = findEvent->second;
        }

        if (obj.find("Continue") != obj.end())
        {
            flag = obj["Continue"];
        }

        // Parse out ledTargetName if it exists
        if (obj.find("ledTargetName") != obj.end())
        {
            ledTargetName = obj["ledTargetName"];
        }

        gpios.push_back(std::make_unique<phosphor::gpio::GpioMonitor>(
            line, config, io, lineMsg, flag, ledTargetName));
    }
    io.run();

    return 0;
}


