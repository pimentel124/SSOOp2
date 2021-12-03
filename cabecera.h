#define _POSIX_C_SOURCE 200112L

#define DEBUGN1 1  // parse_args()
#define DEBUGN3 1  // execute_line()

#define PROMPT_PERSONAL 1  // si no vale 1 el prompt será solo el carácter de PROMPT

#define RESET_FORMATO "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"

#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 24  // cantidad de trabajos permitidos

char const PROMPT = '$';

#include <errno.h>      //errno
#include <fcntl.h>      //O_WRONLY, O_CREAT, O_TRUNC
#include <signal.h>     //signal(), SIG_DFL, SIG_IGN, SIGINT, SIGCHLD
#include <stdio.h>      //printf(), fflush(), fgets(), stdout, stdin, stderr, fprintf()
#include <stdlib.h>     //setenv(), getenv()
#include <string.h>     //strcmp(), strtok(), strerror()
#include <sys/stat.h>   //S_IRUSR, S_IWUSR
#include <sys/types.h>  //pid_t
#include <sys/wait.h>   //wait()
#include <unistd.h>     //NULL, getcwd(), chdir()

int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_bg(char **args);
int internal_fg(char **args);

char *read_line(char *line);
int parse_args(char **args, char *line);
int execute_line(char *line);
int is_background(char **args);

void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);

int jobs_list_add(pid_t pid, char status, char *command_line);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
