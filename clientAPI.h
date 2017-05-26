/*
 *  File name: clientAPI.h
 *
 *  Description: This file describes the interface
 *  API to be used by client programs in order to comunicate and
 *  interact with distributed server system (Psis Project)
 *
 */

/* Prevent multiple inclusions                                      */
#ifndef clientAPIHeader
#define clientAPIHeader

#include <netinet/in.h>  /* definition of in_port_t data type */

/*
 *  Function:
 *    update_peer_loss();
 *
 *  Description:
 *    contacts gateway communcating that peer stopped answering
 *
 *  Arguments:
 *      void
 *
 *  Return value:
 *      void
 *
 */
void update_peer_loss();



/*
 *  Function:
 *    gallery_connect
 *
 *  Description:
 *    connects client to one Peer. Must be called before attempting other
 *    client-server interactions.
 *
 *  Arguments:
 *    char *host: gateway address string
 *    in_port_t port: gateway port
 *
 *  Return value:
 *    socket descriptor in case of success.
 *    -1 if gateway cannot be accessed
 *    0 if no servers are available
 *
 */
int gallery_connect(char *host, in_port_t port);

/*
 *  Function:
 *    gallery_add_photo
 *
 *  Description:
 *    Inserts new photo into the system.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    char *file_name: name of the photo file. The photos should be read from the program directory
 *
 *  Return value:
 *    Positive integer corresponding to the photo identifier
 *    0 in case of error((Invalid file_name or problems in communication with the server).
 *
 */

uint32_t gallery_add_photo(int peer_socket, char *file_name);
 
/*
 *  Function:
 *    gallery_add_keyword
 *
 *  Description:
 *    Add a new keyword to a photo already in the system.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    uint32_t id_photo:
 *    char *keyword:
 *
 *  Return value:
 *    Positive integer corresponding to the photo identifier
 *    0 in case of error((Invalid file_name or problems in communication with the server).
 *
 */

int gallery_add_keyword(int peer_socket, uint32_t id_photo, char *keyword);

/*
 *  Function:
 *    gallery_search_photo
 *
 *  Description:
 *    searches on the system for all photos that contain the provided keyword.
 *    The identifiers of the found photos will be stored in the array pointer by id_photos.
 *    This argument will point to an array created inside the function using calloc.
 *    The length of the id_photos array will returned by the function.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    char *keyword: keyword that will be used in the search
 *    uint32_t ** id_photos: The id of the photos will be stored in an array
 *
 *  Return value:
 *    positive integer corresponding to the number of found photos
 *    0 if no photo contains the provided keyword
 *    -1 if a error occurred
 */

int gallery_search_photo(int peer_socket, char * keyword, uint32_t ** id_photos);

/*
 *  Function:
 *    gallery_delete_photo
 *
 *  Description:
 *    Removes from the system the photo identified by id_photo.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    uint32_t id_photos: identifier of the photo to be removed from the system
 *
 *  Return value:
 *    1 if the photo is removed successfully
 *    0 if the photo does no exist
 *   -1 if case of error
 */

int gallery_delete_photo(int peer_socket, uint32_t id_photo);

int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name);

 /*
  *  Function:
  *    gallery_get_photo
  *
  *  Description:
  *    Retrieves from the system the name of the photo identified by id_photo
  *
  *  Arguments:
  *  int peer_socket:
  *  uint32_t id_photo:
  *  char *file_name:
  *
  *  Return value:
  *    1 if the photo is downloaded successfully
  *    0 if the photo does not exist
  *   -1 if case of error
  */

 int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name);

#endif
