/*----------------------------------------------------------------------------
| File:
|   ecu.c
|
| Description:
|   Test Measurement and Calibration variables for XCP demo
|   C language
 ----------------------------------------------------------------------------*/
 /*
 | Code released into public domain, no attribution required
 */

#include "main.h"
#include "main_cfg.h"
#include "platform.h"
#include "xcpLite.h"
#include "A2L.h"
#include "ecu.h"

#include <sys\timeb.h> 


struct timeb start, end;
int diff;
int i = 0;

/**************************************************************************/
/* ECU Measurements */
/**************************************************************************/

// Event
uint16_t gXcpEvent_EcuCyclic = 0; // XCP event number

// Global measurement variables
double ecuTime = 0;
double channel1 = 0;
double channel2 = 0;
double channel3 = 0;


/**************************************************************************/
/* ECU Parameters */
/**************************************************************************/


struct ecuPar {
    char epk[32];
    uint32_t cycleTimeUs;
    double period;
    double offset;
    double phase;
    double ampl;
    uint8_t map1_8_8[8][8];
    uint8_t curve1_32[32];
};


struct ecuPar ecuPar = {
    __DATE__ " " __TIME__, // EPK
    10000, // Default cycle time in us
    3.0, // period
    0.0, // offset
    0, // phase
    400.0, // ampl
    { {0,0,0,0,0,0,1,2}, // map1_8_8
     {0,0,0,0,0,0,2,3},
     {0,0,0,0,1,1,2,3},
     {0,0,0,1,1,2,3,4},
     {0,1,1,2,3,4,5,7},
     {1,1,1,2,4,6,8,9},
     {1,1,2,4,5,8,9,10},
     {1,1,3,5,8,9,10,10}
    },
    { 0,1,3,6,9,15,20,30,38,42,44,46,48,50,48,45,40,33,25,15,5,4,3,2,1,0,0,1,4,8,4,0} // curve1_32
};

volatile struct ecuPar* ecuCalPage = &ecuPar;

#if OPTION_ENABLE_CAL_SEGMENT
volatile struct ecuPar ecuRamPar;
#endif

/**************************************************************************/
/* ECU Demo Code */
/**************************************************************************/

char* ecuGetEPK() {
    return (char*)ecuPar.epk;
}

#if OPTION_ENABLE_CAL_SEGMENT

// Calibration page handling
// page 0 is RAM, page 1 is ROM
#define RAM 0
#define ROM 1
// A2L file contains ROM addresses

void ecuParInit() {

    memcpy((void*)&ecuRamPar, (void*)&ecuPar, sizeof(struct ecuPar));
    ecuParSetCalPage(0);
}

void ecuParSetCalPage(uint8_t page) {

    ecuCalPage = (page == RAM) ? &ecuRamPar : (struct ecuPar *)&ecuPar;
}

uint8_t ecuParGetCalPage() {

    return (ecuCalPage == &ecuRamPar) ? RAM : ROM;
}

uint8_t *ecuParAddrMapping( uint8_t *a ) {

    if (a >= (uint8_t*)&ecuPar && a < (uint8_t*)&ecuPar + sizeof(struct ecuPar)) {
      if (ecuCalPage == &ecuPar) return a; // RAM
      assert(ecuCalPage == &ecuRamPar);
      return (uint8_t*)&ecuRamPar + (a - (uint8_t*)&ecuPar); // ROM -> RAM
    }
    return a;
}

#endif // OPTION_ENABLE_CAL_SEGMENT

// Init demo parameters and measurements 
void ecuInit() {

    // Initialize calibration parameters
#ifdef OPTION_ENABLE_CAL_SEGMENT
    ecuParInit(); // Initializes parameters in RAM calibration segment
    ecuParSetCalPage(0); // Switch to RAM calibration segment 
#endif

    // Initialize measurement variables
    channel1 = channel2 = channel3 = 0;

    // Create an XCP event for the cyclic task
    gXcpEvent_EcuCyclic = XcpCreateEvent("ecuTask", CLOCK_TICKS_PER_MS, 0, 0, 0);
}


// Create demo A2L file 
void ecuCreateA2lDescription() {
    // Measurements
    A2lSetEvent(gXcpEvent_EcuCyclic); // Associate XCP event "EcuCyclic" to the variables created below
    A2lCreatePhysMeasurement(channel1, A2L_TYPE_DOUBLE, "Sinus signal 1", 1.0, 0.0, "");
    A2lCreatePhysMeasurement(channel2, A2L_TYPE_DOUBLE, "Sinus signal 2", 1.0, 0.0, "");
    A2lCreatePhysMeasurement(channel3, A2L_TYPE_DOUBLE, "Sinus signal 3", 1.0, 0.0, "");

    // Create a group for the measurments (optional)
    A2lMeasurementGroup("EcuTaskSignals", 3,
        "channel1", "channel2", "channel3");
}


// Cyclic demo task 
void ecuCyclic( void )
{
    // Floating point signals
    channel1++;
    channel2++;
    channel3++;

    XcpEvent(gXcpEvent_EcuCyclic); // Trigger XCP measurement data aquisition event 
}


// ECU cyclic (10ms default)
#ifdef _WIN
DWORD WINAPI ecuTask(LPVOID p)
#else
void* ecuTask(void* p)
#endif
{
    (void)p;
    printf("Start C task (cycle = %dus, XCP event = %d)\n", ecuCalPage->cycleTimeUs, gXcpEvent_EcuCyclic);
    for (;;) {
    //    ftime(&start);

        for (unsigned char index = 0; index < 10; index++) {
            sleepNs(1000000); // 10ms
        }
        ecuCyclic();

    //    ftime(&end);
    //    diff = (int)(1000.0 * (end.time - start.time)
    //         + (end.millitm - start.millitm));

    //    printf("\nOperation took %u milliseconds\n", diff);
    }
}
