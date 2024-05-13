#include "GBBMSComm.h"


void v_gb_bms_rx_handle(CAN_DATA_T *pt_msg, GBBMS_CAN_COMM_T *pt_bms)
{

    if (pt_msg->u32_pgn == GB_BMS_PGN_BHM)
    {
        GB_BHM_DATA_T bhm; // BMS握手报文
        if (pt_msg->u16_len >= 2)
        {
            bhm.maxChgVolt = (((unsigned short)(pt_msg->u8_data[1]) << 8) | pt_msg->u8_data[0]) / 10.0;
        }

        if (pt_bms->bhmCallback != NULL)
        {
            pt_bms->bhmCallback(&bhm);
        }
    }

    if ((pt_msg->u32_pgn == GB_BMS_PGN_BRM))
    {
        GB_BRM_DATA_T brm; // BMS和车辆辨识报文

        if (pt_msg->u16_len >= 41)
        {
            brm.protVer = ((unsigned int)(pt_msg->u8_data[2]) << 16) | ((unsigned int)(pt_msg->u8_data[1]) << 8) | pt_msg->u8_data[0];

            brm.batType = pt_msg->u8_data[3];

            brm.ratedCapacity = (((unsigned short)(pt_msg->u8_data[5]) << 8) | pt_msg->u8_data[4]) / 10.0;

            brm.ratedVolt = (((unsigned short)(pt_msg->u8_data[7]) << 8) | pt_msg->u8_data[6]) / 10.0;

            memcpy((char *)(brm.manufacturer), (char *)(&(pt_msg->u8_data[8])), 4);

            brm.batID = ((unsigned int)(pt_msg->u8_data[15]) << 24) | ((unsigned int)(pt_msg->u8_data[14]) << 16) | ((unsigned int)(pt_msg->u8_data[13]) << 8) | pt_msg->u8_data[12];

            memcpy((char *)(brm.vin), (char *)(&(pt_msg->u8_data[24])), 17);
        }

        if (pt_bms->brmCallback != NULL)
        {
            pt_bms->brmCallback(&brm);
        }
    }
}
