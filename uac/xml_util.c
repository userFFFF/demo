#include "xml_util.h"

int sn_generator()
{
    static int sn = 0;
    return ++sn;
}


const char *get_xml_root_type(mxml_node_t *root)
{
    mxml_node_t *node = mxmlGetFirstChild(root);
    return mxmlGetElement(node);
}

const char *get_xml_cmd_type(mxml_node_t *root)
{
    mxml_node_t *node = mxmlGetFirstChild(root);
    node = mxmlGetFirstChild(node);
    return mxmlGetText(node, NULL);
}

const char *get_xml_sn(mxml_node_t *root)
{
    mxml_node_t *node = mxmlFindElement(root, root, "SN",
                                         NULL, NULL,
                                         MXML_DESCEND);

    return mxmlGetText(node, NULL);
}

char *build_catalog_response(const char *device_id, const char *sn)
{
    mxml_node_t *xml = mxmlNewXML("1.0");
    mxml_node_t *response = mxmlNewElement(xml, "Response");

    mxml_node_t *node = mxmlNewElement(response, "CmdType");
    mxmlNewText(node, 0, "Catalog");

    node = mxmlNewElement(response, "SN");
    mxmlNewTextf(node, 0, "%s", sn);

    node = mxmlNewElement(response, "DeviceID");
    mxmlNewText(node, 0, device_id);

    node = mxmlNewElement(response, "SumNum");
    mxmlNewTextf(node, 0, "%d", 1);

    mxml_node_t *device_list = mxmlNewElement(response, "DeviceList");
    mxmlElementSetAttrf(device_list, "Num", "%d", 1);

    mxml_node_t *item = mxmlNewElement(device_list, "Item");

    node = mxmlNewElement(item, "DeviceID");
    mxmlNewText(node, 0, "34010000001320000018");

    node = mxmlNewElement(item, "Name");
    mxmlNewText(node, 0, "Camera");

    node = mxmlNewElement(item, "Manufacturer");
    mxmlNewText(node, 0, "Hik");

    node = mxmlNewElement(item, "Model");
    mxmlNewText(node, 0, "IPC");

    node = mxmlNewElement(item, "Owner");
    mxmlNewText(node, 0, "Owner");

    node = mxmlNewElement(item, "CivilCode");
    mxmlNewText(node, 0, "CivilCode");

    node = mxmlNewElement(item, "Address");
    mxmlNewText(node, 0, "Address");

    node = mxmlNewElement(item, "Parental");
    mxmlNewText(node, 0, "0");

    node = mxmlNewElement(item, "SafetyWay");
    mxmlNewText(node, 0, "0");

    node = mxmlNewElement(item, "RegisterWay");
    mxmlNewText(node, 0, "1");

    node = mxmlNewElement(item, "Secrecy");
    mxmlNewText(node, 0, "0");

    node = mxmlNewElement(item, "Status");
    mxmlNewText(node, 0, "ON");

    char *buf = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);

    mxmlRelease(xml);

    return buf;
}

char *build_keeplive_notify(const char *device_id)
{
    mxml_node_t *xml = mxmlNewXML("1.0");
    mxml_node_t *notify = mxmlNewElement(xml, "Notify");

    mxml_node_t *node = mxmlNewElement(notify, "CmdType");
    mxmlNewText(node, 0, "Keepalive");

    node = mxmlNewElement(notify, "SN");
    mxmlNewTextf(node, 0, "%d", sn_generator());

    node = mxmlNewElement(notify, "DeviceID");
    mxmlNewText(node, 0, device_id);

    node = mxmlNewElement(notify, "Status");
    mxmlNewText(node, 0, "OK");

    node = mxmlNewElement(notify, "Info");

    char *buf = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);

    mxmlRelease(xml);

    return buf;
}

char *build_catalog_subscribe_response(const char *device_id, const char *sn)
{
    mxml_node_t *xml = mxmlNewXML("1.0");
    mxml_node_t *response = mxmlNewElement(xml, "Response");

    mxml_node_t *node = mxmlNewElement(response, "CmdType");
    mxmlNewText(node, 0, "Catalog");

    node = mxmlNewElement(response, "SN");
    mxmlNewTextf(node, 0, "%s", sn);

    node = mxmlNewElement(response, "DeviceID");
    mxmlNewText(node, 0, device_id);

    node = mxmlNewElement(response, "Result");
    mxmlNewText(node, 0, "OK");

    char *buf = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);

    mxmlRelease(xml);

    return buf;
}

char *build_invite_response(const char *device_id, const char *ip)
{
    char *buf = malloc(BUFSIZ);
    sprintf(buf, "v=0\n"
            "o=33030200001180000001 0 0 IN IP4 %s\n"
            "s=Network Video Recorder\n"
            "c=IN IP4 %s\n"
            "t=0 0\n"
            "m=video 62000 RTP/AVP 96\n"
            "a=sendonly\n"
            "a=rtpmap:96 PS/90000\n"
            "a=username:%s\n"
            "a=password:123456\n"
            "a=filesize:0\n"
            "y=0000000001\n"
            "f=", ip, ip, device_id);

    return buf;
}