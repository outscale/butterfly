/* Copyright 2015 Outscale SAS
 *
 * This file is part of Butterfly.
 *
 * Butterfly is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Butterfly is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Butterfly.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef API_SERVER_APP_H_
#define API_SERVER_APP_H_

#include <string>
#include "api/server/model.h"
#include "api/server/graph.h"

// Usefull macros
#define LOG_PRINT_(str, method, args...) \
    (method("(%s, %s, %d): " str, __FILE__, \
            __PRETTY_FUNCTION__, __LINE__, ## args))
#define LOG_DEBUG_(str, args...) LOG_PRINT_(str, app::log.debug, ## args)
#define LOG_INFO_(str, args...) LOG_PRINT_(str, app::log.info, ## args)
#define LOG_WARNING_(str, args...) LOG_PRINT_(str, app::log.warning, ## args)
#define LOG_ERROR_(str, args...) LOG_PRINT_(str, app::log.error, ## args)

namespace app {
struct Config {
    Config();
    bool parse_cmd(int argc, char **argv);
    bool missing_mandatory();
    std::string external_ip;
    std::string config_path;
    std::string pid_path;
    std::string api_endpoint;
    std::string log_level;
    std::string socket_folder;
    int graph_core_id;
    bool packet_trace;
    int tid;
};

struct Stats {
    Stats();
    uint64_t start_date;
    uint64_t request_counter;
};

class Log {
 public:
    Log();
    ~Log();
    bool set_log_level(std::string level);
    void debug(const char *message, ...);
    void debug(const std::string &message, ...);
    void info(const char *message, ...);
    void info(const std::string &message, ...);
    void warning(const char *message, ...);
    void warning(const std::string &message, ...);
    void error(const char *message, ...);
    void error(const std::string &message, ...);
    static void open();

 private:
    std::string build_details(const char *message, const char *file,
                              const char *func, int line);
};

// Manage signals
void SignalRegister();
void SignalHandler(int signum);

// Manage configuration file
bool LoadConfigFile(std::string config_path);

void destroyCGroup();
void setCGroup();

// Manage PID
void WritePid(std::string pid_path);

// Some global app:: variables
extern bool request_exit;
extern Config config;
extern Stats stats;
extern Model model;
extern Log log;
extern Graph graph;
}  // namespace app

#endif  // API_SERVER_APP_H_
