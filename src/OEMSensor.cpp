/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <unistd.h>

#include <OEMSensor.hpp>
#include <Utils.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <limits>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/process.hpp>

static constexpr bool DEBUG = false;

static int executeCmd(std::string exec)
{
    const char *cmd = exec.c_str();
    boost::process::child execProg(cmd);
    execProg.wait();
    return execProg.exit_code();
}

OEMSensor::OEMSensor(boost::asio::io_service& io,
                       std::shared_ptr<sdbusplus::asio::connection>& conn,
                       OEMConfig& sensorconfig) :
    waitTimer(io), mDbusConn(conn), mSnrConfig(sensorconfig)
{
    setupRead();
}

OEMSensor::~OEMSensor()
{
    waitTimer.cancel();
}

void OEMSensor::setupRead(void)
{
    if (DEBUG)
    {
        std::cerr << "enter OEMSensor::setupRead" << "\n";
        std::cerr << "sensor name: " << mSnrConfig.name << "\n";
        std::cerr << "sensor num: " << static_cast<unsigned>(mSnrConfig.snrnum) << "\n";
        std::cerr << "sensor type: " << static_cast<unsigned>(mSnrConfig.snrtype) << "\n";
        std::cerr << "monitor: " << mSnrConfig.monitor << "\n";
        std::cerr << "exec: " << mSnrConfig.exec << "\n";
    }

    std::string monitor = mSnrConfig.monitor;
    std::string exec = mSnrConfig.exec;

    if (monitor == "oneshot")
    {
        // Execute Response
        auto retCode = executeCmd(exec);

        if (0 != retCode)
        {
            std::cerr << "oneshot sensor: " << mSnrConfig.name  << "\n";
            std::cerr << "exec " << exec << " failed !"  << "\n";
        }
    }

    handleResponse();
}

void OEMSensor::handleResponse()
{
    size_t pollTime = OEMSensor::sensorPollMs;
    waitTimer.expires_from_now(boost::posix_time::milliseconds(pollTime));
    waitTimer.async_wait([&](const boost::system::error_code& ec) {
        // case of timer expired
        if (!ec)
        {
            std::string monitor = mSnrConfig.monitor;
            std::string exec = mSnrConfig.exec;
            if (DEBUG)
            {
                std::cerr << "monitor: " << monitor << "\n";
                std::cerr << "exec: " << exec << "\n";
            }

            if (monitor != "oneshot")
            {
                // Execute Response
                auto retCode = executeCmd(exec);

                if (0 != retCode)
                {
                    std::cerr << "polling sensor: " << mSnrConfig.name  << "\n";
                    std::cerr << "exec " << exec << " failed !"  << "\n";
                }
            }

            // trigger next polling
            handleResponse();
        }
        // case of being canceled
        else if (ec == boost::asio::error::operation_aborted)
        {
            std::cerr << "Timer of oem sensor is cancelled. Return \n";
            return;
        }
    });
}
