#ifndef NTP_H
    #define NTP_H 1
#endif

#include <time.h>

struct ntp_packet
{
    unsigned char leap_version_mode;
    unsigned char stratum;
    unsigned char poll;
    unsigned char precision;
    unsigned int root_delay;
    unsigned int root_dispersion;
    char rfid[5];
    unsigned long long ref_timestamp;
    unsigned long long original_timestamp;
    unsigned long long receive_timestamp;
    unsigned long long transmit_timestamp;

} typedef ntp_packet;

int leap = 0;
int version = 4;
int mode = 3; //client

__inline unsigned __int64 htonll ( unsigned __int64 Value );

/// @brief inits a big endian ntp packet
/// @param ntp_pckt the ntp packet to initialize
void init_ntp_packet(ntp_packet *ntp_pckt)
{
    ntp_pckt->leap_version_mode = leap << 6 | version << 3 | mode;
    ntp_pckt->stratum = 0;
    ntp_pckt->poll = 0;
    ntp_pckt->precision = 0;
    ntp_pckt->root_delay = 0;
    ntp_pckt->root_dispersion = 0;
    strcpy_s(ntp_pckt->rfid, sizeof(ntp_pckt->rfid), "\0\0\0\0");
    ntp_pckt->ref_timestamp = 0;
    ntp_pckt->original_timestamp = htonll(time(NULL));
    ntp_pckt->receive_timestamp = 0;
    ntp_pckt->transmit_timestamp = 0;
}