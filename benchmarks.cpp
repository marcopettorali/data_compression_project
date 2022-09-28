#define NAT_LANG_OPT false;

#include <string.h>

#include <chrono>
#include <iostream>
#include <vector>

#include "sdc.hpp"
#include "dac.hpp"
#include "file_scanner.hpp"

#define NUM_SAMPLES 100
#define WINDOW_SIZE 100000
#define DAC_LMAX {1,4,255}

#define DATASETS \
    { "data/text.txt", "data/image.jpg", "data/json.json" }
#define MEASURE(x, o)                                         \
                                                              \
    auto start = std::chrono::high_resolution_clock::now();   \
    x;                                                        \
    auto end = std::chrono::high_resolution_clock::now();     \
    auto duration = duration_cast<microseconds>(end - start); \
    o += duration.count()

using namespace std::chrono;

void emit(std::ofstream& statsFile, const std::string& name, const std::string& type, const std::string& dataset, int repetition,
          const std::string& value) {
    statsFile << name << ";" << type << ";" << dataset << ";" << repetition << ";" << value << std::endl;
}

std::ofstream openStatsCSVFile() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&now_c);
    std::replace(timestamp.begin(), timestamp.end(), ' ', '_');
    std::replace(timestamp.begin(), timestamp.end(), ':', '_');
    std::replace(timestamp.begin(), timestamp.end(), '\n', ' ');
    std::string name = "csv/stats_" + timestamp + ".csv";

    std::ofstream file;
    file.open(name, std::ios::out);
    file << "name;type;dataset;repetition;value" << std::endl;
    return file;
}

int main(int, char**) {
    // open record file
    std::ofstream statsFile = openStatsCSVFile();

    // retrieve datasets
    std::vector<std::string> datasets = DATASETS;

    // iterate through each dataset
    for (auto& dataset : datasets) {
        std::cout << "Dataset: " << dataset << std::endl;
        {
            // SDP
            std::cout << "\tSDP - Compression" << std::endl;
            TextFileIterator tf = TextFileIterator(dataset.c_str());
            auto sdc = tongrams::simple_dense_code_sequence();

            // measure compression time
            for (int i = 0; i < NUM_SAMPLES; i++) {
                double micros = 0;
                MEASURE(sdc.build(tf, -1), micros);
                emit(statsFile, "SDP", "compression_time", dataset, i, std::to_string(micros/1000));
            }

            // measure size
            emit(statsFile, "SDP", "dataset_size", dataset, 0, std::to_string(sdc.size()/1000));
            emit(statsFile, "SDP", "compressed_size", dataset, 0, std::to_string(sdc.bytes()/1000));

            std::cout << "\tSDP - Random access" << std::endl;
            // measure random access time
            for (int i = 0; i < NUM_SAMPLES; i++) {
                double micros = 0;
                MEASURE(
                    for (int j = 0; j < WINDOW_SIZE; j++) { sdc[rand() % sdc.size()]; }, micros);
                emit(statsFile, "SDP", "random_access_time", dataset, i, std::to_string(micros / WINDOW_SIZE));
            }

            std::cout << "\tSDP - Sequential access" << std::endl;
            // measure sequential access time
            for (int i = 0; i < NUM_SAMPLES; i++) {
                double micros = 0;
                tongrams::simple_dense_code_sequence::iterator it = sdc.begin();
                double dummy_sum = 0;
                for (int j = 0; j < WINDOW_SIZE; j++) {
                    MEASURE(++it, micros);
                    dummy_sum += *it * rand();
                }
                // dummy_sum is used to avoid compiler optimizations
                std::ofstream devnull("/dev/null");
                devnull << dummy_sum;
                emit(statsFile, "SDP", "sequential_access_time", dataset, i, std::to_string(micros / WINDOW_SIZE));
            }

            tf.close();
        }

        // DAC
        std::vector<int> lmaxs = DAC_LMAX;
        for(int index=0; index< lmaxs.size(); index++) {
            int lmax = lmaxs[index];
            std::string name = "DAC-" + std::to_string(lmax);

            std::cout << "\t" << name << " - Compression" << std::endl;
            TextFileIterator tf = TextFileIterator(dataset.c_str());
            auto dac = tongrams::directly_accessable_code_sequence();

            // measure compression time
            for (int i = 0; i < NUM_SAMPLES; i++) {
                double micros = 0;
                MEASURE(dac.build(tf, -1, lmax), micros);
                emit(statsFile, name.c_str(), "compression_time", dataset, i, std::to_string(micros/1000));
            }

            // measure size
            emit(statsFile, name.c_str(), "dataset_size", dataset, 0, std::to_string(dac.size()/1000));
            emit(statsFile, name.c_str(), "compressed_size", dataset, 0, std::to_string(dac.bytes()/1000));

            std::cout << "\t" << name << " - Random access" << std::endl;
            // measure random access time
            for (int i = 0; i < NUM_SAMPLES; i++) {
                double micros = 0;
                MEASURE(
                    for (int j = 0; j < WINDOW_SIZE; j++) { dac[rand() % dac.size()]; }, micros);
                emit(statsFile, name.c_str(), "random_access_time", dataset, i, std::to_string(micros / WINDOW_SIZE));
            }

            std::cout << "\t" << name << " - Sequential access" << std::endl;
            // measure sequential access time
            for (int i = 0; i < NUM_SAMPLES; i++) {
                double micros = 0;
                tongrams::directly_accessable_code_sequence::iterator it = dac.begin();
                double dummy_sum = 0;
                for (int j = 0; j < WINDOW_SIZE; j++) {
                    MEASURE(++it, micros);
                    dummy_sum += *it * rand();
                }
                // dummy_sum is used to avoid compiler optimizations
                std::ofstream devnull("/dev/null");
                devnull << dummy_sum;
                emit(statsFile, name.c_str(), "sequential_access_time", dataset, i, std::to_string(micros / WINDOW_SIZE));
            }

            tf.close();
        }
    }
}
