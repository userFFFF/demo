#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <eXosip2/eXosip.h>
#include <zlog.h>
#include <mxml.h>
#include "xml_util.h"

static volatile int keepRunning = 1;

int port = 5060;
char *ua = "uac";
char *username = "34010000001110000018";
char *password = "zjxc";
char *deviceid = "34010000001110000018";
char *fromuser = "sip:34010000001110000018@192.168.78.168:5060";
char *proxy = "sip:33030000002000000001@122.224.82.77:5160";
char *contact = "sip:34010000001110000018@192.168.78.168:5060";
char *localip = "192.168.78.168";
int expiry = 300;
struct eXosip_t *ctx;

int timer_interval = 30;

void send_register()
{
    if (eXosip_add_authentication_info(ctx, username, username, password, NULL, NULL)) {
        dzlog_error("eXosip_add_authentication_info failed");
    }

    osip_message_t *reg = NULL;
    int regid = eXosip_register_build_initial_register(ctx, fromuser, proxy, contact, expiry * 2, &reg);
    if (regid < 1) {
        dzlog_error("eXosip_register_build_initial_register failed");
    }

    if (eXosip_register_send_register(ctx, regid, reg)) {
        dzlog_error("eXosip_register_send_register failed");
    }
}

void send_keeplive(union sigval v)
{
    char *notify = build_keeplive_notify(deviceid);

    osip_message_t *message = NULL;
    int i = eXosip_message_build_request(ctx, &message, "MESSAGE", proxy, fromuser, NULL);
    if (i != 0) {
        dzlog_error("eXosip_message_build_request failed");
    }
    osip_message_set_body(message, notify, strlen(notify));
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(ctx);
    i = eXosip_message_send_request(ctx, message);
    if (i < 0) {
        dzlog_error("eXosip_message_send_request failed: %d", i);
    } else {
        dzlog_debug("eXosip_message_send_request tid: %d", i);
    }
    eXosip_unlock(ctx);
    free(notify);
}

timer_t timerid = 0;

void start_timer()
{
    if(timerid != 0)
        return;

    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent));

    evp.sigev_value.sival_int = 0;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = send_keeplive;

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        dzlog_error("fail to timer_create");
    }

    struct itimerspec it;
    it.it_interval.tv_sec = timer_interval;
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = timer_interval;
    it.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &it, NULL) == -1)
    {
        dzlog_error("fail to timer_settime");
    }
}

void message_body_parser(char *body)
{
    mxml_node_t *root = mxmlLoadString(NULL, body, MXML_TEXT_CALLBACK);

    const char *roottype = get_xml_root_type(root);
    const char *cmdtype = get_xml_cmd_type(root);
    const char *sn = get_xml_sn(root);

    if (strcmp(roottype, "Query") == 0) {
        if (strcmp(cmdtype, "Catalog") == 0) {
            char *response = build_catalog_response(deviceid, sn);
            dzlog_info("build_catalog_response: %s", response);

            osip_message_t *message = NULL;
            int i = eXosip_message_build_request(ctx, &message, "MESSAGE", proxy, fromuser, NULL);
            if (i != 0) {
                dzlog_error("eXosip_message_build_request failed");
            }
            osip_message_set_body(message, response, strlen(response));
            osip_message_set_content_type(message, "Application/MANSCDP+xml");
            i = eXosip_message_send_request(ctx, message);
            if (i < 0) {
                dzlog_error("eXosip_message_send_request failed: %d", i);
            } else {
                dzlog_debug("eXosip_message_send_request tid: %d", i);
            }
            free(response);
        }
    }
}

char *subscribe_body_parser(char *body)
{
    mxml_node_t *root = mxmlLoadString(NULL, body, MXML_TEXT_CALLBACK);

    const char *roottype = get_xml_root_type(root);
    const char *cmdtype = get_xml_cmd_type(root);
    const char *sn = get_xml_sn(root);

    if (strcmp(roottype, "Query") == 0) {
        if (strcmp(cmdtype, "Catalog") == 0) {
            char *response = build_catalog_subscribe_response(deviceid, sn);
            dzlog_info("build_catalog_subscribe_response: %s", response);
            return response;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (dzlog_init("zlog.conf", "uac")) {
        printf("dzlog_init failed\n");
        return EXIT_FAILURE;
    }

    dzlog_info("============uac start==================");

    TRACE_INITIALIZE(6, NULL);

    ctx = eXosip_malloc();

    if (eXosip_init(ctx)) {
        dzlog_error("eXosip_init failed");
        return EXIT_FAILURE;
    }

    if (eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 0)) {
        dzlog_error("eXosip_listen_addr failed");
        return EXIT_FAILURE;
    }

    eXosip_set_user_agent(ctx, ua);

    send_register();

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

                start_timer();
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

                osip_message_t *answer;
                int i = eXosip_message_build_answer(ctx, event->tid, 200, &answer);
                if (i != 0) {
                    dzlog_error("eXosip_message_build_answer failed: %d", i);
                    break;
                }
                i = eXosip_message_send_answer(ctx, event->tid, 200, answer);
                if (i != 0) {
                    dzlog_error("eXosip_message_send_answer failed: %d", i);
                    break;
                }
                dzlog_info("%s accept with 200", event->request->sip_method);

                osip_body_t *body = NULL;
                osip_message_get_body(event->request, 0, &body);
                if (body) {
                    dzlog_info("msg body:%s", body->body);

                    message_body_parser(body->body);
                } else {
                    dzlog_info("msg nobody");
                }
                break;
            }
            case EXOSIP_MESSAGE_ANSWERED: {
                dzlog_info("EXOSIP_MESSAGE_ANSWERED");
                dzlog_info("answered tid:%d", event->tid);
                break;
            }
            case EXOSIP_MESSAGE_SERVERFAILURE: {
                dzlog_info("EXOSIP_MESSAGE_SERVERFAILURE");
                break;
            }
            case EXOSIP_IN_SUBSCRIPTION_NEW: {
                dzlog_info("EXOSIP_IN_SUBSCRIPTION_NEW");
                osip_body_t *body = NULL;
                osip_message_get_body(event->request, 0, &body);
                if (body) {
                    dzlog_info("msg body:%s", body->body);

                    char *resp = subscribe_body_parser(body->body);
                    if(resp) {
                        osip_message_t *answer;
                        int i = eXosip_insubscription_build_answer(ctx, event->tid, 200, &answer);
                        if (i != 0) {
                            dzlog_error("eXosip_message_build_answer failed: %d", i);
                            break;
                        }

                        osip_message_set_body(answer, resp, strlen(resp));
                        osip_message_set_content_type(answer, "Application/MANSCDP+xml");
                        i = eXosip_insubscription_send_answer(ctx, event->tid, 200, answer);
                        if (i != 0) {
                            dzlog_error("eXosip_message_send_answer failed: %d", i);
                            break;
                        }
                        dzlog_info("%s accept with 200", event->request->sip_method);
                        /*
                        char *response = build_catalog_response(deviceid, snbuf);
                        dzlog_info("build_catalog_response: %s", response);

                        osip_message_t *message = NULL;
                        i = eXosip_message_build_request(ctx, &message, "NOTIFY", proxy, fromuser, NULL);
                        if (i != 0) {
                            dzlog_error("eXosip_message_build_request failed");
                        }
                        osip_message_set_header(message, "Subscription-State", "active;expires=90;retry-after=0");
                        osip_message_set_header(message, "Event", "presence");
                        osip_message_set_body(message, response, strlen(response));
                        osip_message_set_content_type(message, "Application/MANSCDP+xml");
                        i = eXosip_message_send_request(ctx, message);
                        if (i < 0) {
                            dzlog_error("eXosip_message_send_request failed: %d", i);
                        } else {
                            dzlog_debug("eXosip_message_send_request tid: %d", i);
                        }
                        free(response);*/
                    }
                } else {
                    dzlog_info("msg nobody");
                }
                break;
            }
            case EXOSIP_CALL_INVITE: {
                dzlog_info("EXOSIP_CALL_INVITE");
                osip_body_t *body = NULL;
                osip_message_get_body(event->request, 0, &body);
                if (body) {
                    dzlog_info("msg body:%s", body->body);
                } else {
                    dzlog_info("msg nobody");
                }

                osip_message_t *answer;
                int i = eXosip_call_build_answer(ctx, event->tid, 200, &answer);
                if (i != 0) {
                    dzlog_error("eXosip_call_build_answer failed: %d", i);
                    break;
                }

                char *resp = build_invite_response(deviceid, localip);
                osip_message_set_body(answer, resp, strlen(resp));
                osip_message_set_content_type(answer, "application/SDP");
                i = eXosip_call_send_answer(ctx, event->tid, 200, answer);
                if (i != 0) {
                    dzlog_error("eXosip_call_send_answer failed: %d", i);
                    break;
                }
                dzlog_info("%s accept with 200", event->request->sip_method);

                break;
            }
            case EXOSIP_CALL_NOANSWER: {
                dzlog_info("EXOSIP_CALL_NOANSWER");
                break;
            }
            case EXOSIP_CALL_ACK: {
                dzlog_info("EXOSIP_CALL_ACK");

//                osip_message_t *ack;
//                int i = eXosip_call_build_ack(ctx, event->did, &ack);
//                if (i != 0) {
//                    dzlog_error("eXosip_call_build_answer failed: %d", i);
//                    break;
//                }
//
//                i = eXosip_call_send_ack(ctx, event->did, ack);
//                if (i != 0) {
//                    dzlog_error("eXosip_call_send_ack failed: %d", i);
//                    break;
//                }

                break;
            }
            case EXOSIP_CALL_MESSAGE_NEW: {
                dzlog_info("EXOSIP_CALL_MESSAGE_NEW");
                break;
            }
            case EXOSIP_CALL_CLOSED:{
                dzlog_info("EXOSIP_CALL_CLOSED");
                break;
            }
            case EXOSIP_CALL_RELEASED:{
                dzlog_info("EXOSIP_CALL_RELEASED");
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