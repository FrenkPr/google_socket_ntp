#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ntp.h"
#include "udp_communication.h"

int main()
{
    udp_client_connection_init();

    char google_pckt_buf[256];
    ntp_packet ntp_pckt;

    init_ntp_packet(&ntp_pckt);

    send_ntp_packet((const char *)&ntp_pckt, 256);

    int ntp_pckt_len = get_ntp_packet((char *)&ntp_pckt, 256);
    unsigned char leap_version_mode_received = ntp_pckt.leap_version_mode;

    printf("Num bytes received: %d\nrfid: %s\n", ntp_pckt_len, ntp_pckt.rfid);
    printf("Leap: %d\nversion: %d\nmode: %d\n", leap_version_mode_received >> 6, (leap_version_mode_received >> 3) & 0b111, leap_version_mode_received & 0b111);
    printf("stratum: %d\npoll: %d\nprecision: %d\n", ntp_pckt.stratum, ntp_pckt.poll, ntp_pckt.precision);
    printf("root delay: %d\nroot dispersion: %d\n", ntp_pckt.root_delay, ntp_pckt.root_dispersion);
    printf("ref timestamp: %llu\noriginal timestamp: %llu\nreceived timestamp: %llu\ntransmit timestamp: %llu\n\n", ntp_pckt.ref_timestamp, 
                                                                                                                  ntp_pckt.original_timestamp,
                                                                                                                  ntp_pckt.receive_timestamp,
                                                                                                                  ntp_pckt.transmit_timestamp);


    time_t time_secs;
    char time_str[30];
    struct tm time_info;

    unsigned long long google_secs = ntp_pckt.receive_timestamp >> 32;

    time(&time_secs);

    printf("time.h epoch day seconds: %llu\n", time_secs);
    printf("google received timestamp seconds: %llu\n\n", google_secs);
    printf("google received timestamp first 4 bits to right: %llu\n\n", ntp_pckt.receive_timestamp & 0x00000000FFFFFFFF);

    google_secs -= (3600 * 24 * 365 * (unsigned long long)70) + (3600 * 24 * 30) - (3600 * 24 * 13);

    localtime_s(&time_info, (time_t *)&google_secs);
    strftime(time_str, sizeof(time_str), "%d/%m/%Y %H:%M:%S", &time_info);
    
    printf("google aligned received timestamp seconds: %llu\n", google_secs);
    printf("%s", time_str);
}