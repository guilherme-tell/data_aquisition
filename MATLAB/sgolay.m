% =========================================================================
% Script para Cálculo e Exportação: Coeficientes Savitzky-Golay
% =========================================================================
clear; clc; close all;

%% 1. Parâmetros do Filtro
ordem = 2;         % Ordem do polinômio (2 ou 3 apresentam boa relação de suavização/tracking)
Fs = 7680;         % Taxa de amostragem definida no firmware
J_original = 128;  % Valor original (1 * 128)

% Garante que a janela seja ímpar
if mod(J_original, 2) == 0
    J_window = J_original + 1;
else
    J_window = J_original;
end

%% 2. Cálculo da Matriz de Projeção Polinomial
% A função sgolay retorna a matriz completa. Para operação FIR em tempo real 
% com atraso constante, extraímos a linha central.
[B, ~] = sgolay(ordem, J_window);
linha_central = (J_window + 1) / 2;
coeficientes = B(linha_central, :);

%% 3. Análise do Filtro (Plots)
fig = figure('Name', 'Design do Filtro Savitzky-Golay', 'NumberTitle', 'off', 'Position', [100, 100, 800, 600]);

% Resposta ao Impulso (Formato do polinômio)
subplot(2,1,1);
plot(coeficientes, 'LineWidth', 1.5, 'Color', '#0072BD');
title(sprintf('Coeficientes FIR Simétricos (Ordem %d, Janela %d amostras)', ordem, J_window));
xlabel('Índice da Amostra'); ylabel('Amplitude');
grid on;

% Resposta em Frequência (Bode Magnitude)
subplot(2,1,2);
[H, f] = freqz(coeficientes, 1, 4096, Fs);
plot(f, 20*log10(abs(H)), 'LineWidth', 1.5, 'Color', '#D95319');
title('Resposta em Frequência (Atenuação de Ruído)');
xlabel('Frequência (Hz)'); ylabel('Magnitude (dB)');
ylim([-100 5]); xlim([0 Fs/2]);
grid on;

%% 4. Exportação do Código C
disp('/* ============================================================');
disp(' * Coeficientes do Filtro Savitzky-Golay para C (ARM CMSIS-DSP)');
disp(' * Insira no seu arquivo signals.c ou defina externamente.');
fprintf(' * Ordem: %d | Janela: %d | Atraso de Grupo: %d amostras\n', ordem, J_window, (J_window-1)/2);
disp(' * ============================================================ */');

fprintf('const float32_t SG_COEFFS[%d] = {\n', J_window);

for i = 1:J_window
    % Quebra de linha a cada 5 elementos para manter o código limpo
    if mod(i-1, 5) == 0 && i > 1
        fprintf('\n    ');
    elseif i == 1
        fprintf('    ');
    end
    
    % Imprime em notação científica forçando o sufixo 'f' (float32_t)
    fprintf('%e%s', coeficientes(i), 'f');
    
    if i < J_window
        fprintf(', ');
    end
end
fprintf('\n};\n');