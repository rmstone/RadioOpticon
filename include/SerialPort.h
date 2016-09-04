#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>
#include <stdint.h>

int set_interface_attribs( int fd, int speed, int parity );
void set_blocking( int fd, int should_block );
int openSerialPort( std::string portname );
void closeSerialPort( int fd );
int writeSerialPort( int fd, uint8_t * data, int count );
int readSerialPort( int fd, uint8_t * data, int count );
int readSerialPortLine( int fd, std::string &data, int count );

#endif