#include "xml_util.h"
#include <mxml.h>

static int sn = 0;

char *build_catalog_response(char *device_id)
{
    mxml_node_t *xml = mxmlNewXML("1.0");
    mxml_node_t *response = mxmlNewElement(xml, "Response");

    mxml_node_t *node = mxmlNewElement(response, "CmdType");
    mxmlNewText(node, 0, "Catalog");

    node = mxmlNewElement(response, "SN");
    mxmlNewTextf(node, 0, "%d", ++sn);

    node = mxmlNewElement(response, "DeviceID");
    mxmlNewText(node, 0, device_id);

    node = mxmlNewElement(response, "SumNum");
    mxmlNewTextf(node, 0, "%d", 1);

    mxml_node_t *device_list = mxmlNewElement(response, "DeviceList");
    mxmlElementSetAttrf(device_list, "Num", "%d", 1);

    mxml_node_t *item = mxmlNewElement(device_list, "Item");

    node = mxmlNewElement(item, "DeviceID");
    mxmlNewText(node, 0, "33030000001180010002");

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
