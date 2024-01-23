#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define SERVER_ADDR "216.239.35.12"
#define SERVER_SOCKET_PORT 123

int server_socket_id = 0;
SOCKADDR_IN sock_in;
int sock_in_size = sizeof(sock_in);

unsigned int Endian_UInt32_Conversion(unsigned int value)
{
    return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
}

unsigned long long Endian_UInt64_Conversion(unsigned long long value)
{
    return ((value >> 56) & 0x00000000000000FF) | ((value >> 40) & 0x000000000000FF00) |
           ((value >> 24) & 0x0000000000FF0000) | ((value >> 8) & 0x00000000FF000000) |
           ((value << 8) & 0x000000FF00000000) | ((value << 24) & 0x0000FF00000000) |
           ((value << 40) & 0x00FF0000000000) | ((value << 56) & 0xFF000000000000);
}

int set_nb(int s)
{
#ifdef _WIN32
    unsigned long nb_mode = 1;
    return ioctlsocket(s, FIONBIO, &nb_mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0)
        return flags;
    flags |= O_NONBLOCK;
    return fcntl(s, F_SETFL, flags);
#endif
}

void udp_client_connection_init()
{
#ifdef _WIN32
    // this part is only required on Windows: it initializes the Winsock2 dll
    WSADATA wsa_data;

    if (WSAStartup(0x0202, &wsa_data))
    {
        printf("unable to initialize winsock2 \n");
        exit(-1);
    }
#endif

    server_socket_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (server_socket_id < 0)
    {
        printf("unable to initialize the UDP socket \n");
        exit(-1);
    }

    // set_nb(server_socket_id);

    printf("socket %d created \n", server_socket_id);

    inet_pton(AF_INET, SERVER_ADDR, &sock_in.sin_addr); // this will create a big endian 32 bit address
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(SERVER_SOCKET_PORT); // converts 9999 to big endian
}

void udp_server_connection_init()
{
    udp_client_connection_init();

    if (bind(server_socket_id, (struct sockaddr *)&sock_in, sock_in_size))
    {
        printf("unable to bind the UDP socket\n");
        exit(-1);
    }
}

void send_message(const char message[4096])
{
    char msg[4096];
    strcpy_s(msg, sizeof(msg), message);

    msg[4095] = 0;
    int sent_bytes = sendto(server_socket_id, msg, strnlen_s(msg, sizeof(msg)) + 1, 0, (const struct sockaddr *)&sock_in, sock_in_size);

    printf("sent %d bytes via UDP \n\n", sent_bytes);
}

int send_ntp_packet(const char *msg, int max_packet_len)
{
    int sent_bytes = sendto(server_socket_id, msg, max_packet_len, 0, (const struct sockaddr *)&sock_in, sock_in_size);

    printf("sent %d bytes via UDP \n\n", sent_bytes);

    return sent_bytes;
}

/// @brief returns a big endian ntp_packet in ntp packet buffer
/// @param ntp_pckt_buff the ntp packet buffer where the ntp packet will be received
/// @param max_packet_len the max ntp packet length
/// @return returns received ntp packet length; returns a value < 0 if failed
int get_ntp_packet(char *ntp_pckt_buff, int max_packet_len)
{
    int ntp_pckt_len = recvfrom(server_socket_id, ntp_pckt_buff, max_packet_len, 0, (struct sockaddr *)&sock_in, &sock_in_size);

    ((ntp_packet *)ntp_pckt_buff)->rfid[4] = '\0';
    ((ntp_packet *)ntp_pckt_buff)->root_delay = htonf(((ntp_packet *)ntp_pckt_buff)->root_delay);
    ((ntp_packet *)ntp_pckt_buff)->root_dispersion = htonf(((ntp_packet *)ntp_pckt_buff)->root_dispersion);
    ((ntp_packet *)ntp_pckt_buff)->ref_timestamp = htonll(((ntp_packet *)ntp_pckt_buff)->ref_timestamp);
    ((ntp_packet *)ntp_pckt_buff)->original_timestamp = htonll(((ntp_packet *)ntp_pckt_buff)->original_timestamp);
    ((ntp_packet *)ntp_pckt_buff)->receive_timestamp = htonll(((ntp_packet *)ntp_pckt_buff)->receive_timestamp);
    ((ntp_packet *)ntp_pckt_buff)->transmit_timestamp = htonll(((ntp_packet *)ntp_pckt_buff)->transmit_timestamp);

    return ntp_pckt_len;
}

void wait_for_message()
{
    char msg[4096];
    int len = recvfrom(server_socket_id, msg, sizeof(msg), 0, (struct sockaddr *)&sock_in, &sock_in_size);

    if (len > 0)
    {
        char addr_as_string[64];
        msg[4095] = 0;

        inet_ntop(AF_INET, &sock_in.sin_addr, addr_as_string, sizeof(addr_as_string));

        printf("received %d bytes from %s:%d\n%s\n\n", len, addr_as_string, ntohs(sock_in.sin_port), msg);
    }
}