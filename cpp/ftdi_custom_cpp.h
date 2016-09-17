#ifndef R2_MIPI_SPI_I2C_APP_H
#define R2_MIPI_SPI_I2C_APP_H

#include <QString>
#include <FTD2XX.H>

class r2_mipi_spi_i2c_app
{
public:

    r2_mipi_spi_i2c_app();
    void init_r2_usb();
    int Set_VIO(int on_off);
    void close_r2_usb();
    int usb_Send(QString cmd);
    //int usb_Send_ext(QString cmd, int len);
    int usb_Receive(char *rxbuf);
    int usb_mipi_write_1byte_long(int devaddr, unsigned int regaddr, unsigned int wdat);
    int usb_mipi_write_1byte(int devaddr, unsigned int regaddr, unsigned int wdat);
    int update_vcore2_dynamic(int v1, int v2, int v3, int v4, int sdelay);
};

#endif // R2_MIPI_SPI_I2C_APP_H
