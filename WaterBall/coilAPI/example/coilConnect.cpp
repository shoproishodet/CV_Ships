// coilConnect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include  <stdio.h>
#include <winsock.h>
#include <process.h>
#include <fcntl.h>
#include <tchar.h>

#include "coilapi.h"

BOOL Done = FALSE;
int Debug = 0;	 

void logTimeStamp() {
    SYSTEMTIME      DateTime;

    GetLocalTime(&DateTime);
	printf("%d %u\n",DateTime.wMilliseconds, GetTickCount());

}

char ** parseString(char * inputString, char * delim, int * cnt ) {

    char * token = strtok(inputString, delim);
    char ** result = NULL;

    *cnt = 0;


    while (token) {
        result = (char **) realloc(result, sizeof (char *) * ++(*cnt));
        result[*cnt-1] = token;

        token = strtok(NULL, delim);
    }
    result = (char **) realloc(result, sizeof(char *) * (*cnt+1));
    result[*cnt] = NULL;
    
    return result;
}

void example_callback(int adr, int event, void *data) {
	switch(event) {
	case ERR_POWER_ALARM:
		printf("power alarm on controller %d\n", adr);
		break;
	case ERR_POWER_ALARM_CLEAR:
		printf("power alarm clear on controller %d\n", adr);
		break;
	case ERR_CONTROLLER_OFFLINE:
		printf("controller %d OFFLINE\n", adr);
		break;
	case ERR_CONTROLLER_ONLINE:
		printf("controller %d ONLINE\n", adr);
		break;
	default:
		printf("unknown event code %d\n", event);
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
    int rc;

    rc = CoilsInit(example_callback);
    
    if (rc != 0) { printf("Error init Coilapi, return code %d\n", rc); exit(1);}

    rc = CoilsConnect();
    if (rc != 0) { printf("Error CoilsConnect, return code %d\n", rc); exit(1);}

//	_beginthread( ReadCom, 0, NULL);
    while (!Done) {
        char cmd[80];
        int argCount;
        char **res;
        int controllerId, coilId, param, i;
		TCoilPower power_info;
        
        printf("Command: ");
        gets(cmd);
        
        res = parseString((char *)&cmd[1], " ", &argCount);
        switch (cmd[0]) {
        case'?' :
        case 'h':
        case 'H':
            {
                printf("h,H,? - help\nq,Q - quit\n");
                printf("d,D - increase debug level(0-2)\n");
                printf("t <controller#> - send several setCoilPower msg\n");
				printf("w <controller#> - set working params from db\n");
				printf("s <controller#> <coil#> <power>- setCoilPower\n");
				printf("p <controller#> <kind> <args...> - set coil power\n");
				printf("                 0 - ANY4, 1 - COILS_01_08, 2 - COILS_09_16,\n");
				printf("                           3 - COILS_17_24, 4 - COILS_25_32,\n");
                break;
            }
        case 'd':
        case 'D':
            {
                Debug++;
                if (Debug > 2)
                    Debug = 0;
                printf("Debug = %d\n", Debug);
                break;
            }
		case 'w':
			if(argCount > 0) {
				controllerId = atoi(res[0]);
				rc = CoilsSetWorkingParams((uint8_t)controllerId);
				if (rc !=0 ) printf("error CoilSetWorkingParams, rc %d\n",rc);
			}
			else {
				printf("please enter controler#\n");
			};
		case 'p':
			if(argCount > 2) {
				controllerId = atoi(res[0]);
				param = atoi(res[1]);
				memset((void *)&power_info,0,sizeof(power_info));
				for(i=2; i<argCount;i++) power_info.level[i-2]=atoi(res[i]);

//				for(i=0; i<argCount-2;i++) printf("%d %d\n", i, power_info.level[i]);

				rc = CoilsSetCoilPowerEx(controllerId, param , power_info);
				if (rc !=0 ) printf("error CoilsSetCoilPowerEx, rc %d\n",rc);
			}
			else {
				printf("please enter args\n");
			};

			break;
        case 's':
            if(argCount > 2) {
                controllerId = atoi(res[0]);
                coilId = atoi(res[1]);
                param = atoi(res[2]);
                rc = CoilsSetCoilPower(controllerId, coilId, param);
                if (rc !=0 ) printf("error CoilsSetCoilPower, rc %d\n",rc);
            }
            else {printf("please enter args\n"); };
            break;
        case 't':
            if(argCount > 0) {
                controllerId = atoi(res[0]);
//				printf("%ld\n",GetTickCount());
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 0, 80);
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 3, 50);
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 4, 60);
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 12, 70);
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 17, 30);
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 22, 10);
				logTimeStamp();
                CoilsSetCoilPower(controllerId, 30, 90);
				logTimeStamp();
            }
            else {printf("please enter args\n"); };
            break;
        case 'q':
        case 'Q':
            {
                Done = 1;
                CoilsDisconnect();
                CoilsExit();
                exit(0);
            }
        }
        free(res);
    }

    return 0;
}

