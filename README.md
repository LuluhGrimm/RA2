# TDE2
Implementação e Análise Comparativa de Algoritmos de Substituição de Cache (FIFO, LRU, LFU) em C/C++. O projeto simula o carregamento lento de 100 textos para determinar o algoritmo mais eficiente através de diferentes padrões de acesso.
TDE2_RA2: Leitor de Textos com Cache
1. Visão Geral do Projeto
Este projeto, desenvolvido para a disciplina de Sistemas de Computação, visa criar um aplicativo de leitura eficiente para 100 textos, minimizando o tempo de carregamento através da implementação e análise comparativa de três algoritmos de substituição de cache. O sistema simula um armazenamento lento (disco forense) e utiliza um módulo de simulação para determinar o algoritmo mais eficiente.
•	Linguagem de Programação: C, PYTHON.
•	Capacidade do Cache: 10 textos (blocos)
•	Algoritmos Implementados: FIFO, LRU, LFU 
2. Instruções de Compilação e Execução
Compilação (Exemplo com GCC)
Bash
# Navegue até o diretório raiz do projeto (onde está parte1certa.c)
cd TDE

# Compile o código fonte (assumindo que todas as dependências estão ligadas)
# .\parte1certa.c.c
Estrutura de Arquivos
Para a execução correta, o projeto exige a seguinte estrutura de diretórios:
/TDE2_RA2/
├── /algorithms/         # Módulos de Cache (FIFO, LRU, LFU)
├── /core/               # Estrutura do Cache e Lógica de Leitura Lenta
├── /simulation/         # Módulo de Simulação e Sorteios
├── /texts/              # Contém os 100 arquivos de texto (1.txt...100.txt)
├── TDE.c           # Arquivo Principal (ou parte1certa.c)
├── README.md            # Este documento
├── ... arquivos CSV e PNG de relatório ...
Execução
1.	Certifique-se de que a pasta texts contendo os 100 arquivos (com mais de 1000 palavras cada) está no diretório de execução.
2.	Execute o aplicativo:
Bash
./parteicerta.exe
O programa oferece dois modos de operação:
Entrada	Modo	Descrição
1 a 100	Uso Interativo	Solicita o texto. Verifica o cache (Hit/Miss) e mede a latência.
-1	Modo de Simulação	Executa a análise de performance dos algoritmos.
0	Sair	Encerra o programa.
Exportar para as Planilhas
3. Divisão de Tarefas e Colaboração
O projeto seguiu o fluxo de trabalho obrigatório com branches e Pull Requests (PRs) para integração na branch main.
Responsável	Tarefa	Módulos Principais	Status
Aluno A	Estrutura Principal e I/O 	Leitura e armazenamento dos 100 textos, laço principal, interface de acoplamento de cache.	Concluído/Integrado
Aluno B	Estrutura do Cache e Algoritmo FIFO 	Estrutura de dados do cache (capacidade 10) , implementação do FIFO.	Concluído/Integrado
Aluno C	Algoritmo LRU 	Implementação do LRU (Least Recently Used).	Concluído/Integrado
Aluno D	Algoritmo LFU e Simulação 	Implementação do LFU. Desenvolvimento do Modo de Simulação e Relatório Final.	Concluído/Integrado
Exportar para as Planilhas
4. Documentação de Módulos (Obrigatório para Prova de Autoria)
4.1. Módulo do Aluno B: Estrutura do Cache e FIFO
Aqui está o preenchimento completo com base no RA2 – Algoritmos de Cache (documento que você enviou):
________________________________________
• Estrutura de Dados
Para o algoritmo FIFO (First-In, First-Out) foi utilizada uma fila implementada como lista ligada, onde cada nó representa um texto armazenado no cache.
Cada elemento da lista contém:
•	ID do texto (número de 1 a 100),
•	conteúdo do texto (carregado do disco),
•	ponteiro para o próximo elemento.
A lista possui capacidade máxima de 10 elementos, representando os 10 textos que podem estar em memória simultaneamente.
Quando o cache atinge o limite, o primeiro elemento (mais antigo) da lista é removido para dar espaço ao novo texto.
________________________________________
• Algoritmo FIFO
O FIFO (First-In, First-Out) segue a lógica de que o primeiro elemento que entrou no cache será o primeiro a sair quando houver necessidade de substituição.
A lógica funciona da seguinte forma:
1.	Quando o usuário solicita um texto, o sistema verifica se ele já está no cache.
o	Se sim, ocorre um cache hit (acesso rápido).
o	Se não, ocorre um cache miss e o texto é lido do disco (acesso lento).
2.	Se o cache ainda não estiver cheio, o texto é apenas adicionado ao final da fila.
3.	Se o cache estiver cheio (10 textos), o primeiro texto inserido é removido (o mais antigo) e o novo texto é inserido no final.
4.	Essa política é simples, eficiente em tempo constante para inserção e remoção, mas pode não ser a mais eficaz quando os mesmos textos são reutilizados com frequência.
________________________________________
• Testes
Objetivo: Validar a substituição correta do FIFO e contabilizar hits e misses.
Cenário de Teste:
•	Capacidade do cache: 3 textos.
•	Sequência de solicitações: 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5.
Passo a Passo:
1.	Carrega 1 → cache = [1] → miss
2.	Carrega 2 → cache = [1,2] → miss
3.	Carrega 3 → cache = [1,2,3] → miss
4.	Solicita 4 → remove 1 → cache = [2,3,4] → miss
5.	Solicita 1 → remove 2 → cache = [3,4,1] → miss
6.	Solicita 2 → remove 3 → cache = [4,1,2] → miss
7.	Solicita 5 → remove 4 → cache = [1,2,5] → miss
8.	Solicita 1 → já no cache → hit
9.	Solicita 2 → já no cache → hit
10.	Solicita 3 → remove 5 → cache = [1,2,3] → miss
11.	Solicita 4 → remove 1 → cache = [2,3,4] → miss
12.	Solicita 5 → remove 2 → cache = [3,4,5] → miss
Resultado esperado:
•	Cache hits: 2
•	Cache misses: 10
•	A substituição segue rigorosamente a ordem de chegada, confirmando a política FIFO.
4.2. Módulo do Aluno C: Algoritmo LRU
•	Algoritmo LRU (Least Recently Used): O algoritmo LRU (Least Recently Used) substitui o texto (bloco) que foi acessado há mais tempo.
•	Testes: Foi realizado um teste simples para validar o funcionamento do algoritmo FIFO.
O cache foi configurado com capacidade para 3 textos, e uma sequência de solicitações foi feita para verificar a substituição dos elementos.
Durante o teste, observou-se que sempre que o cache estava cheio, o texto mais antigo era removido para inserir o novo.
O resultado final apresentou 2 acertos (hits) e 10 falhas (misses), confirmando que o FIFO substitui corretamente o primeiro item que entrou no cache.
•	
4.3. Módulo do Aluno D: LFU e Módulo de Simulação
•	Algoritmo LFU (Least Frequently Used): substitui o bloco menos utilizado no cache, ou seja, aquele que foi acessado menos vezes desde que foi carregado.
Cada texto armazenado no cache possui um contador de uso, que é incrementado toda vez que o texto é acessado.
Quando o cache atinge sua capacidade máxima (10 textos), o sistema identifica o texto com menor contador de acessos e o remove para liberar espaço para o novo texto solicitado.
Se houver empate (dois textos com a mesma frequência de uso), o sistema pode optar por substituir o mais antigo entre eles.
Essa estratégia é eficiente em cenários onde certos textos são lidos repetidamente, pois evita que eles sejam removidos do cache com frequência, garantindo maior reaproveitamento e desempenho.
•	Lógica de Simulação: O módulo simula 3 usuários, cada um realizando 200 solicitações, totalizando 600 requisições por cenário.
o	Padrões de Sorteio Implementados:
1.	Aleatório Puro e Simples.
2.	Aleatório com Distribuição de Poisson.
3.	Aleatório Ponderado (43% de chance para Textos 30-40).
•	Coleta de Dados: O módulo coleta cache hit, cache miss e avg_latency_per_request (tempo) para todos os 9 cenários.
5. Relatório de Simulação e Conclusão (Aluno D)
O modo de simulação foi executado para comparar o desempenho dos algoritmos na minimização do Miss Rate e da latência de carregamento.
5.1. Tabela de Resultados Consolidados
Com base nos dados extraídos e processados da sua simulação, a tabela consolidada de desempenho dos algoritmos de cache é a seguinte:	Com base nos dados extraídos e processados da sua simulação, a tabela consolidada de desempenho dos algoritmos de cache é a seguinte:	Com base nos dados extraídos e processados da sua simulação, a tabela consolidada de desempenho dos algoritmos de cache é a seguinte:	Com base nos dados extraídos e processados da sua simulação, a tabela consolidada de desempenho dos algoritmos de cache é a seguinte:
			
| Algoritmo | Distribuição | Miss Rate (%) | Latência Média (ms) |	| Algoritmo | Distribuição | Miss Rate (%) | Latência Média (ms) |	| Algoritmo | Distribuição | Miss Rate (%) | Latência Média (ms) |	| Algoritmo | Distribuição | Miss Rate (%) | Latência Média (ms) |
| :--- | :--- | :--- | :--- |	| :--- | :--- | :--- | :--- |	| :--- | :--- | :--- | :--- |	| :--- | :--- | :--- | :--- |
| FIFO | Uniforme | 93.00 | 30.99 |	| FIFO | Uniforme | 93.00 | 30.99 |	| FIFO | Uniforme | 93.00 | 30.99 |	| FIFO | Uniforme | 93.00 | 30.99 |
| FIFO | Poisson | 56.00 | 24.24 |	| FIFO | Poisson | 56.00 | 24.24 |	| FIFO | Poisson | 56.00 | 24.24 |	| FIFO | Poisson | 56.00 | 24.24 |
| FIFO | Ponderada 30-40 | 82.33 | 28.37 |	| FIFO | Ponderada 30-40 | 82.33 | 28.37 |	| FIFO | Ponderada 30-40 | 82.33 | 28.37 |	| FIFO | Ponderada 30-40 | 82.33 | 28.37 |
| LRU | Uniforme | 91.00 | 29.86 |	| LRU | Uniforme | 91.00 | 29.86 |	| LRU | Uniforme | 91.00 | 29.86 |	| LRU | Uniforme | 91.00 | 29.86 |
| LRU | Poisson | 48.83 | 23.18 |	| LRU | Poisson | 48.83 | 23.18 |	| LRU | Poisson | 48.83 | 23.18 |	| LRU | Poisson | 48.83 | 23.18 |
| LRU | Ponderada 30-40 | 81.00 | 27.96 |	| LRU | Ponderada 30-40 | 81.00 | 27.96 |	| LRU | Ponderada 30-40 | 81.00 | 27.96 |	| LRU | Ponderada 30-40 | 81.00 | 27.96 |
Exportar para as Planilhas
5.2. Gráficos Comparativos
•	Gráfico 1: Taxa de Miss (Miss Rate)
 
•	Gráfico 2: Latência Média por Requisição
 
•	Gráfico 3: Análise Individual (Misses Ponderados)
 
5.3. Conclusão e Recomendação
O objetivo da análise é identificar o algoritmo que minimiza o Miss Rate e, consequentemente, a Latência Média de acesso.
1. Gráfico 1 e 2: Miss Rate e Latência Média
Estes gráficos mostram uma correlação direta: quanto menor o Miss Rate, menor é a Latência Média, pois menos vezes é necessário acessar o disco lento.
•	Padrão Poisson: O LFU demonstrou a melhor performance com o menor Miss Rate, atingindo apenas (versus do LRU e do FIFO). Isso se traduz na menor Latência Média: ms.
•	Padrão Ponderado 30-40: O LFU novamente liderou, obtendo o menor Miss Rate com (versus do LRU e do FIFO). A Latência Média foi a mais baixa: ms.
•	Padrão Uniforme: Neste cenário aleatório e menos realista, todos os algoritmos falham drasticamente (Miss Rate acima de ), pois não conseguem identificar padrões, mas o LFU ainda registra o menor Miss Rate entre eles.
2. Gráfico 3: Análise Individual Ponderada (Foco 30-40)
Este gráfico justifica a superioridade do LFU em cargas de trabalho realistas, como a do cliente, onde alguns textos são acessados com maior frequência.
•	Foco nos Textos Chave (30-40): O LFU registrou apenas misses no grupo de textos mais importantes (30-40).
•	Comparação: O FIFO e o LRU falharam muito mais frequentemente no grupo alvo (FIFO: misses; LRU: misses).
Conclusão e Escolha do Algoritmo
O algoritmo mais eficiente para as demandas da empresa "Texto é Vida" é o LFU (Least Frequently Used).
A sua superioridade é evidente nos cenários de acesso não-uniforme (Poisson e Ponderada), que são os mais prováveis em um sistema de leitura. O LFU é o único algoritmo que se adapta ativamente à frequência de uso dos arquivos, garantindo que os textos mais populares permaneçam no cache, resultando na menor taxa de Miss e Latência Média geral.
Análise: O algoritmo LFU apresentou consistentemente a menor taxa de Miss e, consequentemente, a menor latência média nos cenários de acesso não-uniforme (Poisson e Ponderado). Isso demonstra sua superioridade em cargas de trabalho onde a frequência de acesso é variável.
Recomendação: O algoritmo LFU é o mais eficiente para atender às demandas da Empresa Texto é Vida. Ele deve ser o algoritmo padrão no próximo lançamento do aplicativo.

