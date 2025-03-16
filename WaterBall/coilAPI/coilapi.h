#ifndef __COILAPI
#define __COILAPI

#include <stdint.h>

#define CONV_CHIPKIT 1
#define CONV_NETCAN 2
#define MAX_COILS 64

#define SET_COIL_POWER_ANY4  0
#define SET_COIL_POWER_1_8   1
#define SET_COIL_POWER_9_16  2
#define SET_COIL_POWER_17_24 3
#define SET_COIL_POWER_25_32 4

#define ERR_POWER_ALARM 1
#define ERR_POWER_ALARM_CLEAR 2
#define ERR_CONTROLLER_OFFLINE 3
#define ERR_CONTROLLER_ONLINE 4
#define ERR_CONV_REINIT_STARTED 5
#define ERR_CONV_REINIT_DONE 6

typedef void (*callback_func)(int adr, int event, void *data);

typedef union {
	uint8_t level[8];
	struct {
		uint8_t coil;
		uint8_t power;
	}
	pAny4[4];
}
TCoilPower;

#define EXPORT __declspec (dllexport)

extern "C" EXPORT int CoilsSetCoilPowerEx(uint8_t address,uint8_t kind,TCoilPower power);
extern "C" EXPORT int CoilsSetCoilPower(BYTE controller,BYTE coil,BYTE power);
extern "C" EXPORT int CoilsResetController(BYTE controller, BYTE kind);

extern "C" EXPORT int CoilsConnect();
extern "C" EXPORT void CoilsDisconnect();
extern "C" EXPORT int CoilsInit(callback_func c);
extern "C" EXPORT void CoilsExit();


#endif