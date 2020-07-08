#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/cm3/nvic.h>
#include <math.h>
#include <time_stm32.h>

#define M_2PI 6.28318530718

volatile int flag_amostra = 0;

void gpio_setup(void);
void tim2_setup(void);
void tim4_setup(void);
uint8_t floatToChar(float);
float calculaIncremento(uint16_t,uint8_t);

void tim2_isr(void)
{
	//gpio_toggle(GPIOC, GPIO13); /* LED on/off. */
	flag_amostra = 1; //Indica que é o momento de amostrar
	/* Manutenção do interrupt */
	timer_clear_flag(TIM2,TIM_SR_CC1IF); /* Clear interrrupt flag. */
	timer_set_counter(TIM2, 0); /* Set counter back to the beginning */
}


void gpio_setup(void) {

	/* Enable clock. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	/* Set GPIO13 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
	gpio_set(GPIOC,GPIO13);


    /***************  Portas dos PWM *******************/

	/* Set GPIOB (in GPIO port B) to 'output alternate function'. */
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO6);
	
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO7);

}
void tim2_setup(void){
	
	//Configurado para 50kHz de frequência de atualização - 20us de taxa de amostragem	

	rcc_periph_clock_enable(RCC_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, 72000000/500000); // frequencia do tick é o denominador
	timer_set_period(TIM2, 10); // quantos ticks dão um período
	timer_set_oc_value(TIM2, TIM_OC1, 10); //contagem até o período p/ trigar o interrupt
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);

//NVIC
	
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_set_priority(NVIC_TIM2_IRQ,1);

	timer_enable_counter(TIM2);	
}


void tim4_setup(void){ //Timer do PWM

	rcc_periph_clock_enable(RCC_TIM4);

	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM4, 72000000/(18900000)); // frequencia do tick é o denominador
	timer_set_period(TIM4, 126); // quantos ticks dão um período

	timer_set_oc_mode(TIM4,TIM_OC1,TIM_OCM_PWM1); 
	timer_enable_oc_output(TIM4,TIM_OC1);

	timer_set_oc_mode(TIM4,TIM_OC2,TIM_OCM_PWM2); 
	timer_enable_oc_output(TIM4,TIM_OC2);

	//Importante: se um PWM tem valor, o outro deve ser zero
	//Nessa configuração acredito que os dois tão desativados
	timer_set_oc_value(TIM4, TIM_OC1, 0); //PB6 
	timer_set_oc_value(TIM4, TIM_OC2, 127); //PB7 (invertido)
	timer_enable_counter(TIM4);	
}

uint8_t floatToChar(float sinFloat)
{
	//sinFloat entra aqui entre -1 e 1 e sai entre 0 e 255
	uint8_t out = (uint8_t)(255*(sinFloat+1)/2);
	return out;
}

float calculaIncremento(uint16_t freq,uint8_t pontos)
{
	return 1000000/(pontos*freq); // Em us
}


int main(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz(); 

	gpio_setup();
	tim2_setup();
	tim4_setup();
	time_init(); //Pra usar a biblioteca de delays
	delay_ms(20);


	/* Parâmetros gerais não configuráveis */	
	uint8_t saida_char = (uint8_t)0; //Valor do seno após conversão de float pra inteiro
	uint8_t qtd_pontos = 64;
	char seno_tabela[qtd_pontos];
	uint8_t t_ms_att_f = 100; //Tempo em ms de atualização das frequências

	/* Parâmetros do experimento configuráveis externamente */
	uint16_t freq_inicial = 30;
	uint16_t freq_final = 1000;
	uint16_t T_experimento = 5000; //Em ms

	/* Consequência da modificação dos parâmetros externos */
	uint16_t f_incremento = (uint16_t)(t_ms_att_f*(freq_final-freq_inicial)/T_experimento);
	int dir = 1; //Direção de mudança da frequência


	int i; //Iterador para gerar o seno
	int cont = 0; //Contagem de pontos do seno_tabela considerados
	for (i=0;i<qtd_pontos;i++) //Gerando o seno - so precisa ocorrer uma vez
	{
		seno_tabela[i] = floatToChar(sin(M_2PI*i/qtd_pontos));
	}
	i = 0;


	gpio_clear(GPIOC,GPIO13);

	/* Parâmetros que mudam automaticamente */
	uint16_t freq_alvo = freq_inicial;
	float T_incremento = calculaIncremento(freq_alvo,qtd_pontos);
	float t_us= 0; //Tempo atual em us
	float t_ms = 0; //Tempo atual em ms


	/* Começo do programa */
	delay_ms(10);
	while(1)
	{
		if (flag_amostra == 1) //Bateu o timer de amostra, atualizo o valor do seno
		{
			i = i+1; //Incremento de um tick
			t_us = t_us+12.5; //Incremento de um tick em us
			if (t_us>=cont*T_incremento) // Se for hora de atualizar o seno, procura novo valor. se nao repete o mesmo
			{
				saida_char = seno_tabela[cont];
				//Atualização do PWM
				
				if (saida_char>127)
				{
					timer_set_oc_value(TIM4, TIM_OC1, 255-saida_char); //PB6
					timer_set_oc_value(TIM4, TIM_OC2, 127); //PB7
				}
				else
				{
					timer_set_oc_value(TIM4, TIM_OC1, 0); //PB6
					timer_set_oc_value(TIM4, TIM_OC2, 127-saida_char); //PB7
				}
				


				cont++;
			}

			if (cont==qtd_pontos) // Se o tempo atual for igual ao período, o seno se repete
			{
				t_ms = t_ms+64*T_incremento/1000; //Incremento o tempo atual em ms

				if (t_ms>=t_ms_att_f)
				{
					freq_alvo = freq_alvo + dir * f_incremento;
					if (freq_alvo>freq_final)
					{
						dir = -1;
						freq_alvo = freq_final;
					}
					else if(freq_alvo<freq_inicial)
					{
						dir = 1;
						freq_alvo = freq_inicial;
					}

					T_incremento = calculaIncremento(freq_alvo,qtd_pontos);
					t_ms = 0;
				}
				t_us = 0;
				cont = 0;
				i = 0;
			}		

			gpio_toggle(GPIOC, GPIO13);
			flag_amostra = 0; //Fim da amostragem
		}
	}
	return 0;
}
