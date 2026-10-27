#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

struct UART {

  int fd=0;

  int open(const char *port)
  {
    fd = ::open(port, O_RDWR|O_NOCTTY|O_NDELAY);
    
    if(!fd)
    {
      std::cerr << "Can't open the serial port\n";
      fd=0;
      return -1;
    }
    
    if(fcntl(fd, F_SETFL, 0) < 0)
    {
      std::cout << "Fcntl failed\n";
      fd=0;
      return -1;
    }
    else
      std::cout << "Fcntl=" << fcntl(fd,F_SETFL,0) << '\n';
    
    if(isatty(STDIN_FILENO)==0)
    {
      std::cout << "Standard input is not a terminal device\n";
      fd=0;
      return -1;
    }
    else
      std::cout << "Isatty success\n";
    
    std::cout << "fd->open=" << fd << '\n';
    return 0;
  }

  int set(int speed, int flow_ctrl, int databits, int stopbits, int parity)
  {
    if(!fd)
      return -1;
    int i;
    int status;
    int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};
    struct termios options;
    if(tcgetattr(fd, &options)!=0)
    {
      std::cerr << "Setup UART failed\n";
      return -1;
    }

    for( i=0; i < sizeof(speed_arr)/sizeof(int); i++)
    {
      if(speed == name_arr[i])
      {
        cfsetispeed(&options, speed_arr[i]);
        cfsetospeed(&options, speed_arr[i]);
      }
    }
/*

    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;

    //needed for some reason... (fadey)
    options.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE);
    options.c_cflag &= ~(IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IUCLC | IXANY | IMAXBEL | IUTF8);
    
    switch(flow_ctrl)
    {
      case -1:
        break;
      case 0:	// No flow control is used
        options.c_cflag &= ~CRTSCTS;
        break;
      case 1:	// Hardware flow control
        options.c_cflag |= CRTSCTS;
        break;
      case 2:	// Software flow control
        options.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }

    options.c_cflag &= ~CSIZE;	// Mask other flag bits
    switch (databits)
    {
      case 5:
        options.c_cflag |= CS5;
        break;
      case 6:
        options.c_cflag |= CS6;
        break;
      case 7:
        options.c_cflag |= CS7;
        break;
      case 8:    
        options.c_cflag |= CS8;
        break;  
      default:   
        std::cerr << "Unsupported data size\n";
        return -1;
    }

    switch(parity)
    {  
      case 'n':
      case 'N':	//No Parity bit
        options.c_cflag &= ~PARENB; 
        options.c_iflag &= ~INPCK;    
        break; 
      case 'o':  
      case 'O':	//Odd parity check
        options.c_cflag |= (PARODD | PARENB); 
        options.c_iflag |= INPCK;             
        break; 
      case 'e': 
      case 'E':	//Even parity check
        options.c_cflag |= PARENB;       
        options.c_cflag &= ~PARODD;       
        options.c_iflag |= INPCK;       
        break;
      case 's':
      case 'S':	//Space
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break; 
      default:  
        std::cerr << "Unsupported parity\n";   
        return -1; 
    } 

    //Set stop bit
    switch (stopbits)
    {  
      case 1:   
        options.c_cflag &= ~CSTOPB; 
        break; 
      case 2:   
        options.c_cflag |= CSTOPB; 
        break;
      default:   
        std::cerr << "Unsupported stop bits\n"; 
      return -1;
    }

      //Modify output mode, RAW data mode
    options.c_oflag &= ~OPOST;

    //set the minimum waiting time and minimum receiving bytes before unblocking
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 1;

*/


    options.c_cflag &= ~(PARODD | CSTOPB | CSIZE | CRTSCTS | HUPCL);
    options.c_cflag |= (CS8 | PARENB | CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ISIG | ECHO | ECHOE | ECHOK | ECHONL | ECHOCTL | ECHOPRT | ECHOKE | IEXTEN);
//    options.c_iflag &= ~(INPCK | IXON | IXOFF | IXANY | ICRNL);
    options.c_iflag &= ~(INPCK | IXON | IXOFF | IXANY | ICRNL);
    options.c_iflag |= INPCK;
//    options.c_oflag &= ~(OPOST | ONLCR);
    options.c_oflag &= ~(OPOST);

/*
    for (unsigned int i = 0; i < sizeof (options.c_cc); i++)
      options.c_cc[i] = _POSIX_VDISABLE;
*/

    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    //active the configuration
    if(tcsetattr(fd, TCSANOW, &options) != 0)
    {
      std::cout << "com set error!\n";  
      return -1; 
    }

    return 0;
  }

  void close()
  {
    ::close(fd);
    fd = 0;
  }

  int recv(char *rcv_buf, int data_len, int tv_sec=13)
  {
    if(!fd)
      return -1;
    int len, fs_sel;
    fd_set fs_read;
    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = tv_sec;
    time.tv_usec = 0;//333000; //0.333s

    // Using Select() function to realize the multiply channels' communication

    fs_sel = select(fd+1, &fs_read, NULL, NULL, &time);
    if(fs_sel)
    {
      len = read(fd, rcv_buf, data_len);
      return len;
    } else
      return 0;
  }

  int send(char *send_buf, int data_len)
  {
    if(!fd)
      return -1;
    int len = 0;
    len = write(fd, send_buf, data_len);
    if(len == data_len)
    {
      return len;
    } else {
      tcflush(fd,TCOFLUSH);
      return -1;
    }
  }

};
