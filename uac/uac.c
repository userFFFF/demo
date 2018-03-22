#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <eXosip2/eXosip.h>
#include <zlog.h>
#include "xml_util.h"

static volatile int keepRunning = 1;

int main(int argc, char *argv[]) {
    if (dzlog_init("zlog.conf", "uac")) {
        printf("dzlog_init failed\n");
        return EXIT_FAILURE;
    }

    dzlog_info("============uac start==================");

    int port = 5060;
    char *ua = "uac";
    char *username = "33030000001180000002";
    char *password = "123456";
    char *deviceid = "33030000001180000002";
    char *fromuser = "sip:33030000001180000002@192.168.78.168:5060";
    char *proxy = "sip:122.224.82.77:5160";
    char *contact = "sip:33030000001180000002@192.168.78.168:5060";
    int expiry = 300;

    TRACE_INITIALIZE(6, NULL);

    struct eXosip_t *ctx = eXosip_malloc();

    if (eXosip_init(ctx)) {
        dzlog_error("eXosip_init failed");
        return EXIT_FAILURE;
    }

    if (eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 0)) {
        dzlog_error("eXosip_listen_addr failed");
        return EXIT_FAILURE;
    }

    eXosip_set_user_agent(ctx, ua);

    if (eXosip_add_authentication_info(ctx, username, username, password, NULL, NULL)) {
        dzlog_error("eXosip_add_authentication_info failed");
        return EXIT_FAILURE;
    }

    osip_message_t *reg = NULL;
    int regid = eXosip_register_build_initial_register(ctx, fromuser, proxy, contact, expiry * 2, &reg);
    if (regid < 1) {
        dzlog_error("eXosip_register_build_initial_register failed");
        return EXIT_FAILURE;
    }

    if (eXosip_register_send_register(ctx, regid, reg)) {
        dzlog_error("eXosip_register_send_register failed");
        return EXIT_FAILURE;
    }

    for (; keepRunning;) {
        eXosip_event_t *event;

        if (!(event = eXosip_event_wait(ctx, 0, 1))) {
            eXosip_automatic_action(ctx);
            osip_usleep(10000);
            continue;
        }

        eXosip_lock(ctx);
        eXosip_automatic_action(ctx);

        switch (event->type) {
            case EXOSIP_REGISTRATION_SUCCESS: {
                dzlog_info("registrered successfully");
                break;
            }
            case EXOSIP_REGISTRATION_FAILURE: {
                if ((event->response != NULL) && (event->response->status_code == 401)) {
                    dzlog_info("401 Unauthorized");
                } else {
                    dzlog_error("registrered failed");
                }
                break;
            }
            case EXOSIP_MESSAGE_NEW: {
                dzlog_info("EXOSIP_MESSAGE_NEW");
                osip_body_t *body = NULL;
                osip_message_get_body(event->request, 0, &body);
                if (body) {
                    dzlog_info("msg body:%s", body->body);
                } else {
                    dzlog_info("msg nobody");
                }

                osip_message_t *answer;
                int i = eXosip_message_build_answer(ctx, event->tid, 200, &answer);
                if (i != 0) {
                    dzlog_error("eXosip_message_build_answer failed: %s", event->request->sip_method);
                    break;
                }
                i = eXosip_message_send_answer(ctx, event->tid, 200, answer);
                if (i != 0) {
                    dzlog_error("eXosip_message_send_answer failed: %s", event->request->sip_method);
                    break;
                }
                dzlog_info("%s accept with 200", event->request->sip_method);

                char *response = build_catalog_response(deviceid);
                dzlog_info("build_catalog_response: %s", response);

                osip_message_t *message = NULL;
                i = eXosip_message_build_request(ctx, &message, "MESSAGE", proxy, fromuser, NULL);
                if (i != 0) {
                    dzlog_error("eXosip_message_build_request failed");
                    break;
                }
                osip_message_set_body(message, response, strlen(response));
                osip_message_set_content_type(message, "Application/MANSCDP+xml");
                i = eXosip_message_send_request(ctx, message);
                if (i != 0 || message == NULL) {
                    dzlog_error("eXosip_message_send_request failed: %d", i);
                    break;
                }
                free(response);
                break;
            }
            default: {
                dzlog_debug("recieved eXosip event (type, did, cid) = (%d, %d, %d)", event->type, event->did, event->cid);
                osip_body_t *body = NULL;
                osip_message_get_body(event->request, 0, &body);
                if (body) {
                    dzlog_info("msg body:%s", body->body);
                } else {
                    dzlog_info("msg nobody");
                }
                break;
            }

        }

        eXosip_unlock(ctx);
        eXosip_event_free(event);
    }

    eXosip_quit(ctx);

    osip_free(ctx);

    dzlog_info("============uac e n d==================");

    zlog_fini();

    return EXIT_SUCCESS;
}