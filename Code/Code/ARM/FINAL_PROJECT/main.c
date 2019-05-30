#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
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
#include "lcd.c"

#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F (*((volatile unsigned long *)0x40025524))


char uartin[25];
char part1[2];
char part2[4];
char part3[2];
char part4[4];
char part5[12];

int LedIntensity = 0;
int LedStatus = 0;
int FanSpeed = 0;
int FanStatus = 0;



int checkvalidASCII(int asciival){
    if(asciival >= 48 && asciival <= 57) return 1;
    if(asciival >= 65 && asciival <= 90) return 1;
    if(asciival >= 97 && asciival <= 122) return 1;
    if(asciival == 95) return 1; // _
    if(asciival == 46) return 1; // .
    return -1;
}


void clearUART(){
    int iter = 0;
    int len = sizeof(uartin)/sizeof(char);
    while(iter < len){
        uartin[iter] = '\0';
        iter++;
    }
}


void fetchUART(){
    int iter = 0;
    int in;
    while((in = UARTCharGetNonBlocking(UART5_BASE)) != 35)
    {
        if(checkvalidASCII(in) == 1){
            uartin[iter] = in;
            UARTCharPutNonBlocking(UART0_BASE,in);
            iter++;
        }
    }
    uartin[iter] = '\0';
    UARTCharPutNonBlocking(UART0_BASE,'\n');
    UARTDisable(UART5_BASE);
}




void processData(char* word){

    int flag = 0, part1counter = 0, part2counter = 0, part3counter = 0, part4counter = 0, part5counter = 0, i;

    for(i = 0; i < stringlength(word); i++){
        if(word[i] != '_') {
            if(flag == 0) part1[part1counter++] = word[i];
            else if (flag == 1) part2[part2counter++] = word[i];
            else if (flag == 2) part3[part3counter++] = word[i];
            else if (flag == 3) part4[part4counter++] = word[i];
            else if (flag == 4) part5[part5counter++] = word[i];
        }
        else if(flag == 0) flag = 1;
        else if(flag == 1) flag = 2;
        else if(flag == 2) flag = 3;
        else flag = 4;
    }

    flag = 0;

    for(i=part1counter;i<2;i++){ part1[i] = '\0'; }
    for(i=part2counter;i<4;i++){ part2[i] = '\0'; }
    for(i=part3counter;i<2;i++){ part3[i] = '\0'; }
    for(i=part4counter;i<4;i++){ part4[i] = '\0'; }
    for(i=part5counter;i<12;i++){ part5[i] = '\0'; }


    if(strcmp(part1,"F")) LedStatus = 1;
    else LedStatus = 0;

    if(strcmp(part3,"F")) FanStatus = 1;
        else FanStatus = 0;

    LedIntensity = atoi(part2);
    FanSpeed = atoi(part4);

    if(LedIntensity < 0) LedIntensity = 0;
    if(LedIntensity > 100) LedIntensity = 100;

    if(FanSpeed < 0) FanSpeed = 0;
    if(FanSpeed > 100) FanSpeed = 100;


}


void SetRelayStates(){
    if(FanStatus == 0) GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0x00);
    else GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3,0xFF);

    if(LedStatus == 0) GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6,0x00);
    else GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6,0xFF);
}

void GeneratePWM(){

    if(LedIntensity > 0 && LedStatus == 1){
        PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 2*LedIntensity);
    }
    else{
        PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);
    }

}


void LCDInit(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // LCD Data/Command Signals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // LCD Control Signals

    // Setting Port B pins as output (For Data/Command Transfer)
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    // Setting Port C 4,5,6 pins as output (For Control Signals)
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);
}


void UARTInit(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // Main UART for Communication with NodeMCU 1.0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Debug Communications with NodeMCU 1.0

    // Enable UART0 (Debug UART)
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1 | GPIO_PIN_0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Enable UART5 (Main UART)
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART5));

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE);
    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 9600, UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE);

    // Enable UART5 (UART0 is by default enabled)
    UARTEnable(UART5_BASE);
    UARTFIFOEnable(UART5_BASE);
}

void PWMInit(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // PWM Pins Configure

    // PWM Configuration
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM1));
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3,PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 400);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 50);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
}


void TimerInit(){

    uint32_t Period;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); // Enable Timer 2 Peripheral Clock
    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC); // Configure Timer 2 mode - periodic
    Period = (SysCtlClockGet()*10); // Period = (CPU Clock / 0.1)
    TimerLoadSet(TIMER2_BASE, TIMER_A, Period); // Set Timer 2 period => Every 10s interrupt is raised
    IntEnable(INT_TIMER2A); // Enable Timer 2 interrupt
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Enable Timer 2 to interrupt CPU
    IntMasterEnable(); // Enable master interrupt

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Peripheral Clock
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer 2 mode - periodic
    Period = (SysCtlClockGet()*(float)(0.01*(float)((float)(100-FanSpeed)/(float)100))); // Period = (CPU Clock * 0.001)
    TimerLoadSet(TIMER1_BASE, TIMER_A, Period); // Set Timer 1 period => Every 1ms interrupt is raised
    IntEnable(INT_TIMER1A); // Enable Timer 2 interrupt
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Enable Timer 1 to interrupt CPU
    IntMasterEnable(); // Enable master interrupt

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable Timer 1 Peripheral Clock
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Configure Timer 2 mode - periodic
    Period = (SysCtlClockGet()*0.001); // Period = (CPU Clock * 0.001)
    TimerLoadSet(TIMER0_BASE, TIMER_A, Period); // Set Timer 1 period => Every 1ms interrupt is raised
    IntEnable(INT_TIMER0A); // Enable Timer 2 interrupt
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Enable Timer 1 to interrupt CPU
    IntMasterEnable(); // Enable master interrupt

}


void Handler(){

    GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_3);        // Disable interrupt for PE1 (in case it was enabled)
    GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);      // Clear pending interrupts for PE1

    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0x00);

    uint32_t Period = (SysCtlClockGet()*(float)(0.01*(float)((float)(100-FanSpeed)/(float)100))); // Period = (CPU Clock * 0.001)
    TimerLoadSet(TIMER1_BASE, TIMER_A, Period); // Set Timer 1 period => Every 1ms interrupt is raised

    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0xFF);

    IntEnable(INT_TIMER1A); // Enable Timer 2 interrupt
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Enable Timer 2 to interrupt CPU
    TimerEnable(TIMER1_BASE, TIMER_A); // Enable Timer 2

    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_3);     // Enable interrupt for PE1
}

void InterruptInit(){
    IntMasterEnable();
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    IntEnable(INT_GPIOE);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_3);        // Disable interrupt for PF4 (in case it was enabled)
    GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);      // Clear pending interrupts for PF4
    GPIOIntRegister(GPIO_PORTE_BASE, Handler);     // Register our handler function for port F
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);             // Configure PF4 for falling edge trigger
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_3);     // Enable interrupt for PF4
}


void RelayInit(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_6);
}


int main(void)
{


    //Set CPU Clock to 20MHz. 400MHz PLL/2 = 200 DIV 10 = 20MHz
    SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); // Set System Clock

    LCDInit();
    UARTInit();
    PWMInit();
    TimerInit();
    InterruptInit();
    RelayInit();

    // Clearing LCD Screen
    clearLCD();

    TimerEnable(TIMER2_BASE, TIMER_A); // Enable Timer 2

    while(1) {

        UARTEnable(UART5_BASE);

        while(!UARTCharsAvail(UART5_BASE));

        fetchUART();

        processData(uartin);

        SetRelayStates();

        GeneratePWM();

        clearUART();
    }

}
