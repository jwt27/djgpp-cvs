/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_termios_h_
#define __dj_include_sys_termios_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#define B0	0x00000000
#define B50	0x00000001
#define B75	0x00000002
#define B110	0x00000003
#define B134	0x00000004
#define B150	0x00000005
#define B200	0x00000006
#define B300	0x00000007
#define B600	0x00000008
#define B1200	0x00000009
#define B1800	0x0000000a
#define B2400	0x0000000b
#define B4800	0x0000000c
#define B9600	0x0000000d
#define B19200	0x0000000e
#define B38400	0x0000000f

#define BRKINT	0x00000100
#define ICRNL	0x00000200
#define IGNBRK	0x00000400
#define IGNCR	0x00000800
#define IGNPAR	0x00001000
#define INLCR	0x00002000
#define INPCK	0x00004000
#define ISTRIP	0x00008000
#define IXOFF	0x00010000
#define IXON	0x00020000
#define PARMRK	0x00040000

#define OPOST	0x00000100

#define CLOCAL	0x00000100
#define CREAD	0x00000200
#define CS5	0x00000000
#define CS6	0x00000400
#define CS7	0x00000800
#define CS8	0x00000c00
#define CSIZE	0x00000c00
#define CSTOPB	0x00001000
#define HUPCL	0x00002000
#define PARENB	0x00004000
#define PARODD	0x00008000

#define ECHO	0x00000100
#define ECHOE	0x00000200
#define ECHOK	0x00000400
#define ECHONL	0x00000800
#define ICANON	0x00001000
#define IEXTEN	0x00002000
#define ISIG	0x00004000
#define NOFLSH	0x00008000
#define TOSTOP	0x00010000

#define TCIFLUSH
#define TCIOFF
#define TCIOFLUSH
#define TCION
#define TCOFLUSH
#define TCOOFF
#define TCOON

#define TCSADRAIN	1
#define TCSAFLUSH	2
#define TCSANOW		3

#define VEOF	1
#define VEOL	2
#define VERASE	3
#define VINTR	4
#define VKILL	5
#define VMIN	6
#define VQUIT	7
#define VSTART	8
#define VSTOP	9
#define VSUSP	10
#define VTIME	11
#define NCCS	12

typedef unsigned cc_t;
typedef unsigned speed_t;
typedef unsigned tcflag_t;

struct termios {
  cc_t		c_cc[NCCS];
  tcflag_t	c_cflag;
  tcflag_t	c_iflag;
  tcflag_t	c_lflag;
  tcflag_t	c_oflag;
};

speed_t	cfgetispeed(const struct termios *_termios_p);
speed_t cfgetospeed(const struct termios *_termios_p);
int	cfsetispeed(const struct termios *_termios_p, speed_t _speed);
int	cfsetospeed(const struct termios *_termios_p, speed_t _speed);
int	tcdrain(int _fildes);
int	tcflow(int _fildes, int _action);
int	tcflush(int _fildes, int _queue_selector);
int	tcgetattr(int _fildes, struct termios *_termios_p);
int	tcsendbreak(int _fildes, int _duration);
int	tcsetattr(int _fildes, int _optional_actions, const struct termios *_termios_p);

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_termios_h_ */
