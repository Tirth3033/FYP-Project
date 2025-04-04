#define ll long double
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <numeric> // For accumulate
using namespace std;

struct Packet {
    string sender_ip;
    int packet_size;
    ll send_time;
    ll recv_time;
    ll start_time;
};

struct Statistics {
    ll total_packets = 0;
    ll total_expected_packets = 0;
    ll total_bytes = 0;
    ll total_time = 0;

    ll total_latency = 0;
    ll min_latency = 1e17;
    ll max_latency = 0;
    ll avg_latency = 0;

    ll total_jitter = 0;
    ll max_jitter = 0;
    ll avg_jitter = 0;

    ll packet_loss_rate = 0;

    ll throughput = 0;
};

map<string, Statistics> stats;

void read_log_file(const string &filename, vector<vector<char>> &log) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    while (getline(file, line)) {
        if (line.rfind("[RECV]", 0) == 0) {
            vector<char> log_line(line.begin(), line.end());
            log.push_back(log_line);
        }
        if (line.rfind("[PACKET_CNT]", 0) == 0) {
            size_t ip_start = line.find("IP_") + 3;
            size_t ip_end = line.find("_PACKET_CNT_", ip_start);
            string ip_address = line.substr(ip_start, ip_end - ip_start);

            size_t cnt_start = line.find("_PACKET_CNT_") + 12;
            ll packet_cnt = stoll(line.substr(cnt_start));

            stats[ip_address].total_expected_packets = packet_cnt;
        }
    }

    file.close();
}

Packet parse_log_entry(const vector<char> &log_line) {
    string line(log_line.begin(), log_line.end());
    Packet entry;

    size_t pos = 0;
    size_t end_pos = 0;

    // Extract sender_ip
    pos = line.find("SENTIP_") + 7;
    end_pos = pos;
    while(line[end_pos] != '_') end_pos++;
    entry.sender_ip = line.substr(pos, end_pos - pos);

    // Extract packet_size
    pos = line.find("SIZE_", end_pos) + 5;
    end_pos = line.find("B_", pos);
    entry.packet_size = stoi(line.substr(pos, end_pos - pos));

    // Extract send_time
    pos = line.find("SENDTIME_", end_pos) + 9;
    end_pos = line.find("_", pos);
    string send_time_str = line.substr(pos, end_pos - pos);
    size_t dot_pos = send_time_str.find('.');
    ll seconds = stold(send_time_str.substr(0, dot_pos));
    ll microseconds = stold(send_time_str.substr(dot_pos + 1)) / 1000.0;
    entry.send_time = seconds * 1000000.0 + microseconds;

    // Extract start_time
    pos = line.find("TIMESTART_", end_pos) + 10;
    end_pos = line.find("_", pos);
    string start_time_str = line.substr(pos, end_pos - pos);
    dot_pos = start_time_str.find('.');
    seconds = stold(start_time_str.substr(0, dot_pos));
    microseconds = stold(start_time_str.substr(dot_pos + 1)) / 1000.0;
    entry.start_time = seconds * 1000000.0 + microseconds;

    // Extract recv_time
    pos = line.find("RECVTIME_", end_pos) + 9;
    end_pos = line.find("_", pos);
    string recv_time_str = line.substr(pos, end_pos - pos);
    dot_pos = recv_time_str.find('.');
    seconds = stold(recv_time_str.substr(0, dot_pos));
    microseconds = stold(recv_time_str.substr(dot_pos + 1)) / 1000.0;
    entry.recv_time = seconds * 1000000.0 + microseconds;

    return entry;
}

ll max(ll a, ll b) {
    return a > b ? a : b;
}

ll min(ll a, ll b) {
    return a < b ? a : b;
}

ll calculate_latency(const Packet &packet) {
    return (packet.recv_time - packet.send_time) / 1000.0; // Convert to milliseconds
}

double calculate_jains_index(const map<string, Statistics>& stats) {
    vector<double> throughputs;
    
    // Collect all throughputs except the combined one (0.0.0.0)
    for (const auto& [ip, stat] : stats) {
        if (ip != "0.0.0.0" && stat.throughput > 0) {
            throughputs.push_back(stat.throughput);
        }
    }
    
    if (throughputs.empty()) {
        return 0.0;
    }
    
    double sum = accumulate(throughputs.begin(), throughputs.end(), 0.0);
    double sum_of_squares = 0.0;
    for (double t : throughputs) {
        sum_of_squares += t * t;
    }
    
    return (sum * sum) / (throughputs.size() * sum_of_squares);
}

void calculate_stats(const vector<Packet> &packets) {
    for (const auto &packet : packets) {
        // Latency calculation
        ll latency = calculate_latency(packet);
        stats[packet.sender_ip].total_time += (latency / 1000.0);
        stats[packet.sender_ip].total_packets++;
        stats[packet.sender_ip].total_bytes += packet.packet_size;
        stats[packet.sender_ip].total_latency += latency;
        stats[packet.sender_ip].min_latency = min(stats[packet.sender_ip].min_latency, latency);
        stats[packet.sender_ip].max_latency = max(stats[packet.sender_ip].max_latency, latency);
        stats[packet.sender_ip].avg_latency = stats[packet.sender_ip].total_latency / stats[packet.sender_ip].total_packets;

        stats["0.0.0.0"].total_time += (latency / 1000.0);
        stats["0.0.0.0"].total_packets++;
        stats["0.0.0.0"].total_bytes += packet.packet_size;
        stats["0.0.0.0"].total_latency += latency;
        stats["0.0.0.0"].min_latency = min(stats["0.0.0.0"].min_latency, latency);
        stats["0.0.0.0"].max_latency = max(stats["0.0.0.0"].max_latency, latency);
        stats["0.0.0.0"].avg_latency = stats["0.0.0.0"].total_latency / stats["0.0.0.0"].total_packets;

        // Jitter calculation
        ll jitter = abs(latency - stats[packet.sender_ip].avg_latency);
        stats[packet.sender_ip].total_jitter += jitter;
        stats[packet.sender_ip].max_jitter = max(stats[packet.sender_ip].max_jitter, jitter);
        stats[packet.sender_ip].avg_jitter = stats[packet.sender_ip].total_jitter / stats[packet.sender_ip].total_packets;

        stats["0.0.0.0"].total_jitter += jitter;
        stats["0.0.0.0"].max_jitter = max(stats["0.0.0.0"].max_jitter, jitter);
        stats["0.0.0.0"].avg_jitter = stats["0.0.0.0"].total_jitter / stats["0.0.0.0"].total_packets;
    }
    // Packet Loss rate
    for(auto &stat : stats) {
        if(stat.first == "0.0.0.0") continue;
        if(stat.second.total_expected_packets > 0) {
            stat.second.packet_loss_rate = (1 - (stat.second.total_packets / stat.second.total_expected_packets)) * 100;
            stats["0.0.0.0"].total_expected_packets += stat.second.total_expected_packets;
        }
    }
    if(stats["0.0.0.0"].total_expected_packets > 0) {
        stats["0.0.0.0"].packet_loss_rate = (1 - (stats["0.0.0.0"].total_packets / stats["0.0.0.0"].total_expected_packets)) * 100;
    }
    
    // Throughput calculation
    for(auto &stat : stats) {
        if(stat.second.total_time > 0) {
            stat.second.throughput = stat.second.total_bytes / stat.second.total_time;
        }
    }
}

void write_stats() {
    // Delete existing files before recreating them
    for (const auto& stat : stats) {
        if (stat.first == "0.0.0.0") continue; // Skip the combined stats for now
        string filename = "log_" + stat.first + ".txt";
        remove(filename.c_str());
    }
    remove("combined_log.txt");

    // Open log files dynamically based on IP addresses
    map<string, ofstream> log_files;
    for (const auto& stat : stats) {
        if (stat.first == "0.0.0.0") continue; // Skip the combined stats for now
        string filename = "log_" + stat.first + ".txt";
        log_files[stat.first].open(filename);
    }
    ofstream combined_log_file("combined_log.txt");

    // Calculate Jain's Fairness Index
    double jains_index = calculate_jains_index(stats);

    // Helper function to write stats to a file
    auto write_stat = [](ofstream& file, const Statistics& stat, const string& ip) {
        file << fixed << setprecision(6);
        file << "Statistics for IP: " << ip << endl;
        file << "Total Packets: " << (long long)stat.total_packets << endl;
        file << "Total Bytes: " << (long long)stat.total_bytes << endl;
        file << "Latency (ms):" << endl;
        file << "  Min: " << stat.min_latency << endl;
        file << "  Avg: " << stat.avg_latency << endl;
        file << "  Max: " << stat.max_latency << endl;
        file << "Jitter (ms):" << endl;
        file << "  Avg: " << stat.avg_jitter << endl;
        file << "  Max: " << stat.max_jitter << endl;
        file << "Packet Loss Rate (%): " << stat.packet_loss_rate << "%" << endl;
        file << "Throughput (bytes/s): " << stat.throughput << endl;
    };

    // Write stats for each IP
    for (const auto& stat : stats) {
        if (stat.first == "0.0.0.0") continue; // Skip the combined stats for now
        write_stat(log_files[stat.first], stat.second, stat.first);
    }
    
    // Write combined stats with Jain's index
    combined_log_file << fixed << setprecision(6);
    write_stat(combined_log_file, stats["0.0.0.0"], "All IPs (Combined)");
    combined_log_file << "Jain's Fairness Index: " << jains_index << endl;

    // Close all log files
    for (auto& log_file : log_files) {
        log_file.second.close();
    }
    combined_log_file.close();
}

int main() {
    vector<vector<char>> log;
    read_log_file("receiver_log.txt", log);

    vector<Packet> log_entries;
    for(auto line : log) {
        log_entries.push_back(parse_log_entry(line));
    }

    calculate_stats(log_entries);
    write_stats();

    return 0;
}