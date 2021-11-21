// Written by Bernie Roehl, August 2021

// #define COLOUR_GAME
// #define PWM
#define REED_SWITCH

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"


void DisplaySensor(GPIO_TypeDef *port, uint16_t pin);

void DisplaySensor(GPIO_TypeDef *port, uint16_t pin){
    
    uint16_t period = 2000, prescale = 16;

    if(!HAL_GPIO_ReadPin(port,pin))
        __TIM2_CLK_ENABLE();  // enable timer 2
        TIM_HandleTypeDef pwmTimerInstance;  // this variable stores an instance of the timer
        InitializePWMTimer(&pwmTimerInstance, TIM2, period, prescale);   // initialize the timer instance
        InitializePWMChannel(&pwmTimerInstance, TIM_CHANNEL_1);          // initialize one channel (can use others for motors, etc)
        // InitializePWMChannel(&pwmTimerInstance, TIM_CHANNEL_2);

        InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM2); // connect the motor to the timer output

            while (true)
            {
                for (uint32_t i = 0; i < period; ++i)
                    {
                    SetPWMDutyCycle(&pwmTimerInstance, TIM_CHANNEL_1, i);
                    HAL_Delay(5);
                    }
            }     
 
    if(!HAL_GPIO_ReadPin(port,pin))
        SerialPutc('X');
    else
        SerialPutc('_');
}


int main(void)
{
    HAL_Init(); // initialize the Hardware Abstraction Layer

    // Peripherals (including GPIOs) are disabled by default to save power, so we
    // use the Reset and Clock Control registers to enable the GPIO peripherals that we're using.

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)

    // initialize the pins to be input, output, alternate function, etc...

    //InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // on-board LED

    // note: the on-board pushbutton is fine with the default values (no internal pull-up resistor
    // is required, since there's one on the board)

    // set up for serial communication to the host computer
    // (anything we write to the serial port will appear in the terminal (i.e. serial monitor) in VSCode)

    SerialSetup(9600);

    // as mentioned above, only one of the following code sections will be used
    // (depending on which of the #define statements at the top of this file has been uncommented)


// initalizing LEDs
    InitializePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
    InitializePin(GPIOB, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);

// initalizing buttons
    InitializePin(GPIOA, GPIO_PIN_9, GPIO_MODE_INPUT, GPIO_NOPULL, 0);  
    
    InitializePin(GPIOC, GPIO_PIN_13, GPIO_MODE_INPUT, GPIO_NOPULL, 0); 

    InitializePin(GPIOA, GPIO_PIN_6, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // start button

    InitializePin(GPIOA, GPIO_PIN_8, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // start button
    InitializePin(GPIOB, GPIO_PIN_10 |  GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // colour buttons



#ifdef REED_SWITCH
    while(1){
        DisplaySensor(GPIOA, GPIO_PIN_6);
        SerialPuts("   \r");
    }

#endif


#ifdef COLOUR_GAME

    // Wait for the user to push button, then blink the LED.

    bool led = true ;
    while (led){
        //  GPIOA, GPIO_PIN_5 == green led
        //  GPIOA, GPIO_PIN_6 == yellow led 
        //  GPIOA, GPIO_PIN_7 == blue led
        //  GPIOB, GPIO_PIN_6 == red led
        // PATTERN: blue, red, blue, green, red, green, green 

        if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))
        {

            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); // blue
            HAL_Delay (600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);

            HAL_Delay(500); 

            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); // red
            HAL_Delay(600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);

            HAL_Delay(500); 

            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7); // blue
            HAL_Delay(600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);

            HAL_Delay(500); 

            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // green
            HAL_Delay(600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

            HAL_Delay(500); 

            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); // red
            HAL_Delay(600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);

            HAL_Delay(500); 

            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // green
            HAL_Delay(600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

            HAL_Delay(500); 

            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // green
            HAL_Delay(600);  // 250 milliseconds == 1/4 second
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

            led = false;

        }
    }

        //  GPIOB, GPIO_PIN_10 == red button
        //  GPIOB, GPIO_PIN_4 == blue button
        //  GPIOB, GPIO_PIN_5 == yellow button
        //  GPIOB, GPIO_PIN_3 == green button
    
    int val = 0;
    int arr[7];
    int real_arr[7]={1,2,1,3,2,3,3};
    char buff[100];
        for (int i=0; i<7; i++){

            while ( (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)) && (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)) && (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) ){}
            sprintf(buff, "%hu", i);
            SerialPuts(buff);
            if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)){//blue
                arr[i] = 1;
            }

            else if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)){//red
                arr[i] = 2;
            }

            else if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)){//green
                arr[i] = 3;
            }
            while ( (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)) || (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)) || (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) ){}
            HAL_Delay(100);
    }

    for (int i=0; i<7; i++){
        if (arr[i] == real_arr[i])
            val +=1;
        
    }

    if (val == 7){
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

        HAL_Delay(2000);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    }

    else {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);

        HAL_Delay(2000);
        
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);


    }


#endif


#ifdef PWM

    uint16_t period = 2000, prescale = 16;

    __TIM2_CLK_ENABLE();  // enable timer 2
    TIM_HandleTypeDef pwmTimerInstance;  // this variable stores an instance of the timer
    InitializePWMTimer(&pwmTimerInstance, TIM2, period, prescale);   // initialize the timer instance
    InitializePWMChannel(&pwmTimerInstance, TIM_CHANNEL_1);          // initialize one channel (can use others for motors, etc)
    // InitializePWMChannel(&pwmTimerInstance, TIM_CHANNEL_2);

    InitializePin(GPIOA, GPIO_PIN_1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM2); // connect the motor to the timer output

   // if (val == 7) {
        while (true)
        {
            for (uint32_t i = 0; i < period; ++i)
                {
                SetPWMDutyCycle(&pwmTimerInstance, TIM_CHANNEL_1, i);
                HAL_Delay(5);
                }
        }
   // }

    
/*
    uint16_t period = 100, prescale = 16;

    __TIM2_CLK_ENABLE();  // enable timer 2
    TIM_HandleTypeDef pwmTimerInstance;  // this variable stores an instance of the timer
    InitializePWMTimer(&pwmTimerInstance, TIM2, period, prescale);   // initialize the timer instance
    InitializePWMChannel(&pwmTimerInstance, TIM_CHANNEL_1);          // initialize one channel

    InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM2); // connect the servo to the timer output

    while (true)
    {
        // fade the LED in by slowly increasing the duty cycle
        for (uint32_t i = 0; i < period; ++i)
        {
            SetPWMDutyCycle(&pwmTimerInstance, TIM_CHANNEL_1, i);
            HAL_Delay(5);
        }
        // fade the LED out by slowly decreasing the duty cycle
        for (uint32_t i = period; i > 0; --i)
        {
            SetPWMDutyCycle(&pwmTimerInstance, TIM_CHANNEL_1, i);
            HAL_Delay(5);
        }
    }
   */ 

#endif
    return 0;
}

// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}
