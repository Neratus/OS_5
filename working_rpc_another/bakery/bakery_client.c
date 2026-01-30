#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bakery.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s server_host\n", argv[0]);
        exit(1);
    }

    char *host = argv[1];
    CLIENT *clnt;
    BAKERY_REQUEST request;
    BAKERY_RESPONSE *response;

    clnt = clnt_create(host, BAKERY_PROG, BAKERY_VER, "tcp");
    if (clnt == NULL)
    {
        clnt_pcreateerror(host);
        exit(1);
    }

    srand(time(NULL));

    // Несколько клиентов с разными ID
    for (int client_id = 1; client_id <= 3; client_id++)
    {
        for (int req_num = 1; req_num <= 3; req_num++)
        {
            // Заполнение запроса
            request.client_id = client_id;

            // Генерация случайной "операции"
            char request_types[][50] = {
                "Buy",
                "Order",
                "Reserve",
                "Pickup",
                "Cancel"};

            char details[][100] = {
                "bread",
                "croissant",
                "cake",
                "birthday cake",
                "baguette",
                "muffin",
                "donut"};

            int type_index = rand() % 5;
            int detail_index = rand() % 7;

            strcpy(request.request_type, request_types[type_index]);
            strcpy(request.details, details[detail_index]);

            printf("Client %d sending request %d: %s %s\n",
                   client_id, req_num, request.request_type, request.details);

            // Случайная задержка между запросами
            usleep((rand() % 500000) + 100000);

            // Вызов RPC
            response = submit_request_1(&request, clnt); // ИЗМЕНИТЬ: submit_request_1 вместо process_request_1
            if (response == NULL)
            {
                clnt_perror(clnt, "RPC call failed");
                continue;
            }

            printf("Response for client %d: %s (ticket %d, position %d)\n\n",
                   response->client_id, response->status,
                   response->ticket_number, response->queue_position);
        }
    }

    clnt_destroy(clnt);
    return 0;
}
