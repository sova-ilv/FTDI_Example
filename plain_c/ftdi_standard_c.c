
#include <windows.h>   
#include <formatio.h>
#include <utility.h>
#include <ansi_c.h>
#include <userint.h>

#include "FTD2XX.h"   



int port_val = 0;
int device_number = -1;
char R2_Serial[100];
FT_HANDLE FTDIHandle = 0;    
FT_STATUS ftStatus; 
DWORD BytesWritten, RxBytes, TxBytes, EventDWord; 
char TxBuffer[956]; // Contains data to write to device 
char recvddat[956];  


//--------------------------------------------------------------------------
int Open_ftdi_device(void)
{
	HRESULT res;
	
	GetDevices();   
	res = init_usb();
	if (res!=0) MessagePopup("Error opening USB", "Could not open USB-RFFE device 0");

        
	return res;
}

//Pool the buffer if data is available
int pool_buffer_status(void)
{
	DWORD RxBytes;
	
	FT_GetQueueStatus(FTDIHandle,&RxBytes);
	if (RxBytes > 0) 
	{
		FT_Purge (FTDIHandle, 3);  
		return 1;				   }
	return 0;
}
//--------------------------------------------------------------------------
int GetDevices(void)
{
	char *BufPtrs[MAX_DEVICES + 1];		// pointer to array of pointers 
	char Buffer[MAX_DEVICES][64];		// buffer for description of devices 
	HRESULT status;
	long num_devs;
	long i;
	char mess[1024];
	
	// initialize the array of pointers 
	for (i=0; i<MAX_DEVICES; i++) BufPtrs[i] = Buffer[i];
	BufPtrs[MAX_DEVICES] = NULL;      // last entry should be NULL 
	//
	status = FT_ListDevices(BufPtrs, &num_devs, FT_LIST_ALL|FT_OPEN_BY_SERIAL_NUMBER); 

	for (i=0; i<num_devs; i++)
		{
		Fmt(mess, "%s<%s", Buffer[i]);
		device_number = i;
		//SetCtrlVal(panelmain, PANELMAIN_TEXTBOX, mess);
		strcpy(R2_Serial, mess);
		if (i==usb.vcp_seq) break;      
		}
	return device_number;
}
	
/* *************************** 
	int Initialize_usb(void) 
 ***************************** */	
int init_usb(void)
{
	ftStatus = FT_OpenEx(R2_Serial, FT_OPEN_BY_SERIAL_NUMBER,&FTDIHandle);
	if(ftStatus != FT_OK)  return -1;
	
    ftStatus = FT_SetBaudRate(FTDIHandle, usb.usb_uart_speed );	
		if (ftStatus != FT_OK) return -1;  
		    
	// Set 8 data bits, 1 stop bit and no parity 
	ftStatus = FT_SetDataCharacteristics(FTDIHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
		if (ftStatus != FT_OK) return -1;  
		
	// Set read timeout to 5 seconds, write timeout to infinite	
	//ftStatus = FT_SetTimeouts(FTDIHandle, 1000, 1000);
	//	if (ftStatus != FT_OK) return -1; 
		
	FT_SetTimeouts(FTDIHandle,10000,0); //5 se 	
	//ftStatus =FT_GetLatencyTimer (FTDIHandle, &lat);	
	//ftStatus =FT_SetLatencyTimer (FTDIHandle, 5);
		
	if (Slave.Protocol == SPI) {
		Configure_SPI_rate(5);   //8MHz	
	    RR_SPI_mode(1);
	    Configure_SPI_phase(3); } 
	
		discover_gecko_board(); 
		
		
	return 0;		
}	

void CloseUsb(void)
{
	FT_Close(FTDIHandle);
}

int usb_Send(unsigned char wdata[100])
{
	int len;
	
	len = strlen(wdata);
	
	ftStatus = FT_Write(FTDIHandle, wdata, len, &BytesWritten);
	if (ftStatus != FT_OK) return -1;       
    return 0;
}

int usb_Receive(unsigned char *rxbuf)
{
	unsigned int  numBytesRead = 0;
	char tempbuf[1956];

	tempbuf[0]='\0';
	rxbuf[0]='\0';
	ftStatus = FT_Read(FTDIHandle,tempbuf,RxBytes,&numBytesRead); 
	tempbuf[0]='\0';
	rxbuf[0]='\0';
	
	while(1) {
	ftStatus = FT_GetStatus(FTDIHandle,&RxBytes,&TxBytes,&EventDWord);
	if (RxBytes > 0) {
		ftStatus = FT_Read(FTDIHandle,tempbuf,RxBytes,&numBytesRead);
		strcat(rxbuf, tempbuf);
		if (ftStatus != FT_OK) 
			return -1; 
	}
	else break;
	}
	

	return 0;
}


//--------------------------------------------------------------------------
int close_usb(void)
{
	HRESULT res;
	
	if (!open_usb()) return -1; 
	res = FT_Close(FTDIHandle);
	FTDIHandle = 0;
	return res;
}

//--------------------------------------------------------------------------
int open_usb(void)
{
	int res;
	
	res = 1;
	if (FTDIHandle == 0) res = 0;
	return res;
}
//--------------------------------------------------------------------------
int GetDeviceInfo(void)
{
	HRESULT res;
	FT_DEVICE  device;
	LPDWORD lpdwID;
	PCHAR SerialNumber;
	PCHAR Description;
	LPVOID Dummy;
	
	if (!open_usb()) return -1;
	
	lpdwID = malloc(256);
	SerialNumber = malloc(256);
	Description = malloc(256);
	res = FT_GetDeviceInfo(FTDIHandle, &device, lpdwID, SerialNumber, Description, &Dummy);
	return res;
}


//--------------------------------------------------------------------------
int SetBitMode(UCHAR mask, UCHAR endis)
{
	HRESULT status; 

	if (!open_usb()) return -1;
	status = FT_SetBitMode(FTDIHandle, mask, endis);
	return status;
} 


//--------------------------------------------------------------------------
int GetBitMode(void)
{
	UCHAR BitMode; 
	HRESULT status; 


	if (!open_usb()) return -1;
	status = FT_GetBitMode(FTDIHandle, &BitMode); 
	return BitMode;
} 

