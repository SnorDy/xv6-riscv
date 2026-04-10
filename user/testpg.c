#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PTE_A (1L << 6)
#define PTE_D (1L << 7)
int global_var = 12;

void run_test(char* name, void* addr, uint size, int is_stack) {
    printf("\n--- Тестирование: %s ---\n", name);
    if (clear_pte_flags(addr, size, PTE_A | PTE_D) < 0) {
        printf("Ошибка: clear_pte_flags вернул ошибку!\n");
        return;
    }
    
    int res = check_pte_flags(addr, size, PTE_A | PTE_D);
    printf("После очистки: установлены ли A или D? %d (ожидается 0)\n", res);

    volatile char c = *((char*)addr);
    (void)c; 
    
    res = check_pte_flags(addr, size, PTE_A);
    printf("После чтения: установлен ли флаг A? %d (ожидается 1)\n", res);
    
    res = check_pte_flags(addr, size, PTE_D);
    if (is_stack) {
        printf("После чтения: установлен ли флаг D? %d (ожидается 1, т.к. вызовы функций пишут в стек)\n", res);
    } else {
        printf("После чтения: установлен ли флаг D? %d (ожидается 0)\n", res);
    }
    *((char*)addr) = 'X';
    
    res = check_pte_flags(addr, size, PTE_D);
    printf("После записи: установлен ли флаг D? %d (ожидается 1)\n", res);
}

int main() {
    int stack_var = 1;
    int stack_arr[100];
    uint heap_size = 4096 * 3; 
    char *heap_arr;

    printf("Таблица страниц при старте\n");
    pgtable_dump();

    heap_arr = malloc(heap_size);
    for(int i = 0; i < heap_size; i += 4096) {
        heap_arr[i] = 1; 
    }

    printf("\nТаблица страниц после выделения памяти в куче\n");
    pgtable_dump();

    run_test("Глобальная переменная", &global_var, sizeof(global_var), 0);
    run_test("Переменная на стеке", &stack_var, sizeof(stack_var), 1);
    run_test("Массив на стеке", stack_arr, sizeof(stack_arr), 1);
    run_test("Массив в куче (несколько страниц)", heap_arr, heap_size, 0);

    free(heap_arr);
    printf("\nТаблица страниц после освобождения памяти\n");
    pgtable_dump();

    exit(0);
}