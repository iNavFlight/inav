/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#define _POSIX_C_SOURCE 200809L

#include "soap_client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static int set_socket_timeout(int fd, int timeout_ms) {
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) {
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
        return -1;
    }
    return 0;
}

static int connect_with_timeout(const struct addrinfo* ai, int timeout_ms) {
    int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (fd < 0) {
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        close(fd);
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(fd);
        return -1;
    }

    int one = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0) {
        close(fd);
        return -1;
    }

    int rc = connect(fd, ai->ai_addr, ai->ai_addrlen);
    if (rc == 0) {
        (void)fcntl(fd, F_SETFL, flags);
        return fd;
    }
    if (errno != EINPROGRESS) {
        close(fd);
        return -1;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    rc = select(fd + 1, NULL, &wfds, NULL, &tv);
    if (rc <= 0) {
        close(fd);
        errno = (rc == 0) ? ETIMEDOUT : errno;
        return -1;
    }

    int so_error = 0;
    socklen_t so_error_len = sizeof(so_error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len) != 0 || so_error != 0) {
        close(fd);
        errno = (so_error != 0) ? so_error : errno;
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static int open_tcp_connection(const char* host, const char* port, int timeout_ms) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* result = NULL;
    if (getaddrinfo(host, port, &hints, &result) != 0) {
        return -1;
    }

    int fd = -1;
    for (const struct addrinfo* ai = result; ai != NULL; ai = ai->ai_next) {
        fd = connect_with_timeout(ai, timeout_ms);
        if (fd >= 0) {
            if (set_socket_timeout(fd, timeout_ms) == 0) {
                break;
            }
            close(fd);
            fd = -1;
        }
    }

    freeaddrinfo(result);
    return fd;
}

static int send_all(int fd, const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(fd, data + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        total += (size_t)n;
    }
    return 0;
}

static char* recv_all(int fd, size_t* out_len) {
    size_t cap = 8192;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) {
        return NULL;
    }

    while (1) {
        if (len == cap) {
            size_t new_cap = cap * 2;
            char* next = (char*)realloc(buf, new_cap);
            if (!next) {
                free(buf);
                return NULL;
            }
            buf = next;
            cap = new_cap;
        }

        ssize_t n = recv(fd, buf + len, cap - len, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            free(buf);
            return NULL;
        }
        if (n == 0) {
            break;
        }
        len += (size_t)n;
    }

    char* out = (char*)realloc(buf, len + 1);
    if (!out) {
        free(buf);
        return NULL;
    }
    out[len] = '\0';
    *out_len = len;
    return out;
}

static int parse_http_status(const char* response) {
    int status = 0;
    if (sscanf(response, "HTTP/%*d.%*d %d", &status) == 1) {
        return status;
    }
    return 0;
}

static char* dechunk_http_body(const char* body, size_t body_len, size_t* out_len) {
    size_t cap = body_len + 1;
    char* out = (char*)malloc(cap);
    if (!out) {
        return NULL;
    }
    size_t out_pos = 0;
    size_t pos = 0;

    while (pos < body_len) {
        const char* line_end = strstr(body + pos, "\r\n");
        if (!line_end) {
            free(out);
            return NULL;
        }

        char size_buf[32];
        size_t size_len = (size_t)(line_end - (body + pos));
        if (size_len == 0 || size_len >= sizeof(size_buf)) {
            free(out);
            return NULL;
        }
        memcpy(size_buf, body + pos, size_len);
        size_buf[size_len] = '\0';

        char* endptr = NULL;
        unsigned long chunk_size = strtoul(size_buf, &endptr, 16);
        if (endptr == size_buf) {
            free(out);
            return NULL;
        }

        pos = (size_t)(line_end - body) + 2;
        if (chunk_size == 0) {
            break;
        }
        if (pos + chunk_size + 2 > body_len) {
            free(out);
            return NULL;
        }

        if (out_pos + chunk_size + 1 > cap) {
            size_t new_cap = cap;
            while (out_pos + chunk_size + 1 > new_cap) {
                new_cap *= 2;
            }
            char* next = (char*)realloc(out, new_cap);
            if (!next) {
                free(out);
                return NULL;
            }
            out = next;
            cap = new_cap;
        }

        memcpy(out + out_pos, body + pos, chunk_size);
        out_pos += chunk_size;
        pos += chunk_size;

        if (!(body[pos] == '\r' && body[pos + 1] == '\n')) {
            free(out);
            return NULL;
        }
        pos += 2;
    }

    out[out_pos] = '\0';
    *out_len = out_pos;
    return out;
}

static char* extract_soap_body_raw(const char* xml, size_t xml_len) {
    const char* p = xml;
    const char* end = xml + xml_len;

    const char* body_open = NULL;
    const char* body_name_start = NULL;
    const char* body_name_end = NULL;

    while (p < end) {
        const char* lt = strchr(p, '<');
        if (!lt || lt >= end) {
            break;
        }
        if (lt + 1 < end && (lt[1] == '/' || lt[1] == '?' || lt[1] == '!')) {
            p = lt + 1;
            continue;
        }
        const char* gt = strchr(lt, '>');
        if (!gt || gt >= end) {
            break;
        }

        const char* name_start = lt + 1;
        const char* name_end = name_start;
        while (name_end < gt && *name_end != ' ' && *name_end != '\t' && *name_end != '\r' && *name_end != '\n' && *name_end != '/') {
            name_end++;
        }

        size_t name_len = (size_t)(name_end - name_start);
        if (name_len >= 4 && strncmp(name_end - 4, "Body", 4) == 0) {
            body_open = gt + 1;
            body_name_start = name_start;
            body_name_end = name_end;
            break;
        }
        p = gt + 1;
    }

    if (!body_open || !body_name_start || !body_name_end) {
        return NULL;
    }

    char close_tag[128];
    size_t tag_len = (size_t)(body_name_end - body_name_start);
    if (tag_len + 4 >= sizeof(close_tag)) {
        return NULL;
    }
    close_tag[0] = '<';
    close_tag[1] = '/';
    memcpy(close_tag + 2, body_name_start, tag_len);
    close_tag[tag_len + 2] = '>';
    close_tag[tag_len + 3] = '\0';

    const char* close = strstr(body_open, close_tag);
    if (!close) {
        return NULL;
    }

    size_t inner_len = (size_t)(close - body_open);
    char* inner = (char*)malloc(inner_len + 1);
    if (!inner) {
        return NULL;
    }
    memcpy(inner, body_open, inner_len);
    inner[inner_len] = '\0';
    return inner;
}

int soap_client_init(soap_client_t* c, const char* host, const char* port, const char* path, int timeout_ms) {
    if (!c || !host || !port || !path) {
        return -1;
    }
    memset(c, 0, sizeof(*c));
    if (snprintf(c->host, sizeof(c->host), "%s", host) >= (int)sizeof(c->host)) {
        return -1;
    }
    if (snprintf(c->port, sizeof(c->port), "%s", port) >= (int)sizeof(c->port)) {
        return -1;
    }
    if (snprintf(c->path, sizeof(c->path), "%s", path) >= (int)sizeof(c->path)) {
        return -1;
    }
    c->timeout_ms = timeout_ms;
    return pthread_mutex_init(&c->lock, NULL);
}

void soap_client_destroy(soap_client_t* c) {
    if (c) {
        pthread_mutex_destroy(&c->lock);
    }
}

int soap_client_call_raw_body(soap_client_t* c,
                              const char* soap_action,
                              const char* request_body_xml,
                              char** response_body_xml,
                              int* http_status) {
    if (!c || !request_body_xml || !response_body_xml || !http_status) {
        return -1;
    }

    char host[256];
    char port[16];
    char path[256];
    int timeout_ms = 0;

    pthread_mutex_lock(&c->lock);
    snprintf(host, sizeof(host), "%s", c->host);
    snprintf(port, sizeof(port), "%s", c->port);
    snprintf(path, sizeof(path), "%s", c->path);
    timeout_ms = c->timeout_ms;
    pthread_mutex_unlock(&c->lock);

    const char* envelope_prefix =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        "<soap:Body>";
    const char* envelope_suffix = "</soap:Body></soap:Envelope>";

    size_t body_len = strlen(request_body_xml);
    size_t envelope_len = strlen(envelope_prefix) + body_len + strlen(envelope_suffix);
    char* envelope = (char*)malloc(envelope_len + 1);
    if (!envelope) {
        return -1;
    }
    snprintf(envelope, envelope_len + 1, "%s%s%s", envelope_prefix, request_body_xml, envelope_suffix);

    const char* action_header = (soap_action && soap_action[0] != '\0') ? soap_action : "";
    int req_len = snprintf(NULL,
                           0,
                           "POST %s HTTP/1.1\r\n"
                           "Host: %s:%s\r\n"
                           "Content-Type: text/xml; charset=utf-8\r\n"
                           "SOAPAction: \"%s\"\r\n"
                           "Content-Length: %zu\r\n"
                           "Connection: close\r\n\r\n%s",
                           path,
                           host,
                           port,
                           action_header,
                           envelope_len,
                           envelope);
    if (req_len < 0) {
        free(envelope);
        return -1;
    }

    char* request = (char*)malloc((size_t)req_len + 1);
    if (!request) {
        free(envelope);
        return -1;
    }
    snprintf(request,
             (size_t)req_len + 1,
             "POST %s HTTP/1.1\r\n"
             "Host: %s:%s\r\n"
             "Content-Type: text/xml; charset=utf-8\r\n"
             "SOAPAction: \"%s\"\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n%s",
             path,
             host,
             port,
             action_header,
             envelope_len,
             envelope);

    int fd = open_tcp_connection(host, port, timeout_ms);
    if (fd < 0) {
        free(request);
        free(envelope);
        return -1;
    }

    int rc = 0;
    size_t resp_len = 0;
    char* response = NULL;
    char* body = NULL;
    char* soap_body = NULL;

    if (send_all(fd, request, (size_t)req_len) != 0) {
        rc = -1;
        goto cleanup;
    }

    response = recv_all(fd, &resp_len);
    if (!response || resp_len == 0) {
        rc = -1;
        goto cleanup;
    }

    *http_status = parse_http_status(response);

    char* headers_end = strstr(response, "\r\n\r\n");
    if (!headers_end) {
        rc = -1;
        goto cleanup;
    }
    char* raw_body = headers_end + 4;
    size_t raw_body_len = resp_len - (size_t)(raw_body - response);

    if (strstr(response, "Transfer-Encoding: chunked") || strstr(response, "transfer-encoding: chunked")) {
        size_t decoded_len = 0;
        body = dechunk_http_body(raw_body, raw_body_len, &decoded_len);
        if (!body) {
            rc = -1;
            goto cleanup;
        }
        soap_body = extract_soap_body_raw(body, decoded_len);
    } else {
        soap_body = extract_soap_body_raw(raw_body, raw_body_len);
    }

    if (!soap_body) {
        rc = -1;
        goto cleanup;
    }

    *response_body_xml = soap_body;
    soap_body = NULL;

cleanup:
    if (fd >= 0) {
        close(fd);
    }
    free(request);
    free(envelope);
    free(response);
    free(body);
    free(soap_body);
    return rc;
}
