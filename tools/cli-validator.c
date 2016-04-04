#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rtrlib/rtrlib.h"

const int connection_timeout = 20;
enum rtr_mgr_status connectionStatus = -1;

static void connectionStatusCallback(const struct rtr_mgr_group *group,
                                     enum rtr_mgr_status status,
                                     const struct rtr_socket *socket,
                                     void *data)
{
    if(status == RTR_MGR_ERROR)
        connectionStatus = status;
}

int connectionError(enum rtr_mgr_status status)
{
    if(status == RTR_MGR_ERROR) {
        // Wait for input before printing error to avoid "broken pipe" error while communicating
        // with the Python program.
        char input[256];
        int inputLength;
        fgets(input, 256, stdin);

        printf("error\n");
        fflush(stdout);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // check arguments, need hostname/IP and port of cache-server
    if (argc < 3) {
        printf("Usage: %s [host] [port]\n", argv[0]);
        return(EXIT_FAILURE);
    }

    // create a TCP transport socket
    struct tr_socket tr_tcp;
    struct tr_tcp_config tcp_config = { argv[1], argv[2], NULL };
    tr_tcp_init(&tcp_config, &tr_tcp);

    // create an rtr_socket and associate it with the transport socket
    struct rtr_socket rtr_tcp;
    rtr_tcp.tr_socket = &tr_tcp;
    struct rtr_mgr_config *conf;

    // create a rtr_mgr_group array with 1 element
    struct rtr_mgr_group groups[1];
    groups[0].sockets = malloc(1 * sizeof(struct rtr_socket*));
    groups[0].sockets_len = 1;
    groups[0].sockets[0] = &rtr_tcp;
    groups[0].preference = 1;

    int ret = rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL,
                        &connectionStatusCallback, NULL);
    rtr_mgr_start(conf);

    char input[256];
    int inputLength;
    int sleepCounter = 0;

    // wait till at least one rtr_mgr_group is fully synchronized with server
    while (!rtr_mgr_conf_in_sync(conf)) {
        if(connectionError(connectionStatus))
            return(EXIT_FAILURE);

        sleep(1);
        sleepCounter++;
        if (sleepCounter >= connection_timeout) {
            // Wait for input before printing "timeout",to avoid "broken pipe"
            // error while communicating with the Python program
            fgets(input, 256, stdin);
            printf("timeout\n");
            fflush(stdout);
            return(EXIT_FAILURE);
        }
    }

    char ip[128];
    int mask;
    int asn;
    int counter;
    int spaces;
    // loop for input
    while (1) {
        // recheck connection, exit on failure
        if(connectionError(connectionStatus))
            return(EXIT_FAILURE);

        // try reading from stdin, exit on failure
        if(!fgets(input, 256, stdin)) {
            printf("input error\n");
            return(EXIT_FAILURE);
        }

        // remove newline, if present
        inputLength = strlen(input) - 1;
        if (input[inputLength] == '\n')
            input[inputLength] = '\0';

        // check if there are exactly 3 arguments
        spaces = 0;
        for (counter = 0; counter < inputLength; counter++) {
            if (input[counter] == ' ' && input[counter + 1] != ' ' &&
                input[counter + 1] != '\0' && counter != 0) {
                spaces++;
            }
        }

        // check input matching pattern
        if (spaces != 2) {
            printf("Arguments required: IP Mask ASN\n");
            fflush(stdout);
            continue;
        }

        char delims[] = " ";
        char *inputToken = NULL;
        inputToken = strtok(input, delims);
        strcpy(ip, inputToken);
        inputToken = strtok(NULL, delims);
        mask = atoi(inputToken);
        inputToken = strtok(NULL, delims); asn = atoi(inputToken);

        struct lrtr_ip_addr pref;
        lrtr_ip_str_to_addr(ip, &pref);
        enum pfxv_state result;
        struct pfx_record* reason = NULL;
        unsigned int reason_len = 0;

        // do validation
        pfx_table_validate_r(groups[0].sockets[0]->pfx_table, &reason,
                             &reason_len, asn, &pref, mask, &result);
        // translate validation result
        int validity_code = -1;
        if (result == BGP_PFXV_STATE_VALID) {
            validity_code = 0;
        } else if (result == BGP_PFXV_STATE_NOT_FOUND) {
            validity_code = 1;
        } else if (result == BGP_PFXV_STATE_INVALID) {
            validity_code = 2;
        }

        // IP Mask BGP-ASN|
        printf("%s %d %d|", ip, mask, asn);

        // ROA-ASN IP MaskMin MaskMax, ..
        if (reason != NULL && reason_len > 0) {
            unsigned int i;
            for (i = 0; i < reason_len; i++) {
                char tmp[100];
                lrtr_ip_addr_to_str(&(reason[i].prefix), tmp, sizeof(tmp));
                printf("%u %s %u %u",   reason[i].asn, tmp,
                                        reason[i].min_len,
                                        reason[i].max_len);
                if ((i + 1) < reason_len)
                printf(",");
            }
        }

        // |validity_code
        printf("|%d", validity_code);

        printf("\n");
        fflush(stdout);
    }

    rtr_mgr_stop(conf);
    rtr_mgr_free(conf);
    free(groups[0].sockets);

    return(EXIT_SUCCESS);
}
