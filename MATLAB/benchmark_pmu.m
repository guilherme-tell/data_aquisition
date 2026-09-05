
% =========================================================================
% Análise de Benchmark PMU: Captura e Decodificação JSON via UART
% =========================================================================
clear; clc; close all;

%% 1. Configuração da Porta Serial
portaSerial = 'COM3';
baudRate = 115200;

try
    s = serialport(portaSerial, baudRate);
    configureTerminator(s, "CR/LF");
    s.Timeout = 10;
    flush(s);

    disp('Aguardando dados do benchmark (Reinicie o STM32)...');

catch
    error('Falha ao abrir a porta serial.');
end

%% 2. Aquisição de Dados
dados_json = {};
contador = 1;

while true
    try
        linha = readline(s);

        if isempty(linha)
            break;
        end

        dados_json{contador} = jsondecode(char(linha));
        contador = contador + 1;

    catch ME
        warning('Fim da transmissão ou erro de leitura: %s', ME.message);
        break;
    end
end

clear s;

disp(['Benchmark concluído. ' ...
      num2str(length(dados_json)) ' frames recebidos.']);

%% 3. Organização dos Dados
N_frames = length(dados_json);

if N_frames == 0
    error(['Nenhum dado recebido. ' ...
           'Verifique a compilação do firmware e a flag de float.']);
end

% -------------------------------------------------------------------------
% Pré-alocação
% -------------------------------------------------------------------------
id  = zeros(1, N_frames);
mag = zeros(1, N_frames);

% frequencia  = Estimated Frequency
% rocof = Rate of Frequency Estimation / ROCOF
frequencia  = zeros(1, N_frames);
rocof = zeros(1, N_frames);

phi = zeros(1, N_frames);

% -------------------------------------------------------------------------
% Decodificação dos frames
% -------------------------------------------------------------------------
for i = 1:N_frames

    id(i)  = dados_json{i}.id;
    mag(i) = dados_json{i}.mag;

    % Frequência estimada 
    frequencia(i)  = dados_json{i}.freq;

    % ROCOF 
    rocof(i) = dados_json{i}.rocof;

    % Fase
    phi(i) = dados_json{i}.phi;

end

%% 4. Eixo de tempo e Separação do Regime Permanente
% O firmware envia um frame a cada NPPC amostras.
% Com FS = 7680 Hz e NPPC = 128: 60 frames/s
fps = 60;
tempo_bruto = (0:N_frames-1) / fps;

% -------------------------------------------------------------------------
% Configuração do corte do transitório (em segundos)
% Ajuste este valor conforme o atraso de grupo da sua SWRDFT + Filtro FIR
% -------------------------------------------------------------------------
tempo_transitorio = 0.5; 

% Encontra os índices válidos (apenas amostras após o tempo de acomodação)
idx_permanente = tempo_bruto >= tempo_transitorio;

if ~any(idx_permanente)
    error('O tempo transitório definido (%.2f s) é maior que a duração total do ensaio.', tempo_transitorio);
end

% Trunca todos os vetores para o regime permanente
tempo      = tempo_bruto(idx_permanente);
mag        = mag(idx_permanente);
frequencia = frequencia(idx_permanente);
rocof      = rocof(idx_permanente);
phi        = phi(idx_permanente);
id         = id(idx_permanente);

% Atualiza o número de frames para as estatísticas
N_frames_permanente = length(tempo);

%% 5. Apresentação Gráfica (Regime Permanente)
fig = figure( ...
    'Name', 'Resultados do Benchmark PMU - Regime Permanente', ...
    'NumberTitle', 'off', ...
    'Position', [100, 50, 900, 700]);
%% ------------------------------------------------------------------------
% Magnitude
% -------------------------------------------------------------------------
subplot(4,1,1);
plot(tempo, mag, 'LineWidth', 1.5);
title('Magnitude Estimada (MAG) - Regime Permanente');
ylabel('Amplitude');
xlim([tempo(1) tempo(end)]);
grid on;
%% ------------------------------------------------------------------------
% frequencia - Frequência Estimada
% -------------------------------------------------------------------------
subplot(4,1,2);
plot(tempo, frequencia, 'LineWidth', 1.5);
title('Frequência Estimada');
ylabel('Frequência (Hz)');
xlim([tempo(1) tempo(end)]);
grid on;
%% ------------------------------------------------------------------------
% rocof - ROCOF
% -------------------------------------------------------------------------
subplot(4,1,3);
plot(tempo, rocof, 'LineWidth', 1.5);
title('ROCOF');
ylabel('ROCOF (Hz/s)');
xlim([tempo(1) tempo(end)]);
grid on;
%% ------------------------------------------------------------------------
% Fase
% -------------------------------------------------------------------------
subplot(4,1,4);
plot(tempo, phi, 'LineWidth', 1.5);
title('Fase Estimada');
xlabel('Tempo (s)');
ylabel('Fase (graus)');
xlim([tempo(1) tempo(end)]);
grid on;
%% 6. Informações do Benchmark (Regime Permanente)
fprintf('\n=============================================\n');
fprintf('      BENCHMARK PMU (REGIME PERMANENTE)\n');
fprintf('=============================================\n');
fprintf('Tempo de descarte: %.3f s\n', tempo_transitorio);
fprintf('Frames válidos   : %d\n', N_frames_permanente);
fprintf('Taxa de relatório: %.2f FPS\n', fps);
fprintf('Janela analisada : %.3f s a %.3f s\n', tempo(1), tempo(end));
fprintf('\n--- frequencia ---\n');
fprintf('Mínimo           : %.6f Hz\n', min(frequencia));
fprintf('Máximo           : %.6f Hz\n', max(frequencia));
fprintf('Média            : %.6f Hz\n', mean(frequencia));
fprintf('\n--- rocof / ROCOF ---\n');
fprintf('Mínimo           : %.6f Hz/s\n', min(rocof));
fprintf('Máximo           : %.6f Hz/s\n', max(rocof));
fprintf('Média            : %.6f Hz/s\n', mean(rocof));
fprintf('\n--- MAG ---\n');
fprintf('Mínimo           : %.6f\n', min(mag));
fprintf('Máximo           : %.6f\n', max(mag));
fprintf('Média            : %.6f\n', mean(mag));
fprintf('\n--- FASE ---\n');
fprintf('Mínimo           : %.6f graus\n', min(phi));
fprintf('Máximo           : %.6f graus\n', max(phi));
fprintf('=============================================\n');
