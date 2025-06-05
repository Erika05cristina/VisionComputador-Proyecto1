import matplotlib.pyplot as plt
import pandas as pd
import sys

# Argumentos: archivo CSV de entrada y archivo PNG de salida
data_path = sys.argv[1]
output_path = sys.argv[2]

# Leer los valores desde el archivo CSV
valores = pd.read_csv(data_path, header=None).squeeze()

# Crear boxplot
plt.figure(figsize=(4, 6))
plt.boxplot(valores, vert=True, patch_artist=True,
            boxprops=dict(facecolor='lightcoral'),
            medianprops=dict(color='red'),
            whiskerprops=dict(color='black'),
            capprops=dict(color='black'),
            flierprops=dict(marker='o', color='black'))

plt.title('Boxplot - Zona Segmentada')
plt.ylabel('Intensidad')
plt.grid(True)
plt.tight_layout()
plt.savefig(output_path)