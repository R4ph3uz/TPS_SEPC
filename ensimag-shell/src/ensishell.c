#include "variante.h"
#include "main.h"
#include "readcmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//https://dev.to/angelotheman/build-your-own-shell-part-2-5gie
#ifndef VARIANTE
#error "Variante non défini !!"
#endif

struct jobs
{
	pid_t pid;
	char** command;
	struct timeval start_time;
	struct jobs* next;
} s_jobs;

struct jobs* joblist=NULL;
int input_fd = STDIN_FILENO;
struct rlimit limitu;
int ulimitSet = 0;

void liberer_job(struct jobs* job){
	int compteur= 0;
	while(job->command[compteur]!=NULL){
        free(job->command[compteur]);
		compteur++;
    }
    free(job->command);
    free(job);
}

/* free line et exit */
void terminate(char *line) {
	if (line)
	  free(line);
	while(joblist!=NULL){
		struct jobs* temp=joblist->next;
		liberer_job(joblist);
		joblist=temp;
	}
	printf("exit\n");
	exit(0);
}


void ajouter_jobs(pid_t pidJob, char** commandJob){
	struct jobs* nouveauJob = malloc(sizeof(struct jobs));
	if(nouveauJob == NULL){perror("can't malloc");exit(5);}
	nouveauJob->pid = pidJob;
	int compteur =0;
	while (commandJob[compteur]!=0)
	{
		compteur++;
	}
	nouveauJob->command = malloc(sizeof(char*)*(compteur+1));
	for (int compteur2 = 0; compteur2 <compteur; compteur2++)
	{
		nouveauJob->command[compteur2]= strdup(commandJob[compteur2]);
	}
	nouveauJob->command[compteur]=NULL;
	gettimeofday(&nouveauJob->start_time, NULL);
	nouveauJob->next = joblist;
	joblist=nouveauJob;
}

void affiche_jobs(){
	struct jobs* iteration_jobs = joblist;
	int status;
	while (iteration_jobs !=NULL) // pour chaque job
	{
		if (waitpid(iteration_jobs->pid,&status,WNOHANG) == 0) // s'il attend encore
		{
			printf("[%i] ",iteration_jobs->pid); //affiche le pid
			int compteur =0;
			while (iteration_jobs->command[compteur] !=NULL)
			{
				printf("%s ",iteration_jobs->command[compteur]); // aficher la commande lancée
				compteur++;
			}
			printf("\n");
		}
		iteration_jobs= iteration_jobs->next;
	}
}

void sigchldHandler(){
	pid_t childPID;
	int status;
	while((childPID=waitpid(-1,&status,WNOHANG)) > 0){
		struct timeval end_time;
		gettimeofday(&end_time,NULL);
		struct jobs* currentJob = joblist;
		struct jobs* ancien = NULL;
		while (currentJob!=NULL)
		{
			int tempsEcouleS = (end_time.tv_sec -currentJob->start_time.tv_sec);
			if(currentJob->pid==childPID && ancien==NULL ){
				printf("[%i] a duré %is\nensishell>",childPID,tempsEcouleS);
				joblist=joblist->next;
				liberer_job(currentJob);
				break;
			}
			else if(currentJob->pid == childPID){
				printf("[%i] a duré %is\nensishell>",childPID,tempsEcouleS);
				ancien->next=currentJob->next;
				liberer_job(currentJob);
				break;
			}
			ancien=currentJob;
			currentJob=currentJob->next;

		}
		
	}
}

void sigcpudHandler(){
	printf("[%i] has been killed because of the rlimit", getpid());
}

char **expandWildcardsGlob(char** cmd) {
	glob_t globbuf;
    int flags = GLOB_BRACE | GLOB_TILDE;
	int argcounter=0;
	while (cmd[argcounter] != NULL) {
        argcounter++;
    }


	char **expanded_cmd = malloc((argcounter + 1) * sizeof(char *)); // future commande
    int expanded_index = 0;

	for (int i = 0; i < argcounter; i++) { // pour chaque mot de la commande
		     int length = strlen(cmd[i]);
		while (length > 0 && isspace((unsigned char)cmd[i][length - 1])) { // enlever les espaces à la fin
			cmd[i][length - 1] = '\0';
			length--;
    }
        if (cmd[i][0] == '\0') {
            continue;
        }

		// fait dans wordexp normalement
        // if (cmd[i][0] == '$') {
        //     char *var_name = cmd[i] + 1; // Ignorer le $
        //     char *env_value = getenv(var_name); // recupere l'interieur du $

        //     if (env_value) {
        //         expanded_cmd[expanded_index++] = strdup(env_value);
        //     } else {
        //         expanded_cmd[expanded_index++] = strdup(cmd[i]);
        //     }
        //     continue; // Passer au prochain argument
        // }

        memset(&globbuf, 0, sizeof(globbuf));

		// s'il y a un caractère interpretable par glob 
        if (strchr(cmd[i], '*') || strchr(cmd[i], '?') || strchr(cmd[i], '[') || strchr(cmd[i], '{') || strchr(cmd[i], '~')) {
            int ret = glob(cmd[i], flags, NULL, &globbuf);
            if (ret == 0 && globbuf.gl_pathc != 0) { // si glob a trouvé quelque chose
                for (size_t j = 0; j < globbuf.gl_pathc; j++) {
                    expanded_cmd = realloc(expanded_cmd, (expanded_index + 1) * sizeof(char *));
                    expanded_cmd[expanded_index] = strdup(globbuf.gl_pathv[j]); // mettre les trouvailles de glob dans la nouvelle commande
                    expanded_index++;
                }
            } else {
                expanded_cmd[expanded_index++] = strdup(cmd[i]); // si on a rien trouvé on met l'ancienne commande
            }
           globfree(&globbuf);
        } else {
            expanded_cmd[expanded_index++] = strdup(cmd[i]); // s'il n'y a pas de *,?,.. on met direct la commande
        }
    }
    expanded_cmd[expanded_index] = NULL;

    return expanded_cmd;
}

char **expandWildcardsWordexp(char **cmd) {
	//voir glob car similaire
    wordexp_t p;
    int argcounter = 0;

    while (cmd[argcounter] != NULL) {
        argcounter++;
    }

    char **expanded_cmd = malloc((argcounter + 1) * sizeof(char *));
    int expanded_index = 0;

    for (int i = 0; i < argcounter; i++) {
        int length = strlen(cmd[i]);
        
        while (length > 0 && isspace((unsigned char)cmd[i][length - 1])) {
            cmd[i][length - 1] = '\0';
            length--;
        }

        if (cmd[i][0] == '\0') {
            continue;
        }

        if (wordexp(cmd[i], &p, 0) == 0) {
            for (size_t j = 0; j < p.we_wordc; j++) {
                expanded_cmd = realloc(expanded_cmd, (expanded_index + 1) * sizeof(char *));
                expanded_cmd[expanded_index] = strdup(p.we_wordv[j]);
                expanded_index++;
            }
            wordfree(&p);
        } else {
            expanded_cmd[expanded_index++] = strdup(cmd[i]);
        }
    }
    expanded_cmd[expanded_index] = NULL;

    return expanded_cmd;
}


void execute(char** command,int background,char* inputFileName,char* outputFileName, int currentPipe, int nbPipe){
	pid_t child_pid;
	int status;
	//https://www.mbillaud.fr/notes/pipeline.html
	int pipe_fd[2];

	//https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf
	int inputFile = -1;
	if(inputFileName !=NULL){// s'il y a un input file
		inputFile = open(inputFileName, O_RDONLY); // on l'ouvre en ReadOnly
		if(inputFile == -1) perror("Can't open the file");
	}

	int outputFile = -1;
	if(outputFileName!=NULL){ // meme chose s'il y a un outputfile
		outputFile = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR ); //on l'ouvre en le créant s'il faut, en le vidant, avec les bons droits
		if(outputFile == -1) perror("Can't open the file");
		if(ftruncate(outputFile,0)==-1) perror("Can't truncate the file");
	}

	if (currentPipe < nbPipe-1) { // si on est pas le dernier pipe
            pipe(pipe_fd); // on ecrit les files descriptor dans pipe_fd
        }

	//https://stackoverflow.com/questions/15102328/how-does-fork-work
	// montre que le pere reçoit un nombre !=0 et le fils reçoit 0
	child_pid = fork(); // on crée le pid du parent en forkant
	if (background)
	{
		ajouter_jobs(child_pid,command); // si en bg, ajouter aux jobs en arière plan
	}
	
	if (child_pid == -1){
		perror("Couldn't create the child process.");
		exit (41);
	}
	else if (child_pid == 0){ // le fils reçoit 0
		if (currentPipe > 0) {
			dup2(input_fd, STDIN_FILENO);//on ecrit input file dans STDIN
			close(input_fd);
		}
		if (currentPipe < nbPipe-1) {
			dup2(pipe_fd[1], STDOUT_FILENO); // on ecrit la sortie dans STDOUT
			close(pipe_fd[0]);
			close(pipe_fd[1]);
		}

		if(inputFile != -1 && currentPipe == 0){
			dup2(inputFile, STDIN_FILENO); // ecirt -1 sur l'inputfile
			close(inputFile);
		} 
		if(outputFile != -1 && currentPipe == nbPipe - 1){
			dup2(outputFile, STDOUT_FILENO); //ecrit l'output sur STDOUT
			close(outputFile);
		} 

		if(ulimitSet){ //si on a une limite on doit arreter le fils
			if(setrlimit(RLIMIT_CPU,&limitu) != 0){
				perror("Problem to set rlimit (ulimit command)");
				exit(2);
			} 
		}
			
		if (execvp(command[0], command) == -1){ //on execute la commande
				perror("Couldn't execute");
				exit(1);
		}
	}
	if(!background && currentPipe == nbPipe-1){// on attend si le dernier enfant attend (sleep par exemple) 
		waitpid(child_pid,&status,0);
	}
	if (currentPipe > 0) { // on ferme tous les fichiers
		close(input_fd);
	}
	if (currentPipe < nbPipe-1) {
		close(pipe_fd[1]);
		input_fd = pipe_fd[0];
	}
}



#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	struct cmdline* l = parsecmd( & line);
	int compteur=0;
	while (l->seq[compteur]!=0)
	{
		compteur++;
	}
	
	for (int i=0; l->seq[i]!=0; i++) {
		char **cmd = l->seq[i];
		printf("seq[%d]: ", i);
					for (int j=0; cmd[j]!=0; j++) {
							printf("'%s' ", cmd[j]);
					}
		printf("\n");

		if (!strncmp(cmd[0],"jobs", 4)) {
			affiche_jobs();
		}
		else{
			execute(cmd,l->bg,l->in,l->out, i, compteur);
		}
	}

	/* Remove this line when using parsecmd as it will free it */
	free(line);
	
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		struct cmdline *l;
		char *line=0;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
						continue;
				}
#endif

		//déclare les sigactions et que faire lorsque le signal est envoyé
		struct sigaction sigchild = {.sa_handler = sigchldHandler , .sa_flags= SA_RESTART | SA_NOCLDSTOP};
		sigemptyset(&sigchild.sa_mask);
		sigaction(SIGCHLD,&sigchild,NULL);

		struct sigaction sigcpu = {.sa_handler = sigcpudHandler , .sa_flags= SA_RESTART};
		sigemptyset(&sigcpu.sa_mask);
		sigaction(SIGXCPU,&sigcpu,NULL);



		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {
			terminate(0);
		}
		
		// if (l->err) {
		// 	/* Syntax error, read another command */
		// 	printf("error: %s\n", l->err);
		// 	continue;
		// }

		// if (l->in) printf("in: %s\n", l->in);
		// if (l->out) printf("out: %s\n", l->out);
		// if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
		int compteur=0;
		while (l->seq[compteur]!=0)
		{
			compteur++;
		}
		
		for (int i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			// printf("seq[%d]: ", i);
            //             for (int j=0; cmd[j]!=0; j++) {
            //                     printf("'%s' ", cmd[j]);
            //             }
			// printf("\n");

			if (!strncmp(cmd[0],"jobs", 4)) {
				affiche_jobs(); 
			}
			else if(!strncmp(cmd[0],"ulimit",6)){ // set la ulimit
				limitu.rlim_cur = atoi(cmd[1]);
				limitu.rlim_max = atoi(cmd[1])+5;
				ulimitSet=1;
			}
			else{
				char** globCommand = expandWildcardsGlob(cmd); // augmente la commande par glob
				char** expCommand = expandWildcardsWordexp(globCommand); // augmente la commande par wordexp 

				execute(expCommand,l->bg,l->in,l->out, i, compteur);
			}
		}

	}
}


