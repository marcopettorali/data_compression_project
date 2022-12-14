# Data Compression Project
## Benchmark for SDC and DAC algorithms
### Authors:
- [Leonardo Bacciottini](https://github.com/Baccios)
- [Marco Pettorali](https://github.com/marcopettorali)

### Description
This project is a benchmark for the SDC and DAC algorithms. The project contains the following files:

- benchmark.cpp is the main file that contains the benchmark for the SDC and DAC algorithms, and generates a CSV file with the results.
- plots_maker.py is a python script that generates the plots from the CSV file.
- stats_util.py is a python script that contains some utilities for the plots_maker.py script.

and the following folders:

- 'include' folder contains the header files for the SDC and DAC algorithms and other utilities
- 'data' folder contains the data used for the benchmark.
- 'csv' folder contains the CSV files generated by the benchmark.
- 'out' folder contains the output plots generated by the plots_maker script.

### How to use
To use the benchmark, you need to compile the benchmark.cpp file with CMake. The CMakeLists.txt file is already configured to compile the benchmark.cpp file. To execute the benchmark and the plots script, you need to have installed the following libraries:
- [CMake](https://cmake.org/)
- [Python](https://www.python.org/)
- [Matplotlib](https://matplotlib.org/)
- [Pandas](https://pandas.pydata.org/)

To execute the benchmark, you need to launch it from the main folder of the project. The benchmark will generate a CSV file in the 'csv' folder. To generate the plots, you need to launch the plots_maker.py script from the main folder of the project. The plots will be generated in the 'out' folder and shown on the screen.

### How the benchmark works
The datasets used for the benchmark are in the 'data' folder. The benchmark will compress each dataset with the SDC and DAC algorithms, and will repeat the compression for each dataset NUM_SAMPLES times, and collecting the compression time and compression ratio. Then, random access times and sequential access times will be measured for each dataset. The results will be saved in a CSV file in the 'csv' folder with the timestamp of the execution.