import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# =================================================================
# 1. DADOS DE ENTRADA (Extraídos do Log de Simulação)
#    Seus dados consolidados de Hits/Misses/Latência por cenário.
#    ATENÇÃO: Este DataFrame deve ser alimentado com os 600
#    requisições (200 * 3 usuários) por algoritmo/distribuição.
# =================================================================

data = {
    'Algoritmo': ['FIFO', 'FIFO', 'FIFO', 'LRU', 'LRU', 'LRU', 'LFU', 'LFU', 'LFU'],
    'Distribuição': ['Uniforme', 'Poisson', 'Ponderada 30-40'] * 3,
    'Hits': [42, 264, 106, 54, 307, 114, 338, 262, 151],
    'Misses': [558, 336, 494, 546, 293, 486, 262, 338, 449],
    'Total_Time_s': [18.608, 14.566, 17.039, 17.926, 13.923, 16.791, 13.623, 18.068, 16.459],
    'Avg_Latency_ms': [30.991, 24.241, 28.372, 29.856, 23.179, 27.964, 22.678, 30.088, 27.411]
}

# Total de requisições por rodada de simulação (3 usuários * 200 reqs/usuário = 600)
TOTAL_REQUESTS = 600

df_summary = pd.DataFrame(data)
df_summary['Miss_Rate'] = (df_summary['Misses'] / TOTAL_REQUESTS) * 100
df_summary = df_summary.round(2) # Arredonda para 2 casas decimais

# Renomear os dados do LFU conforme o último print
df_summary.loc[df_summary['Algoritmo'] == 'LFU', 'Hits'] = [63, 338, 151]
df_summary.loc[df_summary['Algoritmo'] == 'LFU', 'Misses'] = [537, 262, 449]
df_summary.loc[df_summary['Algoritmo'] == 'LFU', 'Avg_Latency_ms'] = [30.088, 22.678, 27.411]
df_summary['Miss_Rate'] = (df_summary['Misses'] / TOTAL_REQUESTS) * 100


# =================================================================
# 2. GERAÇÃO DOS GRÁFICOS COMPARATIVOS (MISS RATE e LATÊNCIA)
# =================================================================
plt.style.use('seaborn-v0_8-darkgrid')
cmap = plt.get_cmap('Set2')

def create_comparative_plot(df, value_col, title, ylabel, filename):
    plt.figure(figsize=(12, 6))
    
    # Pivotar para ter as distribuições no eixo X e os algoritmos como barras agrupadas
    df_pivot = df.pivot(index='Distribuição', columns='Algoritmo', values=value_col)
    
    ax = df_pivot.plot(kind='bar', ax=plt.gca(), width=0.8, color=[cmap(0), cmap(1), cmap(2)])
    
    # Adicionar os valores acima das barras
    for container in ax.containers:
        ax.bar_label(container, fmt='%.2f')

    plt.title(title, fontsize=16)
    plt.ylabel(ylabel, fontsize=12)
    plt.xlabel('Padrão de Distribuição de Acesso', fontsize=12)
    plt.xticks(rotation=0)
    plt.legend(title='Algoritmo de Cache')
    plt.tight_layout()
    plt.savefig(filename)
    plt.close()
    print(f"Gráfico '{filename}' gerado com sucesso.")


# Gráfico 1: Miss Rate
create_comparative_plot(
    df_summary, 
    'Miss_Rate', 
    'Gráfico 1: Comparação da Taxa de Miss (Miss Rate)', 
    'Taxa de Miss (%)', 
    'relatorio_miss_rate_comparativo.png'
)

# Gráfico 2: Latência Média
create_comparative_plot(
    df_summary, 
    'Avg_Latency_ms', 
    'Gráfico 2: Comparação da Latência Média por Requisição', 
    'Latência Média (ms)', 
    'relatorio_latency_comparativo.png'
)


# =================================================================
# 3. ANÁLISE INDIVIDUAL E GRÁFICO DE JUSTIFICATIVA
# =================================================================

# Ler os arquivos CSV da distribuição ponderada (weighted30-40)
weighted_files = [
    {'alg': 'FIFO', 'file': 'sim_FIFO_weighted30-40_per_text_misses.csv'},
    {'alg': 'LRU', 'file': 'sim_LRU_weighted30-40_per_text_misses.csv'},
    {'alg': 'LFU', 'file': 'sim_LFU_weighted30-40_per_text_misses.csv'}
]

weighted_analysis_list = []
misses_per_text_data = {}

for item in weighted_files:
    algoritmo = item['alg']
    filename = item['file']
    
    try:
        # Assumindo que o CSV tem cabeçalho e as colunas são "Text_ID" e "Miss_Count"
        df_text = pd.read_csv(filename, names=['Text_ID', 'Miss_Count'], header=0)
        misses_per_text_data[algoritmo] = df_text
        
        # Filtrar e somar misses para o grupo alvo (Textos 30 a 40)
        df_target = df_text[(df_text['Text_ID'] >= 30) & (df_text['Text_ID'] <= 40)]
        total_misses_target = df_target['Miss_Count'].sum()
        
        # Somar misses para o grupo de outros textos
        df_other = df_text[~((df_text['Text_ID'] >= 30) & (df_text['Text_ID'] <= 40))]
        total_misses_other = df_other['Miss_Count'].sum()

        weighted_analysis_list.append({
            'Algoritmo': algoritmo,
            'Misses_Textos_30_40': total_misses_target,
            'Misses_Outros_Textos': total_misses_other
        })
        
    except FileNotFoundError:
        print(f"AVISO: Arquivo '{filename}' não encontrado. Pulando análise individual para {algoritmo}. Certifique-se de que os CSVs estão no diretório de execução.")
        
df_weighted = pd.DataFrame(weighted_analysis_list)

# Gráfico 3: Misses Agregados em Textos Ponderados (30-40)
if not df_weighted.empty:
    plt.figure(figsize=(10, 6))
    ax = df_weighted.set_index('Algoritmo')[['Misses_Textos_30_40', 'Misses_Outros_Textos']].plot(kind='bar', stacked=True, ax=plt.gca(), color=[cmap(3), cmap(4)])
    
    # Adicionar rótulos para o total de Misses (altura da barra)
    for c in ax.containers:
        labels = [f'{v.get_height()}' for v in c]
        ax.bar_label(c, labels=labels, label_type='center')

    plt.title('Gráfico 3: Misses Agregados na Distribuição Ponderada (Foco 43% nos Textos 30-40)', fontsize=14)
    plt.ylabel('Contagem Total de Misses (No Cenário Ponderado)', fontsize=12)
    plt.xlabel('Algoritmo', fontsize=12)
    plt.xticks(rotation=0)
    plt.legend(title='Grupo de Textos', labels=['Textos 30-40 (Alvo)', 'Outros Textos (1-29, 41-100)'])
    plt.tight_layout()
    plt.savefig('relatorio_misses_weighted_analysis.png')
    plt.close()
    print(f"Gráfico 'relatorio_misses_weighted_analysis.png' gerado com sucesso.")

# =================================================================
# 4. EXIBIÇÃO DA TABELA DE CONCLUSÃO
# =================================================================

print("\n\n=== TABELA 1: RESULTADO CONSOLIDADO DA SIMULAÇÃO (Para Relatório) ===")
print(df_summary[['Algoritmo', 'Distribuição', 'Miss_Rate', 'Avg_Latency_ms']].to_markdown(index=False, floatfmt=".2f"))

if not df_weighted.empty:
    print("\n\n=== TABELA 2: ANÁLISE INDIVIDUAL PONDERADA (Misses nos Textos 30-40) ===")
    print(df_weighted.to_markdown(index=False))

print("\n\n--> PRÓXIMO PASSO: Inserir as Tabelas 1 e 2 e os Gráficos 1, 2 e 3 no README.md junto com a conclusão textual.")