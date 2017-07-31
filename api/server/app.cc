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
extern "C" {
#include <unistd.h>
#include <syslog.h>
#include <glib.h>
#include <packetgraph/packetgraph.h>
}
#include <iostream>
#include <ctime>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>
#include "api/server/app.h"
#include "api/server/graph.h"
#include "api/server/server.h"
#include "api/server/simpleini/SimpleIni.hpp"
#include "api/version.h"
#include "api/common/crypto.h"

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
    dpdk_args = DPDK_DEFAULT_ARGS;
    tid = 0;
    nic_mtu = "";
    dpdk_port = 0;
    no_offload = 0;
}

bool Config::parse_cmd(int argc, char **argv) {
    bool ret = true;
    bool showversion;
    bool dpdkhelp;

    auto gfree = [](gchar *p) { g_free(p); };
    std::unique_ptr<gchar, decltype(gfree)> config_path_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> external_ip_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> api_endpoint_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> log_level_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> socket_folder_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> graph_core_id_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> dpdk_args_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> nic_mtu_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> dpdk_port_cmd(nullptr, gfree);
    std::unique_ptr<gchar, decltype(gfree)> key_path_cmd(nullptr, gfree);

    static GOptionEntry entries[] = {
        {"config", 'c', 0, G_OPTION_ARG_FILENAME, &config_path_cmd,
         "Path to configuration file", "FILE"},
        {"ip", 'i', 0, G_OPTION_ARG_STRING, &external_ip_cmd,
         "IP address to use on VXLAN endpoint", "IP_ADDRESS"},
        {"endpoint", 'e', 0, G_OPTION_ARG_STRING, &api_endpoint_cmd,
         "API endpoint to bind (default is 'tcp://0.0.0.0:9999')",
         "API_ENDPOINT"},
        {"log-level", 'l', 0, G_OPTION_ARG_STRING, &log_level_cmd,
         "Log level to use. LOG_LEVEL can be 'none', 'error' (default), " \
         "'warning', 'info' or 'debug'", "LOG_LEVEL"},
        {"version", 'V', 0, G_OPTION_ARG_NONE, &showversion,
         "Show butterflyd version and exit", nullptr},
        {"socket-dir", 's', 0, G_OPTION_ARG_FILENAME, &socket_folder_cmd,
         "Create network sockets in specified directory", "DIR"},
        {"graph-cpu-core", 'u', 0, G_OPTION_ARG_STRING, &graph_core_id_cmd,
         "Choose your CPU core where to run packet processing (default=0)",
         "ID"},
        {"packet-trace", 't', 0, G_OPTION_ARG_NONE, &config.packet_trace,
         "Trace packets going through Butterfly", nullptr},
        {"dpdk-help", 0, 0, G_OPTION_ARG_NONE, &dpdkhelp,
         "print DPDK help", nullptr},
        {"dpdk-args", 0, 0, G_OPTION_ARG_STRING, &dpdk_args_cmd,
         "set dpdk arguments (default='" DPDK_DEFAULT_ARGS "'", nullptr},
        {"nic-mtu", 'm', 0, G_OPTION_ARG_STRING, &nic_mtu_cmd,
         "set MTU your physical NIC, may fail if not supported. Parameter can"
         " be set to 'max' and butterfly will try to find the maximal MTU.",
         "MTU"},
        {"no-offload", 0, 0, G_OPTION_ARG_NONE, &config.no_offload,
         "block all offloadind features", nullptr},
        {"dpdk-port", 0, 0, G_OPTION_ARG_STRING, &dpdk_port_cmd,
         "choose which dpdk port to use (default=0)", "PORT"},
        {"key", 'k', 0, G_OPTION_ARG_STRING, &key_path_cmd,
         "path to encryption key (raw randomized 32B)", "PATH"},
        { nullptr }
    };
    std::shared_ptr<GOptionContext> context(g_option_context_new(""),
                                            g_option_context_free);

    g_option_context_set_summary(context.get(),
            "butterflyd [OPTIONS]");
    g_option_context_set_description(context.get(), "example:\n"
            "butterflyd --dpdk-args \"-c0xF -n1 --socket-mem 64\" "
            "-i 43.0.0.1 -e tcp://127.0.0.1:8765 -s /tmp");
    g_option_context_add_main_entries(context.get(), entries, nullptr);

    GError *error = nullptr;

    if (!g_option_context_parse(context.get(), &argc, &argv, &error)) {
        if (error != nullptr)
            std::cout << error->message << std::endl;
        return false;
    }

    if (dpdkhelp) {
        dpdk_args = "--help";
        app::graph.Start(dpdk_args);
        app::graph.Stop();
        return false;
    }

    // Ask for version number ?
    if (showversion) {
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
    if (socket_folder_cmd != nullptr)
        socket_folder = std::string(&*socket_folder_cmd);
    if (graph_core_id_cmd != nullptr)
        graph_core_id = std::atoi(&*graph_core_id_cmd);
    if (dpdk_args_cmd != nullptr)
        dpdk_args = std::string(&*dpdk_args_cmd);
    if (nic_mtu_cmd != nullptr)
        nic_mtu = std::string(&*nic_mtu_cmd);
    if (dpdk_port_cmd != nullptr)
        nic_mtu = std::atoi(&*dpdk_port_cmd);
    if (key_path_cmd != nullptr)
        encryption_key_path = std::string(&*key_path_cmd);

    // Load from configuration file if provided
    if (config_path.length() > 0 && !LoadConfigFile(config_path)) {
        std::cerr << "Failed to open configuration file" << std::endl;
        app::log.Error("Failed to open configuration file");
        return false;
    }

    // Load encryption key from file if provided
    if (encryption_key_path.length()) {
        if (Crypto::KeyFromPath(encryption_key_path, &encryption_key)) {
            std::cerr << "Encryption key loaded" << std::endl;
            app::log.Debug("Encryption key loaded");
        } else {
            std::cerr << "Cannot load encryption key" << std::endl;
            app::log.Error("Failed to open encryption key");
            return false;
        }
    } else {
        std::cerr << "No encryption configured" << std::endl;
        app::log.Warning("No encryption configured");
    }

    if (!ret) {
        std::cerr << "wrong usage, butterflyd use -h" << std::endl;
    }
    return ret;
}

bool Config::MissingMandatory() {
    bool ret = false;
    if (external_ip.length() == 0) {
        std::cerr << "IP to use is not set" << std::endl;
        app::log.Error("IP to use is not set");
        ret = true;
    }
    if (socket_folder.length() == 0) {
        std::cerr << "socket folder is not set" << std::endl;
        app::log.Error("socket folder is not set");
        ret = true;
    }

    if (ret)
        app::log.Error("missing mandatory configuration items");
    return ret;
}

Log::Log() {
    // Set default log level
    SetLogLevel("error");

    // Openlog
    Open();
}

Log::~Log() {
    closelog();
}

void Log::Open() {
    openlog("butterfly", LOG_CONS | LOG_PID | LOG_NDELAY | LOG_PERROR,
            LOG_LOCAL0);
}

bool Log::SetLogLevel(std::string level) {
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
        Error("Log::SetLogLevel: non existing log level");
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

void Log::Debug(const char *message, ...) {
    DEBUG_INTERNAL(DEBUG);
}

void Log::Info(const char *message, ...) {
    DEBUG_INTERNAL(INFO);
}

void Log::Warning(const char *message, ...) {
    DEBUG_INTERNAL(WARNING);
}

void Log::Error(const char *message, ...) {
    DEBUG_INTERNAL(ERR);
}

void Log::Debug(const std::string &message, ...) {
    DEBUG_INTERNAL(DEBUG);
}

void Log::Info(const std::string &message, ...) {
    DEBUG_INTERNAL(INFO);
}

void Log::Warning(const std::string &message, ...) {
    DEBUG_INTERNAL(WARNING);
}

void Log::Error(const std::string &message, ...) {
    DEBUG_INTERNAL(ERR);
}

#undef DEBUG_INTERNAL

bool LoadConfigFile(std::string config_path) {
    CSimpleIniA ini;
    ini.SetUnicode();
    if (ini.LoadFile(config_path.c_str()) != SI_OK)
        return false;
    const char *v;

    v = ini.GetValue("general", "log-level", "_");
    if (std::string(v) != "_") {
        config.log_level = v;
        log.SetLogLevel(config.log_level);
        std::string m = "LoadConfig: get log-level from config: " +
            config.log_level;
        log.Debug(m);
    }

    v = ini.GetValue("general", "ip", "_");
    if (std::string(v) != "_") {
        config.external_ip = v;
        std::string m = "LoadConfig: get ip from config: " +
            config.external_ip;
        log.Debug(m);
    }

    v = ini.GetValue("general", "endpoint", "_");
    if (std::string(v) != "_") {
        config.api_endpoint = v;
        std::string m = "LoadConfig: get endpoint from config: " +
            config.api_endpoint;
        log.Debug(m);
    }

    v = ini.GetValue("general", "socket-dir", "_");
    if (std::string(v) != "_") {
        config.socket_folder = v;
        std::string m = "LoadConfig: get socket-dir from config: " +
            config.socket_folder;
        log.Debug(m);
    }

    v = ini.GetValue("general", "graph-core-id", "_");
    if (std::string(v) != "_") {
        config.graph_core_id = std::stoi(v);
        std::string m = "LoadConfig: get graph-core-id from config: " +
            config.graph_core_id;
        log.Debug(m);
    }

    v = ini.GetValue("general", "dpdk-args", "_");
    if (std::string(v) != "_") {
        config.dpdk_args = v;
        std::string m = "LoadConfig: get dpdk-args from config: " +
            config.dpdk_args;
        log.Debug(m);
    }

    v = ini.GetValue("general", "nic-mtu", "_");
    if (std::string(v) != "_") {
        config.nic_mtu = v;
        std::string m = "LoadConfig: get nic_mtu from config: " +
            config.nic_mtu;
        log.Debug(m);
    }

    v = ini.GetValue("general", "dpdk-port", "_");
    if (std::string(v) != "_") {
        config.dpdk_port = std::stoi(v);
        std::string m = "LoadConfig: get dpdk-port from config: " +
            config.dpdk_port;
        log.Debug(m);
    }

    v = ini.GetValue("security", "encryption_key_path", "_");
    if (std::string(v) != "_") {
        config.encryption_key_path = v;
        log.Debug("LoadConfig: get encryption key path from config");
    }

    return true;
}

void SignalRegister() {
    signal(SIGINT, SignalHandler);
    signal(SIGQUIT, SignalHandler);
    signal(SIGSTOP, SignalHandler);
}

void SignalHandler(int signum) {
    std::string m = "got signal " + std::to_string(signum);
    app::log.Info(m);
    app::request_exit = true;
}

std::string GraphDot(struct pg_brick *brick) {
    char buf[10000];
    FILE *fd = fmemopen(buf, 10000, "w+");
    if (pg_brick_dot(brick, fd, &pg_error) < 0) {
        PG_ERROR_(pg_error);
        return std::string("");
    }
    fflush(fd);
    std::string ret(buf);
    fclose(fd);
    return ret;
}

bool PgStart(std::string dpdk_args) {
    pg_npf_nworkers = 0;
    log.Debug(dpdk_args);
    return pg_start_str(dpdk_args.c_str()) >= 0;
}

// Global instances in app namespace
bool request_exit(false);
Config config;
Stats stats;
Model model;
Log log;
Graph graph;
struct pg_error *pg_error;

}  // namespace app

#define BASH(STR) if (system(("/bin/bash -c \"" + (STR) + "\"").c_str()))

static const char *SrcCgroup() {
    if (!access("/sys/fs/cgroup/cpu/cpu.shares", R_OK)) {
        return "/sys/fs/cgroup/cpu";
    } else if (!access("/sys/fs/cgroup/cpu.shares", R_OK)) {
        return "/sys/fs/cgroup";
    }
    return NULL;
}

static int InitCgroup(int multiplier) {
    if (!SrcCgroup())
        return -1;
    std::string create_dir("mkdir " + std::string(SrcCgroup()) + "/butterfly");

    BASH(create_dir) {
        LOG_WARNING_("can't create butterfly cgroup, fail cmd '%s'",
                     create_dir.c_str());
    }
    BASH("echo $(( `cat " + std::string(SrcCgroup()) + "/cpu.shares` * " +
         std::to_string(multiplier) + " )) > " +
         SrcCgroup() + "/butterfly/cpu.shares") {
        LOG_WARNING_("can't set cgroup priority");
    }
    return 0;
}

void app::SetCgroup() {
    if (!SrcCgroup() || !app::config.tid)
        return;
    std::string setStr;
    std::string unsetOtherStr;
    std::ostringstream oss;

    oss << app::config.tid;
    setStr = "echo " + oss.str() + " > " + SrcCgroup() + "/butterfly/tasks";
    unsetOtherStr = "grep -v " + oss.str() + " " + SrcCgroup() +
                    "/butterfly/tasks | while read ligne; do echo $ligne > " +
                    SrcCgroup() + "/tasks ; done";

    BASH(setStr) {
        LOG_WARNING_("can't set cgroup pid");
    }
    BASH(unsetOtherStr) {
        LOG_WARNING_("can't properly set cgroup pid");
    }
}

void app::DestroyCgroup() {
    if (!SrcCgroup())
        return;
    BASH("cat " + std::string(SrcCgroup()) +
         "/butterfly/tasks | while read ligne; do echo $ligne > " +
         SrcCgroup() + "/bin/bash /tasks ; done") {
        LOG_WARNING_("can't unset task from butterfly cgroup");
    }
    BASH("rmdir " + std::string(SrcCgroup()) + "/butterfly") {
        LOG_WARNING_("can't destroy cgroup");
    }
}

#undef BASH

int
main(int argc, char *argv[]) {
    try {
        // Register signals
        app::SignalRegister();

        // Check parameters
        if (!app::config.parse_cmd(argc, argv))
            return 0;

        // Set log level from options
        app::log.SetLogLevel(app::config.log_level);

        // Ready to start ?
        if (app::config.MissingMandatory()) {
            std::cerr << "Some arguments are missing, please check " \
            "configuration or use --help" << std::endl;
            return 0;
        }

        app::log.Info("butterfly starts");

        // Prepare & run packetgraph
        if (!app::graph.Start(app::config.dpdk_args)) {
            app::log.Error("cannot start packetgraph, exiting");
            app::request_exit = true;
        }
        InitCgroup(POLL_THREAD_MULTIPLIER);
        // Prepare & run API server
        ApiServer server(app::config.api_endpoint, &app::request_exit);
        server.RunThreaded();

        while (!app::request_exit)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (std::exception & e) {
        LOG_ERROR_("%s", e.what());
    }

    // Ask graph to stop
    app::graph.Stop();

    app::log.Info("butterfly exit");
    return 0;
}
