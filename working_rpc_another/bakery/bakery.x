struct BAKERY_REQUEST {
    int client_id;
    char request_type[50];
    char details[100];
};

struct BAKERY_RESPONSE {
    int client_id;
    int ticket_number;
    int queue_position;
    char status[100];
};

program BAKERY_PROG {
    version BAKERY_VER {
        BAKERY_RESPONSE SUBMIT_REQUEST(BAKERY_REQUEST) = 1;
        int GET_TICKET_NUMBER(int) = 2;
        int GET_QUEUE_POSITION(int) = 3;
    } = 1;
} = 0x30000001;