# Atividade Etapa 2 - Firmware: Medição Fasorial (PMU)

Este repositório contém a implementação de um firmware para o microcontrolador **STM32F446RET6 (ARM Cortex-M4F)**, desenvolvido para a aquisição de sinais analógicos e extração em tempo real de parâmetros fasoriais (Magnitude, Frequência, ROCOF e Fase). O projeto segue os princípios da norma IEC/IEEE 60255-118-1 para Unidades de Medição Fasorial (PMU).

## 🚀 Principais Características

* **Aquisição Determinística:** Conversor Analógico-Digital (ADC) engatilhado por temporizador em hardware (TIM2) operando a $F_s = 7680$ Hz.
* **Processamento Digital de Sinais (DSP):** 
  * Extração contínua através da Transformada Discreta de Fourier Deslizante (SWRDFT).
  * Filtragem de fase e magnitude utilizando um filtro FIR Savitzky-Golay implementado via buffer circular manual .
* **Sincronismo via Hardware:** Gatilho de início de benchmark via interrupção externa (EXTI) e pino de sincronização (PA8).
* **Telemetria JSON:** Envio estruturado dos parâmetros estimados via UART a 60 FPS (Quadros por Segundo) para fácil decodificação.

## 🛠️ Requisitos de Hardware

* Placa de desenvolvimento STM32 Nucleo-F446RE (ou placa customizada compatível).
* Módulo transformador de tensão isolador **ZMPT101B** (alimentado em 3.3V com referência GND comum).
* Cabos de conexão (Jumpers) e gerador de sinais (opcional, para validação metrológica).

## 💻 Requisitos de Software

* **IDE/Compilador:** STM32CubeIDE ou ambiente compatível com toolchain ARM GCC.
* **Análise de Dados:** MATLAB: plotagem do benchmark.

## ⚙️ Instruções de Compilação

O firmware utiliza formatação de strings em ponto flutuante via `snprintf` para estruturar o pacote JSON transmitido pela UART. Para que os valores não sejam omitidos na transmissão, é **obrigatório** habilitar o suporte a float na biblioteca C.

**No STM32CubeIDE:**
1. Clique com o botão direito no projeto e vá em **Properties**.
2. Navegue até **C/C++ Build** > **Settings**.
3. Na aba **Tool Settings**, expanda **MCU GCC Linker** e clique em **Miscellaneous**.
4. Marque a opção: `Use float with printf from newlib-nano (-u _printf_float)`.
5. Clique em **Apply and Close** e recompile o projeto.

## 🔌 Conexões e Pinos

| Periférico | Pino STM32 | Função |
| :--- | :--- | :--- |
| **ADC** | `PA1` | Entrada analógica do sinal condicionado (ZMPT101B) |
| **UART TX** | `PA2` | Transmissão de dados (conectado via USB-Serial da Nucleo) |
| **EXTI** | `B1` | Gatilho do push-button |
| **SYNC OUT** | `PA8` | Saída de sincronização na borda de subida |

## 📊 Como Executar e Validar

1. **Gravação:** Compile e grave o firmware no STM32.
2. **Conexão Analógica:** Conecte a saída do ZMPT101B ao pino `PA1`. Certifique-se de que o trimpot do módulo foi ajustado para limitar o sinal entre 0V e 3.3V, com offset em +1.65V.
3. **Recepção de Dados (MATLAB):**
   * Abra o arquivo `benchmark_pmu.m` localizado na pasta `MATLAB/`.
   * Edite a variável `portaSerial` para a porta COM correspondente à sua placa (ex: `COM3`).
   * Execute o script. Ele aguardará a transmissão dos dados.
4. **Início do Teste:** Pressione o push-button `B1` (Azul) STM32. O microcontrolador iniciará a amostragem e transmitirá os dados formatados em JSON durante o regime predeterminado.
5. **Resultados:** O MATLAB interceptará o payload, calculará o regime permanente e plotará automaticamente os gráficos de Magnitude, Frequência, ROCOF, Fase e Componentes Retangulares.

## 📂 Estrutura do Repositório      
```text
├── Docs/                   # Documentação técnica do projeto (PDF gerado via LaTeX)
├── MATLAB/                 # Scripts de recepção, cálculo de Savitzky-Golay e visualização
├── Src/                    # Arquivos-fonte (.c) do firmware bare-metal e DSP
├── Inc/                    # Arquivos de cabeçalho (.h)
└── README.md               # Este arquivo
