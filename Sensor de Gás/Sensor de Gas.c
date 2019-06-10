/*
 * File:   Sensor de Gas.c
 * Last Version: 1.00 22/12/2017 18:44
 * Author: Francisco Mateus Rocha Filho
 * Federal University of Ceará - IFCE, Fortaleza - Ceará.
 * Todos os Direitos Reservados
 * ---------------------------------------------------
 * Copyright - Este trabalho é protegido pelas leis de direitos autorais contidas na
 * constituição brasileira. É proibido qualquer tipo de venda deste material. As
 * informações contidas neste trabalho podem ser utilizadas para fins de estudo,
 * desde que previamente solicitada a autorização dos autores, desde que dentro
 * dessas leis.
 * 
 * Questões,comentários e solicitações podem ser enviadas ao email:
 * franciscomateus0119@gmail.com
 * ---------------------------------------------------
 *
 * Created on 22/02/2017, 14:04
 */

#include <xc.h>
#include <stdio.h>

//Configuracoes do pic
#pragma config MCLRE = OFF
#pragma config WDT = OFF
#pragma config PWRT = OFF
#pragma config LVP = OFF
#pragma config OSC = INTIO67

#define _XTAL_FREQ 8000000
#define led PORTDbits.RD0
#define rele PORTDbits.RD1



void __init(){
    OSCCON = 0b11110010;  // configura o oscilador interno para 8MHz
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    ADCON0 = 0b00000001;
    //         --0000--    select AN0 <- mudável
    //         ------01     GO/DONE (it's done)  
    //         -------1     hardware main screen on
    ADCON1 = 0b00001011;
    //         --0-----     use Vss as a ref
    //         ---0----     use Vdd as a ref
    //         ----1011     AN0-AN4 enabled <- mudável
    ADCON2 = 0b10111100;
    
    //pino RA0 como entrada
    TRISA = 0b00001111;

    TRISDbits.RD0 = 0; // configura o pino do encoder como entrada
    TRISDbits.RD1 = 0;
    led = 0;
    
}
void UART_init(void){
    TRISC = 0x00;
    TRISCbits.RC7= 1; // pino RX como entrada
    TRISCbits.RC6 = 1; // configura o TX como saída
    TXSTA = 0b00100100; // transmissão habilitada
    //        --1--1--;   Habilita o TXEN = 1 e BRGH = 1
    RCSTA = 0b10010000; //transmissão em alta velocidade
    //        1--1----;   Habilita o SPEN = 1 e CREN = 1
    BAUDCON = 0b00000000; //TX ocioso em nível alto
    SPBRG = 51;
    
}
void Transmite_d_UART(char dado){ // transmissão de caractere
    while(TXSTAbits.TRMT);
    TXREG = dado;
}
void Transmite_s_UART(char *string){
    while(*string){
        while(!PIR1bits.TXIF);
        TXREG = *string;
        string++;
    }
}
//void TDU(unsigned char c){
//    while(TXSTAbits.TRMT); // wait until transmit shief is empty
//    TXREG = c; // write character to txreg and start transmission
//}

//void TSU(unsigned char *s){
//    while(*s)
//    {
//        TDU(*s); // send character pointed by s
//        s++; // increase pointer location to the next character
//    }
//}

int a=0;
unsigned int ADC_Read(unsigned char channel) // Leitura da Porta
{
  if(channel > 7)              //Channel range is 0 ~ 7
    return 0;
  ADCON0 = 0b00000001;
  __delay_us(20);
  ADCON0bits.GO=1;
  while(ADCON0bits.GO==1);
  return ((ADRESH<<8)+ADRESL); 
}

void piscaled(void){
    for (a=0;a<5;a++){
        led = 1;
        __delay_ms(500);
        led = 0;
        __delay_ms(500);
    }
}

void main(){
    __init();
    __delay_ms(1000);
    UART_init();
    __delay_ms(1000);
    char inicio[] = "Iniciado com Sucesso!\n";
    char semgas[] = "Sem vazamento de gas!\n";
    char comgas[] = "Gas vazando! Verificar com urgencia!\n";
    char data[]="\r\n";
    char ID[]="ID: CflorAP504T2";
    Transmite_s_UART(inicio);//Avisa aos responsáveis que há um vazamento.
    //__delay_ms(100);
    Transmite_s_UART(data);
    Transmite_s_UART(ID);//Identifica o Local de Vazamento
    Transmite_s_UART(data);
    //TSU(inicio);
    unsigned int ar=0;
    unsigned int x=0;
    unsigned int gas=0;
    unsigned int y=0;
    int i=0;
    int j=0;
    int l=0;
    for(i=0;i<10;i++){
        x = ADC_Read(0);
        ar = ar + x;
    }
    ar = ar/10;
    
  
    
    while(1){
      gas = 0;
      while(gas<ar){
        for(j=0;j<10;j++){
            y = ADC_Read(0);
            gas = gas + y;
        }
        gas = gas/10;
      }
      if((gas-ar)>200){
        led = 1;
        __delay_ms(100);
        rele = 1;
        if(l==1){
            Transmite_s_UART(comgas);
            Transmite_s_UART(" -- ");
            Transmite_s_UART(ID);
            Transmite_s_UART(data);
            //TSU(semgas);
        }
        l=0;
      }
      else{
          led = 0;
          __delay_ms(100);
          rele = 0;
          if(l==0){
            Transmite_s_UART(semgas);
            Transmite_s_UART(data);
            //TSU(comgas);
          }
          l=1;
      }
      gas = 0;
    }
}