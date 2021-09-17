// Support routines for ECE 198 (University of Waterloo)

// Written by Bernie Roehl, July 2021

#include <stdbool.h> // bool type, true and false
#include "ece198.h"

/////////////////
// Serial Port //
/////////////////

UART_HandleTypeDef UART_Handle;  // the serial port we're sing

// Initialize a pin (or pins) to a particular mode, with optional pull-up or pull-down resistors
// and possible alternate function
// (we use this so students don't need to know about structs)

void InitializePin(GPIO_TypeDef *port, uint16_t pins, uint32_t mode, uint32_t pullups, uint8_t alternate)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = pins;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pullups;
    GPIO_InitStruct.Alternate = alternate;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

// initialize the serial port at a particular baud rate (PlatformIO defaults to 9600, so should normally use that)

HAL_StatusTypeDef SerialSetup(uint32_t baudrate)
{
    __USART2_CLK_ENABLE();        // enable clock to USART2
    __HAL_RCC_GPIOA_CLK_ENABLE(); // serial port is on GPIOA

    GPIO_InitTypeDef GPIO_InitStruct;

    // pin 2 is serial RX
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // pin 3 is serial TX (most settings same as for RX)
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // configure the serial port
    UART_Handle.Instance = USART2;
    UART_Handle.Init.BaudRate = baudrate;
    UART_Handle.Init.WordLength = UART_WORDLENGTH_8B;
    UART_Handle.Init.StopBits = UART_STOPBITS_1;
    UART_Handle.Init.Parity = UART_PARITY_NONE;
    UART_Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART_Handle.Init.Mode = UART_MODE_TX_RX;

    return HAL_UART_Init(&UART_Handle);
}

// wait for a character to arrive from the serial port, then return it

char SerialGetc()
{
    while ((UART_Handle.Instance->SR & USART_SR_RXNE) == 0)
        ;                             // wait for the receiver buffer to be Not Empty
    return UART_Handle.Instance->DR;  // return the incoming character
}

// send a single character out the serial port

void SerialPutc(char c)
{
    while ((UART_Handle.Instance->SR & USART_SR_TXE) == 0)
        ;                          // wait for transmitter buffer to be empty
    UART_Handle.Instance->DR = c;  // send the character
}

// get a string of characters (up to maxlen) from the serial port into a buffer
// also echoes the typed characters back to the user, and handles backspacing

void SerialGets(char *buff, int maxlen)
{
    int i = 0;
    while (1)
    {
        char c = SerialGetc();
        if (c == '\r') // user pressed Enter key
        {
            buff[i] = '\0';
            SerialPuts("\r\n"); // echo return and newline
            return;
        }
        else if (c == '\b') // user pressed Backspace key
        {
            if (i > 0)
            {
                --i;
                SerialPuts("\b \b"); // overwrite previous character with space
            }
        }
        else if (i < maxlen - 1) // user pressed a regular key
        {
            buff[i++] = c;  // store it in the buffer
            SerialPutc(c);  // echo it
        }
    }
}

// write a string of characters to the serial port

void SerialPuts(char *ptr)
{
    while (*ptr)
        SerialPutc(*ptr++);
}

////////////////////
// Rotary Encoder //
////////////////////

// read a rotary encoder (handles the quadrature encoding, and uses a previousClk boolean variable provided by the caller)

int ReadEncoder(GPIO_TypeDef *clkport, int clkpin, GPIO_TypeDef *dtport, int dtpin, bool *previousClk)
{
    bool clk = !HAL_GPIO_ReadPin(clkport, clkpin);  // active low
    bool dt = !HAL_GPIO_ReadPin(dtport, dtpin);     // active low
    int result = 0;
    if (!clk && *previousClk)  // if the clk signal has changed since last time we were called...
        result = clk != dt ? 1 : -1;   // set the result to the direction (-1 if clk == dt, 1 if they differ)
    *previousClk = clk;        // store for next time
    return result;
}

////////////////////////////
// Pulse Width Modulation //
////////////////////////////

// set up a specified timer with the given period (whichTimer is something like TIM2 or TIM1)

void InitializePWMTimer(TIM_HandleTypeDef *timer, TIM_TypeDef *whichTimer, uint16_t period, uint16_t prescale )
{
    timer->Instance = whichTimer;
    timer->Init.CounterMode = TIM_COUNTERMODE_UP;
    timer->Init.Prescaler = prescale;
    timer->Init.Period = period;
    timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer->Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(timer);
}

// set up a particular channel on the given timer

void InitializePWMChannel(TIM_HandleTypeDef *timer, uint32_t channel)
{
    TIM_OC_InitTypeDef outputChannelInit;
    outputChannelInit.OCMode = TIM_OCMODE_PWM1;
    outputChannelInit.OCPolarity = TIM_OCPOLARITY_HIGH;
    outputChannelInit.OCFastMode = TIM_OCFAST_ENABLE;
    outputChannelInit.Pulse = timer->Init.Period;
    HAL_TIM_PWM_ConfigChannel(timer, &outputChannelInit, channel);
    HAL_TIM_PWM_Start(timer, channel);
}

// set the number of ticks in a cycle (i.e. <= the timer's period) for which the output should be high

void SetPWMDutyCycle(TIM_HandleTypeDef *timer, uint32_t channel, uint32_t value)
{
    switch (channel)
    {
    case TIM_CHANNEL_1:
        timer->Instance->CCR1 = value;
        break;
    case TIM_CHANNEL_2:
        timer->Instance->CCR2 = value;
        break;
    case TIM_CHANNEL_3:
        timer->Instance->CCR3 = value;
        break;
    case TIM_CHANNEL_4:
        timer->Instance->CCR4 = value;
        break;
    }
}

/////////////////////
// Keypad Scanning //
/////////////////////

struct { GPIO_TypeDef *port; uint32_t pin; }
rows[] = {
    { GPIOC, GPIO_PIN_7 },
    { GPIOA, GPIO_PIN_9 },
    { GPIOA, GPIO_PIN_8 },
    { GPIOB, GPIO_PIN_10 }
},
cols[] = {
    { GPIOB, GPIO_PIN_4 },
    { GPIOB, GPIO_PIN_5 },
    { GPIOB, GPIO_PIN_3 },
    { GPIOA, GPIO_PIN_10 }
};

void InitializeKeypad() {  // rows are outputs, columns are inputs
    for (int i = 0; i < 4; ++i) {
        InitializePin(rows[i].port, rows[i].pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
        InitializePin(cols[i].port, cols[i].pin, GPIO_MODE_INPUT, GPIO_PULLDOWN, 0);  // inputs are low if not pulled high
    }
}

int ReadKeypad() {
    // scan a 4x4 key matrix by applying a voltage to each row in succession and seeing which column is active
    // should work with a 4x3 matrix, since last column will just return zero
    for (int row = 0; row < 4; ++row) {
        // enable the pin for (only) this row
        for (int i = 0; i < 4; ++i)
            HAL_GPIO_WritePin(rows[i].port, rows[i].pin, i == row);  // all low except the row we care about
        for (int col = 0; col < 4; ++col)  // check all the column pins to see if any are high
            if (HAL_GPIO_ReadPin(cols[col].port, cols[col].pin))
                return row*4+col;
    }
    return -1;  // none of the keys were pressed
}

///////////////////////
// 7-Segment Display //
///////////////////////

struct { GPIO_TypeDef *port; uint32_t pin; }
segments[] = {
    { GPIOA, GPIO_PIN_0 },  // A
    { GPIOA, GPIO_PIN_1 },  // B
    { GPIOA, GPIO_PIN_4 },  // C
    { GPIOB, GPIO_PIN_0 },  // D
    { GPIOC, GPIO_PIN_1 },  // E
    { GPIOC, GPIO_PIN_0 },  // F
    { GPIOB, GPIO_PIN_8 },  // G
    { GPIOB, GPIO_PIN_9 },  // H (also called DP)
};

// for each digit, we have a byte (uint8_t) which stores which segments are on and off
// (bits are ABCDEFGH, right to left, so the low-order bit is segment A)
uint8_t digitmap[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7C, 0x07, 0x7F, 0x67 };

void Initialize7Segment() {
    for (int i = 0; i < 8; ++i)
        InitializePin(segments[i].port, segments[i].pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
}

void Display7Segment(int digit) {
    int value = 0;  // by default, don't turn on any segments
    if (digit >= 0 && digit <= 9)  // see if it's a valid digit
        value = digitmap[digit];   // convert digit to a byte which specifies which segments are on and which are off
    for (int i = 0; i < 8; ++i)    // go through the segments, turning them on or off depending on the corresponding bit
        HAL_GPIO_WritePin(segments[i].port, segments[i].pin, (value >> i) & 0x01);  // move bit into bottom position and isolate it
}