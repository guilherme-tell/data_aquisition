///* ============================================================
// *  Processamento Digital de Sinais – SWRDFT e Filtragem FIR
// *  Plataforma: STM32F446 (ARM Cortex-M4F)
// * ============================================================ */
//
//#include <stdio.h>
//#include <math.h>
//#include "stm32f446xx.h"
//#include "arm_math.h"
//
//#include "signals.h"
//#include "uart.h"
//#include "adc.h"
//#include "tim.h"
//#include "fpu.h"
//#include "clock.h"
//#include "IEC60255.h"
//
///* ============================================================
// *  Constantes e Definições de Configuração
// * ============================================================ */
//#define FS              7680.0f
//#define F0              60.0f
//#define NPPC            (int)(FS / F0)     // 128 amostras por ciclo
//#define N_CICLOS        300
//#define N_SAMPLES       (N_CICLOS * NPPC)
//
//#define PI              3.1415926535897932f
//#define DOIS_PI         6.2831853071795864f
//#define DEG2RAD         0.0174532925f
//
//#define J_WINDOW        (1 * NPPC + 1)         // Tamanho da janela do filtro Savitzky-Golay
//#define NJAN            (SWRDFT_LEN * NPPC)
//#define DF_HZ			(FS/(float32_t)NJAN)
//
///* ============================================================
// *  Estruturas de Dados (Isolamento de Escopo)
// * ============================================================ */
//
//// Estado da SWRDFT
//typedef struct {
//    float32_t re_ant;
//    float32_t im_ant;
//    float32_t phi_ant;
//    float32_t fase_acumulada;
//} SWRDFT_State_t;
//
//// Buffers Circulares Manuais
//typedef struct {
//    float32_t amostras[NJAN];
//    float32_t fase_ciclo[NPPC];
//    float32_t mag_sg[J_WINDOW];    // Buffer para Savitzky-Golay
//    float32_t delfi_sg[J_WINDOW];  // Buffer para Savitzky-Golay
//    float32_t phi_sg[J_WINDOW];    // Buffer para Savitzky-Golay
//
//    uint32_t idx_delfi;
//    uint32_t idx_janela;
//    uint32_t idx_j;
//} Buffers_t;
//
//// Grandezas Estimadas
//typedef struct {
//    float32_t magnitude;
//    float32_t frequencia;
//    float32_t freq_prev;
//    float32_t fase;
//    float32_t rocof;
//} Phasor_Data_t;
//
///* ============================================================
// *  Variáveis Globais e Instâncias
// * ============================================================ */
//
//static const uint32_t harmonic_order = 1U;
//static harmonic_bin;
//volatile uint8_t g_process_flg = 0;
//uint32_t g_sample_idx = 0;
//uint32_t g_send_counter = 0;
//
//SWRDFT_State_t dft_state = {0};
//Buffers_t      buffers   = {0};
//Phasor_Data_t  phasor    = {0};
//
//IEC60255_Frame_t frame_tx;
//
//// Coeficientes do Filtro Savitzky-Golay (Devem ser pré-calculados conforme a ordem e janela)
//extern const float32_t SG_COEFFS[J_WINDOW];
//
//extern float32_t cc[NJAN];
//extern float32_t ss[NJAN];
//extern float32_t signal_data[N_SAMPLES];
//
///* ============================================================
// *  Protótipos Privados
// * ============================================================ */
//static void System_Init(void);
//static void Process_SWRDFT(float32_t current_sample);
//static void Send_UART_Frame(void);
//void UART_SendStruct(IEC60255_Frame_t *frame);
//
///* ============================================================
// *  Função Principal
// * ============================================================ */
//int main(void)
//{
//    System_Init();
//
//    while (1)
//    {
//        if (g_process_flg)
//        {
//            g_process_flg = 0; // Limpa flag o mais rápido possível
//
//            if (g_sample_idx < N_SAMPLES)
//            {
//                // 1. Aquisição
//                float32_t x_n = signal_data[g_sample_idx];
//
//                // 2. Processamento (Algoritmo Principal)
//                Process_SWRDFT(x_n);
//
//                // 3. Incremento e Envio
//                g_send_counter++;
//                if (g_send_counter >= NPPC) // Envia a 60 FPS
//                {
//                    g_send_counter = 0;
//                    Send_UART_Frame();
//                }
//
//                g_sample_idx++;
//            }
//            else
//            {
//                g_sample_idx = 0; // Reinicia para loop contínuo de teste
//            }
//        }
//    }
//}
//
///* ============================================================
// *  Lógica de Processamento e Filtragem (FIR SG)
// * ============================================================ */
//static void Process_SWRDFT(float32_t current_sample)
//{
//    // Variáveis locais para cálculos intermediários
//    float32_t re_X, im_X, mag_DFT, phi_DFT;
//    float32_t delta_phi_sample, Xfase;
//
//    // Amostra mais antiga na janela da DFT
//    float32_t sample_old = buffers.amostras[buffers.idx_janela];
//
//    // Para rastreamento de inter-harmônicos ou componentes específicos (ex: 140/150 Hz),
//    // a variável 'h' ou a lógica do índice base da exponencial (cc/ss) deve ser ajustada.
//    uint32_t base_idx = (harmonic_bin * buffers.idx_janela) % NJAN;
//
//    /* --- SWRDFT --- */
//    re_X = dft_state.re_ant + (current_sample - sample_old) * cc[base_idx];
//    im_X = dft_state.im_ant + (current_sample - sample_old) * ss[base_idx];
//
//    if (re_X == 0.0f) re_X = 1e-9f;
//
//    arm_sqrt_f32(re_X * re_X + im_X * im_X, &mag_DFT);
//    phi_DFT = atan2f(-im_X, re_X);
//
//    /* --- Descontinuidade de Fase (Unwrapping) --- */
//    delta_phi_sample = phi_DFT - dft_state.phi_ant;
//    if (delta_phi_sample > PI)       delta_phi_sample -= DOIS_PI;
//    else if (delta_phi_sample < -PI) delta_phi_sample += DOIS_PI;
//
//    dft_state.fase_acumulada += delta_phi_sample;
//    Xfase = dft_state.fase_acumulada;
//
//    /* --- Diferença de Fase por Ciclo --- */
//    float32_t Delfi = 0.0f;
//    if (g_sample_idx > (NPPC - 1)) {
//        Delfi = (Xfase - buffers.fase_ciclo[buffers.idx_delfi]) * (180.0f / PI);
//    }
//
//    // Salva históricos nos buffers
//    dft_state.re_ant  = re_X;
//    dft_state.im_ant  = im_X;
//    dft_state.phi_ant = phi_DFT;
//
//    buffers.amostras[buffers.idx_janela]     = current_sample;
//    buffers.fase_ciclo[buffers.idx_delfi]    = Xfase;
//    buffers.delfi_sg[buffers.idx_j]          = Delfi;
//    buffers.mag_sg[buffers.idx_j]            = mag_DFT;
//
//    /* --- Filtragem FIR (Savitzky-Golay com Buffer Circular Manual) --- */
//    // Substitui a média móvel padrão pela soma de convolução do SG
//    float32_t Delfimed_SG = 0.0f;
//    float32_t Xmed_SG = 0.0f;
//
//    for (uint32_t i = 0; i < J_WINDOW; i++) {
//        // Cálculo do índice circular reverso (ou direto, dependendo dos coeficientes)
//        uint32_t idx_fir = (buffers.idx_j + J_WINDOW - i) % J_WINDOW;
//
//        Delfimed_SG += buffers.delfi_sg[idx_fir] * SG_COEFFS[i];
//        Xmed_SG     += buffers.mag_sg[idx_fir]   * SG_COEFFS[i];
//    }
//
//    /* --- Estimativa de Frequência e Amplitude --- */
//    float32_t Delf = (F0 / 360.0f) * Delfimed_SG;
//    float32_t Hmod, Hfase;
//
//    if (fabsf(Delf) < 1e-6f) {
//        Hmod = 1.0f;
//        Hfase = 0.0f;
//    } else {
//        Hmod  = (1.0f / NJAN) * fabsf(sinf(NJAN * PI * Delf / FS) / sinf(PI * Delf / FS));
//        Hfase = -(NJAN - 1) * PI * Delf / FS;
//    }
//
//    phasor.frequencia = F0 + Delf;
//    phasor.magnitude  = (2.0f / NJAN) * Xmed_SG / Hmod;
//
//    // Atualiza ROCOF iterativamente (agora fora do bloco if para precisão contínua, ou deixado para o loop de TX)
//    phasor.rocof = (phasor.frequencia - phasor.freq_prev) * FS;
//    phasor.freq_prev = phasor.frequencia;
//
//    /* --- Correção de Fase --- */
//    float32_t phi_bruto = (Xfase - Hfase) * (180.0f / PI);
//    buffers.phi_sg[buffers.idx_j] = phi_bruto;
//
//    // Aplicação do filtro Savitzky-Golay também na fase, se desejado
//    float32_t phimed_SG = 0.0f;
//    for (uint32_t i = 0; i < J_WINDOW; i++) {
//        uint32_t idx_fir = (buffers.idx_j + J_WINDOW - i) % J_WINDOW;
//        phimed_SG += buffers.phi_sg[idx_fir] * SG_COEFFS[i];
//    }
//
//    phasor.fase = phimed_SG;
//
//    /* --- Atualização de Índices --- */
//    buffers.idx_delfi  = (buffers.idx_delfi + 1) % NPPC;
//    buffers.idx_janela = (buffers.idx_janela + 1) % NJAN;
//    buffers.idx_j      = (buffers.idx_j + 1) % J_WINDOW;
//}
//
///* ============================================================
// *  Preparação e Envio do Pacote UART
// * ============================================================ */
//static void Send_UART_Frame(void)
//{
//    frame_tx.frame_id = 0xAAAAAAAA; // Sync word para o MATLAB
//    frame_tx.amp = phasor.magnitude;
//    frame_tx.rfe = phasor.rocof;
//
//    // Garante que o wrap seja mantido após a soma do delay_comp
//	phasor.fase = fmodf(phasor.fase, 360.0f);
//	if (phasor.fase > 180.0f) {
//		phasor.fase -= 360.0f;
//	} else if (phasor.fase < -180.0f) {
//		phasor.fase += 360.0f;
//	}
//
//    frame_tx.fe  = phasor.frequencia;
//    frame_tx.phi = phasor.fase;
//
//    // Conversão Retangular com fase corrigida
//    float32_t phi_rad = phasor.fase * DEG2RAD;
//    frame_tx.re = phasor.magnitude * arm_cos_f32(phi_rad);
//    frame_tx.im = phasor.magnitude * arm_sin_f32(phi_rad);
//
//    UART_SendStruct(&frame_tx);
//}
//
///* ============================================================
// *  Inicializações de Hardware
// * ============================================================ */
//static void System_Init(void)
//{
//    fpu_enable();
//    clock_config();
//    uart2_tx_init();
//    pa1_adc_init();
//    gpio_interrupt();
//
//    harmonic_bin = roundf(((float32_t)harmonic_order * F0) / DF_HZ);
//
//    // Zera histórico de fase na inicialização
//    dft_state.fase_acumulada = 0.0f;
//
//    start_conversion();
//    tim2_interrupt();
//}
//
//void EXTI15_10_IRQHandler(void)
//{
//	GPIOA->BSRR = GPIO_BSRR_BS8;		/*Set GPIOA Pin 8 to HIGH*/
//	TIM2->CR1 	= TIM_CR1_CEN;			/*Enable Timer 2 Counter*/
//
////	sensor_data = adc_read();    /*Acquisition from ADC*/
////	g_process_flg = 1;
//
//
//	EXTI->PR = EXTI_PR_PR13;		//Trigger request ocurred
//}
//
//void TIM2_IRQHandler(void)
//{
//    TIM2->SR &= ~SR_UIF;
//    g_process_flg = 1;
//}
//
//void UART_SendStruct(IEC60255_Frame_t *frame)
//{
//    uint8_t *data_ptr = (uint8_t *)frame;
//    uint16_t size = sizeof(IEC60255_Frame_t);
//
//    for (uint16_t i = 0; i < size; i++)
//    {
//        while (!(USART2->SR & USART_SR_TXE));
//        USART2->DR = (data_ptr[i] & 0xFF);
//    }
//}
