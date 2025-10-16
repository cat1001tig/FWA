the provided compilation and usage instructions for the scheduling algorithm:

Compilation environment notes:

The project must be compiled with C++17 or newer.


I. Data

Two types of data are required:

Satellite time-window data

Grid-point coverage data at overlapping time windows for multiple satellites at each time point

Paths to these files must be specified in the parameters. Both files are in CSV format.

Satellite time-window data file

Filename: satellite_1.csv

File content format: "1 May 2025,08:23:00.000,6.57,6.57"

Grid-point data

Filename: s2_263.csv

Divide the observation time segment into time points with a step of 1. The label "s2_263" indicates that the current time point is 263 hours, and satellites 2 and 5 are scheduled (powered on) to observe concurrently (multi-satellite joint scheduling).

File content: The grid points are listed in order of latitude and longitude, top to bottom, containing only a single column of data. A value of 1 means the grid point is observed; 0 means it is not observed.


II. Dynamic Library usage of the algorithm

Basic configuration

Place FWADll.dll and FWADll.lib in the same directory as the test files. If you are using Visual Studio, specify the project properties: Linker -> Additional Library Directories and set it to the path where FWADll.lib resides.

Calling the algorithm

The algorithm exposes the following callable interfaces (functions):


(1) Parameter setting

You can configure the following parameters:


int max_switches = 5; // maximum number of consecutive on/off switches for satellites (constraint)

int q = 6665; // number of grid points for observation targets

std::vector<double> weights = { 0.34, 0.33, 0.33 }; // objective function weights (number of satellites turned on, coverage, load balancing)

int max_sparks = 30; // maximum number of sparks

int max_length = 10; // maximum length of a mutation-adjusted decision window

bool update_bounds = true; // update the fireworks explosion boundary (enabled by default)

int max_variation = 3; // maximum number of variations


std::vector<int> special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 }; // time points with overlapping windows

std::vector<int> satellites = { 2, 4, 5 }; // satellite identifiers


int num_satellites_ = 10; // total number of satellites

int total_minutes_ = 721; // 12*60 + 1


int starthour = 8; // observation start hour

int startminute = 0; // observation start minute

int startsecond = 0; // observation start second


int endhour = 20; // observation end hour

int endminute = 0; // observation end minute

int endsecond = 0; // observation end second



std::string directoryPath = ""; // path to time-window file

std::string data_dir_ = ""; // path to per-satellite grid data for overlapping observation times



(2) Executing the scheduling

Step 1: Prepare the scheduling result receiver, and initialize it

Step 2: Create the scheduler by calling CreateScheduler, passing the configured parameters

Step 3: Prepare the scheduling result receiver

Step 4: Execute scheduling by calling ExecuteScheduling, with the following parameters:


the scheduler instance

the storage name for the processed data files

the number of iterations

the initial number of firework individuals

the total number of decision points for all satellites that can vary in mutation (controls the mutation amplitude)

the scheduling result receiver


(3) Obtaining the scheduling results

Step 1: Get the number of optimal solutions by calling GetSolutionCount, with the scheduler as the parameter

Step 2: Retrieve details for each optimal solution, including:


solution index

number of satellites turned on in this schedule

coverage

load variance

Step: Output all optimal solutions: for each time point within the observation window, the decision status of each satellite (0 = off, 1 = on, -1 = no time window for that satellite). You can use this information to plot visualization results.



III. Notes:

Ensure the CSV files satellite_1.csv and s2_263.csv are correctly formatted as described.

The parameter values above are example defaults; adjust them as needed for your use case.

The dynamic library should be linked properly in your build environment (Visual Studio: specify the appropriate lib path; other environments: ensure the library is discoverable at runtime/link time).


IV. Screenshot of the sample run results：
<img width="2551" height="1215" alt="image" src="https://github.com/user-attachments/assets/351d5d31-09a0-444a-b387-772c22a5939e" />

<img width="2555" height="687" alt="屏幕截图 2025-10-16 155743" src="https://github.com/user-attachments/assets/22e617dc-11cb-4ba7-8c46-3b6f4eb340cd" />

<img width="2527" height="1384" alt="屏幕截图 2025-10-16 155719" src="https://github.com/user-attachments/assets/c8c1fde2-ab11-4076-a3d6-c5bf83efefa4" />


