#pragma once

#include "Utils.hpp"

#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <fstream>
#include <sdbusplus/asio/object_server.hpp>
#include <string>

struct OEMInfo
{
    OEMInfo(const std::string& iface, const std::string& property,
            const std::string& ptype, const std::string& dfvalue) :
        iface(iface), property(property), ptype(ptype), dfvalue(dfvalue) 
    {
    }
    std::string iface;
    std::string property;
    std::string ptype;
    std::string dfvalue;

    bool operator<(const OEMInfo& rhs) const
    {
        return (iface < rhs.iface);
    }
};

struct OEMConfig
{
    OEMConfig(const uint8_t& snrnum, const uint8_t& snrtype, const std::string& name, 
              const std::string& monitor, const std::string& exec, 
              const std::vector<OEMInfo>& oeminfo) :
        snrnum(snrnum), snrtype(snrtype), name(name), monitor(monitor), exec(exec), oeminfo(std::move(oeminfo))
    {
    }
    uint8_t snrnum;
    uint8_t snrtype;
    std::string name;
    std::string monitor;
    std::string exec;
    std::vector<OEMInfo> oeminfo;

    bool operator<(const OEMConfig& rhs) const
    {
        return (name < rhs.name);
    }
};

class OEMSensor
{
  public:
    OEMSensor(boost::asio::io_service& io,
               std::shared_ptr<sdbusplus::asio::connection>& conn,
               OEMConfig & sensorconfig);
    ~OEMSensor();

    static constexpr unsigned int sensorPollMs = 2000;

  private:
    boost::asio::deadline_timer waitTimer;
    std::shared_ptr<sdbusplus::asio::connection> mDbusConn;
    OEMConfig mSnrConfig;

    void setupRead(void);
    void handleResponse(void);
};

extern boost::container::flat_map<std::string, std::unique_ptr<OEMSensor>>
    gOemSensors;

struct CmpStr
{
    bool operator()(const char* a, const char* b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

const static boost::container::flat_map<const char*, char*, CmpStr>
    sensorTypeToString{{{"12", "/memory"},
                        {"19", "/criticalinterrupt"},
                        {"15", "/posterror"},
                        {"16", "/eventloggingdisable"},
                        {"7", "/processor"},
                        {"40", "/utilization"}, /*managementsubsystemhealth*/
                        {"111", "/oem"} /*managementsubsystemhealth*/
                      }};
