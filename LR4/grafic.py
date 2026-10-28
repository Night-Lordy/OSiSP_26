import matplotlib.pyplot as plt
import numpy as np
import csv

# Настройка стиля графиков
plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['axes.unicode_minus'] = False


class ExperimentPlotter:
    """Класс для визуализации результатов экспериментов"""

    def __init__(self, output_dir='plots'):
        self.output_dir = output_dir
        import os
        os.makedirs(output_dir, exist_ok=True)

    def plot_execution_time(self, data, title='Время выполнения экспериментов'):
        """Столбчатая диаграмма времени выполнения за 3 запуска + среднее"""
        configs = [d['config'] for d in data]
        run1 = [d['run1'] for d in data]
        run2 = [d['run2'] for d in data]
        run3 = [d['run3'] for d in data]
        avg = [d['avg'] for d in data]

        x = np.arange(len(configs))
        width = 0.25

        fig, ax = plt.subplots(figsize=(12, 7))

        bars1 = ax.bar(x - width, run1, width, label='Запуск 1', color='#4C72B0', alpha=0.8)
        bars2 = ax.bar(x, run2, width, label='Запуск 2', color='#55A868', alpha=0.8)
        bars3 = ax.bar(x + width, run3, width, label='Запуск 3', color='#C44E52', alpha=0.8)

        ax.plot(x, avg, 'ko-', linewidth=2, markersize=10, label='Среднее', zorder=5)

        for bars in [bars1, bars2, bars3]:
            for bar in bars:
                height = bar.get_height()
                ax.annotate(f'{int(height)}',
                            xy=(bar.get_x() + bar.get_width() / 2, height),
                            xytext=(0, 3),
                            textcoords="offset points",
                            ha='center', va='bottom', fontsize=8)

        ax.set_ylabel('Время выполнения, мс', fontsize=12)
        ax.set_title(title, fontsize=14, fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels(configs, fontsize=10)
        ax.legend(fontsize=11)
        ax.grid(axis='y', alpha=0.3)

        plt.tight_layout()
        plt.savefig(f'{self.output_dir}/execution_time.png', dpi=150, bbox_inches='tight')
        plt.show()

    def plot_all(self, data):
        """Построение всех графиков"""
        print("Построение графиков...")

        self.plot_execution_time(data)
        print("✓ График времени выполнения сохранён")

        print(f"\nВсе графики сохранены в директории: {self.output_dir}/")

def load_data_from_csv(csv_file):
    """Загрузка данных из CSV файла"""
    data = []
    with open(csv_file, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            data.append({
                'config': f"{row['experiment']}\n({row['philosophers']} фил., {row['thinking_time']} мс)",
                'run1': int(row['run1_time']),
                'run2': int(row['run2_time']),
                'run3': int(row['run3_time']),
                'avg': int(row['avg_time']),
                'violations': int(row['avg_violations']),
                'cycles': 100 if row['experiment'] != 'Indiv' else 715,
                'philosophers': int(row['philosophers'])
            })
    return data


def main():
    """Главная функция"""
    csv_file = 'data.csv'

    try:
        data = load_data_from_csv(csv_file)
        print(f"Загружено {len(data)} конфигураций из {csv_file}")
    except FileNotFoundError:
        print(f"Файл {csv_file} не найден!")
        return

    plotter = ExperimentPlotter(output_dir='experiment_plots')
    plotter.plot_all(data)


if __name__ == '__main__':
    main()