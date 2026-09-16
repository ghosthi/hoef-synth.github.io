# Proposta de Projeto: HOEF Synth

---

## 1. O projeto

O HOEF Synth é um sintetizador modular no formato **Eurorack**, combinando módulos analógicos com um módulo digital construído em torno de um microcontrolador **ESP32**. O Eurorack é o formato dominante para sintetizadores modulares, definindo especificações de hardware, elétrica e interfaces, e se diferencia de um sintetizador tradicional (com fluxo de sinal fixo em placa fechada) por ser composto de módulos independentes que podem ser conectados de formas variadas, permitindo diferentes combinações de notas, tons, batidas e melodias.

## 2. Motivação

Módulos Eurorack, apesar de serem o padrão de mercado, não são amplamente acessíveis no Brasil: o custo elevado, a ausência de fabricantes nacionais e a dependência de importação (câmbio, taxas de importação) dificultam o acesso de entusiastas brasileiros a equipamentos de qualidade. O HOEF Synth propõe prototipar um equipamento de baixo custo que mantenha a qualidade e versatilidade dos equipamentos internacionais.

## 3. Objetivos

### Objetivo Geral
Prover módulos de Eurorack de custo reduzido em comparação a produtos importados, produzidos com processo simples e replicável por entusiastas e "hobbystas", usando componentes eletrônicos e microcontroladores de amplo acesso — fortalecendo a cultura Open Source e DIY (Do It Yourself).

### Objetivos Específicos
- Projetar e construir módulos analógicos seguindo especificações de projetos Open Source, buscando resultado similar a produtos importados com custo inferior.
- Projetar e construir (hardware e software) um módulo digital usando microcontrolador e conversor Digital-Analógico, gerando sinais de controle para os módulos analógicos.
- Garantir documentação clara, replicável e adaptável do processo, permitindo customização e adaptação em outros contextos.

## 4. Componentes / Módulos do Projeto

O projeto é composto por 4 módulos principais:

### 4.1 Sequencer Digital
Módulo construído com um microcontrolador **ESP32**, 9 encoders rotativos e saídas padrão P2, além de uma fonte de alimentação.
- **8 encoders** controlam o nível de tensão gerado a cada instante (correspondente a um semitom).
- **1 encoder** controla o BPM (batidas por minuto) do sinal gerado.

**Saídas do Sequencer Digital:**
- **Clock:** onda quadrada periódica (alterna entre 3,3V e 0V), com frequência proporcional ao BPM.
- **Gate:** sinal digital periódico relacionado ao valor do step atual — alto (3,3V) quando o step é diferente de zero, baixo (0V) caso contrário.
- **CvOUT:** saída analógica que representa a tensão correspondente a cada passo (step) de uma sequência de 8 posições, atualizada a cada ciclo de Clock. Cada posição armazena um valor de 0 a 12, convertido em um nível de tensão entre 0V e 3,3V pelo pino DAC.
  - Exemplo dado no relatório: um arpejo do acorde Am7 (lá, dó, mi, sol, lá, dó, mi, sol) representado pelo vetor [1, 4, 8, 11, 1, 4, 8, 11] geraria as tensões 0,25V, 1V, 2V, 2,75V, 0,25V, 1V, 2V, 2,75V.
- **LCD:** Visualização de formato da saída e step atual em display LCD.

Todas as saídas passam por um amplificador operacional **TL072**, que amplifica o sinal em aproximadamente 3,61dB (≈1,515 vezes), necessário pois a saída máxima do ESP32 é 3,3V enquanto o padrão Eurorack adotado no projeto é de 5V.

Os módulos analógicos foram desenvolvidos com inspiração nos projetos da **Erica Synths Edu DIY Series**, cujo manual prático auxiliou na compreensão do desenvolvimento e construção de sintetizadores.

### 4.2 VCO — Oscilador Controlado por Tensão (Voltage-Controlled Oscillator)
Gera formas de onda periódicas e contínuas, com frequência fundamental controlada de forma precisa pelo nível de tensão, servindo como fonte primária do sinal sonoro (timbres do sintetizador).
- Arquitetura: oscilador dente de serra (*sawtooth-core oscillator*), com descarga linear da tensão pelo capacitor e recarga rápida via circuito integrado inversor Schmitt Trigger.
- **Entradas:**
  - CvOUT — recebe tensão no padrão 1 Volt por Oitava (1V/Oct).
  - FM CV — recebe correntes adicionais de outros sinais de áudio, provocando variações de frequência.
  - PWM CV — modula geometricamente o ciclo de onda do sinal resultante.
- **Controles:** Course (ajuste de frequência), Fine (ajuste fino), nível da entrada FM, nível da entrada PWM, controle de pulso (amplitude da onda).

### 4.3 VCF — Filtro Controlado por Tensão (Voltage-Controlled Filter)
Fundamental para a síntese analógica subtrativa, manipulando a dinâmica e resposta em frequência do sinal de áudio.
- Opera como filtro passa-baixas (*low pass filter*) com controle de ressonância, permitindo manejo de harmônicos e componentes de alta frequência.
- **Entradas:** Audio In (sinal a ser filtrado), CV In (tensão externa que modula a aplicação do filtro).
- **Saída:** áudio após o filtro.
- **Controles:** Cutoff (frequência de corte) e ressonância.

### 4.4 VCA — Amplificador Controlado por Tensão (Voltage-Controlled Amplifier)
Regula a amplitude do sinal elétrico/áudio recebido, permitindo aumento de amplitude (não apenas controle de ganho).
- **Entradas:** Signal In (sinal a ser amplificado), CV In (tensão de controle).
- **Saída:** sinal modulado.
- **Controle:** Offset (nível de amplificação aplicada ao sinal).

## 5. Funcionalidades Gerais

- Geração digital de sequências musicais (Sequencer) com controle de BPM e steps via encoders rotativos.
- Conversão de sequências digitais em sinais analógicos de controle (Clock, Gate, CvOUT) compatíveis com padrão Eurorack.
- Síntese sonora analógica completa: geração (VCO), filtragem (VCF) e amplificação (VCA) do sinal.
- Possibilidade de interconexão modular entre os componentes, permitindo diferentes combinações de notas, tons, batidas e melodias.

## 6. Resultados Esperados

- Um protótipo funcional de sintetizador Eurorack de baixo custo, com qualidade e versatilidade comparáveis a equipamentos importados.
- Módulos analógicos e digitais replicáveis, seguindo princípios Open Source e DIY.
- Documentação clara e adaptável do processo de desenvolvimento, viabilizando customização e reprodução do projeto por outros entusiastas em diferentes contextos.
- Fortalecimento do acesso de entusiastas brasileiros a equipamentos de síntese modular, reduzindo a dependência de produtos importados e seus custos associados (câmbio, importação).

## 7. Equações Relevantes

- Frequência a partir do BPM:
  f(BPM) = BPM / 60

- Ganho do amplificador operacional (TL072) em dB:
  Av(dB) = 20·log(V/V) *(conforme aplicado, resultando em ≈3,61dB / ≈1,515 vezes de amplificação)*

---
