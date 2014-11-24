#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <utils/Log.h>

#define POINT_COUNT 10

int     m_uiHighThreshold = 10000;
//int     m_uiLowThreshold = 10;
int     m_uiLowThreshold = 15;
//float   m_uiTimeFreq = 0.010;
float   m_uiTimeFreq = 0.005;//5ms
float   m_uiRateRatioMax = 1.1;
float   m_uiRateRatioMin = 0.9;
float   m_uiRateMax = 200.0;
float   m_uiRateMin = 40.0;

#define RAW_MA_NUM (2)
#define RATE_MA_NUM (8)
#define RATE_PERSIST (1)

