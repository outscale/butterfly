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
#include <unistd.h>
#include <syslog.h>
#include <glib.h>
#include <iostream>
#include <ctime>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include "api/server/app.h"
#include "api/server/graph.h"
#include "api/server/server.h"
#include "api/server/simpleini/SimpleIni.hpp"
#include "./version.h"

namespace app {
Stats::Stats() {
    start_date = time(nullptr);
    request_counter = 0;
}

Config::Config() {
    api_endpoint = "tcp://0.0.0.0:9999";
    log_level = "error";
    graph_core_id = 0;
    packet_trace = false;
}

bool Config::parse_cmd(int argc, char **argv) {
    bool ret = true;
    int i;
    bool show_revision;
    bool dpdk_help;

    auto gfree = [](gchar *p) { g_free(p); };
    std::unique_ptr<gchar, decltype(gfree)> config_path_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> external_ip_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> api_endpoint_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> log_level_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> pid_path_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> socket_folder_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> graph_core_id_cmd(nullptr, gfree);

    static GOptionEntry entries[] = {
        {"config", 'c', 0, G_OPTION_ARG_FILENAME, &config_path_cmd,
         "Path to configuration file", "FILE"},
        {"ip", 'i', 0, G_OPTION_ARG_STRING, &external_ip_cmd,
         "IP address to use", "IP_ADDRESS"},
        {"endpoint", 'e', 0, G_OPTION_ARG_STRING, &api_endpoint_cmd,
         "API endpoint to bind (default is 'tcp://0.0.0.0:9999')",
         "API_ENDPOINT"},
        {"log-level", 'l', 0, G_OPTION_ARG_STRING, &log_level_cmd,
         "Log level to use. LOG_LEVEL can be 'none', 'error' (default), " \
         "'warning', 'info' or 'debug'", "LOG_LEVEL"},
        {"revision", 'r', 0, G_OPTION_ARG_NONE, &show_revision,
         "Show the protocol revision number and exit", nullptr},
        {"pid", 'p', 0, G_OPTION_ARG_FILENAME, &pid_path_cmd,
         "Write PID of process in specified file", "FILE"},
        {"socket-dir", 's', 0, G_OPTION_ARG_FILENAME, &socket_folder_cmd,
         "Create network sockets in specified directory", "DIR"},
        {"graph-cpu-core", 'u', 0, G_OPTION_ARG_STRING, &graph_core_id_cmd,
         "Choose your CPU core where to run packet processing (default=0)",
         "ID"},
        {"packet-trace", 't', 0, G_OPTION_ARG_NONE, &config.packet_trace,
         "Trace packets going through Butterfly", nullptr},
        {"dpdk-help", 0, 0, G_OPTION_ARG_NONE, &dpdk_help,
         "print DPDK help", nullptr},
        { nullptr }
    };
    GOptionContext *context = g_option_context_new("");
    g_option_context_set_summary(context,
            "butterfly-server [EAL options] -- [butterfly options]");
    g_option_context_set_description(context, "example:\n"
            "butterfly-server -c0xF -n1  --socket-mem 64"
            " -- -i 43.0.0.1 -e tcp://127.0.0.1:8765 -s /tmp");
    g_option_context_add_main_entries(context, entries, nullptr);

    GError *error = nullptr;

    for (i = 0; i < argc; i++) {
        if (g_strcmp0(argv[i], "--") == 0) {
            break;
        }
    }
    if (i != argc) {
        argc -= i;
        argv += i;
    } else {
        ret = false;
    }

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        if (error != nullptr)
            std::cout << error->message << std::endl;
        return false;
    }

    if (dpdk_help) {
        argc = 1;
        argv[1] = const_cast<char *>("-h");

        app::graph.start(argc, argv);
        app::graph.stop();
        return false;
    }

    // Ask for revision number ?
    if (show_revision) {
        std::cout << VERSION_INFO << std::endl;
        return false;
    }

    // Get back gchar to config in std::string
    if (config_path_cmd != nullptr)
        config_path = std::string(&*config_path_cmd);
    if (external_ip_cmd != nullptr)
        external_ip = std::string(&*external_ip_cmd);
    if (api_endpoint_cmd != nullptr)
        api_endpoint = std::string(&*api_endpoint_cmd);
    if (log_level_cmd != nullptr)
        log_level = std::string(&*log_level_cmd);
    if (pid_path_cmd != nullptr)
        pid_path = std::string(&*pid_path_cmd);
    if (socket_folder_cmd != nullptr)
        socket_folder = std::string(&*socket_folder_cmd);
    if (graph_core_id_cmd != nullptr)
        graph_core_id = std::atoi(&*graph_core_id_cmd);
    // Load from configuration file if provided
    if (config_path.length() > 0 && !LoadConfigFile(config_path)) {
        std::cerr << "Failed to open configuration file" << std::endl;
        app::log.error("Failed to open configuration file");
        return false;
    }

    if (!ret) {
        std::cerr << "wrong usage, butterfly-server use -h" << std::endl;
    }
    return ret;
}

bool Config::missing_mandatory() {
    bool ret = false;
    if (external_ip.length() == 0) {
        std::cerr << "IP to use is not set" << std::endl;
        app::log.error("IP to use is not set");
        ret = true;
    }
    if (socket_folder.length() == 0) {
        std::cerr << "socket folder is not set" << std::endl;
        app::log.error("socket folder is not set");
        ret = true;
    }

    if (ret)
        app::log.error("missing mandatory configuration items");
    return ret;
}

Log::Log() {
    // Set default log level
    set_log_level("error");

    // Openlog
    open();
}

Log::~Log() {
    closelog();
}

void Log::open() {
    openlog("butterfly", LOG_CONS | LOG_PID | LOG_NDELAY | LOG_PERROR,
            LOG_LOCAL0);
}

bool Log::set_log_level(std::string level) {
    if (level == "none") {
        setlogmask(0);
    } else if (level == "error") {
        setlogmask(LOG_UPTO(LOG_ERR));
    } else if (level == "warning") {
        setlogmask(LOG_UPTO(LOG_WARNING));
    } else if (level == "info") {
        setlogmask(LOG_UPTO(LOG_INFO));
    } else if (level == "debug") {
        setlogmask(LOG_UPTO(LOG_DEBUG));
    } else {
        error("Log::set_log_level: non existing log level");
        return false;
    }
    return true;
}

#define DEBUG_INTERNAL(TYPE) do {                                       \
        va_list ap;                                                     \
                                                                        \
        va_start(ap, message);                                          \
        vsyslog(LOG_##TYPE, \
            (std::string("<"#TYPE"> ") + message).c_str(), ap);         \
        va_end(ap);                                                     \
    } while (0)

void Log::debug(const char *message, ...) {
    DEBUG_INTERNAL(DEBUG);
}

void Log::info(const char *message, ...) {
    DEBUG_INTERNAL(INFO);
}

void Log::warning(const char *message, ...) {
    DEBUG_INTERNAL(WARNING);
}

void Log::error(const char *message, ...) {
    DEBUG_INTERNAL(ERR);
}

void Log::debug(const std::string &message, ...) {
    DEBUG_INTERNAL(DEBUG);
}

void Log::info(const std::string &message, ...) {
    DEBUG_INTERNAL(INFO);
}

void Log::warning(const std::string &message, ...) {
    DEBUG_INTERNAL(WARNING);
}

void Log::error(const std::string &message, ...) {
    DEBUG_INTERNAL(ERR);
}

#undef DEBUG_INTERNAL

void WritePid(std::string pid_path) {
    std::ofstream f;
    f.open(pid_path, std::ios::out);
    if (!f.good()) {
        log.error("Cannot write to PID file");
        f.close();
        return;
    }
    pid_t pid = getpid();
    f << pid;
    f.close();
}

bool LoadConfigFile(std::string config_path) {
    CSimpleIniA ini;
    ini.SetUnicode();
    if (ini.LoadFile(config_path.c_str()) != SI_OK)
        return false;
    const char *v;

    v = ini.GetValue("general", "log-level", "_");
    if (std::string(v) != "_") {
        config.log_level = v;
        log.set_log_level(config.log_level);
        std::string m = "LoadConfig: get log-level from config: " +
            config.log_level;
        log.debug(m);
    }

    v = ini.GetValue("general", "ip", "_");
    if (std::string(v) != "_") {
        config.external_ip = v;
        std::string m = "LoadConfig: get ip from config: " +
            config.external_ip;
        log.debug(m);
    }

    v = ini.GetValue("general", "endpoint", "_");
    if (std::string(v) != "_") {
        config.api_endpoint = v;
        std::string m = "LoadConfig: get endpoint from config: " +
            config.api_endpoint;
        log.debug(m);
    }

    v = ini.GetValue("general", "pid", "_");
    if (std::string(v) != "_") {
        config.pid_path = v;
        std::string m = "LoadConfig: get pid path from config: " +
            config.pid_path;
        log.debug(m);
    }

    v = ini.GetValue("general", "socket-dir", "_");
    if (std::string(v) != "_") {
        config.socket_folder = v;
        std::string m = "LoadConfig: get socket-dir from config: " +
            config.socket_folder;
        log.debug(m);
    }

    v = ini.GetValue("general", "graph-core-id", "_");
    if (std::string(v) != "_") {
        config.graph_core_id = std::stoi(v);
        std::string m = "LoadConfig: get graph-core-id from config: " +
            config.graph_core_id;
        log.debug(m);
    }
    config.tid = 0;

    return true;
}

void SignalRegister() {
    signal(SIGINT, SignalHandler);
    signal(SIGQUIT, SignalHandler);
    signal(SIGSTOP, SignalHandler);
}

void
SignalHandler(int signum) {
    std::string m = "got signal " + std::to_string(signum);
    app::log.info(m);
    app::request_exit = true;
}

// Global instances in app namespace
bool request_exit(false);
Config config;
Stats stats;
Model model;
Log log;
Graph graph;
}  // namespace app

int initCGroup() {
  system("mkdir /sys/fs/cgroup/cpu/butterfly");
  system("echo 4096 > /sys/fs/cgroup/cpu/butterfly/cpu.shares");
  return 0;
}

void app::setCGroup() {
  if (!app::config.tid)
    return;
  std::string setStr;
  std::string unsetOtherStr;
  std::ostringstream oss;

  oss << app::config.tid;
  setStr = "echo " + oss.str() + " > /sys/fs/cgroup/cpu/butterfly/tasks";
  unsetOtherStr = "grep -v " + oss.str() +
          " /sys/fs/cgroup/cpu/butterfly/tasks |" +
          " while read ligne; do echo $ligne >" +
          " /sys/fs/cgroup/cpu/tasks ; done";

  system(setStr.c_str());
  system(unsetOtherStr.c_str());
}

void app::destroyCGroup() {
  system("cat /sys/fs/cgroup/cpu/butterfly/tasks |"
         " while read ligne; do echo $ligne > /sys/fs/cgroup/cpu/tasks ; done");
  system("rmdir /sys/fs/cgroup/cpu/butterfly");
}

int
main(int argc, char *argv[]) {
    try {
        // Register signals
        app::SignalRegister();

        // Check parameters
        if (!app::config.parse_cmd(argc, argv))
            return 0;

        // Set log level from options
        app::log.set_log_level(app::config.log_level);

        // Ready to start ?
        if (app::config.missing_mandatory()) {
            std::cerr << "Some arguments are missing, please check " \
            "configuration or use --help" << std::endl;
            return 0;
        }

        // Write PID
        if (app::config.pid_path.length() > 0)
            app::WritePid(app::config.pid_path);

        app::log.info("butterfly starts");

        // Prepare & run packetgraph
        if (!app::graph.start(argc, argv)) {
            app::log.error("cannot start packetgraph, exiting");
            app::request_exit = true;
        }
        initCGroup();
        // Prepare & run API server
        APIServer server(app::config.api_endpoint, &app::request_exit);
        server.run_threaded();

        while (!app::request_exit)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (std::exception & e) {
        LOG_ERROR_("%s", e.what());
    }

    // Ask graph to stop
    app::graph.stop();

    app::log.info("butterfly exit");
    return 0;
}
