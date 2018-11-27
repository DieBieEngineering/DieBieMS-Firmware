//#define HWVersion_0_3 /* Define this when HW les then HW 0.4 */
#define HWVersion_0_4 /* Define this with the newer hardware */

//#define EFoilV0
//#define EFoilV1
#define EfoilV2
//#define ESK8

// Hardware name
#ifdef EFoilV0
  #define HW_NAME "EFoilBMSHW1"
#endif

#ifdef EFoilV1
  #define HW_NAME "EFoilBMSHW1"
#endif

#ifdef EfoilV2
  #define HW_NAME "EFoilBMSHW2"
#endif

#ifdef ESK8
  #define HW_NAME "DieBieMS"
#endif

// Firmware version
#define FW_VERSION_MAJOR		0
#define FW_VERSION_MINOR		22
#define FW_REAL_VERSION			"V0.22"

// UUID
#define STM32_UUID					((uint32_t*)0x1FFFF7AC)
#define STM32_UUID_8				((uint8_t*)0x1FFFF7AC)
