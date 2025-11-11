#include <stdio.h>
#include <pthread.h>

void* print_done(void* arg) {
    (void)arg;
    printf("done\n");
    return 0;
}

void* print_hello(void* arg) {
    (void)arg;
    printf("hello world\n");
    return 0;
}


int main() {

    pthread_t threads[11];
    pthread_create(&threads[10], NULL, print_done, NULL);
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, print_hello, NULL);
        pthread_detach(threads[i]);
    }
    //soit detach les threads, soit les join
    
    // for (int i = 0; i < 11; i++) {
    //     pthread_join(threads[i], NULL);
    // }

    pthread_join(threads[10], NULL);
    return 0;
}


