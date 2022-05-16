/*
 * The prompt functions and conversation function were
 * inspired by/copied from openpam's openpam_ttyconv.c:
 *
 * Copyright (c) 2002-2003 Networks Associates Technology, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <security/pam_appl.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "auth.h"

#define PROMPT_BUFFER_SIZE 512

// 0 ~ normal
// 1 ~ timeout error
// 2 ~ select error
static int prompt_error = 0;


char *prompt(const char *msg, const struct timespec *timeout)
{
    prompt_error = 0;
    char buffer[PROMPT_BUFFER_SIZE];
    char *result = NULL;
    ssize_t len;
    struct timeval *timeout_val = NULL;
    fd_set readfds;

    /* Initialize file descriptor set. */
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);


before_select:
    /* copy timeout */
    if (timeout != NULL)
    {
        timeout_val = malloc(sizeof * timeout_val);

        if (timeout_val == NULL)
            return NULL;

        timeout_val->tv_sec = timeout->tv_sec;
        timeout_val->tv_usec = timeout->tv_nsec / 1000;
    }

    /* Reset errno. */
    errno = 0;

    /* Wait until a string was entered. */
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, timeout_val) != 1)
    {
        switch (errno)
        {
        case 0:
            // ERROR: timeout.
            prompt_error = 1;
            goto out;
        case EINTR:
            // interrupt
            /* A signal was caught. Restart. */
            goto before_select;
        default:
            // ERROR: select error.
            prompt_error = 2;
            goto out;
        }
    }


    /* Read the string from stdin.  At most buffer length - 1 bytes, to
     * leave room for the terminating zero byte. */
    if ((len = read(STDIN_FILENO, buffer, sizeof buffer - 1)) < 0)
        goto out;

    /* Terminate the string. */
    buffer[len] = '\0';

    /* Strip trailing newline characters. */
    for (len = strlen(buffer); len > 0; --len)
        if (buffer[len - 1] != '\r' && buffer[len - 1] != '\n')
            break;

    /* Terminate the string, again. */
    buffer[len] = '\0';

    /* Copy the string.  Success and error paths are the same. */
    result = strdup(buffer);

    /* Clear our buffer. */
    memset(buffer, 0, sizeof buffer);

out:
    free(timeout_val);

    return result;
}

static int conversation(int num_msg, const struct pam_message **msg, struct
                        pam_response **resp, void *appdata_ptr)
{
    struct pam_response *aresp;
    struct timespec *timeout = appdata_ptr;

    if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG)
        return PAM_CONV_ERR;

    if ((aresp = calloc((size_t) num_msg, sizeof * aresp)) == NULL)
        return PAM_BUF_ERR;

    for (int i = 0; i < num_msg; i++)
    {
        switch (msg[i]->msg_style)
        {
        case PAM_PROMPT_ECHO_OFF:
            aresp[i].resp = prompt(msg[i]->msg, timeout);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_PROMPT_ECHO_ON:
            aresp[i].resp = prompt(msg[i]->msg, timeout);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_TEXT_INFO:
        case PAM_ERROR_MSG:
        {
            // ERROR: pam error message.
            /* size_t msg_len = strlen(msg[i]->msg); */
            /* (void) fputs(msg[i]->msg, stderr); */
            /* if (msg_len > 0 && msg[i]->msg[msg_len - 1] != '\n') */
            /*     (void) fputc('\n', stderr); */
        }
        break;
        default:
            goto fail;
        }
    }

    *resp = aresp;
    return PAM_SUCCESS;

fail:
    for (int i = 0; i < num_msg; ++i)
    {
        if (aresp[i].resp != NULL)
        {
            memset(aresp[i].resp, 0, strlen(aresp[i].resp));
            free(aresp[i].resp);
        }
    }

    memset(aresp, 0, num_msg * sizeof * aresp);
    free(aresp);
    *resp = NULL;

    return PAM_CONV_ERR;
}

bool auth(const char *user, struct timespec *timeout, struct AuthResult *ar)
{
    char *pam_tty;
    pam_handle_t *pamh;
    int pam_status;
    int pam_end_status;
    struct pam_conv pamc =
    {
        .conv = conversation,
        .appdata_ptr = timeout,
    };

    ar->status = 0;

    /* initialize pam */
    // "wlock" is a service name to be recorded in pam.
    pam_status = pam_start("wlock", user, &pamc, &pamh);

    if (pam_status != PAM_SUCCESS)
    {
        // ERROR: pam error.
        ar->status = 1;
        strcpy(ar->error_msg, pam_strerror(pamh, pam_status));
        goto end;
    }

    /* get the name of stdin's tty device, if any */
    pam_tty = ttyname(STDIN_FILENO);

    /* set PAM_TTY */
    if (pam_tty != NULL)
    {
        pam_status = pam_set_item(pamh, PAM_TTY, pam_tty);

        if (pam_status != PAM_SUCCESS)
        {
            // ERROR: pam error.
            ar->status = 2;
            strcpy(ar->error_msg, pam_strerror(pamh, pam_status));
            goto end;
        }
    }

    /* authenticate the user */
    pam_status = pam_authenticate(pamh, 0);

    if (pam_status != PAM_SUCCESS)
    {
        // ERROR: pam error.
        if(prompt_error == 0)
        {
            ar->status = 3;
            strcpy(ar->error_msg, pam_strerror(pamh, pam_status));
        }
        else if(prompt_error == 1)
        {
            ar->status = 4;
            strcpy(ar->error_msg, "timeout!");
        }
        else if(prompt_error == 2)
        {
            ar->status = 5;
            strcpy(ar->error_msg, "select() on stdin failed.");
        }
    }

end:
    /* finish pam */
    pam_end_status = pam_end(pamh, pam_status);

    if (pam_end_status != PAM_SUCCESS)
    {
        // ERROR: pam error.
        ar->status = 6;
        strcpy(ar->error_msg, pam_strerror(pamh, pam_status));
    }

    return (pam_status == PAM_SUCCESS);
}
