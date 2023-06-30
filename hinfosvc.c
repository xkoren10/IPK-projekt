#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define OK_MSG "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n"
#define ERROR_MSG "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\nContent-Length: 15;\r\n\r\n400 Bad Request"

bool isNumeric(const char *str)
{

    for (int i = 0, n = strlen(str); i < n; i++)
    {
        if (!isdigit(str[i]))
        {
            return false;
        }
    }
    return true;
}

int cpusage(void)
{
    long double a[7], b[7], loadavg;
    FILE *fp;

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6]);
    fclose(fp);
    sleep(1);
    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6]);
    fclose(fp);

    loadavg = ((b[0] + b[1] + b[2] + b[4] + b[5] + b[6]) - (a[0] + a[1] + a[2] + a[4] + a[5] + a[6])) / ((b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6]) - (a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6]));

    int output = (int)(round(loadavg * 100));
    return (output);
}

int main(int argc, char *argv[])
{

    int rc;
    int main_socket;

    char host_name_buff[1024];
    char cpu_name_buff[1024];
    char load_buff[4];
    char buff[1024];

    if (argc != 2)
    {
        fprintf(stderr, "Nesprávny počet argumentov.\n");
        return 1;
    }

    else
    {
        if (isNumeric(argv[1]) == false)
        {
            fprintf(stderr, "Nesprávny formát portu.\n");
            return 1;
        }

        int port = atoi(argv[1]);

        // Proc info
        FILE *hostname = popen("cat /proc/sys/kernel/hostname", "r");
        fgets(host_name_buff, 1024, hostname);
        host_name_buff[strcspn(host_name_buff, "\n")] = 0;
        pclose(hostname);
        FILE *cpuname = popen("cat /proc/cpuinfo | grep -E 'model name[[:space:]]*:'| awk -F ': ' '{print $2}' |sort|uniq", "r");
        fgets(cpu_name_buff, 1024, cpuname);
        cpu_name_buff[strcspn(cpu_name_buff, "\n")] = 0;
        pclose(cpuname);

        if ((main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            perror("ERROR: socket");
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in sa_client;
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;
        socklen_t len = sizeof(address);
        socklen_t sa_client_len = sizeof(sa_client);

        if (setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0)
        {
            perror("ERROR: setsockopt");
        }

        // Main loop

        if ((rc = bind(main_socket, (struct sockaddr *)&address, len)) < 0)
        {
            perror("ERROR: bind");
            exit(EXIT_FAILURE);
        }
        if ((rc = listen(main_socket, 1)) < 0)
        {
            perror("ERROR: listen");
            exit(EXIT_FAILURE);
        }

        while (1)
        {

            int comm_socket = accept(main_socket, (struct sockaddr *)&sa_client, &sa_client_len);

            if (comm_socket >= 0)
            {
                int res = 0;
                for (;;)
                {
                    res = recv(comm_socket, buff, 1024, 0);
                    if (res > 0)
                    {
                        buff[res] = '\0';
                        if (strstr(buff, "GET /load "))
                        {
                            sprintf(load_buff, "%d%%", cpusage());
                            send(comm_socket, OK_MSG, strlen(OK_MSG), 0);
                            send(comm_socket, load_buff, strlen(load_buff), 0);
                            close(comm_socket);
                            memset(load_buff, 0, sizeof(load_buff));
                        }
                        else if (strstr(buff, "GET /cpu-name "))
                        {
                            send(comm_socket, OK_MSG, strlen(OK_MSG), 0);
                            send(comm_socket, cpu_name_buff, strlen(cpu_name_buff), 0);
                            close(comm_socket);
                        }
                        else if (strstr(buff, "GET /hostname "))
                        {
                            send(comm_socket, OK_MSG, strlen(OK_MSG), 0);
                            send(comm_socket, host_name_buff, strlen(host_name_buff), 0);
                            close(comm_socket);
                        }
                        else
                        {
                            write(comm_socket, ERROR_MSG, strlen(ERROR_MSG));
                            close(comm_socket);
                        }
                    }
                    else if (res == 0)
                    {
                        close(comm_socket);
                        break;
                    }
                    break;
                }
            }
            else
            {
                perror("ERROR: accept");
                break;
            }
        }
    }
}