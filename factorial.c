/*Implementar un programa en C que calcule el factorial de los números que se le pasan
como argumentos. El programa creará tantos hilos con Pthreads como argumentos se
introduzcan y cada uno de los hilos recibirá uno de esos argumentos y calculará su
factorial de manera concurrente, pero la salida que imprimen los hilos deberán ser en
orden (utilice las herramientas de sincronización estudiadas para cumplir con este
proposito). Cada hilo ha de recibir como argumento el número cuyo factorial debe calcular.
Un ejemplo de la ejecución del programa podria ser la siguiente:
$ factorial 3 6 4 1 0
Salida
Factorial de 3: 6
Factorial de 6: 720
Factorial de 4: 24
Factorial de 1: 1
Factorial de 0: 1*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct{
    int id;
    int num;
    pthread_t* t_ant;
} t_data;

unsigned long calcularFactorial(int n){
    //n! = n × (n - 1) × (n - 2) × (n - 3) × ... × 1
    //n! = n*(n-1)!
    if (n == 0) {
        return (unsigned long)1;
    } else if (n == 1){
        return (unsigned long)1;
    } else {
        return (unsigned long)(n*calcularFactorial(n-1));
    }
}

void* crear_thread(void* arg){
    t_data* args = (t_data*) arg; //struct del thread

    //esperar al thread anterior
    if (args->t_ant != NULL){
        pthread_join(*(args->t_ant), NULL);
    }

    printf("Factorial de %d: %ld\n", args->num, calcularFactorial(args->num));
    return NULL;
}

int main(int argc, char *argv[]) {
    //argc = cuenta los argumentos, el nombre del programa es siempre el primer argumento
    //argv = contiene los argumentos, argv[0] es el nombre del programa
    if (argc == 1){
        printf("Introducir al menos un numero para el calculo de factorial.\n");
        return 0;
    }

    //creacion de threads
    pthread_t threads[argc - 1];
    t_data args[argc - 1];

    for (int i = 0; i < (argc - 1); i++){
        args[i].id = i + 1;
        if (i == 0){
            args[i].t_ant = NULL;
        } else {
            args[i].t_ant = &threads[i-1];
        };

        args[i].num = atoi(argv[i + 1]);

        //si la funcion create sale 0, es exitosa
        if (pthread_create(&threads[i],NULL,crear_thread,&args[i]) != 0){
            perror("Fallo algo en el hilo.\n");
            exit(1); //1 = EXIT_FAILURE
        }
        
    }

    //esperar al ultimo thread
    pthread_join(threads[argc - 2], NULL);

    return 0;
}
