import matplotlib.pyplot as plt

def average_plot_from_files(filenames, label):
    """
    Строит график по нескольким файлам со статистикой, усредняя значения и добавляет метку.
    """
    all_x = []
    all_y = []

    # Читаем данные из всех файлов
    for filename in filenames:
        x = []
        y = []
        with open(filename, 'r') as file:
            for line in file:
                line = line.strip()
                if line:
                    values = line.split()
                    x_value = float(values[0])
                    y_value = float(values[1])
                    x.append(x_value)
                    y.append(y_value)
        all_x.append(x)
        all_y.append(y)

    # Находим уникальные значения x, чтобы по ним усреднять
    unique_x = sorted(list(set().union(*all_x)))

    # Усредняем значения y для каждого x
    average_y = []
    for x_val in unique_x:
        y_values_for_x = []
        for i in range(len(filenames)):
            if len(all_x[i]) == 0: # Проверяем, есть ли данные в all_x[i]
                continue # Если нет, переходим к следующему файлу

            try:
                index = all_x[i].index(x_val)
                y_values_for_x.append(all_y[i][index])
            except ValueError:
                pass # x_val отсутствует в текущем файле

        if y_values_for_x:
            average_y.append(sum(y_values_for_x) / len(y_values_for_x))
        else:
            average_y.append(None)

    # Убираем значения None, чтобы matplotlib не ругался
    x_filtered = []
    y_filtered = []
    for i in range(len(unique_x)):
        if average_y[i] is not None:
            x_filtered.append(unique_x[i])
            y_filtered.append(average_y[i])

    return x_filtered, y_filtered, label

if __name__ == '__main__':
    plt.figure(figsize=(10, 6))  # Увеличиваем размер графика для лучшей читаемости

    # Данные для 8 потоков
    filenames_8 = [f'Parallel-prog\\matrix_out\\matrix_statistic8{i}.txt' for i in range(1, 6)]
    x_8, y_8, label_8 = average_plot_from_files(filenames_8, '8 потоков')
    plt.plot(x_8, y_8, marker='o', label=label_8)

    # Данные для 4 потоков
    filenames_4 = [f'Parallel-prog\\matrix_out\\matrix_statistic4{i}.txt' for i in range(1, 6)]
    x_4, y_4, label_4 = average_plot_from_files(filenames_4, '4 потока')
    plt.plot(x_4, y_4, marker='o', label=label_4)

    # Данные для 2 потоков
    filenames_2 = [f'Parallel-prog\\matrix_out\\matrix_statistic2{i}.txt' for i in range(1, 6)]
    x_2, y_2, label_2 = average_plot_from_files(filenames_2, '2 потока')
    plt.plot(x_2, y_2, marker='o', label=label_2)

    plt.xlabel('Количество элементов матрицы')
    plt.ylabel('Среднее время выполнения (секунды)')
    plt.title('Усредненное время выполнения умножения матриц для разного количества потоков')
    plt.grid(True)
    plt.legend()  # Добавляем легенду, чтобы различать графики
    plt.show()