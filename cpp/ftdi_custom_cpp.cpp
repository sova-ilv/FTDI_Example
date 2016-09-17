#include "r2_mipi_spi_i2c_app.h"

#include <QString>
#include <iostream>
#include <QMessageBox>
#include "r2_utils.h"

#define MAX_DEVICES 4
FT_STATUS ftstatus;
HRESULT status;
FT_HANDLE FTDIHandle = 0;
DWORD BytesWritten,  EventDWord, TxBytes, RxBytes;
char R2_Serial[100];

r2_mipi_spi_i2c_app::r2_mipi_spi_i2c_app()
{

}

void r2_mipi_spi_i2c_app::init_r2_usb()
{
    char *BufPtrs[MAX_DEVICES + 1];		// pointer to array of pointers
    char Buffer[MAX_DEVICES][64];		// buffer for description of devices
    unsigned long numDevs;
    int i;

         // initialize the array of pointers
           for (i=0; i<MAX_DEVICES; i++) BufPtrs[i] = Buffer[i];
                BufPtrs[MAX_DEVICES] = NULL;      // last entry should be NULL
                //

           status = FT_ListDevices(BufPtrs, &numDevs, FT_LIST_ALL|FT_OPEN_BY_SERIAL_NUMBER);
                //ftstatus = FT_CreateDeviceInfoList(&numDevs);
           strcpy (R2_Serial, Buffer[0]); //The index of Buffer is typically vcp seq number or the index of how many FTDI devices connected
                                               //NOTE: this should be on ini file

           ftstatus = FT_OpenEx(R2_Serial, FT_OPEN_BY_SERIAL_NUMBER,&FTDIHandle);
           if (FTDIHandle) {
                    ftstatus = FT_SetBaudRate(FTDIHandle, 460800 );
                    ftstatus = FT_SetDataCharacteristics(FTDIHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
                    FT_SetTimeouts(FTDIHandle,500,5);

                    ftstatus = FT_SetUSBParameters(FTDIHandle, 640,640); //default is is 4096 bytes, (way too high)

                    QMessageBox::information(0, "info", "now connected to r2usb.");
                }
           else
                {
                    QMessageBox::critical(0, "Error", "R2-USB gecho device was not found!");
                }

           //Set_VIO(1);
          // Set_VIO(0);
          // Set_VIO(1);
}

void r2_mipi_spi_i2c_app::close_r2_usb()
{
    FT_Close(FTDIHandle);
}

int r2_mipi_spi_i2c_app::usb_Send(QString cmd)
{
    QByteArray array = cmd.toLocal8Bit();
    char* buffer = array.data();

            int len = strlen(buffer);
            ftstatus = FT_Write(FTDIHandle, buffer, len, &BytesWritten);
            if (ftstatus != FT_OK) return -1;
            return 0;
}

/*
int r2_mipi_spi_i2c_app::usb_Send_ext(QString cmd, int len)
{
    QByteArray array = cmd.toLocal8Bit();
    char* buffer = array.data();

            len = strlen(buffer);
            ftstatus = FT_Write(FTDIHandle, buffer, len, &BytesWritten);
            if (ftstatus != FT_OK) return -1;
            return 0;
}
*/
int r2_mipi_spi_i2c_app::usb_Receive(char *rxbuf)
        {
            char tempbuf[20];

            DWORD numberOfCharactersToRead = 20; //256;
            DWORD bytesWrittenOrRead;

            rxbuf[0]='\0';
            ftstatus = FT_Read(FTDIHandle, tempbuf, (DWORD)numberOfCharactersToRead, &bytesWrittenOrRead);
            strcpy(rxbuf, tempbuf);

            //while(1) {
            //    ftstatus = FT_GetStatus(FTDIHandle,&RxBytes,&TxBytes,&EventDWord);
            //    if (RxBytes > 0) {
            //        ftstatus = FT_Read(FTDIHandle, tempbuf, (DWORD)numberOfCharactersToRead, &bytesWrittenOrRead);
            //        strcat(rxbuf, tempbuf);
            //        if (ftstatus != FT_OK)
            //            return -1;
            //}
            //else break;
            //}

            return 0;
        }

int r2_mipi_spi_i2c_app::Set_VIO(int on_off)
{
        int ret=0;

        if (on_off)
        {
        ret = usb_Send("pd 0002\r");
        if (ret != FT_OK) return -1;
        }
        else
        {
        ret = usb_Send("pd 0000\r");
        if (ret != FT_OK) return -1;
        }
        return 0;
}

//extended long write
int r2_mipi_spi_i2c_app::usb_mipi_write_1byte_long(int devaddr, unsigned int regaddr, unsigned int wdat)
{
char read_com[256];
int ret=0;

       read_com[0] = '\0';
       sprintf(read_com, "wL %x %.4x %.2x\r", devaddr, regaddr, wdat & 0xFF);

                        ret = usb_Send(read_com);
                        if (ret != FT_OK) return -1;

                        //Sleep(450);
                        //ret = usb_Receive(read_dat);
                        Sleep(50);

                        return ret;
}

int r2_mipi_spi_i2c_app::usb_mipi_write_1byte(int devaddr, unsigned int regaddr, unsigned int wdat)
{
char read_com[256];
int ret=0;

       read_com[0] = '\0';
       sprintf(read_com, "w1 %x %.2x %.2x\r", devaddr, regaddr, wdat & 0xFF);

                        ret = usb_Send(read_com);
                        if (ret != FT_OK) return -1;

                        //Sleep(450);
                        //ret = usb_Receive(read_dat);
                        Sleep(50);

                        return ret;
}

int r2_mipi_spi_i2c_app::update_vcore2_dynamic(int v1, int v2, int v3, int v4, int sdelay)
{
    char write_com[256];
    int ret=0;

           write_com[0] = '\0';
           sprintf(write_com, "uv %.2x %.2x %.2x %.2x %.3x\r", v1, v2, v3, v4, sdelay);
           //uv 01 01 01 01 019\r
                            ret = usb_Send(write_com);
                            if (ret != FT_OK) return -1;

                            Sleep(50);
                            return ret;
}
