#ifndef UAC_XML_UTIL_H
#define UAC_XML_UTIL_H

#include <mxml.h>

const char *get_xml_root_type(mxml_node_t *root);

const char *get_xml_cmd_type(mxml_node_t *root);

const char *get_xml_sn(mxml_node_t *root);

char *build_catalog_response(const char *device_id, const char *sn);

char *build_keeplive_notify(const char *device_id);

char *build_catalog_subscribe_response(const char *device_id, const char *sn);

char *build_invite_response(const char *device_id, const char *ip);

#endif //UAC_XML_UTIL_H
