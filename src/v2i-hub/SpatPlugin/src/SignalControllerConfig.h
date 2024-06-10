#include <vector>

namespace SignalControllerConfig {   
    /*The signal group state stores the traffic signal controller configuration parameters for a J2735 signal group
    * The parameters within it define the phase mapped to the signal group and other data necessary to modify a given phase*/
    struct SignalGroupConfig
    {
        /* signal group id identifier for J2735 which is mapped to phase number in NTCIP */
        int signal_group_id;
        /*Phase associated with signal group*/
        int phase_num;
        /*Minimum green durations for each active vehicle phase in milliseconds*/
        int min_green;
        /*Maximum green durations for each active vehicle phase in milliseconds*/
        int max_green;
        /*Default Green duration for phase in milliseconds*/
        int green_duration;
        /*Yellow signal duration for phase in milliseconds*/
        int yellow_duration;
        /* Red clearace time for phase in milliseconds*/
        int red_clearance;
        /*Red signal duration for phase. This is the total time a phase is predicted to be red before its next green. In milliseconds*/
        int red_duration;
        /*Phase sequence in ring. Stores the sequence starting from the current phase*/
        std::vector<int> phase_seq;
        /*Phases in the same barrier or concurrent group excluding phases from same ring*/
        std::vector<int> concurrent_signal_groups;
    };

    struct SignalControllerConfigState
    {
        /* data */
    };
    
}