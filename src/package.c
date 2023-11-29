#include "package.h"

char *createFirmwareVersionPush(const char *id, const char *version, const char *module)
{
    if (id == NULL || version == NULL || module == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string(version));
    if (module != NULL)
    {
        json_object_object_add(jsonObj, "module", json_object_new_string(module));
    }

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseFirmwareInfo(const char *jsondata, struct firmwareInfo *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj != NULL)
    {
        json_object_object_foreach(jsonObj, key, val)
        {
            if (strcmp(key, "id") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->id, json_object_get_string(val));
            }
            else if (strcmp(key, "message") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->message, json_object_get_string(val));
            }
            else if (strcmp(key, "code") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->code, json_object_get_string(val));
            }
            else if (strcmp(key, "version") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->version, json_object_get_string(val));
            }
            else if (strcmp(key, "size") == 0 && json_object_get_type(val) == json_type_int)
            {
                data->size = json_object_get_int(val);
            }
            else if (strcmp(key, "url") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->url, json_object_get_string(val));
            }
            else if (strcmp(key, "sign") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->sign, json_object_get_string(val));
            }
            else if (strcmp(key, "signMethod") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->signMethod, json_object_get_string(val));
            }
            else if (strcmp(key, "md5") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->md5, json_object_get_string(val));
            }
            else if (strcmp(key, "module") == 0 && json_object_get_type(val) == json_type_string)
            {
                strcpy(data->module, json_object_get_string(val));
            }
        }

        json_object_put(jsonObj);
        return 0;
    }

    return -1;
}

char *createFirmwareRatePush(const char *id, const char *step, const char *desc, const char *module)
{
    if (id == NULL || step == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "step", json_object_new_string(step));
    if (desc != NULL)
    {
        json_object_object_add(jsonObj, "desc", json_object_new_string(desc));
    }

    if (module != NULL)
    {
        json_object_object_add(jsonObj, "module", json_object_new_string(module));
    }

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAskFirmwareInfoRequest(const char *id, const char *module)
{
    if (id == NULL)
    {
        return NULL;
    }
    json_object *jsonObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    if (module != NULL)
    {
        json_object_object_add(jsonObj, "module", json_object_new_string(module));
    }
    else
    {
        json_object_object_add(jsonObj, "module", json_object_new_string(""));
    }
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.ota.firmware.get"));
    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAskClockSynRequest(long long milliseconds)
{

    json_object *jsonObj = json_object_new_object();
    json_object_object_add(jsonObj, "deviceSendTime", json_object_new_int64(milliseconds));
    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseDevTimeInfo(const char *jsondata, struct devTime *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    json_object *jsonObj = json_tokener_parse(jsondata);

    if (jsonObj != NULL)
    {
        json_object_object_foreach(jsonObj, key, val)
        {
            if (strcmp(key, "deviceSendTime") == 0 && json_object_get_type(val) == json_type_int)
            {
                data->deviceSendTime = json_object_get_int(val);
            }
            else if (strcmp(key, "serverRecvTime") == 0 && json_object_get_type(val) == json_type_int)
            {
                data->serverRecvTime = json_object_get_int(val);
            }
            else if (strcmp(key, "serverSendTime") == 0 && json_object_get_type(val) == json_type_int)
            {
                data->serverSendTime = json_object_get_int(val);
            }
        }

        json_object_put(jsonObj);
        return 0;
    }

    return -1;
}

char *createFirmwareEvtRequest(const char *id, const struct firmwareEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *firmwareEvtObj = json_object_new_object();
    json_object *inMeterArray = json_object_new_array();
    json_object *outMeterArray = json_object_new_array();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.firmwareEvt.post"));

    json_object_object_add(firmwareEvtObj, "simNo", json_object_new_string(data->simNo));
    json_object_object_add(firmwareEvtObj, "eleModelId", json_object_new_string(data->eleModelId));
    json_object_object_add(firmwareEvtObj, "serModelId", json_object_new_string(data->serModelId));
    json_object_object_add(firmwareEvtObj, "stakeModel", json_object_new_string(data->stakeModel));
    json_object_object_add(firmwareEvtObj, "vendorCode", json_object_new_int(data->vendorCode));
    json_object_object_add(firmwareEvtObj, "devSn", json_object_new_string(data->devSn));
    json_object_object_add(firmwareEvtObj, "devType", json_object_new_int(data->devType));
    json_object_object_add(firmwareEvtObj, "portNum", json_object_new_int(data->portNum));
    json_object_object_add(firmwareEvtObj, "simMac", json_object_new_string(data->simMac));
    json_object_object_add(firmwareEvtObj, "longitude", json_object_new_int(data->longitude));
    json_object_object_add(firmwareEvtObj, "latitude", json_object_new_int(data->latitude));
    json_object_object_add(firmwareEvtObj, "height", json_object_new_int(data->height));
    json_object_object_add(firmwareEvtObj, "gridType", json_object_new_int(data->gridType));
    json_object_object_add(firmwareEvtObj, "btMac", json_object_new_string(data->btMac));
    json_object_object_add(firmwareEvtObj, "meaType", json_object_new_int(data->meaType));
    json_object_object_add(firmwareEvtObj, "otRate", json_object_new_int(data->otRate));
    json_object_object_add(firmwareEvtObj, "otMinVol", json_object_new_int(data->otMinVol));
    json_object_object_add(firmwareEvtObj, "otMaxVol", json_object_new_int(data->otMaxVol));
    json_object_object_add(firmwareEvtObj, "otCur", json_object_new_int(data->otCur));

    json_object_array_add(inMeterArray, json_object_new_string(data->inMeter));
    json_object_object_add(firmwareEvtObj, "inMeter", inMeterArray);

    json_object_array_add(outMeterArray, json_object_new_string(data->outMeter));
    json_object_object_add(firmwareEvtObj, "outMeter", outMeterArray);

    json_object_object_add(firmwareEvtObj, "CT", json_object_new_int(data->CT));
    json_object_object_add(firmwareEvtObj, "isGateLock", json_object_new_int(data->isGateLock));
    json_object_object_add(firmwareEvtObj, "isGroundLock", json_object_new_int(data->isGroundLock));

    json_object_object_add(paramsObj, "firmwareEvt", firmwareEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createVerInfoEvtRequest(const char *id, const struct verInfoEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *verInfoEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.verInfoEvt.post"));

    json_object_object_add(verInfoEvtObj, "devRegMethod", json_object_new_int(data->devRegMethod));
    json_object_object_add(verInfoEvtObj, "pileHardwareVer", json_object_new_string(data->pileHardwareVer));
    json_object_object_add(verInfoEvtObj, "pileSoftwareVer", json_object_new_string(data->pileSoftwareVer));
    json_object_object_add(verInfoEvtObj, "sdkVer", json_object_new_string(data->sdkVer));

    json_object_object_add(paramsObj, "verInfoEvt", verInfoEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createDevMdunInfoEvtRequest(const char *id, const struct devMduInfoEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *devMduInfoEvtObj = json_object_new_object();
    json_object *mduInfoIntArray = json_object_new_array();
    json_object *mduInfoStringArray = json_object_new_array();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.devMdunInfoEvt.post"));

    json_object_object_add(devMduInfoEvtObj, "netMduInfo", json_object_new_string(data->netMduInfo));
    json_object_object_add(devMduInfoEvtObj, "netMduSoftVer", json_object_new_string(data->netMduSoftVer));
    json_object_object_add(devMduInfoEvtObj, "netMduImei", json_object_new_string(data->netMduImei));
    json_object_object_add(devMduInfoEvtObj, "smartGun", json_object_new_int(data->smartGun));

    int i;
    for (i = 0; i < 2; i++)
    {
        json_object_array_add(mduInfoIntArray, json_object_new_int(data->mduInfoInt[i]));
        json_object_array_add(mduInfoStringArray, json_object_new_string(data->mduInfoString[i]));
    }
    json_object_object_add(devMduInfoEvtObj, "mduInfoInt", mduInfoIntArray);
    json_object_object_add(devMduInfoEvtObj, "mduInfoString", mduInfoStringArray);

    json_object_object_add(paramsObj, "devMduInfoEvt", devMduInfoEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAskConfigEvtRequest(const char *id)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *askConfigEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.firmwareEvt.post"));

    json_object_object_add(paramsObj, "askConfigEvt", askConfigEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseConfUpdateSrv(const char *jsondata, struct confUpdateSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }

    json_object *confUpdateSrvObj;
    if (!json_object_object_get_ex(jsonObj, "confUpdateSrv", &confUpdateSrvObj))
    {
        // 不存在 confUpdateSrv 字段
        json_object_put(jsonObj);
        return ret;
    }

    // 解析 confUpdateSrv 字段内部字段
    json_object_object_foreach(confUpdateSrvObj, key, val)
    {
        if (strcmp(key, "equipParamFreq") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->equipParamFreq = json_object_get_int(val);
        }
        else if (strcmp(key, "gunElecFreq") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->gunElecFreq = json_object_get_int(val);
        }
        else if (strcmp(key, "nonElecFreq") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->nonElecFreq = json_object_get_int(val);
        }
        else if (strcmp(key, "faultWarnings") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->faultWarnings = json_object_get_int(val);
        }
        else if (strcmp(key, "acMeterFreq") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->acMeterFreq = json_object_get_int(val);
        }
        else if (strcmp(key, "offlinChaLen") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->offlinChaLen = json_object_get_int(val);
        }
        else if (strcmp(key, "grndLock") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->grndLock = json_object_get_int(val);
        }
        else if (strcmp(key, "doorLock") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->doorLock = json_object_get_int(val);
        }
        else if (strcmp(key, "qrCode") == 0 && json_object_get_type(val) == json_type_array)
        {
            int arrayLen = json_object_array_length(val);
            if (arrayLen > 0)
            {
                json_object *arrayItem = json_object_array_get_idx(val, 0);
                if (json_object_get_type(arrayItem) == json_type_string)
                {
                    const char *stringValue = json_object_get_string(arrayItem);
                    strncpy(data->qrCode, stringValue, sizeof(data->qrCode) - 1);
                }
            }
        }
        ret = 0;
    }

    json_object_put(jsonObj);
    return ret;
}

char *createConfUpdateSrvReply(const char *id, int code)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *confUpdateSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.confUpdateSrv"));

    json_object_object_add(confUpdateSrvObj, "resCode", json_object_new_int(code));
    json_object_object_add(paramsObj, "confUpdateSrv", confUpdateSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseGetConfSrv(const char *jsondata)
{
    if (jsondata == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }

    json_object *getConfSrvObj;
    if (!json_object_object_get_ex(jsonObj, "getConfSrv", &getConfSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    ret = 0;
    json_object_put(jsonObj);
    return ret;
}

char *createGetConfigSrvReply(const char *id, const struct confUpdateSrv *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *confUpdateSrvObj = json_object_new_object();
    json_object *qrCodeArray = json_object_new_array();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.getConfSrv"));

    json_object_object_add(confUpdateSrvObj, "equipParamFreq", json_object_new_int(data->equipParamFreq));
    json_object_object_add(confUpdateSrvObj, "gunElecFreq", json_object_new_int(data->gunElecFreq));
    json_object_object_add(confUpdateSrvObj, "nonElecFreq", json_object_new_int(data->nonElecFreq));
    json_object_object_add(confUpdateSrvObj, "faultWarnings", json_object_new_int(data->faultWarnings));
    json_object_object_add(confUpdateSrvObj, "acMeterFreq", json_object_new_int(data->acMeterFreq));
    json_object_object_add(confUpdateSrvObj, "offlinChaLen", json_object_new_int(data->offlinChaLen));
    json_object_object_add(confUpdateSrvObj, "grndLock", json_object_new_int(data->grndLock));
    json_object_object_add(confUpdateSrvObj, "doorLock", json_object_new_int(data->doorLock));
    json_object_array_add(qrCodeArray, json_object_new_string(data->qrCode));
    json_object_object_add(confUpdateSrvObj, "qrCode", qrCodeArray);

    json_object_object_add(paramsObj, "getConfSrv", confUpdateSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseFunConfUpdateSrv(const char *jsondata, struct funConfUpdate_srv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *funConfUpdateSrvObj;
    if (!json_object_object_get_ex(jsonObj, "funConfUpdateSrv", &funConfUpdateSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }

    // 解析 funConfUpdateSrv 字段内部字段
    json_object_object_foreach(funConfUpdateSrvObj, key, val)
    {
        if (strcmp(key, "funCode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->funCode = json_object_get_int(val);
        }
        else if (strcmp(key, "confInt") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->confInt = json_object_get_int(val);
        }
        else if (strcmp(key, "confString") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->confString, json_object_get_string(val));
        }
        ret = 0;
    }

    json_object_put(jsonObj);
    return ret;
}

char *createFunConfUpdateSrvReply(const char *id, int code)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *funConfUpdateSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.funConfUpdateSrv"));

    json_object_object_add(funConfUpdateSrvObj, "resCode", json_object_new_int(code));

    json_object_object_add(paramsObj, "funConfUpdateSrv", funConfUpdateSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseGetFunConfSrv(const char *jsondata)
{
    if (jsondata == NULL)
    {
        return -1;
    }
    int funCode = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return funCode;
    }
    json_object *getFunConfSrvObj;
    if (!json_object_object_get_ex(jsonObj, "getFunConfSrv", &getFunConfSrvObj))
    {
        json_object_put(jsonObj);
        return funCode;
    }
    // 解析 getFunConfSrv 字段内部字段
    json_object_object_foreach(getFunConfSrvObj, key, val)
    {
        json_object_object_foreach(jsonObj, key, val)
        {
            if (strcmp(key, "funCode") == 0 && json_object_get_type(val) == json_type_int)
            {
                funCode = json_object_get_int(val);
            }
        }
    }
    json_object_put(jsonObj);
    return funCode;
}

char *createGetFunConfSrvSrvReply(const char *id, const struct funConfUpdate_srv *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *getFunConfSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.getFunConfSrv"));

    json_object_object_add(getFunConfSrvObj, "funCode", json_object_new_int(data->funCode));
    if (strlen(data->confString) != 0)
    {
        json_object_object_add(getFunConfSrvObj, "confString", json_object_new_string(data->confString));
    }
    else
    {
        json_object_object_add(getFunConfSrvObj, "confInt", json_object_new_int(data->confInt));
    }

    json_object_object_add(paramsObj, "getFunConfSrv", getFunConfSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseQueDataSrv(const char *jsondata, struct queDataSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *queDataSrvObj;
    if (!json_object_object_get_ex(jsonObj, "queDataSrv", &queDataSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }

    // 解析 queDataSrv 字段内部字段
    json_object_object_foreach(queDataSrvObj, key, val)
    {

        if (strcmp(key, "gunNo") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->gunNo = json_object_get_int(val);
        }
        else if (strcmp(key, "startDate") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->startDate, json_object_get_string(val));
        }
        else if (strcmp(key, "stopDate") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->stopDate, json_object_get_string(val));
        }
        else if (strcmp(key, "askType") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->askType = json_object_get_int(val);
        }
        else if (strcmp(key, "logQueryNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->logQueryNo, json_object_get_string(val));
        }
        ret = 0;
    }

    json_object_put(jsonObj);
    return ret;
}

char *createQueDataSrvReply(const char *id, const struct queDataSrv *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *queDataSrvSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.queDataSrv"));

    json_object_object_add(queDataSrvSrvObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(queDataSrvSrvObj, "startDate", json_object_new_string(data->startDate));
    json_object_object_add(queDataSrvSrvObj, "stopDate", json_object_new_string(data->stopDate));
    json_object_object_add(queDataSrvSrvObj, "askType", json_object_new_int(data->askType));
    json_object_object_add(queDataSrvSrvObj, "logQueryNo", json_object_new_string(data->logQueryNo));

    json_object_object_add(paramsObj, "queDataSrv", queDataSrvSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createLogQueryEvtReply(const char *id, const struct logQueryEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *logQueryEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.logQueryEvt.post"));

    json_object_object_add(logQueryEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(logQueryEvtObj, "startDate", json_object_new_string(data->startDate));
    json_object_object_add(logQueryEvtObj, "stopDate", json_object_new_string(data->stopDate));
    json_object_object_add(logQueryEvtObj, "askType", json_object_new_int(data->askType));
    json_object_object_add(logQueryEvtObj, "result", json_object_new_int(data->result));
    json_object_object_add(logQueryEvtObj, "logQueryNo", json_object_new_string(data->logQueryNo));
    json_object_object_add(logQueryEvtObj, "retType", json_object_new_int(data->retType));
    json_object_object_add(logQueryEvtObj, "logQueryEvtSum", json_object_new_int(data->logQueryEvtSum));
    json_object_object_add(logQueryEvtObj, "logQueryEvtNo", json_object_new_int(data->logQueryEvtNo));
    json_object_object_add(logQueryEvtObj, "dataArea", json_object_new_string(data->dataArea));

    json_object_object_add(paramsObj, "logQueryEvt", logQueryEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseDevMaintainSrv(const char *jsondata)
{
    if (jsondata == NULL)
    {
        return -1;
    }
    int ctrlType = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ctrlType;
    }
    json_object *devMaintainSrvObj;
    if (!json_object_object_get_ex(jsonObj, "devMaintainSrv", &devMaintainSrvObj))
    {
        json_object_put(jsonObj);
        return ctrlType;
    }
    // 解析 devMaintainSrv 字段内部字段
    json_object_object_foreach(devMaintainSrvObj, key, val)
    {
        if (strcmp(key, "ctrlType") == 0 && json_object_get_type(val) == json_type_int)
        {
            ctrlType = json_object_get_int(val);
        }
    }

    json_object_put(jsonObj);
    return ctrlType;
}

char *createDevMaintainSrvReply(const char *id, int ctrlType, int reason)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *devMaintainSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.devMaintainSrv"));

    json_object_object_add(devMaintainSrvObj, "ctrlType", json_object_new_int(ctrlType));
    json_object_object_add(devMaintainSrvObj, "reason", json_object_new_int(reason));

    json_object_object_add(paramsObj, "devMaintainSrv", devMaintainSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseDevMaintainQuerySrv(const char *jsondata)
{
    if (jsondata == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *devMaintainQuerySrvObj;
    if (!json_object_object_get_ex(jsonObj, "devMaintainQuerySrv", &devMaintainQuerySrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    ret = 0;
    json_object_put(jsonObj);
    return ret;
}

char *createDevMaintainQuerySrvReply(const char *id, int ctrlType, int reason)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *devMaintainQuerySrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.devMaintainQuerySrvObj"));

    json_object_object_add(devMaintainQuerySrvObj, "ctrlType", json_object_new_int(ctrlType));
    json_object_object_add(devMaintainQuerySrvObj, "reason", json_object_new_int(reason));

    json_object_object_add(paramsObj, "devMaintainQuerySrvObj", devMaintainQuerySrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAskFeeModelEvtRequest(const char *id, const struct askFeeModelEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *askFeeModelEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.askConfigEvt.post"));

    json_object_object_add(askFeeModelEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(askFeeModelEvtObj, "eleModelId", json_object_new_string(data->eleModelId));
    json_object_object_add(askFeeModelEvtObj, "serModelId", json_object_new_string(data->serModelId));

    json_object_object_add(paramsObj, "askConfigEvt", askFeeModelEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseFeeModelUpdateSrv(const char *jsondata, struct feeModelUpdateSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int i;
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *feeModelUpdateSrvObj;
    if (!json_object_object_get_ex(jsonObj, "feeModelUpdateSrv", &feeModelUpdateSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    // 解析 feeModelUpdateSrv 字段内部字段
    json_object_object_foreach(feeModelUpdateSrvObj, key, val)
    {
        if (strcmp(key, "eleModelId") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->eleModelId, json_object_get_string(val));
        }
        else if (strcmp(key, "serModelId") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->serModelId, json_object_get_string(val));
        }
        else if (strcmp(key, "timeNum") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->timeNum = json_object_get_int(val);
        }
        else if (strcmp(key, "timeSeg") == 0 && json_object_get_type(val) == json_type_array)
        {
            int array_len = json_object_array_length(val);

            for (i = 0; i < array_len || i < FEE_MODE_TIME_NUM; i++)
            {
                json_object *array_element = json_object_array_get_idx(val, i);

                if (json_object_get_type(array_element) == json_type_string)
                {
                    strcpy(data->timeSeg[i], json_object_get_string(val));
                }
            }
        }
        else if (strcmp(key, "segFlag") == 0 && json_object_get_type(val) == json_type_array)
        {
            int array_len = json_object_array_length(val);

            for (i = 0; i < array_len || i < FEE_MODE_TIME_NUM; i++)
            {
                json_object *array_element = json_object_array_get_idx(val, i);

                if (json_object_get_type(array_element) == json_type_int)
                {
                    data->segFlag[i] = json_object_get_int(array_element);
                }
            }
        }
        else if (strcmp(key, "chargeFee") == 0 && json_object_get_type(val) == json_type_array)
        {
            int array_len = json_object_array_length(val);

            for (i = 0; i < array_len || i < FEE_MODE_TIME_NUM; i++)
            {
                json_object *array_element = json_object_array_get_idx(val, i);

                if (json_object_get_type(array_element) == json_type_int)
                {
                    data->chargeFee[i] = json_object_get_int(array_element);
                }
            }
        }
        else if (strcmp(key, "serviceFee") == 0 && json_object_get_type(val) == json_type_array)
        {
            int array_len = json_object_array_length(val);

            for (i = 0; i < array_len || i < FEE_MODE_TIME_NUM; i++)
            {
                json_object *array_element = json_object_array_get_idx(val, i);

                if (json_object_get_type(array_element) == json_type_int)
                {
                    data->serviceFee[i] = json_object_get_int(array_element);
                }
            }
        }
        ret = 0;
    }

    json_object_put(jsonObj);
    return ret;
}

char *createFeeModelUpdateSrvReply(const char *id, const struct feeModelUpdateSrvRep *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *feeModelUpdateSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.feeModelUpdateSrv"));

    json_object_object_add(feeModelUpdateSrvObj, "eleModelId", json_object_new_string(data->eleModelId));
    json_object_object_add(feeModelUpdateSrvObj, "serModelId", json_object_new_string(data->serModelId));
    json_object_object_add(feeModelUpdateSrvObj, "result", json_object_new_int(data->result));

    json_object_object_add(paramsObj, "feeModelUpdateSrv", feeModelUpdateSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseFeeModelQuerySrv(const char *jsondata)
{
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return -1;
    }
    int gunNo = -1;
    json_object *feeModelQuerySrvObj;
    if (!json_object_object_get_ex(jsonObj, "feeModelQuerySrv", &feeModelQuerySrvObj))
    {
        json_object_put(jsonObj);
        return -1;
    }
    json_object_object_foreach(feeModelQuerySrvObj, key, val)
    {
        if (strcmp(key, "gunNo") == 0 && json_object_get_type(val) == json_type_int)
        {
            gunNo = json_object_get_int(val);
        }
    }
    json_object_put(jsonObj);
    return gunNo;
}

char *createFeeModelQuerySrvReply(const char *id, const struct feeModelQuerySrv *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    int i;
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *feeModelQuerySrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.feeModelQuerySrv"));

    json_object_object_add(feeModelQuerySrvObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(feeModelQuerySrvObj, "eleModelId", json_object_new_string(data->eleModelId));
    json_object_object_add(feeModelQuerySrvObj, "serModelId", json_object_new_string(data->serModelId));
    json_object_object_add(feeModelQuerySrvObj, "timeNum", json_object_new_int(data->timeNum));
    json_object *timeSegArray = json_object_new_array();
    for (i = 0; i < data->timeNum; i++)
    {
        json_object_array_add(timeSegArray, json_object_new_string(data->timeSeg[i]));
    }
    json_object_object_add(feeModelQuerySrvObj, "timeSeg", timeSegArray);

    json_object *segFlagArray = json_object_new_array();
    for (i = 0; i < data->timeNum; i++)
    {
        json_object_array_add(segFlagArray, json_object_new_int(data->segFlag[i]));
    }
    json_object_object_add(feeModelQuerySrvObj, "segFlag", segFlagArray);

    json_object *chargeFeeArray = json_object_new_array();
    for (i = 0; i < data->timeNum; i++)
    {
        json_object_array_add(chargeFeeArray, json_object_new_int(data->chargeFee[i]));
    }
    json_object_object_add(feeModelQuerySrvObj, "chargeFee", chargeFeeArray);

    json_object *serviceFeeArray = json_object_new_array();
    for (i = 0; i < data->timeNum; i++)
    {
        json_object_array_add(serviceFeeArray, json_object_new_int(data->serviceFee[i]));
    }

    json_object_object_add(paramsObj, "feeModelQuerySrv", feeModelQuerySrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseStartChargeSrv(const char *jsondata, struct startChargeSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *startChargeSrvObj;
    if (!json_object_object_get_ex(jsonObj, "startChargeSrv", &startChargeSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    json_object_object_foreach(startChargeSrvObj, key, val)
    {
        if (strcmp(key, "gunNo") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->gunNo = json_object_get_int(val);
        }
        else if (strcmp(key, "preTradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->preTradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "tradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->tradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "startType") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->startType = json_object_get_int(val);
        }
        else if (strcmp(key, "chargeMode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->chargeMode = json_object_get_int(val);
        }
        else if (strcmp(key, "limitData") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->limitData = json_object_get_int(val);
        }
        else if (strcmp(key, "stopCode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->stopCode = json_object_get_int(val);
        }
        else if (strcmp(key, "startMode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->startMode = json_object_get_int(val);
        }
        else if (strcmp(key, "insertGunTime") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->insertGunTime = json_object_get_int(val);
        }
        ret = 0;
    }

    json_object_put(jsonObj);

    return ret;
}

char *createStartChargeSrvReply(const char *id, const struct startChargeSrvRep *data)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *startChargeSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.startChargeSrv"));

    json_object_object_add(startChargeSrvObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(startChargeSrvObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(startChargeSrvObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(paramsObj, "startChargeSrv", startChargeSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createStartChaResEvtReply(const char *id, const struct startChaResEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *startChaResEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.startChaResEvt.post"));

    json_object_object_add(startChaResEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(startChaResEvtObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(startChaResEvtObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(startChaResEvtObj, "startResult", json_object_new_int(data->startResult));
    json_object_object_add(startChaResEvtObj, "faultCode", json_object_new_int(data->faultCode));
    json_object_object_add(startChaResEvtObj, "vinCode", json_object_new_string(data->vinCode));

    json_object_object_add(paramsObj, "startChaResEvt", startChaResEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createStartChargeAuthEvtRequest(const char *id, const struct startChargeAuthEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *startChargeAuthEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.startChargeAuthEvt.post"));

    json_object_object_add(startChargeAuthEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(startChargeAuthEvtObj, "preTradeNo", json_object_new_string(data->preTradeNo));

    json_object_object_add(startChargeAuthEvtObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(startChargeAuthEvtObj, "startType", json_object_new_int(data->startType));
    json_object_object_add(startChargeAuthEvtObj, "authCode", json_object_new_string(data->authCode));
    json_object_object_add(startChargeAuthEvtObj, "batterySoc", json_object_new_int(data->batterySoc));
    json_object_object_add(startChargeAuthEvtObj, "batteryCap", json_object_new_int(data->batteryCap));
    json_object_object_add(startChargeAuthEvtObj, "chargeTimes", json_object_new_int(data->chargeTimes));
    json_object_object_add(startChargeAuthEvtObj, "batteryVol", json_object_new_int(data->batteryVol));

    json_object_object_add(paramsObj, "startChargeAuthEvt", startChargeAuthEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseAuthResultSrv(const char *jsondata, struct authResultSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *authResultSrvObj;
    if (!json_object_object_get_ex(jsonObj, "authResultSrv", &authResultSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    json_object_object_foreach(authResultSrvObj, key, val)
    {
        if (strcmp(key, "gunNo") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->gunNo = json_object_get_int(val);
        }
        else if (strcmp(key, "preTradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->preTradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "tradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->tradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "startType") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->startType = json_object_get_int(val);
        }
        else if (strcmp(key, "authCode") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->authCode, json_object_get_string(val));
        }
        else if (strcmp(key, "result") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->result = json_object_get_int(val);
        }
        else if (strcmp(key, "chargeMode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->chargeMode = json_object_get_int(val);
        }
        else if (strcmp(key, "limitData") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->limitData = json_object_get_int(val);
        }
        else if (strcmp(key, "stopCode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->stopCode = json_object_get_int(val);
        }
        else if (strcmp(key, "startMode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->startMode = json_object_get_int(val);
        }
        else if (strcmp(key, "insertGunTime") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->insertGunTime = json_object_get_int(val);
        }
        ret = 0;
    }

    json_object_put(jsonObj);

    return ret;
}

int parseStopChargeSrv(const char *jsondata, struct stopChargeSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *stopChargeSrvObj;
    if (!json_object_object_get_ex(jsonObj, "stopChargeSrv", &stopChargeSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    json_object_object_foreach(stopChargeSrvObj, key, val)
    {
        if (strcmp(key, "gunNo") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->gunNo = json_object_get_int(val);
        }
        else if (strcmp(key, "preTradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->preTradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "tradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->tradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "stopReason") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->stopReason = json_object_get_int(val);
        }
        ret = 0;
    }

    json_object_put(jsonObj);

    return ret;
}

char *createStopChargeSrvReply(const char *id, const struct startChargeSrvRep *data)
{
    if (id == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *stopChargeSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.stopChargeSrv"));

    json_object_object_add(stopChargeSrvObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(stopChargeSrvObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(stopChargeSrvObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(paramsObj, "stopChargeSrv", stopChargeSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createStopChaResEvtReply(const char *id, const struct stopChaResEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *stopChaResEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.stopChaResEvt.post"));

    json_object_object_add(stopChaResEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(stopChaResEvtObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(stopChaResEvtObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(stopChaResEvtObj, "stopResult", json_object_new_int(data->stopResult));
    json_object_object_add(stopChaResEvtObj, "resultCode", json_object_new_int(data->resultCode));
    json_object_object_add(stopChaResEvtObj, "stopFailReson", json_object_new_int(data->stopFailReson));

    json_object_object_add(paramsObj, "stopChaResEvt", stopChaResEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createOrderUpdateEvtRequest(const char *id, const struct orderUpdateEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *orderUpdateEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.orderUpdateEvt.post"));

    json_object_object_add(orderUpdateEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(orderUpdateEvtObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(orderUpdateEvtObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(orderUpdateEvtObj, "vinCode", json_object_new_string(data->vinCode));
    json_object_object_add(orderUpdateEvtObj, "timeDivType", json_object_new_int(data->timeDivType));
    json_object_object_add(orderUpdateEvtObj, "chargeStartTime", json_object_new_string(data->chargeStartTime));
    json_object_object_add(orderUpdateEvtObj, "chargeEndTime", json_object_new_string(data->chargeEndTime));
    json_object_object_add(orderUpdateEvtObj, "startSoc", json_object_new_int(data->startSoc));
    json_object_object_add(orderUpdateEvtObj, "endSoc", json_object_new_int(data->endSoc));
    json_object_object_add(orderUpdateEvtObj, "reason", json_object_new_int(data->reason));
    json_object_object_add(orderUpdateEvtObj, "eleModelId", json_object_new_string(data->eleModelId));
    json_object_object_add(orderUpdateEvtObj, "serModelId", json_object_new_string(data->serModelId));
    json_object_object_add(orderUpdateEvtObj, "sumStart", json_object_new_string(data->sumStart));
    json_object_object_add(orderUpdateEvtObj, "sumEnd", json_object_new_string(data->sumEnd));
    json_object_object_add(orderUpdateEvtObj, "totalElect", json_object_new_int(data->totalElect));
    json_object_object_add(orderUpdateEvtObj, "sharpElect", json_object_new_int(data->sharpElect));
    json_object_object_add(orderUpdateEvtObj, "peakElect", json_object_new_int(data->peakElect));
    json_object_object_add(orderUpdateEvtObj, "flatElect", json_object_new_int(data->flatElect));
    json_object_object_add(orderUpdateEvtObj, "valleyElect", json_object_new_int(data->valleyElect));
    json_object_object_add(orderUpdateEvtObj, "totalPowerCost", json_object_new_int(data->totalPowerCost));
    json_object_object_add(orderUpdateEvtObj, "totalServCost", json_object_new_int(data->totalServCost));
    json_object_object_add(orderUpdateEvtObj, "sharpPowerCost", json_object_new_int(data->sharpPowerCost));
    json_object_object_add(orderUpdateEvtObj, "peakPowerCost", json_object_new_int(data->peakPowerCost));
    json_object_object_add(orderUpdateEvtObj, "flatPowerCost", json_object_new_int(data->flatPowerCost));
    json_object_object_add(orderUpdateEvtObj, "valleyPowerCost", json_object_new_int(data->valleyPowerCost));
    json_object_object_add(orderUpdateEvtObj, "sharpServCost", json_object_new_int(data->sharpServCost));
    json_object_object_add(orderUpdateEvtObj, "peakServCost", json_object_new_int(data->peakServCost));
    json_object_object_add(orderUpdateEvtObj, "flatServCost", json_object_new_int(data->flatServCost));
    json_object_object_add(orderUpdateEvtObj, "valleyServCost", json_object_new_int(data->valleyServCost));

    json_object_object_add(paramsObj, "orderUpdateEvt", orderUpdateEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseOrderCheckSrv(const char *jsondata, struct orderCheckSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *orderCheckSrvObj;
    if (!json_object_object_get_ex(jsonObj, "orderCheckSrv", &orderCheckSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    json_object_object_foreach(orderCheckSrvObj, key, val)
    {
        if (strcmp(key, "gunNo") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->gunNo = json_object_get_int(val);
        }
        else if (strcmp(key, "preTradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->preTradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "tradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->tradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "errCode") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->errCode = json_object_get_int(val);
        }
        ret = 0;
    }

    json_object_put(jsonObj);

    return ret;
}

char *createTotalFaultEvtRequest(const char *id, const struct totalFaultEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    int i;
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *totalFaultEvtObj = json_object_new_object();
    json_object *faultValueArray = json_object_new_array();
    json_object *warnValueArray = json_object_new_array();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.totalFaultEvt.post"));

    json_object_object_add(totalFaultEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(totalFaultEvtObj, "faultSum", json_object_new_int(data->faultSum));
    json_object_object_add(totalFaultEvtObj, "warnSum", json_object_new_int(data->warnSum));

    for (i = 0; i < data->faultSum; i++)
    {
        json_object_array_add(faultValueArray, json_object_new_int(data->faultValue[i]));
    }
    json_object_object_add(totalFaultEvtObj, "faultValue", faultValueArray);

    for (i = 0; i < data->warnSum; i++)
    {
        json_object_array_add(warnValueArray, json_object_new_int(data->warnValue[i]));
    }
    json_object_object_add(totalFaultEvtObj, "warnValue", warnValueArray);

    json_object_object_add(paramsObj, "totalFaultEvt", totalFaultEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcDeRealItyProperty(const char *id, const struct acDeRealIty *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    int i;
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acDeRealItyObj = json_object_new_object();
    json_object *valueObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.property.post"));

    json_object_object_add(valueObj, "netType", json_object_new_int(data->netType));
    json_object_object_add(valueObj, "sigVal", json_object_new_int(data->sigVal));
    json_object_object_add(valueObj, "netId", json_object_new_int(data->netId));
    json_object_object_add(valueObj, "acVolA", json_object_new_int(data->acVolA));
    json_object_object_add(valueObj, "acCurA", json_object_new_int(data->acCurA));
    json_object_object_add(valueObj, "acVolB", json_object_new_int(data->acVolB));
    json_object_object_add(valueObj, "acCurB", json_object_new_int(data->acCurB));
    json_object_object_add(valueObj, "acVolC", json_object_new_int(data->acVolC));
    json_object_object_add(valueObj, "acCurC", json_object_new_int(data->acCurC));
    json_object_object_add(valueObj, "caseTemp", json_object_new_int(data->caseTemp));
    json_object_object_add(valueObj, "eleModelId", json_object_new_string(data->eleModelId));
    json_object_object_add(valueObj, "serModelId", json_object_new_string(data->serModelId));

    json_object_object_add(acDeRealItyObj, "value", valueObj);
    json_object_object_add(paramsObj, "acDeRealIty", acDeRealItyObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcGunRunItyProperty(const char *id, const struct acGunRunIty *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    int i;
    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acGunRunItyObj = json_object_new_object();
    json_object *valueObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.property.post"));

    json_object_object_add(valueObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(valueObj, "workStatus", json_object_new_int(data->workStatus));
    json_object_object_add(valueObj, "conStatus", json_object_new_int(data->conStatus));
    json_object_object_add(valueObj, "outRelayStatus", json_object_new_int(data->outRelayStatus));
    json_object_object_add(valueObj, "eLockStatus", json_object_new_int(data->eLockStatus));
    json_object_object_add(valueObj, "gunTemp", json_object_new_int(data->gunTemp));
    json_object_object_add(valueObj, "acVolA", json_object_new_int(data->acVolA));
    json_object_object_add(valueObj, "acCurA", json_object_new_int(data->acCurA));
    json_object_object_add(valueObj, "acVolB", json_object_new_int(data->acVolB));
    json_object_object_add(valueObj, "acCurB", json_object_new_int(data->acCurB));
    json_object_object_add(valueObj, "acVolC", json_object_new_int(data->acVolC));
    json_object_object_add(valueObj, "acCurC", json_object_new_int(data->acCurC));
    json_object_object_add(valueObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(valueObj, "tradeNo", json_object_new_string(data->tradeNo));
    json_object_object_add(valueObj, "realPower", json_object_new_int(data->realPower));
    json_object_object_add(valueObj, "chgTime", json_object_new_int(data->chgTime));
    json_object_object_add(valueObj, "totalElect", json_object_new_int(data->totalElect));
    json_object_object_add(valueObj, "sharpElect", json_object_new_int(data->sharpElect));
    json_object_object_add(valueObj, "peakElect", json_object_new_int(data->peakElect));
    json_object_object_add(valueObj, "flatElect", json_object_new_int(data->flatElect));
    json_object_object_add(valueObj, "valleyElect", json_object_new_int(data->valleyElect));
    json_object_object_add(valueObj, "totalCost", json_object_new_int(data->totalCost));
    json_object_object_add(valueObj, "totalPowerCost", json_object_new_int(data->totalPowerCost));
    json_object_object_add(valueObj, "totalServCost", json_object_new_int(data->totalServCost));
    json_object_object_add(valueObj, "PwmDutyRadio", json_object_new_int(data->PwmDutyRadio));

    json_object_object_add(acGunRunItyObj, "value", valueObj);
    json_object_object_add(paramsObj, "acGunRunIty", acGunRunItyObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcGunIdleItyProperty(const char *id, const struct acGunIdleIty *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acGunIdleItyObj = json_object_new_object();
    json_object *valueObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.property.post"));

    json_object_object_add(valueObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(valueObj, "workStatus", json_object_new_int(data->workStatus));
    json_object_object_add(valueObj, "conStatus", json_object_new_int(data->conStatus));
    json_object_object_add(valueObj, "outRelayStatus", json_object_new_int(data->outRelayStatus));
    json_object_object_add(valueObj, "eLockStatus", json_object_new_int(data->eLockStatus));
    json_object_object_add(valueObj, "gunTemp", json_object_new_int(data->gunTemp));
    json_object_object_add(valueObj, "acVolA", json_object_new_int(data->acVolA));
    json_object_object_add(valueObj, "acCurA", json_object_new_int(data->acCurA));
    json_object_object_add(valueObj, "acVolB", json_object_new_int(data->acVolB));
    json_object_object_add(valueObj, "acCurB", json_object_new_int(data->acCurB));
    json_object_object_add(valueObj, "acVolC", json_object_new_int(data->acVolC));
    json_object_object_add(valueObj, "acCurC", json_object_new_int(data->acCurC));

    json_object_object_add(acGunIdleItyObj, "value", valueObj);
    json_object_object_add(paramsObj, "acGunIdleIty", acGunIdleItyObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcOutMeterItyProperty(const char *id, const struct acOutMeterIty *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acOutMeterItyObj = json_object_new_object();
    json_object *valueObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.property.post"));

    json_object_object_add(valueObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(valueObj, "acqTime", json_object_new_string(data->acqTime));
    json_object_object_add(valueObj, "mailAddr", json_object_new_string(data->mailAddr));
    json_object_object_add(valueObj, "meterNo", json_object_new_string(data->meterNo));
    json_object_object_add(valueObj, "assetId", json_object_new_string(data->assetId));
    json_object_object_add(valueObj, "sumMeter", json_object_new_string(data->sumMeter));
    json_object_object_add(valueObj, "lastTrade", json_object_new_string(data->lastTrade));
    json_object_object_add(valueObj, "power", json_object_new_int(data->power));

    json_object_object_add(acOutMeterItyObj, "value", valueObj);
    json_object_object_add(paramsObj, "acOutMeterIty", acOutMeterItyObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

int parseAcOrderlyChargeSrv(const char *jsondata, struct acOrderlyChargeSrv *data)
{
    if (jsondata == NULL || data == NULL)
    {
        return -1;
    }
    int i;
    int ret = -1;
    json_object *jsonObj = json_tokener_parse(jsondata);
    if (jsonObj == NULL)
    {
        return ret;
    }
    json_object *acOrderlyChargeSrvObj;
    if (!json_object_object_get_ex(jsonObj, "acOrderlyChargeSrv", &acOrderlyChargeSrvObj))
    {
        json_object_put(jsonObj);
        return ret;
    }
    // 解析 acOrderlyChargeSrv 字段内部字段
    json_object_object_foreach(acOrderlyChargeSrvObj, key, val)
    {
        if (strcmp(key, "preTradeNo") == 0 && json_object_get_type(val) == json_type_string)
        {
            strcpy(data->preTradeNo, json_object_get_string(val));
        }
        else if (strcmp(key, "validTime") == 0 && json_object_get_type(val) == json_type_array)
        {
            int array_len = json_object_array_length(val);

            for (i = 0; i < array_len || i < CHARGE_PLOY_MAX_NUM; i++)
            {
                json_object *array_element = json_object_array_get_idx(val, i);

                if (json_object_get_type(array_element) == json_type_string)
                {
                    strcpy(data->validTime[i], json_object_get_string(val));
                }
            }
        }
        else if (strcmp(key, "kw") == 0 && json_object_get_type(val) == json_type_array)
        {
            int array_len = json_object_array_length(val);

            for (i = 0; i < array_len || i < CHARGE_PLOY_MAX_NUM; i++)
            {
                json_object *array_element = json_object_array_get_idx(val, i);

                if (json_object_get_type(array_element) == json_type_int)
                {
                    data->kw[i] = json_object_get_int(array_element);
                }
            }
        }
        if (strcmp(key, "num") == 0 && json_object_get_type(val) == json_type_int)
        {
            data->num = json_object_get_int(val);
        }
        ret = 0;
    }

    json_object_put(jsonObj);
    return ret;
}

char *createAcOrderlyChargeSrvReply(const char *id, const struct acOrderlyChargeSrvRep *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acOrderlyChargeSrvObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.service.acOrderlyChargeSrv"));

    json_object_object_add(acOrderlyChargeSrvObj, "preTradeNo", json_object_new_string(data->preTradeNo));
    json_object_object_add(acOrderlyChargeSrvObj, "result", json_object_new_int(data->result));
    json_object_object_add(acOrderlyChargeSrvObj, "reason", json_object_new_int(data->reason));
    json_object_object_add(paramsObj, "acOrderlyChargeSrv", acOrderlyChargeSrvObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcStChEvtRequest(const char *id, const struct acStChEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acStChEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.acStChEvt.post"));

    json_object_object_add(acStChEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(acStChEvtObj, "yxOccurTime", json_object_new_string(data->yxOccurTime));
    json_object_object_add(acStChEvtObj, "connCheckStatus", json_object_new_int(data->connCheckStatus));

    json_object_object_add(paramsObj, "acStChEvt", acStChEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcCarInfoEvtRequest(const char *id, const struct acCarInfoEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acCarInfoEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.acCarInfoEvt.post"));

    json_object_object_add(acCarInfoEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(acCarInfoEvtObj, "batterySOC", json_object_new_int(data->batterySOC));
    json_object_object_add(acCarInfoEvtObj, "batteryCap", json_object_new_int(data->batteryCap));
    json_object_object_add(acCarInfoEvtObj, "vinCode", json_object_new_string(data->vinCode));
    json_object_object_add(acCarInfoEvtObj, "state", json_object_new_int(data->state));

    json_object_object_add(paramsObj, "acCarInfoEvt", acCarInfoEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}

char *createAcCarConChEvtRequest(const char *id, const struct acCarConChEvt *data)
{
    if (id == NULL || data == NULL)
    {
        return NULL;
    }

    json_object *jsonObj = json_object_new_object();
    json_object *paramsObj = json_object_new_object();
    json_object *acCarConChEvtObj = json_object_new_object();

    json_object_object_add(jsonObj, "id", json_object_new_string(id));
    json_object_object_add(jsonObj, "version", json_object_new_string("1.0"));
    json_object_object_add(jsonObj, "method", json_object_new_string("thing.event.acCarConChEvt.post"));

    json_object_object_add(acCarConChEvtObj, "gunNo", json_object_new_int(data->gunNo));
    json_object_object_add(acCarConChEvtObj, "cpStatus", json_object_new_int(data->cpStatus));
    json_object_object_add(acCarConChEvtObj, "cpVolt", json_object_new_int(data->cpVolt));
    json_object_object_add(acCarConChEvtObj, "s3Status", json_object_new_int(data->s3Status));
    json_object_object_add(acCarConChEvtObj, "yxOccurTime", json_object_new_string(data->yxOccurTime));
    json_object_object_add(paramsObj, "acCarConChEvt", acCarConChEvtObj);

    json_object_object_add(jsonObj, "params", paramsObj);

    char *json_str = strdup(json_object_to_json_string_ext(jsonObj, JSON_C_TO_STRING_PRETTY));

    json_object_put(jsonObj);

    return json_str;
}