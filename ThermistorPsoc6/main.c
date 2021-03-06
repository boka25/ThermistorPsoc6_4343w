#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "math.h"
#include "cy_retarget_io.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
/* Channel 0 input pin */
#define VPLUS_CHANNEL_0             (P10_1)


/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
/* Single channel initialization function*/
void adc_single_channel_init(void);

/* Function to read input voltage from channel 0 */
void adc_single_channel_process(_Bool);

/* Function change timer */
void adc_timer_change(int);

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
/* ADC Object */
cyhal_adc_t adc_obj;

/* ADC Channel 0 Object */
cyhal_adc_channel_t adc_chan_0_obj;

/* timer default 1 second value */
int timer_val = 1000;

/* read from UART */
uint8_t uart_read_value;

/* temperature flag */
bool temperature_active_flag = true;

/* Default ADC configuration */
const cyhal_adc_config_t adc_config = {
		.continuous_scanning=false, // Continuous Scanning is disabled
		.average_count=1,           // Average count disabled
		.vref=CYHAL_ADC_REF_VDDA,   // VREF for Single ended channel set to VDDA
		.vneg=CYHAL_ADC_VNEG_VSSA,  // VNEG for Single ended channel set to VSSA
		.resolution = 12u,          // 12-bit resolution
		.ext_vref = NC,             // No connection
		.bypass_pin = NC };       // No connection


/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 * This is the main function for CM4 CPU. It does...
 *    1. Configure and initialize ADC.
 *    2. Every "timer_val" read the voltage from analog pin and calculate temperature.
 *    Display temperature on UART.
 *
 * Parameters:
 *  none
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main(void)
{
	/* Variable to capture return value of functions */
	cy_rslt_t result;

	/* Initialize the device and board peripherals */
	result = cybsp_init();

	/* Board init failed. Stop program execution */
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	/* Enable global interrupts */
	__enable_irq();

	/* Initialize retarget-io to use the debug UART port */
	result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
			CY_RETARGET_IO_BAUDRATE);

	/* retarget-io init failed. Stop program execution */
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	/* Print message */
	/* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
	printf("\x1b[2J\x1b[;H");
	printf("-----------------------------------------------------------\r\n");
	printf("PSoC 6 MCU: ADC using HAL\r\n");
	printf("-----------------------------------------------------------\r\n\n");

	/* Initialize Channel 0 */
	adc_single_channel_init();


	/* Update ADC configuration */
	result = cyhal_adc_configure(&adc_obj, &adc_config);
	if(result != CY_RSLT_SUCCESS)
	{
		printf("ADC configuration update failed. Error: %ld\n", (long unsigned int)result);
		CY_ASSERT(0);
	}

	for (;;)
	{

		/* Sample input voltage at channel 0 */
		adc_single_channel_process(temperature_active_flag);

		/* timer_val in ms delay between scans */
		cyhal_system_delay_ms(timer_val);


		/* Check if 's' key was pressed */
		if (cyhal_uart_getc(&cy_retarget_io_uart_obj, &uart_read_value, 1)
				== CY_RSLT_SUCCESS)
		{
			switch (uart_read_value)
			{
			case 's':
			{
				printf("s pressed \r\n");

				/* Pause temperature output */
				if (temperature_active_flag)
				{
					printf("Temperature measure paused \r\n");
				}

				else /* Resume temperature output */
				{
					printf("Temperature measure resumed \r\n");
				}

				/* Move cursor to previous line */
				printf("\x1b[1F");

				temperature_active_flag ^= 1;
				break;
			}
			case '1':
			{
				printf("time set change to 1 second \r\n");
				adc_timer_change(1000);
				break;
			}
			case '2':
			{
				printf("time set change to 2 second \r\n");
				adc_timer_change(2000);
				break;
			}
			default:
			{
				printf("incorrect input \r\n");
				break;
			}
			}

		}
	}
}


/*******************************************************************************
 * Function Name: adc_single_channel_init
 *******************************************************************************
 *
 * Summary:
 *  ADC single channel initialization function. This function initializes and
 *  configures channel 0 of ADC.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void adc_single_channel_init(void)
{
	/* Variable to capture return value of functions */
	cy_rslt_t result;

	/* Initialize ADC. The ADC block which can connect to pin 10[0] is selected */
	result = cyhal_adc_init(&adc_obj, VPLUS_CHANNEL_0, NULL);
	if(result != CY_RSLT_SUCCESS)
	{
		printf("ADC initialization failed. Error: %ld\n", (long unsigned int)result);
		CY_ASSERT(0);
	}

	/* ADC channel configuration */
	const cyhal_adc_channel_config_t channel_config = {
			.enable_averaging = false,  // Disable averaging for channel
			.min_acquisition_ns = 1000, // Minimum acquisition time set to 1us
			.enabled = true };          // Sample this channel when ADC performs a scan

	/* Initialize a channel 0 and configure it to scan P10_0 in single ended mode. */
	result  = cyhal_adc_channel_init_diff(&adc_chan_0_obj, &adc_obj, VPLUS_CHANNEL_0,
			CYHAL_ADC_VNEG, &channel_config);
	if(result != CY_RSLT_SUCCESS)
	{
		printf("ADC single ended channel initialization failed. Error: %ld\n", (long unsigned int)result);
		CY_ASSERT(0);
	}

	printf("ADC is configured in single channel configuration\r\n\n");
	printf("Provide input voltage at pin P10_0. \r\n\n");
}

/*******************************************************************************
 * Function Name: adc_single_channel_process
 *******************************************************************************
 *
 * Summary:
 *  ADC single channel process function. This function reads the input voltage
 *  and calculates temperature and prints it in Celsius on UART.
 *  Input defines if temperature is printed.
 *
 * Parameters:
 *  boolean - print/not_print flag
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void adc_single_channel_process(_Bool fl)
{
	if(fl){

		float V1 = CY_CFG_PWR_VDDA_MV;					/* supply voltage */
		float R_const = 10000; 							/* 10k Reference Resistor */
		float B = 3455;									/* value is a material constant which is determined by the ceramic material from which it is made */
		float T1 = 298;									/* is the first temperature point in Kelvin */

		int32_t adc_result_0 = 0;
		adc_result_0 = cyhal_adc_read_uv(&adc_chan_0_obj)/1000;

		float V_out = adc_result_0 ;					/* output from pin P10_1 */

		float R2 = (R_const*V_out)/(V1-V_out);			/* Calculate resistance of thermistor */
		float logR = (float)log((float)(R_const/R2));	/* log(R_const/R2) */
		float T2 = B*T1/(B-logR*T1);					/* the second temperature point in Kelvin */
		float temperatureC = T2 - 273;					/* temperature point in Celsius */

		printf("TEMPERATURE %.2f Celsius\r\n", temperatureC);
	}
}

/*******************************************************************************
 * Function Name: adc_timer_change
 *******************************************************************************
 *
 * Summary:
 *  Change time reading data from P10_1.
 *
 * Parameters:
 *  int - sample rate in seconds
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void adc_timer_change(int sec){
	timer_val = sec;
}

/* [] END OF FILE */
