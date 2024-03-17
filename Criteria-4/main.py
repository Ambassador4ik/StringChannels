import os
import random
import string
import subprocess
import sys

def generate_random_string(length):
    """Генерирует случайную ASCII строку заданной длины."""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

def calculate_expected_result(s, n):
    best_sequence = ""
    for i in range(len(s) - n + 1):
        # Предполагаем, что текущая последовательность удовлетворяет условиям
        current_sequence = s[i:i+n]
        if all(ord(current_sequence[j]) < ord(current_sequence[j+1]) for j in range(n-1)):
            # Сохраняем последовательность, если она строго возрастающая
            best_sequence = current_sequence
    return best_sequence

def run_test(input_str, count, input_filename, output_filename):
    """Запускает тест с использованием makefile."""
    input_path = os.path.join('test', 'in', input_filename)
    output_path = os.path.join('test', 'out', output_filename)
    
    with open(input_path, 'w') as f:
        f.write(input_str)

    # Запуск программы на C через makefile, передавая только имена файлов, без пути
    subprocess.run(['make', 'run', f'input={input_filename}', f'output={output_filename}', f'count={str(count)}'], check=True)

    # Чтение результата выполнения программы
    with open(output_path, 'r') as f:
        result = f.read().strip()

    return result

def main(n):
    for i in range(n):
        input_str = generate_random_string(random.randint(1000, 2000))
        count = random.randint(3, 5)

        expected_result = calculate_expected_result(input_str, count)
        input_filename = f'test_{i}.txt'
        output_filename = f'output_{i}.txt'

        result = run_test(input_str, count, input_filename, output_filename)

        if result == expected_result:
            print(f"Тест {i+1}: УСПЕШНО")
        else:
            print(f"Тест {i+1}: ОШИБКА. Ожидалось: {expected_result}, Получено: {result}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Использование: python main.py <N>")
        sys.exit(1)

    random.seed(10)

    n = int(sys.argv[1])
    main(n)
