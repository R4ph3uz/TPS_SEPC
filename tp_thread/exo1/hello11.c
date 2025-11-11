#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pthread_t threads[11];
int counter = 0;

typedef struct Monitor_s{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Monitor;


Monitor moniteur = {.cond= PTHREAD_COND_INITIALIZER, .mutex= PTHREAD_MUTEX_INITIALIZER};

void wait_if_ten() {
    pthread_mutex_lock(&moniteur.mutex);
    pthread_cond_wait(&moniteur.cond, &moniteur.mutex);
    pthread_mutex_unlock(&moniteur.mutex);
}

void increment_counter() {
    pthread_mutex_lock(&moniteur.mutex);
    counter++;
    if (counter == 10) {
        pthread_cond_broadcast(&moniteur.cond);
        printf("Counter reached 10\n");
    }
    pthread_mutex_unlock(&moniteur.mutex);
}


void* print_done(void* arg) {
    wait_if_ten();
    (void)arg;
    printf("done\n");
    return 0;
}

void* print_hello(void* arg) {
    (void)arg;
    printf("hello world\n");
    increment_counter();
    return 0;
}

int main()
{
    pthread_create(&threads[10], NULL, print_done, NULL);
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, print_hello, NULL);
        pthread_detach(threads[i]);
    }

    pthread_join(threads[10], NULL);
    return 0;
}
