#include "cabecera.h"

static char mi_shell[COMMAND_LINE_SIZE]; // variable global para guardar el nombre del minishell
int n_pids = 1;                          // variable global n_pids, trabajos no finalizados

// static pid_t foreground_pid = 0;
struct info_process
{
    pid_t pid;
    char status;
    char cmd[COMMAND_LINE_SIZE];
};
static struct info_process jobs_list[N_JOBS]; // Tabla de procesos. La posición 0 será para el foreground

void imprimir_prompt();

int check_internal(char **args)
{
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
    if (!strcmp(args[0], "ps"))
        return internal_fg(args);
    if (!strcmp(args[0], mi_shell))
        return internal_fg(args);
    if (!strcmp(args[0], "exit"))
        exit(0);
    return 0; // no es un comando interno
}

int internal_cd(char **args)
{
    printf("[internal_cd()→ comando interno no implementado]\n");
    return 1;
}

int internal_export(char **args)
{
    printf("[internal_export()→ comando interno no implementado]\n");
    return 1;
}

int internal_source(char **args)
{
    printf("[internal_source()→ comando interno no implementado]\n");
    return 1;
}

int internal_jobs(char **args)
{
    int i;
    for (i = 1; i < n_pids; i++)
    {
        printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
    }
    return 0;
}

int internal_fg(char **args)
{
#if DEBUGN1
    //printf(GRIS "[internal_fg()→ Esta función enviará un trabajo detenido al foreground reactivando su ejecución, o uno del background al foreground ]\n");
#endif
    int nargs, pos;
    nargs = 0;
    while (args[nargs] != NULL) {
        nargs++;
    }
    // si el comando no tiene el nº de tokens adecuado
    if (nargs != 2){}
        //imprimir_error("Error de sintaxis. Uso: <fg NºTRABAJO>");
    else
    {
        pos = atoi(args[1]); // convierte la posición de string a int
        if (pos > 0 && pos <= n_pids)
        { // si la posición de la lista es de un
            // proceso en background
            if (jobs_list[pos].status == 'D')
            {                                      // si está detenido
                kill(jobs_list[pos].pid, SIGCONT); // hace que el proceso continúe
                jobs_list[pos].status = 'E';       // se vuelve a poner como ejecutándose
                printf("[internal_fg()→ señal 18 (SIGCONT) enviada a %d]\n", jobs_list[pos].pid);
            }
            int i = 0;
            while (jobs_list[pos].cmd[i] != 0 && jobs_list[pos].cmd[i] != '&')
            { // buscamos "&"
                i++;
            }
            if (jobs_list[pos].cmd[i] == '&')
                jobs_list[pos].cmd[i - 1] = 0; // quitamos " &"
            jobs_list[0] = jobs_list[pos];              // mueve el proceso a foreground
            jobs_list_remove(pos);                      // elimina el proceso de background
            printf("%s\n", jobs_list[0].cmd);
            while (jobs_list[0].pid != 0)
            {
                pause(); // hace que el padre espere mientras no acabe el proceso
            }
        }
        else
        {
            fprintf(stderr, ROJO_T "%s", args[1]);
            //imprimir_error(": no existe ese trabajo");
        }
    }
    return 1;
}

int internal_bg(char **args)
{
#if DEBUGN1
    printf("[internal_bg()→ Esta función reactivará un proceso detenido para que siga ejecutándose pero en segundo plano]\n");
#endif
    int nargs, pos;
    nargs = 0;
    while (args[nargs] != NULL) {
        nargs++;
    }
    if (nargs != 2){}
        //imprimir_error("Error de sintaxis. Uso: <bg NºTRABAJO>");
    else
    {
        pos = atoi(args[1]);
        if (pos > 0 && pos <= n_pids)
        {
            if (jobs_list[pos].status == 'E')
            { // si ya está ejecutándose en background
                fprintf(stderr, ROJO_T "%s", args[1]);
                //imprimir_error(": el trabajo ya está en 2º plano");
            }
            else
            {
                jobs_list[pos].status = 'E';
                int i = 0;
                while (jobs_list[pos].cmd[i] != 0 && jobs_list[pos].cmd[i] != '&')
                { // tiene & ?
                    i++;
                }
                if (jobs_list[pos].cmd[i] == 0)
                { // añadimos " &"
                    jobs_list[pos].cmd[i] = ' ';
                    jobs_list[pos].cmd[i + 1] = '&';
                    jobs_list[pos].cmd[i + 2] = 0;
                }
                kill(jobs_list[pos].pid, SIGCONT); // hace que el proceso continúe
                printf("[internal_bg()→ señal 18 (SIGCONT) enviada a %d]\n", jobs_list[pos].pid);
                printf("[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);
            }
        }
        else
        {
            fprintf(stderr, ROJO_T "%s", args[1]);
            //imprimir_error(": no existe ese trabajo");
        }
    }
    return 1;
}

void imprimir_prompt()
{
#if PROMPT_PERSONAL == 1
    printf(NEGRITA ROJO_T "%s" BLANCO_T ":", getenv("USER"));
    printf(AZUL_T "MINISHELL" BLANCO_T "%c " RESET_FORMATO, PROMPT);
#else
    printf("%c ", PROMPT);

#endif
    fflush(stdout);
    return;
}

char *read_line(char *line)
{
    imprimir_prompt();
    char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin); // leer linea
    if (ptr)
    {
        // ELiminamos el salto de línea (ASCII 10) sustituyéndolo por el \0
        char *pos = strchr(line, 10);
        if (pos != NULL)
        {
            *pos = '\0';
        }
    }
    else
    { // ptr==NULL por error o eof
        printf("\r");
        if (feof(stdin))
        { // se ha pulsado Ctrl+D
            fprintf(stderr, "Bye bye\n");
            exit(0);
        }
    }
    return ptr;
}

int parse_args(char **args, char *line)
{
    int i = 0;

    args[i] = strtok(line, " \t\n\r");
#if DEBUGN1
    //fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
#endif
    while (args[i] && args[i][0] != '#')
    { // args[i]!= NULL && *args[i]!='#'
        i++;
        args[i] = strtok(NULL, " \t\n\r");
#if DEBUGN1
        //fprintf(stderr, GRIS "[parse_args()→ token %i: %s]\n" RESET_FORMATO, i, args[i]);
#endif
    }
    if (args[i])
    {
        args[i] = NULL; // por si el último token es el símbolo comentario
#if DEBUGN1
        //fprintf(stderr, GRIS "[parse_args()→ token %i corregido: %s]\n" RESET_FORMATO, i, args[i]);
#endif
    }
    return i;
}

int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    pid_t pid;
    char command_line[COMMAND_LINE_SIZE];
    int bkg;
    memset(command_line, '\0', sizeof(command_line));
    strcpy(command_line, line); //antes de llamar a parse_args() que modifica line
    if (parse_args(args, line) > 0)
    {
        if (check_internal(args))
        {
#if DEBUGN3
            fprintf(stderr, GRIS "[execute_line()→ PID padre: %d (%s)]\n" RESET_FORMATO, getpid(), mi_shell);
#endif

            bkg = is_background(args);
            pid = fork();
            if (pid == 0) /*Proceso hijo*/
            {
                signal(SIGCHLD, SIG_DFL);
                signal(SIGINT, SIG_IGN);
                if (bkg) /*BACKGROUND*/
                {
                    signal(SIGTSTP, SIG_IGN);
                }
                else /*FOREGROUND*/
                {
                    signal(SIGTSTP, SIG_DFL);
                }
                execvp(args[0], args);
                fprintf(stderr, "%s: no se encontró la orden\n", command_line);
                exit(EXIT_FAILURE);
            }
            else if (pid > 0) /*Proceso padre*/
            {
                fprintf(stderr, "[execute_line()→ PID padre: %d(%s)]\n", getpid(), mi_shell);
                fprintf(stderr, "[execute_line()→ PID hijo: %d(%s)]\n", pid, command_line);
                if (bkg) /*BACKGROUND*/
                {
                    jobs_list_add(pid, 'E', command_line);
                }
                else /*FOREGROUND*/
                {
                    jobs_list[0].pid = pid;
                    jobs_list[0].status = 'E';
                    strcpy(jobs_list[0].cmd, command_line);
                    while (jobs_list[0].pid != 0)
                    {
                        pause();
                    }
                }
            }
        }
    }
    return 0;
}

int is_background(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        i++;
    }
    if (strcmp(args[i - 1], "&") == 0)
    {
        args[i - 1] = NULL;
        return 1;
    }
    return 0;
}

int jobs_list_add(pid_t pid, char status, char *command_line)
{
    if (n_pids < N_JOBS)
    {
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].cmd, command_line);
        printf("[%d]\t%d\t%c\t%s\n", n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].status, jobs_list[n_pids].cmd);
        n_pids++;
        return 0;
    }
    else
    {
        fprintf(stderr, "Límite de procesos en jobs_list alcanzado\n");
        return -1;
    }
}

int jobs_list_find(pid_t pid)
{
    int i;
    for (i = 1; i < N_JOBS; i++)
    {
        if (jobs_list[i].pid == pid)
        {
            return i;
        }
    }
    return 0;
}

int jobs_list_remove(int pos)
{
    if (pos < n_pids)
    {
        n_pids--;
        jobs_list[pos].pid = jobs_list[n_pids].pid;
        strcpy(jobs_list[pos].cmd, jobs_list[n_pids].cmd);
        jobs_list[pos].status = jobs_list[n_pids].status;
    }
    else
    {
        return -1;
    }
    return 0;
}

void reaper(int signum)
{
    signal(SIGCHLD, reaper);
    int status;
    pid_t pid;
    int numerojob;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (pid == jobs_list[0].pid)
        { /*FOREGROUND*/
            if (WIFEXITED(status))
            {
                fprintf(stderr, GRIS "[reaper()→ Proceso hijo %d (%s) finalizado con exit(), estado: %d]\n", pid, jobs_list[0].cmd, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                fprintf(stderr, GRIS "[reaper()→ Proceso hijo %d(%s) finalizado por señal %d]\n", pid, jobs_list[0].cmd, WEXITSTATUS(status));
            }

            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
        }
        else
        {
            numerojob = jobs_list_find(pid);
            if (numerojob)
            {
                if (WIFEXITED(status))
                {
                    fprintf(stderr, "\n[reaper()→ Proceso hijo %d en background (%s) finalizado con exit code %d]\n", pid, jobs_list[numerojob].cmd, WEXITSTATUS(status));
                    fflush(stderr);
                }
                else if (WIFSIGNALED(status))
                {
                    fprintf(stderr, "\n[reaper()→ Proceso hijo %d en background (%s) finalizado por señal %d]\n", pid, jobs_list[numerojob].cmd, WTERMSIG(status));
                    fflush(stderr);
                }
                fprintf(stderr, "\nTerminado PID %d (%s) en jobs_list[%d] con estatus %d\n", jobs_list[numerojob].pid, jobs_list[numerojob].cmd, numerojob, status);
                jobs_list_remove(numerojob);
                fflush(stderr);
            }
        }
    }
}
void ctrlc(int signum)
{ //Manejador propio para la señal SIGINT (Ctrl+C).
    signal(SIGINT, ctrlc);
    printf("\n");
    fflush(stdout);
    fprintf(stderr, GRIS "[ctrlc()→ Soy el proceso con PID %d(%s), el proceso en foreground es %d(%s)]\n", getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    if (jobs_list[0].pid > 0)
    { // if theres a process running in foreground
        if (strcmp(jobs_list[0].cmd, mi_shell))
        { //if the process IS NOT the minishell
            fprintf(stderr, GRIS "[ctrlc()→ Señal %d enviada a %d(%s) por %d(%s)]", SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            kill(jobs_list[0].pid, SIGTERM);
        }
        else
        { // if the process is the minishell
            fprintf(stderr, ROJO_T "[ctrlc()→ Señal no enviada debido a que el proceso en foreground es el shell]");
        }
    }
    else
    { //if theres no process running in foreground
        fprintf(stderr, GRIS "[ctrlc()→ Señal %d no enviada por %d(%s) debido a que no hay proceso en foreground]", SIGTERM, getpid(), mi_shell);
    }
    printf("\n");
    fflush(stdout);
}

void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);
    if (jobs_list[0].pid > 0)
    {
        if (strcmp(jobs_list[0].cmd, mi_shell))
        {
            jobs_list[0].status = 'D';
            jobs_list_add(jobs_list[0].pid, jobs_list[0].status, jobs_list[0].cmd);
            fprintf(stderr, "[ctrlz()→ Soy el proceso con PID %d(%s), el proceso en foreground es %d(%s)]\n", getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
            fprintf(stderr, "[ctrlz()→ Señal %d (SIGSTP) enviada a %d(%s) por %d(%s)]\n", signum, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            kill(jobs_list[0].pid, SIGTSTP);
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            strcpy(jobs_list[0].cmd, "");
        }
        else
        {
            fprintf(stderr, "[ctrlz()→ Soy el proceso con PID %d(%s), el proceso en foreground es %d(%s)]\n", getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
            fprintf(stderr, "[ctrlz()→ Señal %d no enviada por %d(%s) debido a que su proceso en foreground es el shell]\n", signum, getpid(), mi_shell);
        }
    }
    else
    {
        fprintf(stderr, "[ctrlz()→ Soy el proceso con PID %d(%s), el proceso en foreground es %d(%s)]\n", getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
        fprintf(stderr, "[ctrlz()→ Señal %d no enviada por %d(%s)] debido a que no hay proceso en foreground]\n", signum, getpid(), mi_shell);
    }
}

int main(int argc, char *argv[])
{
    char line[COMMAND_LINE_SIZE];

    signal(SIGCHLD, reaper);
    //llamada al enterrador de zombies cuando un hijo acaba (señal SIGCHLD)
    signal(SIGINT, ctrlc);
    //SIGINT es la señal de interrupción que produce Ctrl+C
    signal(SIGTSTP, ctrlz);

    memset(line, 0, COMMAND_LINE_SIZE);

    // Inicializamos jobs[0]
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'N';
    memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
    strcpy(mi_shell, argv[0]);
    while (1)
    {
        if (read_line(line))
        { // !=NULL
            execute_line(line);
        }
    }
    return 0;
}