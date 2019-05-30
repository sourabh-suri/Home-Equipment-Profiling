#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"


static int commandStart = 0x00;
static int commandStop = 0x40;
static int dataStart = 0x10;
static int dataStop = 0x50;

static int stringlength(char* word){
    int i;
    for(i=0;i<25;i++){
        if(word[i] == '\0') break;
    }
    if(i<25) return i;
    else return 0;
}


static void setControl(int value){
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6,value);
}

static void putData(int value){
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,value);
}

static void clearLCD(){

    int initCommands[] = {0x38,0x0C,0x06,0x01,0x80};

    int iter;

    for(iter=0;iter<(sizeof(initCommands)/sizeof(initCommands[0]));iter++){
        setControl(commandStart); // Enable = 0
        putData(initCommands[iter]);
        SysCtlDelay(50000);
        setControl(commandStop); // Enable = 1
    }

}

static void displayIP(char* word, char* preText){

    int iter;
    int length = stringlength(word);
    int initCommands[] = {0x3C,0x0C,0x06,0x80};

    for(iter=0;iter<(sizeof(initCommands)/sizeof(initCommands[0]));iter++){
        setControl(commandStart); // Enable = 0
        putData(initCommands[iter]);
        SysCtlDelay(50000);
        setControl(commandStop); // Enable = 1
    }

    for(iter=0;iter<=stringlength(preText);iter++){
            setControl(dataStart); // Enable = 0
            int data = (int)preText[iter];
            if(data == 0) putData((int)' ');
            else putData(data);
            SysCtlDelay(50000);
            setControl(dataStop); // Enable = 1
        }

    for(iter=0;iter<=stringlength(word);iter++){
        setControl(dataStart); // Enable = 0
        int data = (int)word[iter];
        if(data == 0) putData((int)' ');
        else putData(data);
        SysCtlDelay(50000);
        setControl(dataStop); // Enable = 1
    }

}
