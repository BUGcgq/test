#include "config.h"
#include "APIS_Json.h"

void CFG_DefaultProtocol(pstCFG_PROTOCOLINFO pConfigProtocol)
{
    if (pConfigProtocol == NULL)
    {
        return;
    }

#ifdef OCPP
    pConfigProtocol->ocpp.bisWss = 0;
    pConfigProtocol->ocpp.bisSupported = 0;
    snprintf(pConfigProtocol->ocpp.strprotocolName, sizeof(pConfigProtocol->ocpp.strprotocolName), "ocpp1.6");
    snprintf(pConfigProtocol->ocpp.straddress, sizeof(pConfigProtocol->ocpp.straddress), "evcs.hetivert.com");
    pConfigProtocol->ocpp.u32port = 80;
    snprintf(pConfigProtocol->ocpp.strpath, sizeof(pConfigProtocol->ocpp.strpath), "/inc-server/inc-chargecloud-websocket/inc-websocket");
    snprintf(pConfigProtocol->ocpp.strusername, sizeof(pConfigProtocol->ocpp.strusername), "h5wa015ajqA2Eg8D");
    snprintf(pConfigProtocol->ocpp.strpassword, sizeof(pConfigProtocol->ocpp.strpassword), "LDhf7qM2Pw5P8p92");
#endif


#ifdef NETWORK
    pConfigProtocol->network.u32dhcp = 0;
    strcpy(pConfigProtocol->network.streth0_MAC,"MAC");
    snprintf(pConfigProtocol->network.streth0_IP, sizeof(pConfigProtocol->network.streth0_IP), "192.168.10.100");
    snprintf(pConfigProtocol->network.streth0_MASK, sizeof(pConfigProtocol->network.streth0_MASK), "255.255.255.0");
    snprintf(pConfigProtocol->network.streth0_GATE, sizeof(pConfigProtocol->network.streth0_GATE), "192.168.10.254");
    snprintf(pConfigProtocol->network.strDNS_ADDR0, sizeof(pConfigProtocol->network.strDNS_ADDR0), "8.8.8.8");
#endif

}

int CFG_ParseJsonProtocol(const char *root, pstCFG_PROTOCOLINFO pConfigProtocol)
{
    if (root == NULL || pConfigProtocol == NULL)
    {
        return -1;
    }

    cJSON *jsonObj = cJSON_Parse(root);
    if (jsonObj == NULL)
    {
        return -1;
    }

#ifdef OCPP
    cJSON_GetObjectToNumber(jsonObj, "isWss", pConfigProtocol->ocpp.bisWss);
    cJSON_GetObjectToNumber(jsonObj, "isSupported", pConfigProtocol->ocpp.bisSupported);
    cJSON_GetObjectToString(jsonObj, "protocolName", pConfigProtocol->ocpp.strprotocolName, sizeof(pConfigProtocol->ocpp.strprotocolName));
    cJSON_GetObjectToString(jsonObj, "address", pConfigProtocol->ocpp.straddress, sizeof(pConfigProtocol->ocpp.straddress));
    cJSON_GetObjectToNumber(jsonObj, "port", pConfigProtocol->ocpp.u32port);
    cJSON_GetObjectToString(jsonObj, "path", pConfigProtocol->ocpp.strpath, sizeof(pConfigProtocol->ocpp.strpath));
    cJSON_GetObjectToString(jsonObj, "username", pConfigProtocol->ocpp.strusername, sizeof(pConfigProtocol->ocpp.strusername));
    cJSON_GetObjectToString(jsonObj, "password", pConfigProtocol->ocpp.strpassword, sizeof(pConfigProtocol->ocpp.strpassword));
#endif

#ifdef NETWORK
      cJSON_GetObjectToNumber(jsonObj, "DHCP", pConfigProtocol->network.u32dhcp);
    cJSON_GetObjectToString(jsonObj, "eth0_IP", pConfigProtocol->network.streth0_IP, sizeof(pConfigProtocol->network.streth0_IP));
    cJSON_GetObjectToString(jsonObj, "eth0_MASK", pConfigProtocol->network.streth0_MASK, sizeof(pConfigProtocol->network.streth0_MASK));
    cJSON_GetObjectToString(jsonObj, "eth0_GATE", pConfigProtocol->network.streth0_GATE, sizeof(pConfigProtocol->network.streth0_GATE));
    cJSON_GetObjectToString(jsonObj, "eth0_MAC", pConfigProtocol->network.streth0_MAC, sizeof(pConfigProtocol->network.streth0_MAC));
    cJSON_GetObjectToString(jsonObj, "DNS_ADDR0", pConfigProtocol->network.strDNS_ADDR0, sizeof(pConfigProtocol->network.strDNS_ADDR0));
#endif

    cJSON_Delete(jsonObj);

    return 0;
}



char *CFG_MakeJsonProtocol(const char *root, pstCFG_PROTOCOLINFO pConfigProtocol)
{
    if (pConfigProtocol == NULL)
    {
        return NULL;
    }

    cJSON *jsonObj = cJSON_CreateObject();
    if (jsonObj == NULL)
        return NULL;
#ifdef OCPP
    cJSON_AddNumberToObject(jsonObj, "isSupported", pConfigProtocol->ocpp.bisSupported);
    cJSON_AddNumberToObject(jsonObj, "isWss", pConfigProtocol->ocpp.bisWss);
    cJSON_AddStringToObject(jsonObj, "protocolName", pConfigProtocol->ocpp.strprotocolName);
    cJSON_AddStringToObject(jsonObj, "address", pConfigProtocol->ocpp.straddress);
    cJSON_AddNumberToObject(jsonObj, "port", pConfigProtocol->ocpp.u32port);
    cJSON_AddStringToObject(jsonObj, "path", pConfigProtocol->ocpp.strpath);
    cJSON_AddStringToObject(jsonObj, "username", pConfigProtocol->ocpp.strusername);
    cJSON_AddStringToObject(jsonObj, "password", pConfigProtocol->ocpp.strpassword);
#endif

#ifdef NETWORK
    cJSON_AddNumberToObject(jsonObj, "DHCP", pConfigProtocol->network.u32dhcp);
    cJSON_AddStringToObject(jsonObj, "eth0_IP", pConfigProtocol->network.streth0_IP);
    cJSON_AddStringToObject(jsonObj, "eth0_MASK", pConfigProtocol->network.streth0_MASK);
    cJSON_AddStringToObject(jsonObj, "eth0_GATE", pConfigProtocol->network.streth0_GATE);
    cJSON_AddStringToObject(jsonObj, "eth0_MAC", pConfigProtocol->network.streth0_MAC);
    cJSON_AddStringToObject(jsonObj, "DNS_ADDR0", pConfigProtocol->network.strDNS_ADDR0);
#endif

    char *result = cJSON_Print(jsonObj);
    cJSON_Delete(jsonObj);
    return result;
}


int CFG_ReadProtocol(const char *filePath, pstCFG_PROTOCOLINFO pConfigProtocol)
{
    if (NULL == filePath || NULL == pConfigProtocol)
    {
        printf("filePath or pConfigProtocol is NULL\n");
        return -1;
    }

    char *root = readJsonFromFile(filePath);
    if (root == NULL)
    {
        return -1;
    }

    if (CFG_ParseJsonProtocol(root, pConfigProtocol) == -1)
    {
        free(root);
        return -1;
    }
    free(root);

    return 0;
}

int CFG_SaveProtocol(const char *filePath, pstCFG_PROTOCOLINFO pConfigProtocol)
{
    if (NULL == filePath || NULL == pConfigProtocol)
    {
        printf("filePath or pConfigProtocol is NULL\n");
        return -1;
    }

    char *jsonStr = CFG_MakeJsonProtocol(filePath,pConfigProtocol);
    if (jsonStr == NULL)
    {
        return -1;
    }

    if (saveJsonToFile(jsonStr, filePath) == -1)
    {
        free(jsonStr);
        return -1;
    }
    free(jsonStr);

    return 0;
}