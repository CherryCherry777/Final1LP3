#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// --- Constantes y Variables Globales ---
#define MAX 20
int vector[MAX];
int numero_a_buscar;

int encontrado = 0;
int indice_encontrado = -1;
long hilo_ganador = -1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int id_hilo;
    int inicio_rango;
    int fin_rango;
} DatosHilo;


// --- Función que ejecutará cada hilo ---

void* funcion_busqueda(void* arg) {
    DatosHilo* datos = (DatosHilo*)arg;
    int i = datos->inicio_rango;

    printf("Hilo %d: buscando en el rango [%d, %d].\n", datos->id_hilo, datos->inicio_rango, datos->fin_rango -1);

    // Bucle de búsqueda en el fragmento asignado
    while (i < datos->fin_rango) {
        // Si el numero ya fue encontrado por otro hilo, salimos del bucle
        pthread_mutex_lock(&mutex);
        if (encontrado) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        if (vector[i] == numero_a_buscar) {
            pthread_mutex_lock(&mutex);
            // Si el número ya fue encontrado por otro hilo, no lo volvemos a marcar
            // En el eventual caso que justo encuentren el número al mismo tiempo, solo uno lo marcará como encontrado
            if (!encontrado) { 
                encontrado = 1;
                indice_encontrado = i;
                hilo_ganador = datos->id_hilo;
                printf("Hilo %d: ¡Número %d encontrado en el índice %d!\n", datos->id_hilo, numero_a_buscar, i);
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        i++;
    }

    free(datos);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // 1. Validar los argumentos de entrada
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numero_a_buscar> <numero_de_hilos>\n", argv[0]);
        return 1;
    }

    numero_a_buscar = atoi(argv[1]);
    int num_hilos = atoi(argv[2]);

    if (num_hilos <= 0) {
        fprintf(stderr, "El número de hilos debe ser un entero positivo.\n");
        return 1;
    }

    // 2. Generar el vector de números aleatorios 
    srand(time(NULL));
    printf("Vector generado (%d elementos):\n[ ", MAX);
    for (int i = 0; i < MAX; i++) {
        vector[i] = rand() % 10;
        printf("%d ", vector[i]);
    }
    printf("]\n\n");

    // 3. Crear y lanzar los hilos 
    pthread_t hilos[num_hilos];
    int fragmento_base = MAX / num_hilos;
    int inicio_actual = 0;

    for (int i = 0; i < num_hilos; i++) {
        DatosHilo* datos_hilo = malloc(sizeof(DatosHilo));
        datos_hilo->id_hilo = i + 1;
        datos_hilo->inicio_rango = inicio_actual;

        // El último hilo toma todos los elementos restantes para manejar divisiones no exactas.
        if (i == num_hilos - 1) {
            datos_hilo->fin_rango = MAX;
        } else {
            datos_hilo->fin_rango = inicio_actual + fragmento_base;
        }
        inicio_actual = datos_hilo->fin_rango;

        // Crear el hilo con sus datos correspondientes
        pthread_create(&hilos[i], NULL, funcion_busqueda, (void*)datos_hilo);
    }

    // 4. Esperar a que todos los hilos terminen su ejecución 
    for (int i = 0; i < num_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    printf("\nTodos los hilos han finalizado la búsqueda.\n");

    // 5. Mostrar el resultado final 
    if (encontrado) {
        printf("RESULTADO: El número %d fue encontrado por el hilo %ld en el índice %d del vector.\n",
               numero_a_buscar, hilo_ganador, indice_encontrado);
    } else {
        printf("RESULTADO: El número %d no está en el vector.\n", numero_a_buscar);
    }

    // 6. Limpieza
    pthread_mutex_destroy(&mutex);

    return 0;
}