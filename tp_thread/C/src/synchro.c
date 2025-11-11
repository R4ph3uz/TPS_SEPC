#include "ensitheora.h"
#include "synchro.h"
#include <pthread.h>

/* les variables pour la synchro, ici */
typedef struct Monitor_s{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Monitor;
Monitor moniteurTaille = {.cond= PTHREAD_COND_INITIALIZER, .mutex= PTHREAD_MUTEX_INITIALIZER};
Monitor moniteurTexture = {.cond= PTHREAD_COND_INITIALIZER, .mutex= PTHREAD_MUTEX_INITIALIZER};

pthread_mutex_t mutexTexture = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condDepot = PTHREAD_COND_INITIALIZER;
pthread_cond_t condConso = PTHREAD_COND_INITIALIZER;

int nombreTexture = 0;

/* l'implantation des fonctions de synchro ici */
void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&moniteurTaille.mutex);
    windowsx = buffer[0].width;
    windowsy = buffer[0].height;
    pthread_cond_broadcast(&moniteurTaille.cond);
    pthread_mutex_unlock(&moniteurTaille.mutex);
}

void attendreTailleFenetre() {
    pthread_mutex_lock(&moniteurTaille.mutex);
    pthread_cond_wait(&moniteurTaille.cond, &moniteurTaille.mutex);
    pthread_mutex_unlock(&moniteurTaille.mutex);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&moniteurTexture.mutex);
    pthread_cond_broadcast(&moniteurTexture.cond);
    pthread_mutex_unlock(&moniteurTexture.mutex);
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&moniteurTexture.mutex);
    pthread_cond_wait(&moniteurTexture.cond, &moniteurTexture.mutex);
    pthread_mutex_unlock(&moniteurTexture.mutex);
}

void debutConsommerTexture() {
    pthread_mutex_lock(&mutexTexture);
    while (nombreTexture == 0)
    {
        pthread_cond_wait(&condDepot, &mutexTexture);
    }
    nombreTexture--;
    pthread_mutex_unlock(&mutexTexture);
}

void finConsommerTexture() {
    pthread_mutex_lock(&mutexTexture);
    pthread_cond_signal(&condConso);
    pthread_mutex_unlock(&mutexTexture);
}

void debutDeposerTexture() {
    pthread_mutex_lock(&mutexTexture);
    while (nombreTexture >= NBTEX)
    {
        pthread_cond_wait(&condConso, &mutexTexture);
    }
    nombreTexture++;
    pthread_mutex_unlock(&mutexTexture);
}

void finDeposerTexture() {
    pthread_mutex_lock(&mutexTexture);
    pthread_cond_signal(&condDepot);
    pthread_mutex_unlock(&mutexTexture);
}
