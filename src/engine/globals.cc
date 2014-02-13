#include "version.h"
#include "global.h"
#include "Lightning.h"
#include "Metadata.h"

std::ofstream logfile;

/** 
 * @brief set real-time priority to next to highest configured
 *
 */
void setprio()
{
    struct rlimit rlim;
    if (getrlimit(RLIMIT_RTPRIO, &rlim) != 0)
    {
        return;
    }
    if (rlim.rlim_max > 1)
    {
        --rlim.rlim_max;
    }
    struct sched_param params;
    params.sched_priority=rlim.rlim_max;
    int rv=pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
    if (rv != 0)
    {
        LOG("pthread_setschedparam problem " << rv << " rlim.rlim_max: " <<
            rlim.rlim_max);
    }    
}
