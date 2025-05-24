import matplotlib.pyplot as plt
import os
import re

# Проверяем, существует ли папка
if not os.path.exists('matrix_out'):
    print("Ошибка: папка 'matrix_out' не найдена!")
    exit()

files = [
    'matrix_out/matrix_statistic1.txt',
    'matrix_out/matrix_statistic2.txt',
    'matrix_out/matrix_statistic4.txt', 
    'matrix_out/matrix_statistic8.txt'   
]

plt.figure()

for file in files:
    x, y = [], []
    try:
        # Извлекаем число потоков из имени файла
        thread_count = re.search(r'statistic(\d+)', file).group(1)
        
        with open(file) as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) == 2:  # Проверяем, что в строке 2 элемента
                    x.append(int(parts[0]))
                    y.append(float(parts[1]))
        plt.plot(x, y, 'o-', label=thread_count)
    except FileNotFoundError:
        print(f"Файл {file} не найден, пропускаем")
    except ValueError as e:
        print(f"Ошибка в данных файла {file}: {e}")

plt.xlabel('Размер матрицы')
plt.ylabel('Время (сек)')
plt.grid(True)
plt.legend(title='Потоки')
plt.show()