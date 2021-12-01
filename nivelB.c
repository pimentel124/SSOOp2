/*
nivelA.c - Adelaida Delgado (adaptación de nivel3.c)
Cada nivel incluye la funcionalidad de la anterior.
nivel A: Muestra el prompt, captura la línea de comandos,
la trocea en tokens y chequea si se trata de comandos internos
(cd, export, source, jobs, fg, bg o exit). Si son externos los ejecuta con execvp()
*/

#include "cabecera.h"

static char mi_shell[COMMAND_LINE_SIZE];  // variable global para guardar el nombre del minishell

// static pid_t foreground_pid = 0;
struct info_process {
    pid_t pid;
    char status;
    char cmd[COMMAND_LINE_SIZE];
};
static struct info_process jobs_list[N_JOBS];  // Tabla de procesos. La posición 0 será para el foreground

void imprimir_prompt();
void ctrlc();
void reaper();

int check_internal(char **args) {
    if (!strcmp(args[0], "cd"))
        return internal_cd(args);
    if (!strcmp(args[0], "export"))
        return internal_export(args);
    if (!strcmp(args[0], "source"))
        return internal_source(args);
    if (!strcmp(args[0], "jobs"))
        return internal_jobs(args);
    if (!strcmp(args[0], "bg"))
        return internal_bg(args);
    if (!strcmp(args[0], "pwd"))
        return internal_fg(args);
    if (!strcmp(args[0], "pws"))
        return internal_fg(args);
    if (!strcmp(args[0], "ls"))
        return internal_fg(args);
    if (!strcmp(args[0], "fg"))
        return internal_fg(args);
    if (!strcmp(args[0], "sleep"))
        return internal_fg(args);
    if (!strcmp(args[0], "exit"))
        exit(0);
    return 0;  // no es un comando interno
}

int internal_cd(char **args) {
    printf("[internal_cd()→ comando interno no implementado]\n");
    return 1;
}

int internal_export(char **args) {
    printf("[internal_export()→ comando interno no implementado]\n");
    return 1;
}

int internal_source(char **args) {
    printf("[internal_source()→ comando interno no implementado]\n");
    return 1;
}

int internal_jobs(char **args) {
#if DEBUGN1
    printf("[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n");
#endif
    return 1;
}

int internal_fg(char **args) {
#if DEBUGN1
    printf(GRIS "[internal_fg()→ Esta función enviará un trabajo detenido al foreground reactivando su ejecución, o uno del background al foreground ]\n");
#endif
    return 1;
}

int internal_bg(char **args) {
#if DEBUGN1
    printf("[internal_bg()→ Esta función reactivará un proceso detenido para que siga ejecutándose pero en segundo plano]\n");
#endif
    return 1;
}

void imprimir_prompt() {
#if PROMPT_PERSONAL == 1
    printf(NEGRITA ROJO_T "%s" BLANCO_T ":", getenv("USER"));
    printf(AZUL_T "MINISHELL" BLANCO_T "%c " RESET_FORMATO, PROMPT);
#else
    printf("%c ", PROMPT);

#endif
    fflush(stdout);
    return;
}

char *read_line(char *line) {
    imprimir_prompt();
    char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);  // leer linea
    if (ptr) {
        // ELiminamos el salto de línea (ASCII 10) sustituyéndolo por el \0
        char *pos = strchr(line, 10);
        if (pos != NULL) {
            *pos = '\0';
        }
    } else {  // ptr==NULL por error o eof
        printf("\r");
        if (feof(stdin)) {  // se ha pulsado Ctrl+D
            fprintf(stderr, "Bye bye\n");
            exit(0);
        }
    }
    return ptr;
}

int parse_args(char **args, char *line) {
    int i = 0;

    args[i] = strtok(line, " \t\n\r");
#if DEBUGN1
    fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
#endif
    while (args[i] && args[i][0] != '#') {  // args[i]!= NULL && *args[i]!='#'
        i++;
        args[i] = strtok(NULL, " \t\n\r");
#if DEBUGN1
        fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
#endif
    }
    if (args[i]) {
        args[i] = NULL;  // por si el último token es el símbolo comentario
#if DEBUGN1
        fprintf(stderr, GRIS "[parse_args()→ token %i corregido: %s]\n" RESET_FORMATO, i, args[i]);
#endif
    }
    return i;
}

int execute_line(char *line) {
    char *args[ARGS_SIZE];
    pid_t pid, status;
    char cmd[COMMAND_LINE_SIZE];

    // copiamos la línea de comandos sin '\n' para guardarlo en el array de structs de los procesos
    memset(cmd, '\0', sizeof(cmd));
    strcpy(cmd, line);  // antes de llamar a parse_args() que modifica line

    if (parse_args(args, line) > 0) {
        if (check_internal(args)) {
#if DEBUGN3
            fprintf(stderr, GRIS "[execute_line()→ PID padre: %d (%s)]\n" RESET_FORMATO, getpid(), mi_shell);
#endif

            pid = fork();
            if (pid == 0)  // Proceso Hijo:
            {
                signal(SIGCHLD, SIG_DFL);  // Defaul signal. Interruption by stop signal or exit of child process

                signal(SIGINT, SIG_IGN);  // interruption signal Ctrl+C
                
                fprintf(stderr, GRIS "[execute_line()→ PID hijo: %d(%s)]\n" RESET_FORMATO, getpid(), line);
                execvp(args[0], args);
                fprintf(stderr, "%s: No se ha encontrado el comando\n", line);
                exit(-1);
            } else if (pid > 0)  // Proceso Padre:
            {
                signal(SIGINT, ctrlc);  // associate ctrl+c handler to ctrlc function
                jobs_list[0].status = 'E';
                wait(&status);
                if (WIFEXITED(status)) {
                    fprintf(stderr, GRIS "[execute_line()→ Proceso hijo %d (%s) finalizado con exit(), estado: %d]\n", pid, line, WEXITSTATUS(status));
                } else {
                    if (WIFSIGNALED(status)) {
                        fprintf(stderr, GRIS "[execute_line()→ Proceso hijo %d (%s) finalizado por señal %d]\n", pid, line, WTERMSIG(status));
                    }
                }
            }
        }
    }
    jobs_list[0].pid = 0;
    return 0;
}

int main(int argc, char *argv[]) {
    char line[COMMAND_LINE_SIZE];
    memset(line, 0, COMMAND_LINE_SIZE);

    // Inicializamos jobs[0]
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'N';
    memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));

    strcpy(mi_shell, argv[0]);
    while (1) {
        if (read_line(line)) {  // !=NULL
            execute_line(line);
        }
    }
    return 0;
}

//ctrlc handler of SIGINT signal
void ctrlc(int signum) {
    signal(SIGINT, ctrlc);

    if (jobs_list[0].pid > 0) {  // if theres a process running in foreground

        if (strcmp(jobs_list[0].cmd, mi_shell)) {  //if the process IS NOT the minishell
            fprintf(stderr, GRIS "ctrlc()→ Soy el proceso con PID %d(%s), el proceso en foreground es %d(%s)]\n", getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
            fprintf(stderr, GRIS "ctrlc()→ Señal %d enviada a %d(%s) por %d(%s)]", SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            kill(jobs_list[0].pid, SIGTERM);
        } else {  // if the process is the minishell
            fprintf(stderr, ROJO_T "ctrlc()→ Señal no enviada debido a que el proceso en foreground es el shell");
        }
        }else {  //if theres no process running in foreground
            fprintf(stderr, GRIS "ctrlc())→ Señal %d no enviada por %d(%s) debido a que no hay proceso en foreground", SIGTERM, getpid(), mi_shell);
        }
        printf("\n");
        fflush(stdout);
    


}

//reaper function. Handles the SIGCHLD signal.
void reaper(int signum) {

    pid_t pid;
    int status;

    signal(SIGCHLD, reaper);

    while ((pid = waitpid(-1, &status, WNOHANG) > 0)) {
        if (WIFSIGNALED(status)) {  //if the process has been killed by a signal
            fprintf(stderr, GRIS "[reaper()→ Proceso hijo %d(%s) finalizado por señal %d]\n", pid, jobs_list[0].cmd, WTERMSIG(status));
        }

        else if (WIFEXITED(status)) {  //if the process has exited normally
            //do nothing
        }

        jobs_list[0].pid = 0;
        jobs_list[0].status = 'F';
        strcpy(jobs_list[0].cmd, "");
    }
}