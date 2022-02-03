# unix-system-monitor
Simple monitor of CPU, Mem and User utilization and information for UNIX operating systems.

System Monitor for UNIX Operating Systems
------------------------------

When run, this program will display the following information:

General Sys Info:
 -- System Name
 -- OS Version
 -- OS Release
 -- Machine Name

Memory Info:
 -- Used Physical Memory
 -- Used Virtual Memory
 -- Total Physical Memory
 -- Total Virtual Memory
	(displays this information for each sample.)

General CPU Info: 
 -- Model Name
 -- Num. Cores


Util CPU Info:
 -- Current Total Utilization (%)
 -- Depending on CLA, Utilization per sample of data collected.
	(by default there will be 10 samples, sampled once per second.)

User Info:
 -- Users, their TTY Name and Hostname (if remote) or Kernel version.
 -- Number of Total Users

------------------------------

The following CLAs are accepted and will result in the following:

--graphics
 -- Will output graphical representation of change in memory per Sample
 -- Will output graphical representation of total CPU utilization per Sample

--user
 -- Will output only General Sys Info and User Info

--system
 -- Will output only General Sys Info, Memory Info and CPU (util and general) Info.

--samples=N
 -- specifies number of trials for collecting memory and CPU data.

--tdelay=T
 -- specifies number (in integers) of seconds between samples. 

NOTE: Samples and Delay can also be specified through inputs as positional arguments
      i.e. inputting CLAs "15 3" would specify 15 samples collected every 3 seconds.

------------------------------
IMPLEMENTATION INFORMATION:

In order to write this program I had to come up with a number of functions that read specific data from specific locations in Linux's OS. Initially, in order to meet the specifications input as CLAs the  program determines using the function updateconstants() what needs to be output into STDOUT. These constants which are stored in the main body of the program are then run through a function display(), which in turn gathers information about the CPU Utilization and Memory through a call of fetchSysData(), which populates this data into an input array that display() later uses to output that data. Display also handles the collection of more simple data, like General CPU Info, User Info and General Sys Info. 


FUNCTIONS:

void breaker() --> This function simply prints a long string of '-' to create a divider in output.

void fetchSysData() --> This function reads memory and CPU utilization data from 'sys/sysinfo.h' and '/proc/stat' respectively and populates a 3xN+1 array (passed in as a pointer, where numSamples = N) with CPU Utilization for N trials on row 1 starting at index 1, Physical Used memory for N trials starting on row 2 index 1, and Virtual used memory for N trials starting on row 3 index 1. 

void updateconstants() --> This function takes a pointer to a CLA string and updates constants whose pointers' are inputs into the function correspondingly. 

void display() --> This function takes constants corresponding to CLA specifications as inputs, creates an array for CPU and Men data to be stored, runs fetchSysData to populate the array, then uses that data to output. Depending on constants input, display() will organize and display all information required by the user based on the specifications above. 
