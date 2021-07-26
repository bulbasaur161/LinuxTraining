#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
//#include <linux/i2c.h>   // Was in the original code, but doesn't seem to be needed

#define DS3231_SLAVE_ADDR		0x68			// Slave Address
#define I2C_FILE_PATH			"/dev/i2c-2"	// Files path for the i2c-2 directory

int file;

// Convert the bcd registers to decimal
int bcd2dec(char b) { return ((b/16)*10 + (b%16)); }

//convert decimal back to bcd, when writing to module (works only with two digits)
int DecimalToBCD (char d) { return (((d/10) << 4) | (d % 10)); }


int set_time(int hh, int mm, int ss)
{
	char writeBuffer[2]; // Store address and data

	// Open file
	if ((file=open(I2C_FILE_PATH, O_RDWR)) < 0)
	{
		perror("Failed to open the bus\n");
		return -1;
	}

	// Connect to slave
	if ((ioctl(file, I2C_SLAVE, DS3231_SLAVE_ADDR)) < 0)
	{
		perror("Failed to connect to the sensor\n");
		return -1;
	}

	//Set seconds address and data
	writeBuffer[0] = 0x00;
	writeBuffer[1] = DecimalToBCD(ss);

	//Write data to seconds address
	if (write(file, writeBuffer, 2) != 2)
	{
		perror("Failed to write to the register\n");
		return -1;
	}

	//Set seconds address and data
	writeBuffer[0] = 0x01;
	writeBuffer[1] = DecimalToBCD(mm);
	if (write(file, writeBuffer, 2) != 2)
	{
		perror("Failed to write to the register\n");
		return -1;
	}

	// Set hours and data
	writeBuffer[0] = 0x02;
	writeBuffer[1] = DecimalToBCD(hh);
	if (write(file, writeBuffer, 2) != 2)
	{
		perror("Failed to write to the register\n");
		return -1;
	}

	close(file);

	return 0;

}

int set_date(int dd, int mm, int yy)
{
	char writeBuffer[2];

	if ((file=open(I2C_FILE_PATH, O_RDWR)) < 0)
	{
		perror("Failed to open the bus\n");
		return -1;
	}

	if ((ioctl(file, I2C_SLAVE, DS3231_SLAVE_ADDR)) < 0)
	{
		perror("Failed to connect to the sensor\n");
		return -1;
	}

	writeBuffer[0] = 0x04;
	writeBuffer[1] = DecimalToBCD(dd);

	if (write(file, writeBuffer, 2) != 2)
	{
		perror("Failed to write to the register\n");
		return -1;
	}

	writeBuffer[0] = 0x05;
	writeBuffer[1] = DecimalToBCD(mm);
	if (write(file, writeBuffer, 2) != 2)
	{
		perror("Failed to write to the register\n");
		return -1;
	}

	writeBuffer[0] = 0x06;
	writeBuffer[1] = DecimalToBCD(yy);
	if (write(file, writeBuffer, 2) != 2)
	{
		perror("Failed to write to the register\n");
		return -1;
	}

	close(file);

	return 0;

}

int get_time()
{

	if ((file=open(I2C_FILE_PATH, O_RDWR)) < 0)
	{
		perror("Failed to open the bus\n");
		return -1;
	}

	if ((ioctl(file, I2C_SLAVE, DS3231_SLAVE_ADDR)) < 0)
	{
		perror("Failed to connect to the sensor\n");
		return -1;
	}

	// Writing to this address will set the first read to this
	char writeBuffer[1] = {0x00}; // Needs to be in an array (buffer) or won't work
	if (write(file, writeBuffer, 1) != 1)
	{
		perror("Failed to reset the read address\n");
		return -1;
	}

	// Read and hold the three time values
	char buf[3];
	if (read(file, buf, 3) != 3)
	{
		perror("Failed to read to the buffer\n");
		return -1;
	}

	close(file);

	printf("The RTC time is %02d:%02d:%02d\n", bcd2dec(buf[2]), bcd2dec(buf[1]), bcd2dec(buf[0]));

	return 0;
}

int get_date()
{
	if ((file=open(I2C_FILE_PATH, O_RDWR)) < 0)
	{
		perror("Failed to open the bus\n");
		return -1;
	}

	if ((ioctl(file, I2C_SLAVE, DS3231_SLAVE_ADDR)) < 0)
	{
		perror("Failed to connect to the sensor\n");
		return -1;
	}

	char writeBuffer[1] = {0x04};
	if (write(file, writeBuffer, 1) != 1)
	{
		perror("Failed to reset the read address\n");
		return -1;
	}

	char buf[3];
	if (read(file, buf, 3) != 3)
	{
		perror("Failed to read to the buffer\n");
		return -1;
	}

	close(file);

	printf("The date is %02d/%02d/%02d\n", bcd2dec(buf[0]), bcd2dec(buf[1]), bcd2dec(buf[2]));

	return 0;


}

int get_temperature()
{
	if ((file=open(I2C_FILE_PATH, O_RDWR)) < 0)
	{
		perror("Failed to open the bus\n");
		return -1;
	}

	if ((ioctl(file, I2C_SLAVE, DS3231_SLAVE_ADDR)) < 0)
	{
		perror("Failed to connect to the sensor\n");
		return -1;
	}

	char writeBuffer[1] = {0x11}; // Needs to be in an array (buffer) or won't work
	if (write(file, writeBuffer, 1) != 1)
	{
		perror("Failed to reset the read address\n");
		return -1;
	}

	char buf[2];
	if (read(file, buf, 2) != 2)
	{
		perror("Failed to read to the buffer\n");
		return -1;
	}

	float temperature = buf[0] + ((buf[1] >> 6) * 0.25);
	close(file);

	printf("The temperature is %0.2fC\n", temperature);

	return 0;

}

int main(void)
{
	printf("Starting the DS3231 application\n");
	//set_time(15, 18, 00); 	// Set the time (hours, minutes, seconds)
	get_time();					// Get the current time (GMT)
	//set_date(9, 10, 20);  	// Get the date (day, month, year)
	get_date();					// Get the date
	get_temperature();  		// Get the temperature

	return 0;
}
