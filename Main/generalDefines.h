//#define HWVersion_0_3 /* Define this when HW les then HW 0.4 */
#define HWVersion_0_4 /* Define this with the newer hardware */

// Define the hardware version here:
//#define EFoilV1
//#define EFoilV2
//#define ESK8
#define TDHVSolar
//#define DSBTSolar

// Firmware version
#define FW_VERSION_MAJOR		0
#define FW_VERSION_MINOR		30
#define FW_REAL_VERSION			"V0.30"

// UUID
#define STM32_UUID					((uint32_t*)0x1FFFF7AC)
#define STM32_UUID_8				((uint8_t*)0x1FFFF7AC)

// Hardware names and limits
#ifdef EFoilV1
  #define HW_NAME "EFoilBMSHW1"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 5
	#define HW_LIM_CELL_MONITOR_IC_TYPE                    1
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   1
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#ifdef EFoilV2
  #define HW_NAME "EFoilBMSHW2"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 12
	#define HW_LIM_CELL_MONITOR_IC_TYPE                    2
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   1
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#ifdef DSBTSolar
  #define HW_NAME "DSBTSolarHW1"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 12
	#define HW_LIM_CELL_MONITOR_IC_TYPE                    2
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   1
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#ifdef TDHVSolar
	#define HW_NAME "TDHVSolar"
	#define HAS_FLIPFLOP
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 7
	#define HW_LIM_CELL_MONITOR_IC_TYPE                    2
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   5
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#ifdef ESK8
  #define HW_NAME "DieBieMS"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 12
	#define HW_LIM_CELL_MONITOR_IC_TYPE                    1
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   1
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
