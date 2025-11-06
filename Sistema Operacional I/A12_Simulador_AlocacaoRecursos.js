// VARIÁVEIS GLOBAIS
let P, R; // Número de Processos e Recursos
let MAX = []; // Matriz Máximo
let ALLOCATION = []; // Matriz Alocação
let NEED = []; // Matriz Necessidade (MAX - ALLOCATION)
let AVAILABLE = []; // Vetor Recursos Disponíveis
let RECURSO_NAMES = []; // Nomes dos Recursos (A, B, C...)

// --- FUNÇÕES DE INICIALIZAÇÃO ---

/**
 * Inicializa a simulação com os valores de P e R definidos pelo usuário.
 */
function inicializarSimulacao() {
    P = parseInt(document.getElementById('numProcessos').value);
    R = parseInt(document.getElementById('numRecursos').value);
    
    if (isNaN(P) || isNaN(R) || P < 1 || R < 1) {
        alert("Por favor, insira valores válidos para Processos e Recursos.");
        return;
    }

    // Geração de nomes de recursos (A, B, C...)
    RECURSO_NAMES = Array.from({ length: R }, (_, i) => String.fromCharCode(65 + i));

    // Inicializa todas as matrizes e o vetor AVAILABLE com valores aleatórios iniciais
    inicializarValoresAleatorios();

    // Renderiza as tabelas na interface
    renderizarMatrizes();
    renderizarControlesSolicitacao();
    
    // Verifica e exibe o estado inicial
    verificarEstadoAtual(true);
    logEvent("Simulação inicializada com sucesso!");
}

/**
 * Preenche as matrizes e o vetor com dados aleatórios válidos.
 */
function inicializarValoresAleatorios() {
    MAX = [];
    ALLOCATION = [];
    AVAILABLE = Array(R).fill(0).map(() => Math.floor(Math.random() * 8) + 5); // Recursos totais
    let TOTAL = [...AVAILABLE]; // Cópia para calcular a alocação total

    for (let i = 0; i < P; i++) {
        MAX[i] = [];
        ALLOCATION[i] = [];
        NEED[i] = [];
        
        for (let j = 0; j < R; j++) {
            // MAX: até 10
            MAX[i][j] = Math.floor(Math.random() * 8) + 2; 
            
            // ALLOCATION: não pode ser maior que MAX e não pode exceder o TOTAL
            ALLOCATION[i][j] = Math.min(Math.floor(Math.random() * MAX[i][j]), TOTAL[j]);
            TOTAL[j] -= ALLOCATION[i][j]; // Subtrai do total para garantir que a soma seja válida
            
            // NEED
            NEED[i][j] = MAX[i][j] - ALLOCATION[i][j];
        }
    }
    
    // Recalcula AVAILABLE: TOTAL - ALLOCATION acumulada
    AVAILABLE = Array(R).fill(0);
    for (let j = 0; j < R; j++) {
        let allocatedTotal = 0;
        for (let i = 0; i < P; i++) {
            allocatedTotal += ALLOCATION[i][j];
        }
        // Usamos um valor base alto (ex: 20) para os recursos totais, para garantir que o sistema não comece em Deadlock.
        let totalRecursos = allocatedTotal + Math.floor(Math.random() * 5) + 3; // Total = Alocado + Disponível
        AVAILABLE[j] = totalRecursos - allocatedTotal;
    }
}

// --- FUNÇÕES DE RENDERIZAÇÃO DA INTERFACE ---

/**
 * Cria a estrutura de grade CSS para uma matriz.
 * @param {HTMLElement} container - O elemento div onde a tabela será colocada.
 */
function configurarGrid(container) {
    container.style.gridTemplateColumns = `auto repeat(${R}, 1fr)`;
}

/**
 * Renderiza uma única matriz/vetor na interface.
 * @param {string} id - ID do container no HTML.
 * @param {Array<Array<number>> | Array<number>} data - Dados da matriz ou vetor.
 * @param {boolean} isVector - true se for um vetor (Available).
 * @param {string} name - Nome da matriz para fins de animação.
 */
function renderizarMatriz(id, data, isVector = false, name) {
    const container = document.getElementById(id);
    container.innerHTML = ''; // Limpa o conteúdo anterior
    configurarGrid(container);

    // Renderiza o cabeçalho dos recursos
    if (isVector) {
        // Para Available (Vetor): só o nome dos recursos
        RECURSO_NAMES.forEach(r => {
            const header = document.createElement('div');
            header.className = 'cell header-cell';
            header.textContent = r;
            container.appendChild(header);
        });
        
        // Renderiza os valores (uma única linha)
        data.forEach((value, j) => {
            const cell = document.createElement('div');
            cell.className = 'cell';
            cell.id = `${name}-${j}`;
            cell.textContent = value;
            container.appendChild(cell);
        });

    } else {
        // Para Matrizes (Allocation, Max, Need): headers de recurso e de processo
        
        // Canto superior esquerdo vazio
        const empty = document.createElement('div');
        empty.className = 'cell header-cell';
        container.appendChild(empty);

        // Headers dos Recursos
        RECURSO_NAMES.forEach(r => {
            const header = document.createElement('div');
            header.className = 'cell header-cell';
            header.textContent = r;
            container.appendChild(header);
        });

        // Valores da Matriz
        data.forEach((row, i) => {
            // Header do Processo
            const pHeader = document.createElement('div');
            pHeader.className = 'cell processo-header';
            pHeader.textContent = `P${i}`;
            container.appendChild(pHeader);

            // Valores
            row.forEach((value, j) => {
                const cell = document.createElement('div');
                cell.className = 'cell';
                cell.id = `${name}-P${i}-R${j}`; // Ex: allocation-P0-R1
                cell.textContent = value;
                container.appendChild(cell);
            });
        });
    }
}

/**
 * Renderiza todas as matrizes na interface.
 */
function renderizarMatrizes() {
    renderizarMatriz('available-table', AVAILABLE, true, 'available');
    renderizarMatriz('allocation-table', ALLOCATION, false, 'allocation');
    renderizarMatriz('max-table', MAX, false, 'max');
    renderizarMatriz('need-table', NEED, false, 'need');
}

/**
 * Cria os controles para a solicitação de recursos (dropdown e inputs).
 */
function renderizarControlesSolicitacao() {
    const select = document.getElementById('processo-solicitante');
    select.innerHTML = '';
    for (let i = 0; i < P; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = `P${i}`;
        select.appendChild(option);
    }
    
    const inputsDiv = document.getElementById('solicitacao-inputs');
    inputsDiv.innerHTML = '';
    RECURSO_NAMES.forEach((r, j) => {
        inputsDiv.innerHTML += `
            <label for="req-R${j}">${r}:</label>
            <input type="number" id="req-R${j}" value="0" min="0" max="${MAX.map(row => row[j]).reduce((a, b) => a + b, 0)}">
        `;
    });
}

// --- FUNÇÕES DO ALGORITMO DO BANQUEIRO ---

/**
 * Implementa o Algoritmo de Segurança (Safety Algorithm).
 * Verifica se o estado atual do sistema é seguro.
 * @param {Array<number>} currentAvailable - O vetor Available a ser testado.
 * @param {Array<Array<number>>} currentAllocation - A matriz Allocation a ser testada.
 * @param {Array<Array<number>>} currentNeed - A matriz Need a ser testada.
 * @returns {{isSafe: boolean, sequence: Array<number>}} - Objeto com resultado e sequência segura.
 */
function safetyAlgorithm(currentAvailable, currentAllocation, currentNeed) {
    let work = [...currentAvailable];
    let finish = Array(P).fill(false);
    let safeSequence = [];
    let progress = true;

    while (progress) {
        progress = false;
        for (let i = 0; i < P; i++) {
            if (!finish[i]) {
                let canFinish = true;
                // Verifica se Need[i] <= Work
                for (let j = 0; j < R; j++) {
                    if (currentNeed[i][j] > work[j]) {
                        canFinish = false;
                        break;
                    }
                }
                
                if (canFinish) {
                    // Simula a liberação de recursos: Work = Work + Allocation[i]
                    for (let j = 0; j < R; j++) {
                        work[j] += currentAllocation[i][j];
                    }
                    finish[i] = true;
                    safeSequence.push(i);
                    progress = true; // Houve progresso, tenta o loop novamente
                }
            }
        }
        
        // Proteção contra loop infinito em caso de erro lógico
        if (safeSequence.length === P) break;
    }

    return {
        isSafe: safeSequence.length === P,
        sequence: safeSequence
    };
}

/**
 * Processa a solicitação de recursos de um processo.
 */
function processarSolicitacao() {
    const pIndex = parseInt(document.getElementById('processo-solicitante').value);
    const request = [];
    for (let j = 0; j < R; j++) {
        const value = parseInt(document.getElementById(`req-R${j}`).value);
        if (isNaN(value) || value < 0) {
            alert("Solicitação inválida. Use apenas números positivos.");
            return;
        }
        request.push(value);
    }

    logEvent(`P${pIndex} está solicitando recursos: [${RECURSO_NAMES.map((r, i) => `${r}:${request[i]}`).join(', ')}]...`, 'info');

    // 1. Verificação: Request <= Need[pIndex]?
    for (let j = 0; j < R; j++) {
        if (request[j] > NEED[pIndex][j]) {
            logEvent(`ERRO: Solicitação excede a Necessidade Máxima de P${pIndex} (Need: ${NEED[pIndex]}). Solicitação Rejeitada.`, 'error');
            animateCellFailure('need', pIndex, j);
            return;
        }
    }

    // 2. Verificação: Request <= Available?
    for (let j = 0; j < R; j++) {
        if (request[j] > AVAILABLE[j]) {
            logEvent(`RECURSOS INSUFICIENTES: Solicitação de P${pIndex} (Request: ${request[j]} do recurso ${RECURSO_NAMES[j]}) é maior que Disponível (Available: ${AVAILABLE[j]}). P${pIndex} espera.`, 'warning');
            animateCellFailure('available', -1, j); // Anima o Available
            return;
        }
    }
    
    // 3. Simula a alocação e verifica a segurança
    
    // Cria cópias temporárias
    const tempAvailable = [...AVAILABLE];
    const tempAllocation = ALLOCATION.map(row => [...row]);
    const tempNeed = NEED.map(row => [...row]);

    // Simula a alocação (Do Allocation)
    for (let j = 0; j < R; j++) {
        tempAvailable[j] -= request[j];
        tempAllocation[pIndex][j] += request[j];
        tempNeed[pIndex][j] -= request[j];
    }
    
    // Chama o Algoritmo de Segurança
    const { isSafe, sequence } = safetyAlgorithm(tempAvailable, tempAllocation, tempNeed);

    if (isSafe) {
        logEvent(`Verificação de Segurança OK! O estado é SEGURO (Sequência: <${sequence.map(i => 'P' + i).join(', ')}>).`, 'success');
        
        // Confirma a alocação
        AVAILABLE = tempAvailable;
        ALLOCATION = tempAllocation;
        NEED = tempNeed;

        logEvent(`ALOCAÇÃO CONFIRMADA: Recursos alocados para P${pIndex}.`, 'success');
        
        // Animação de transferência
        animateResourceTransfer(pIndex, request);
        
        // Se a necessidade do processo for zero, ele termina e libera recursos
        if (NEED[pIndex].every(v => v === 0)) {
            finalizarProcesso(pIndex);
        }

    } else {
        logEvent(`VERIFICAÇÃO DE SEGURANÇA FALHOU! O estado não é seguro. Alocação REJEITADA para EVITAR DEADLOCK.`, 'error');
        // Nenhum recurso é alocado (as variáveis globais não são alteradas)
    }

    // Atualiza a interface (mesmo que falhe, pois o AVAILABLE pode ter sido animado)
    renderizarMatrizes();
    verificarEstadoAtual(false); // Reverifica o estado após a tentativa (o estado global não mudou se falhou, mas é bom revalidar)
}

/**
 * Simula a finalização de um processo e a liberação de seus recursos.
 * @param {number} pIndex - Índice do processo a ser finalizado.
 */
function finalizarProcesso(pIndex) {
    logEvent(`PROCESSO FINALIZADO: P${pIndex} completou a execução e está liberando seus recursos...`, 'success');
    
    // Libera os recursos alocados
    for (let j = 0; j < R; j++) {
        AVAILABLE[j] += ALLOCATION[pIndex][j];
        ALLOCATION[pIndex][j] = 0; // Zera a alocação
    }
    
    // Reajusta a necessidade máxima e necessidade (opcionalmente) para simular que o processo "saiu"
    // Mantemos por simplicidade, mas na prática o processo sairia da lista P.
    // Para a simulação, basta zerar a alocação e necessidade
    for (let j = 0; j < R; j++) {
        NEED[pIndex][j] = 0; 
        MAX[pIndex][j] = 0;
    }
    
    logEvent(`P${pIndex} liberou recursos. Estado atualizado.`, 'success');
}


// --- FUNÇÕES DE LOG E ANIMAÇÃO ---

/**
 * Adiciona uma mensagem ao log de eventos com formatação opcional.
 * @param {string} message - A mensagem a ser logada.
 * @param {string} type - 'info', 'success', 'warning', 'error'.
 */
function logEvent(message, type = 'info') {
    const log = document.getElementById('log-animacao');
    const p = document.createElement('p');
    p.innerHTML = `[${new Date().toLocaleTimeString()}] <strong>(${type.toUpperCase()})</strong>: ${message}`;
    p.className = `log-${type}`;
    log.prepend(p);
    
    // Garante que o log não fique muito grande
    if (log.children.length > 20) {
        log.removeChild(log.lastChild);
    }
}

/**
 * Anima uma célula da matriz para indicar sucesso ou falha.
 * @param {string} matrixName - 'available', 'allocation', 'need', 'max'.
 * @param {number} pIndex - Índice do processo (-1 para Available).
 * @param {number} rIndex - Índice do recurso.
 */
function animateCellFailure(matrixName, pIndex, rIndex) {
    let cellId;
    if (matrixName === 'available') {
        cellId = `available-${rIndex}`;
    } else {
        cellId = `${matrixName}-P${pIndex}-R${rIndex}`;
    }
    
    const cell = document.getElementById(cellId);
    if (cell) {
        cell.classList.add('flash-fail');
        setTimeout(() => cell.classList.remove('flash-fail'), 800);
    }
}

/**
 * Simula visualmente a transferência de recursos entre Available e Allocation/Need.
 * @param {number} pIndex - Índice do processo.
 * @param {Array<number>} request - O vetor de solicitação.
 */
function animateResourceTransfer(pIndex, request) {
    for (let j = 0; j < R; j++) {
        if (request[j] > 0) {
            // Células envolvidas
            const availableCell = document.getElementById(`available-${j}`);
            const allocationCell = document.getElementById(`allocation-P${pIndex}-R${j}`);
            const needCell = document.getElementById(`need-P${pIndex}-R${j}`);

            // Adiciona classe de animação e atualiza o texto (para que a animação seja visível no novo valor)
            if (availableCell) {
                availableCell.textContent = AVAILABLE[j];
                availableCell.classList.add('resource-transfer');
            }
            if (allocationCell) {
                allocationCell.textContent = ALLOCATION[pIndex][j];
                allocationCell.classList.add('flash-success');
            }
            if (needCell) {
                needCell.textContent = NEED[pIndex][j];
                needCell.classList.add('flash-success');
            }

            // Remove a classe após a animação
            setTimeout(() => {
                if (availableCell) availableCell.classList.remove('resource-transfer');
                if (allocationCell) allocationCell.classList.remove('flash-success');
                if (needCell) needCell.classList.remove('flash-success');
            }, 1000);
        }
    }
}

/**
 * Exibe o resultado da verificação de segurança no painel de controle.
 * @param {boolean} isInitial - Indica se é a verificação inicial.
 */
function verificarEstadoAtual(isInitial) {
    const resultDiv = document.getElementById('resultado-verificacao');
    const { isSafe, sequence } = safetyAlgorithm(AVAILABLE, ALLOCATION, NEED);

    resultDiv.innerHTML = `
        <h3>Verificação de Segurança</h3>
        <p>Estado ${isInitial ? 'Inicial' : 'Atual'}: 
        <strong style="color: ${isSafe ? 'green' : 'red'};">${isSafe ? 'SEGURO' : 'INSEGURO (POSSÍVEL DEADLOCK)'}</strong></p>
        <p>Sequência de Segurança: 
        <strong>${isSafe ? '<' + sequence.map(i => 'P' + i).join(', ') + '>' : 'Nenhuma sequência segura encontrada.'}</strong></p>
    `;
}

/**
 * Reinicia a simulação
 */
function resetarSimulacao() {
    window.location.reload(); // Maneira mais simples de reiniciar o estado global
}

// Inicializa a simulação na primeira carga (opcional, para exibir algo logo de cara)
document.addEventListener('DOMContentLoaded', inicializarSimulacao);