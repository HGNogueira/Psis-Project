#ifndef MESSAGES_H
#define MESSAGES_H
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
     *  3 : peer lost father peer
     *  4 : peer demanding photo_id generation
     *  5 : peer demanding father peer to connect
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


/* Define task structure indicating information action to take, these can be:
 *      - Delete photo with photo_id: type = -1
 *      - Search keyword: type = 0
 *      - Add photo with nbytes and photo_name: type = 1
 *      - Add keyword to photo (based on photo_id): type = 2
 *
 *      Note: in case of the "Add photo" functionality this serves as a way to 
 *  comunicate to peer that a photo will be transfered right after this message is sent.
 *
 * this is usefull to be able to trace back what needs be done to update fellow peer
 */

typedef struct task task_t;
struct task{
    /* task type contains information about the action to take with this task
     * -3: dummy task, should be ignored
     * -2: update terminated (sent by updating peer indicating end of update routine)
     * -1: delete photo with photo_id
     *  0: add keyword to photo_id
     *  1: add new photo with photo_name and nbytes;
     *  2: print photo_list
     *  3: print keywordlist
     *  4: return photo_id array related to a keyword (gallery_search_photo)
     *  5: return name of photo with photo_id
     *  6: upload photo back to client
     *
     */
    int type;
    int ID;              //will correspond to the ID of the responsible peer
    uint32_t photo_id;   //corresponds to the filename
    char photo_name[50]; //corresponds to the filename to save photo
    char keyword[50];    //added keyword or lookup keyword
    unsigned photo_size;
    int turns;           //additional indicator to control perpetual propagation
};
#endif
