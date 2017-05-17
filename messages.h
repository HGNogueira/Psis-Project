#define MESSAGE_LEN 100
#define SRV1_PORT 3001
#define IP_ADDR "10.0.2.15"

typedef struct message_gw{
    /* message type contains information about the purpose of the message
     * sent to the gateway, depends on origin and destination
     *
     *  -> gateway
     * -1 : indicates that a server has died, other fields identify the server
     *  0 : client initiating contact
     *  1 : server initiating contact
     *  2 : peer lost one client (load balancing)
     *
     *  gateway -> client
     *  0 : no available peers in response to client contact
     *  1 : available peers, other fields contain peer info
     *
     *  gateway -> peer
     */
    int  type;
    char address[50];
    int  port;
    int  ID;
} message_gw;

typedef struct message{
    int type;
    char buffer[MESSAGE_LEN];
    int  warning;
} message;

typedef struct photo{
    uint32_t photo_id;
    char photo_name[50];
    uint64_t photo_size;
} photo;
