#include "soc-hw.h"

//Aqui se ponen las direcciones de los módulos

uart_t       	*uart0       	= (uart_t *)       	0x20000000;
timerH_t     	*timer0   	= (timerH_t *)     	0x40000000;
i2c_t	  	*i2c0		= (i2c_t *)		0x60000000;
fuente_t    	*fuente0    	= (fuente_t*)      	0x80000000;
//gpio_t       	*gpio0       	= (gpio_t *)       	0x40000000;

isr_ptr_t isr_table[32];

/***************************************************************************
 * IRQ handling
 */
void isr_null(void)
{
}

void irq_handler(uint32_t pending)
{     
    uint32_t i;

    for(i=0; i<32; i++) {
        if (pending & 0x00000001){
            (*isr_table[i])();
        }
        pending >>= 1;
    }
}

void isr_init(void)
{
    int i;
    for(i=0; i<32; i++)
        isr_table[i] = &isr_null;
}

void isr_register(int irq, isr_ptr_t isr)
{
    isr_table[irq] = isr;
}

void isr_unregister(int irq)
{
    isr_table[irq] = &isr_null;
}

/***************************************************************************
 * TIMER Functions
 */
uint32_t tic_msec;

void mSleep(uint32_t msec)
{
    uint32_t tcr;

    // Use timer0.1
    timer0->compare1 = (FCPU/1000)*msec;
    timer0->counter1 = 0;
    timer0->tcr1 = TIMER_EN;

    do {
        //halt();
         tcr = timer0->tcr1;
     } while ( ! (tcr & TIMER_TRIG) );
}

void uSleep(uint32_t usec)
{
    uint32_t tcr;

    // Use timer0.1
    timer0->compare1 = (FCPU/1000000)*usec;
    timer0->counter1 = 0;
    timer0->tcr1 = TIMER_EN;

    do {
        //halt();
         tcr = timer0->tcr1;
     } while ( ! (tcr & TIMER_TRIG) );
}

void tic_isr(void)
{
    tic_msec++;
    timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;
}

void tic_init(void)
{
    tic_msec = 0;

    // Setup timer0.0
    timer0->compare0 = (FCPU/10000);
    timer0->counter0 = 0;
    timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;

    isr_register(1, &tic_isr);
}


/***************************************************************************
 * UART Functions
 */
void uart_init(void)
{
    //uart0->ier = 0x00;  // Interrupt Enable Register
    //uart0->lcr = 0x03;  // Line Control Register:    8N1
    //uart0->mcr = 0x00;  // Modem Control Register

    // Setup Divisor register (Fclk / Baud)
    //uart0->div = (FCPU/(57600*16));
}

char uart_getchar(void)
{   
    while (! (uart0->ucr & UART_DR)) ;
    return uart0->rxtx;
}

void uart_putchar(char c)
{
    while (uart0->ucr & UART_BUSY) ;
    uart0->rxtx = c;
}

void uart_putstr(char *str)
{
    char *c = str;
    while(*c) {
        uart_putchar(*c);
        c++;
    }
}

/***************************************************************************
 * I2C Functions
 */
 
uint8_t i2c_read(uint32_t slave_addr, uint32_t per_addr)
{
		
	while(!(i2c0->scr & I2C_DR));		//Se verifica que el bus esté en espera
	i2c0->s_address = slave_addr;
	i2c0->s_reg 	= per_addr;
	i2c0->start_rd 	= 0x00;
	while(!(i2c0->scr & I2C_DR));
	return i2c0->i2c_rx_data;
}

void i2c_write(uint32_t slave_addr, uint32_t per_addr, uint32_t data){
	
	while(!(i2c0->scr & I2C_DR));		//Se verifica que el bus esté en espera
	i2c0->s_address = slave_addr;
	i2c0->s_reg 	= per_addr;
	i2c0->tx_data 	= data;
	i2c0->start_wr 	= 0x00;
}

/***************************************************************************
 * Fuente Functions
 */

uint32_t fuente_read_data(uint32_t addr){
	fuente0->addr_rd	= addr;
	fuente0->rd		= 1;
	fuente0->rd		= 0;
	return fuente0->d_out;
};

/***************************************************************************
 * Pantalla Functions
 */

void send_command_display(uint32_t addr, uint32_t command){
	i2c_write(addr, DISPLAY_COMMAND, command);
};

void send_data_display(uint32_t addr, uint32_t data){
	i2c_write(addr, DISPLAY_INDEX, data);
};

void sec_on_display(void){
        mSleep(1);
	send_command_display(DISPLAY_ADDR,0xAE); //OFF PANTALLA 
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0X20); // MODO DE DIRECCIONAMIENTO
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0x00);
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0xB0); // CUADRAR DIRECCIÓN INICIAL DE PAGINA
	mSleep(1);
        send_command_display(DISPLAY_ADDR,0xC8); //OUTPUT SCAN COM DIRECTORY
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x00); // --- SET LOW COLUMN ADDR ADDRES       
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x10); // --- SET HIGH COLUMN ADDR
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x40); // --- SET STAR LINE ADDR
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x81); // SET CONTRAST
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x3F); //
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xA1); // SET SEGMENT RE-MAP. A1=addr 127 MAPPED
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xA6); // SET DISPLAY MODE. A6=NORMAL
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xA8); // SET MUX RATIO
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x3F);
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xA4); // OUTPUT RAM TO DISPLAY
        //send_command_display(DISPLAY_ADDR,0xA5); // ENTIRE DISPLAY ON
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xD3); // DISPLAY OFFSET. 00= NO
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x00);
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xD5); //---SET DISPLAY CLOCK  DIVIDE RATIO /OSCILATOR
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xF0); //-- SET DIVIDE RATIO
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xD9); //SET PRE-CHARGUE PERIOD
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x22);
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xDA); //SET COM PINS 
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x12);
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xDB); //--SET VCOMH
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x20); //0x20,0.77xVcc
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x8D); //SET DC-DC ENABLE
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0x14); 
        mSleep(1);
        send_command_display(DISPLAY_ADDR,0xAF); //ON PANTALLA
        mSleep(1);
};	

void clear_GDRAM(void){
        int i, j;
        mSleep(1);
	send_command_display(DISPLAY_ADDR,0x21); //Configurar el direccionamiento por columna
        mSleep(1);
	send_command_display(DISPLAY_ADDR,0x00);
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0x7F);
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0x22); //Configurar el diferccionamiento por página
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0x00);
	mSleep(1);
	send_command_display(DISPLAY_ADDR,0x07);
	for(j=1;j<9;j++){
          	for (i=1;i<129;i++) {  
          		mSleep(1);
            		send_data_display(0x3C, 0x00);
          	}
        }
	
};

