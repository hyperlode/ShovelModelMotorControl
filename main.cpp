

//notes: to convert to cpp:
//-change name
//-configuration -> compile "use CPP"
//-configurations-> link ->"don't use the standard system startup files"


#define HSE_VALUE ((uint32_t)8000000) /* STM32 discovery uses a 8Mhz external crystal */



#include "main.h"



#ifdef USE_VCP
/*
 * The USB data must be 4 byte aligned if DMA is enabled. This macro handles
 * the alignment, if necessary (it's actually magic, but don't tell anyone).
 */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

#endif


/*
 * Define prototypes for interrupt handlers here. The conditional "extern"
 * ensures the weak declarations from startup_stm32f4xx.c are overridden.
 */
#ifdef __cplusplus
 extern "C" {
#endif
 void init();
 void ColorfulRingOfDeath(void);
void setUpHardWareInterrupt_motor1_channelA();
void setUpInputPin_motor1_channelB();
//motor2
void setUpHardWareInterrupt_motor2_channelA();
void setUpInputPin_motor2_channelB();


void SysTick_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
#ifdef USE_VCP
void OTG_FS_IRQHandler(void);
void OTG_FS_WKUP_IRQHandler(void);
#endif

#ifdef __cplusplus
}
#endif

int main(void)
{
	/* Set up the system clocks */
	SystemInit();

	/* Initialize USB, IO, SysTick, and all those other things you do in the morning */
	init();

	initDiscoveryBoard();



	//panel 1
	//IOBoard panel1(PANEL_1);
	//panel1.initADC();
	//panel1.initButtons();
	//panel1.initLeds();
	//IOBoardHandler[0] = &panel1; //link the panel instance to the handler.
	//for (uint16_t i = 0;i<4;i++){
	//	panel1.setLed(i,false);
	//}

/*
	//panel 2
	IOBoard panel2(PANEL_2);
	panel2.initADC();
	panel2.initButtons();
	panel2.initLeds();
	IOBoardHandler[1] = &panel2; //link the panel instance to the handler.

	//panel 3
	IOBoard panel3(PANEL_3);
	panel3.initLeds();
	panel3.initButtons();
*/
	//panel4
	IOBoard panel4(PANEL_4);
	panel4.initLeds();
	panel4.initButtons();
	IOBoardHandler[3] = &panel4; //link the panel instance to the handler.


	for (uint16_t i = 0;i<16;i++){

		//panel4.setLed(i,true);
		panel4.setLed(i,false);
	}
	panel4.ledSequenceInterruptHandler(false);

	//motor1
	setUpHardWareInterrupt_motor1_channelA();
	setUpInputPin_motor1_channelB();
	//motor2
	setUpHardWareInterrupt_motor2_channelA();
	setUpInputPin_motor2_channelB();


	//motor1 hoist
	MotorControl motor1(1);
	MotorControlHandles[0] = &motor1;

	//motor2 crowd
	MotorControl motor2(2);
	MotorControlHandles[1] = &motor2;



	//INIT mode for motorcontrollermode
	motorControllerMode = MODE_NORMAL;
	panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
	motorControllerMode  = 0;
	for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
		MotorControlHandles[i]->setMode(motorControllerMode);
	}
//	panel1.setLed(3,true);
//	panel1.setLedBlinkPeriodMillis(3,0);


#ifdef USE_VCP
	printf("Userinterface: \r\n");
	printf("To interact. Please send v, s, m or a  \r\n");
#endif

	while (1)
	{
		//panel1.refresh(millis);
		panel4.refresh(millis);
		//panel1.demoModeLoop();
		//panel4.demoModeLoop();

		//MOTOR CONTROL test

		//select mode with button4 on panel
		if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_SELECT_MODE)){
				motorControllerMode++;
				if (motorControllerMode>2){
					motorControllerMode  = 0;
				}

				switch (motorControllerMode){
					case MODE_NORMAL:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
						panel4.setLedBlinkPeriodMillis(LED_MOTORCONTROLLER_MODE,0);
						break;
					case MODE_TEST:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
						panel4.setLedBlinkPeriodMillis(LED_MOTORCONTROLLER_MODE,1000);
						break;
					case MODE_CALIBRATE:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
						panel4.setLedBlinkPeriodMillis(LED_MOTORCONTROLLER_MODE,250);
						break;
					default:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,false);
						break;
				}

				//update the motors
				for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
					MotorControlHandles[i]->setMode(motorControllerMode);
				}

		}


		switch (motorControllerMode){
			case MODE_NORMAL:
				break;
			case MODE_TEST:
				break;
			case MODE_CALIBRATE:
				//if calibration selected select limit and set limit buttons active
				if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_SELECT_LIMIT_FOR_SETTING)){
					//select limit to configure

					activeLimit = MotorControlHandles[activeMotorForTestingOrCalibration]->getSelectedLimitForCalibration();

					if (activeLimit >= 2){
						MotorControlHandles[activeMotorForTestingOrCalibration]->selectLimitToBeCalibrated(0);
						activeMotorForTestingOrCalibration++;
						activeLimit = 0;//because of ++ will be set to 1 further down this routine

					}

					if (activeMotorForTestingOrCalibration >= NUMBER_OF_MOTORS){
						activeMotorForTestingOrCalibration = 0;
					}
					activeLimit++;
					printf("%d-",activeMotorForTestingOrCalibration);
					printf("%d\r\n",activeLimit);
					MotorControlHandles[activeMotorForTestingOrCalibration]->selectLimitToBeCalibrated(activeLimit);
				}
				if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_SET_SELECTED_LIMIT_TO_CURRENT_POSITION)){
					MotorControlHandles[activeMotorForTestingOrCalibration]->setCurrentPositionAsLimit();
				}
				if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_RESET_ALL_LIMITS)){
					MotorControlHandles[activeMotorForTestingOrCalibration]->resetLimit();
				}

				break;
			default:
				panel4.setLed(LED_MOTORCONTROLLER_MODE,false);
				break;
		}



		if (millis%10 > 5 && edgeMemory ==0){
			edgeMemory =1;
			panel4.setLed(LED_MOTOR_HOIST_LIMIT_MIN,motor1.getStatusLed(LED_LIMIT_MIN,millis));
			panel4.setLed(LED_MOTOR_HOIST_INRANGE,motor1.getStatusLed(LED_WITHIN_RANGE, millis));
			panel4.setLed(LED_MOTOR_HOIST_LIMIT_MAX,motor1.getStatusLed(LED_LIMIT_MAX, millis));

			panel4.setLed(LED_MOTOR_CROWD_LIMIT_MIN,motor2.getStatusLed(LED_LIMIT_MIN,millis));
			panel4.setLed(LED_MOTOR_CROWD_INRANGE,motor2.getStatusLed(LED_WITHIN_RANGE, millis));
			panel4.setLed(LED_MOTOR_CROWD_LIMIT_MAX,motor2.getStatusLed(LED_LIMIT_MAX, millis));

		}

		if (millis%10 <10){
			edgeMemory = 0;
		}



		//each second triggered
		if (millis%1000 >= 500 && secondEdgeMemory ==0){
			secondEdgeMemory = 1;

			/*
			for (uint16_t i = 0;i<4;i++){
				if (panel1.getButtonState(i)){
					printf("button %d pressed!\r\n", i);
					printf("buttonToggle Switch value: %d \r\n",panel1.getButtonValueToggleSwitch(i));
				}
			}

			for (uint16_t i = 0;i<4;i++){
				if (panel2.getButtonState(i)){
					printf("panel 2 button %d pressed!\r\n", i);
				}
			}
			*/
			STM_EVAL_LEDToggle(LED3) ;
		}

		//edge handling
		if (millis%1000 < 100){
			secondEdgeMemory = 0;
		}



#ifdef USE_VCP
		// If there's data on the virtual serial port:
		 //  - Echo it back
		 //  - Turn the green LED on for 10ms
		 //
		uint8_t theByte;
		if (VCP_get_char(&theByte))
		{
			if ( theByte != '\r' &&  theByte != '\n'){
				printf("Char Sent: %c  \r\n", theByte); //VCP_put_char(theByte);

				if ( theByte == 'v'){
					//printf ("value %d /r/n", ConvertedValue);

					printf ("samples taken: %d \r\n", adcNumberOfSampleCycles);
					printf ("value TEMPERATURE %d \r\n", temp);
					printf ("value VREF %d \r\n", vref);

					for (uint8_t i=0; i<4;i++){
						//printf ("value slider: %d = %d \r\n", i, adcValues[i]);
						//printf ("panel1 slider %d: %d \r\n", i, panel1.getSliderValue(i));

					}
					/*
					for (uint8_t i=0; i<4;i++){
						//printf ("value slider: %d = %d \r\n", i, adcValues[i]);
						printf ("panel2 slider  %d: %d \r\n", i, panel2.getSliderValue(i));
					}
*/
				}else if (theByte == 's'){
					//panel1.stats(lodeStrTest);
					//printf ("lets do this: %s \r\n", lodeStrTest);

				}else if (theByte == 'a') {
					printf("Doing some action here. \r\n");
				}else if (theByte == 'm'){
					printf("motor id: %d \r\n", motor2.getMotorId());
					printf("position: %d \r\n", motor2.getPosition());
					printf("limits:  min:  %d  --  max: %d \r\n", motor2.getLimit(false), motor2.getLimit(true));
				}else{
					//IOBoard testje;
					printf("No valid command detected. Please send v, \r\n s, \r\nm for motor status  ,\r\nor a . \r\n");
				}
			}
		}

#endif
	}


}


void initDiscoveryBoard(){
	//init the leds on the discoveryboard
	STM_EVAL_LEDInit(LED5);
	STM_EVAL_LEDOn(LED5);

	STM_EVAL_LEDInit(LED3);

	STM_EVAL_LEDInit(LED4);


	STM_EVAL_LEDInit(LED6);
	STM_EVAL_LEDOff(LED6);

}
#ifdef __cplusplus
 extern "C" {
#endif
void init()
{
	/* STM32F4 discovery LEDs */
	//GPIO_InitTypeDef LED_Config;

	/* Always remember to turn on the peripheral clock...  If not, you may be up till 3am debugging... */
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	//LED_Config.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	//LED_Config.GPIO_Mode = GPIO_Mode_OUT;
	//LED_Config.GPIO_OType = GPIO_OType_PP;
	//LED_Config.GPIO_Speed = GPIO_Speed_25MHz;
	//LED_Config.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//GPIO_Init(GPIOD, &LED_Config);




	// GPIOD Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	//Setup SysTick or CROD!
	if (SysTick_Config(SystemCoreClock / 1000))
	{
		ColorfulRingOfDeath();
}

#ifdef USE_VCP
	// Setup USB
	USBD_Init(&USB_OTG_dev,
	            USB_OTG_FS_CORE_ID,
	            &USR_desc,
	            &USBD_CDC_cb,
	            &USR_cb);
#endif
	return;

}


/*
 * Call this to indicate a failure.
 */

void ColorfulRingOfDeath(void){
	STM_EVAL_LEDOn(LED6);
	while (1)
	{
		//get stuck here forever.

	}

}

/*
 * Interrupt Handlers
 */

void SysTick_Handler(void)
{
	ticker++;
	if (downTicker > 0)
	{
		downTicker--;
	}

	ticker20ms++;
	millis++;

}

void NMI_Handler(void)       {}
void HardFault_Handler(void) { ColorfulRingOfDeath(); }
void MemManage_Handler(void) { ColorfulRingOfDeath(); }
void BusFault_Handler(void)  { ColorfulRingOfDeath(); }
void UsageFault_Handler(void){ ColorfulRingOfDeath(); }
void SVC_Handler(void)       {}
void DebugMon_Handler(void)  {}
void PendSV_Handler(void)    {}

#ifdef USE_VCP
void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}

#endif

/*
 * void adc_multiChannelConfigure(){

}
*/
void ADC_IRQHandler() {
        /* acknowledge interrupt */
        uint16_t value;
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);


        value = ADC_GetConversionValue(ADC1);
		switch (adcSampleChannelCounter){
		   case 0:
				temp = value;
				adcSampleChannelCounter++;
				break;
		   case 1:
				vref = value;
				adcSampleChannelCounter++;
				break;

			//all the other channels.
		   default:
				if (adcSampleChannelCounter<6){
					IOBoardHandler[0]->ADCInterruptHandler(adcSampleChannelCounter - 2, value); //IOBoard handle triggers.
				}else{
					//set adcSampleChannelCounter to 10IOBoardHandler[1]->ADCInterruptHandler(adcSampleChannelCounter - 6, value); //IOBoard handle triggers.
				}

				adcSampleChannelCounter++;
				if (adcSampleChannelCounter ==6){
					adcSampleChannelCounter =0;
					adcNumberOfSampleCycles++;
				}
				break;

		}



}


void setUpInputPin_motor2_channelB(){
	//PB8
	 /* Set variables used */
		GPIO_InitTypeDef GPIO_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;

		/* Enable clock for GPIOB */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		/* Enable clock for SYSCFG */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		/* Set pin as input */
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
		//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void setUpHardWareInterrupt_motor2_channelA(){
	//PB4
	//https://stm32f4-discovery.net/2014/08/stm32f4-external-interrupts-tutorial/

	 /* Set variables used */
	    GPIO_InitTypeDef GPIO_InitStruct;
	    EXTI_InitTypeDef EXTI_InitStruct;
	    NVIC_InitTypeDef NVIC_InitStruct;

	    /* Enable clock for GPIOB */
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	    /* Enable clock for SYSCFG */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	    /* Set pin as input */
	    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	    //GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStruct);


	    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

	    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
	    /* Enable interrupt */
	    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	    /* Interrupt mode */
	    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	    /* Triggers on rising and falling edge */
	    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	    //EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	    /* Add to EXTI */
	    EXTI_Init(&EXTI_InitStruct);

	    /* Add IRQ vector to NVIC */
	    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	    /* Set priority */
	    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	    /* Set sub priority */
	    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	    /* Enable interrupt */
	    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	    /* Add to NVIC */
	    NVIC_Init(&NVIC_InitStruct);


}


void EXTI4_IRQHandler(void) {

	//triggers on rising and falling edge of encoder channel
	//we are not interested in the added accuracy, but we need to check the edges (jitter at standstill could cause erroneous possition change)
	//edge up --> position change ,(only if channel 2 is different from edge down value)
	//edge down --> store channel 2
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) { //Make sure that interrupt flag is set
    	//printf ("motor2\r\n");
    	bool isCCW = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);//check channel B
    	//printf("chB: %d\r\n", isCCW);
    	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)){
    		//positive edge
    		if (isCCW != motor2_chBMemory){
				IOBoardHandler[3]->ledSequenceInterruptHandler(!isCCW); //input defines direction
				MotorControlHandles[1]->updatePositionOneStep(isCCW); //2 channel encoder update.
    		}
		}else{
			//negative edge
			motor2_chBMemory = isCCW; //store ch2.
    	}
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line4);

    }
}










//---------------------------------------------------------------------------------------





void setUpInputPin_motor1_channelB(){
	//PB5
	 /* Set variables used */
		GPIO_InitTypeDef GPIO_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;

		/* Enable clock for GPIOB */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		/* Enable clock for SYSCFG */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		/* Set pin as input */
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
		//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);


		//GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);

}

void setUpHardWareInterrupt_motor1_channelA(){
	//PB3
	//https://stm32f4-discovery.net/2014/08/stm32f4-external-interrupts-tutorial/

	 /* Set variables used */
	    GPIO_InitTypeDef GPIO_InitStruct;
	    EXTI_InitTypeDef EXTI_InitStruct;
	    NVIC_InitTypeDef NVIC_InitStruct;

	    /* Enable clock for GPIOB */
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	    /* Enable clock for SYSCFG */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	    /* Set pin as input */
	    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	    //GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStruct);

	    /* Tell system that you will use PB3 for EXTI_Line3 */
	    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource3);

	    /* PB3 is connected to EXTI_Line3 */
	    EXTI_InitStruct.EXTI_Line = EXTI_Line3;
	    /* Enable interrupt */
	    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	    /* Interrupt mode */
	    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	    /* Triggers on rising and falling edge */
	    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	    //EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	    /* Add to EXTI */
	    EXTI_Init(&EXTI_InitStruct);

	    /* Add IRQ vector to NVIC */
	    /* PB12 is connected to EXTI_Line12, which has EXTI15_10_IRQn vector */
	    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	    /* Set priority */
	    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	    /* Set sub priority */
	    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	    /* Enable interrupt */
	    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	    /* Add to NVIC */
	    NVIC_Init(&NVIC_InitStruct);


}

/* Set interrupt handlers */
/* Handle PB3 interrupt */
void EXTI3_IRQHandler(void) {

	//triggers on rising and falling edge of encoder channel
	//we are not interested in the added accuracy, but we need to check the edges (jitter at standstill could cause erroneous possition change)
	//edge up --> position change ,(only if channel 2 is different from edge down value)
	//edge down --> store channel 2
    if (EXTI_GetITStatus(EXTI_Line3) != RESET) { //Make sure that interrupt flag is set
    	printf ("checkkflelfl;f");
    	bool isCCW = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
    	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)){
    		//positive edge
    		if (isCCW != ch2Memory){
				IOBoardHandler[3]->ledSequenceInterruptHandler(!isCCW); //input defines direction
				MotorControlHandles[0]->updatePositionOneStep(isCCW); //2 channel encoder update.
    		}
		}else{
			//negative edge
			ch2Memory = isCCW; //store ch2.
    	}
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line3);

    }
}


#ifdef __cplusplus
 }
#endif
