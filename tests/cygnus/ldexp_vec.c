#include "test.h"
 one_line_type ldexp_vec[] = {
{62, 0, 0,__LINE__, {{0x3f355555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0240000, 0x00000000}}},	/* +3.2552E-04=F( +0.3333,     -10) */
{62, 0, 0,__LINE__, {{0x3f455555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0220000, 0x00000000}}},	/* +6.5104E-04=F( +0.3333,      -9) */
{62, 0, 0,__LINE__, {{0x3f555555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0200000, 0x00000000}}},	/* +1.3021E-03=F( +0.3333,      -8) */
{62, 0, 0,__LINE__, {{0x3f655555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc01c0000, 0x00000000}}},	/* +2.6042E-03=F( +0.3333,      -7) */
{62, 0, 0,__LINE__, {{0x3f755555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0180000, 0x00000000}}},	/* +5.2083E-03=F( +0.3333,      -6) */
{62, 0, 0,__LINE__, {{0x3f855555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0140000, 0x00000000}}},	/* +1.0417E-02=F( +0.3333,      -5) */
{62, 0, 0,__LINE__, {{0x3f955555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0100000, 0x00000000}}},	/* +2.0833E-02=F( +0.3333,      -4) */
{62, 0, 0,__LINE__, {{0x3fa55555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0080000, 0x00000000}}},	/* +4.1667E-02=F( +0.3333,      -3) */
{62, 0, 0,__LINE__, {{0x3fb55555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xc0000000, 0x00000000}}},	/* +8.3333E-02=F( +0.3333,      -2) */
{62, 0, 0,__LINE__, {{0x3fc55555, 0x55555555}, {0x3fd55555, 0x55555555}, {0xbff00000, 0x00000000}}},	/* +1.6667E-01=F( +0.3333,      -1) */
{62, 0, 0,__LINE__, {{0x3fd55555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x00000000, 0x00000000}}},	/* +3.3333E-01=F( +0.3333,      +0) */
{62, 0, 0,__LINE__, {{0x3fe55555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x3ff00000, 0x00000000}}},	/* +6.6667E-01=F( +0.3333,      +1) */
{62, 0, 0,__LINE__, {{0x3ff55555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40000000, 0x00000000}}},	/* +1.3333E+00=F( +0.3333,      +2) */
{62, 0, 0,__LINE__, {{0x40055555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40080000, 0x00000000}}},	/* +2.6667E+00=F( +0.3333,      +3) */
{62, 0, 0,__LINE__, {{0x40155555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40100000, 0x00000000}}},	/* +5.3333E+00=F( +0.3333,      +4) */
{62, 0, 0,__LINE__, {{0x40255555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40140000, 0x00000000}}},	/* +1.0667E+01=F( +0.3333,      +5) */
{62, 0, 0,__LINE__, {{0x40355555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40180000, 0x00000000}}},	/* +2.1333E+01=F( +0.3333,      +6) */
{62, 0, 0,__LINE__, {{0x40455555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x401c0000, 0x00000000}}},	/* +4.2667E+01=F( +0.3333,      +7) */
{62, 0, 0,__LINE__, {{0x40555555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40200000, 0x00000000}}},	/* +8.5333E+01=F( +0.3333,      +8) */
{62, 0, 0,__LINE__, {{0x40655555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40220000, 0x00000000}}},	/* +1.7067E+02=F( +0.3333,      +9) */
{62, 0, 0,__LINE__, {{0x40755555, 0x55555555}, {0x3fd55555, 0x55555555}, {0x40240000, 0x00000000}}},	/* +3.4133E+02=F( +0.3333,     +10) */
{62, 0, 0,__LINE__, {{0x3f299999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0240000, 0x00000000}}},	/* +1.9531E-04=F(    +0.2,     -10) */
{62, 0, 0,__LINE__, {{0x3f399999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0220000, 0x00000000}}},	/* +3.9063E-04=F(    +0.2,      -9) */
{62, 0, 0,__LINE__, {{0x3f499999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0200000, 0x00000000}}},	/* +7.8125E-04=F(    +0.2,      -8) */
{62, 0, 0,__LINE__, {{0x3f599999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc01c0000, 0x00000000}}},	/* +1.5625E-03=F(    +0.2,      -7) */
{62, 0, 0,__LINE__, {{0x3f699999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0180000, 0x00000000}}},	/* +3.1250E-03=F(    +0.2,      -6) */
{62, 0, 0,__LINE__, {{0x3f799999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0140000, 0x00000000}}},	/* +6.2500E-03=F(    +0.2,      -5) */
{62, 0, 0,__LINE__, {{0x3f899999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0100000, 0x00000000}}},	/* +1.2500E-02=F(    +0.2,      -4) */
{62, 0, 0,__LINE__, {{0x3f999999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0080000, 0x00000000}}},	/* +2.5000E-02=F(    +0.2,      -3) */
{62, 0, 0,__LINE__, {{0x3fa99999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xc0000000, 0x00000000}}},	/* +5.0000E-02=F(    +0.2,      -2) */
{62, 0, 0,__LINE__, {{0x3fb99999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0xbff00000, 0x00000000}}},	/* +1.0000E-01=F(    +0.2,      -1) */
{62, 0, 0,__LINE__, {{0x3fc99999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x00000000, 0x00000000}}},	/* +2.0000E-01=F(    +0.2,      +0) */
{62, 0, 0,__LINE__, {{0x3fd99999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x3ff00000, 0x00000000}}},	/* +4.0000E-01=F(    +0.2,      +1) */
{62, 0, 0,__LINE__, {{0x3fe99999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40000000, 0x00000000}}},	/* +8.0000E-01=F(    +0.2,      +2) */
{62, 0, 0,__LINE__, {{0x3ff99999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40080000, 0x00000000}}},	/* +1.6000E+00=F(    +0.2,      +3) */
{62, 0, 0,__LINE__, {{0x40099999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40100000, 0x00000000}}},	/* +3.2000E+00=F(    +0.2,      +4) */
{62, 0, 0,__LINE__, {{0x40199999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40140000, 0x00000000}}},	/* +6.4000E+00=F(    +0.2,      +5) */
{62, 0, 0,__LINE__, {{0x40299999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40180000, 0x00000000}}},	/* +1.2800E+01=F(    +0.2,      +6) */
{62, 0, 0,__LINE__, {{0x40399999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x401c0000, 0x00000000}}},	/* +2.5600E+01=F(    +0.2,      +7) */
{62, 0, 0,__LINE__, {{0x40499999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40200000, 0x00000000}}},	/* +5.1200E+01=F(    +0.2,      +8) */
{62, 0, 0,__LINE__, {{0x40599999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40220000, 0x00000000}}},	/* +1.0240E+02=F(    +0.2,      +9) */
{62, 0, 0,__LINE__, {{0x40699999, 0x9999999a}, {0x3fc99999, 0x9999999a}, {0x40240000, 0x00000000}}},	/* +2.0480E+02=F(    +0.2,     +10) */
{62, 0, 0,__LINE__, {{0x3f224924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0240000, 0x00000000}}},	/* +1.3951E-04=F( +0.1429,     -10) */
{62, 0, 0,__LINE__, {{0x3f324924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0220000, 0x00000000}}},	/* +2.7902E-04=F( +0.1429,      -9) */
{62, 0, 0,__LINE__, {{0x3f424924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0200000, 0x00000000}}},	/* +5.5804E-04=F( +0.1429,      -8) */
{62, 0, 0,__LINE__, {{0x3f524924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc01c0000, 0x00000000}}},	/* +1.1161E-03=F( +0.1429,      -7) */
{62, 0, 0,__LINE__, {{0x3f624924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0180000, 0x00000000}}},	/* +2.2321E-03=F( +0.1429,      -6) */
{62, 0, 0,__LINE__, {{0x3f724924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0140000, 0x00000000}}},	/* +4.4643E-03=F( +0.1429,      -5) */
{62, 0, 0,__LINE__, {{0x3f824924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0100000, 0x00000000}}},	/* +8.9286E-03=F( +0.1429,      -4) */
{62, 0, 0,__LINE__, {{0x3f924924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0080000, 0x00000000}}},	/* +1.7857E-02=F( +0.1429,      -3) */
{62, 0, 0,__LINE__, {{0x3fa24924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xc0000000, 0x00000000}}},	/* +3.5714E-02=F( +0.1429,      -2) */
{62, 0, 0,__LINE__, {{0x3fb24924, 0x92492492}, {0x3fc24924, 0x92492492}, {0xbff00000, 0x00000000}}},	/* +7.1429E-02=F( +0.1429,      -1) */
{62, 0, 0,__LINE__, {{0x3fc24924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x00000000, 0x00000000}}},	/* +1.4286E-01=F( +0.1429,      +0) */
{62, 0, 0,__LINE__, {{0x3fd24924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x3ff00000, 0x00000000}}},	/* +2.8571E-01=F( +0.1429,      +1) */
{62, 0, 0,__LINE__, {{0x3fe24924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40000000, 0x00000000}}},	/* +5.7143E-01=F( +0.1429,      +2) */
{62, 0, 0,__LINE__, {{0x3ff24924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40080000, 0x00000000}}},	/* +1.1429E+00=F( +0.1429,      +3) */
{62, 0, 0,__LINE__, {{0x40024924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40100000, 0x00000000}}},	/* +2.2857E+00=F( +0.1429,      +4) */
{62, 0, 0,__LINE__, {{0x40124924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40140000, 0x00000000}}},	/* +4.5714E+00=F( +0.1429,      +5) */
{62, 0, 0,__LINE__, {{0x40224924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40180000, 0x00000000}}},	/* +9.1429E+00=F( +0.1429,      +6) */
{62, 0, 0,__LINE__, {{0x40324924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x401c0000, 0x00000000}}},	/* +1.8286E+01=F( +0.1429,      +7) */
{62, 0, 0,__LINE__, {{0x40424924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40200000, 0x00000000}}},	/* +3.6571E+01=F( +0.1429,      +8) */
{62, 0, 0,__LINE__, {{0x40524924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40220000, 0x00000000}}},	/* +7.3143E+01=F( +0.1429,      +9) */
{62, 0, 0,__LINE__, {{0x40624924, 0x92492492}, {0x3fc24924, 0x92492492}, {0x40240000, 0x00000000}}},	/* +1.4629E+02=F( +0.1429,     +10) */
{62, 0, 0,__LINE__, {{0x3f1c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0240000, 0x00000000}}},	/* +1.0851E-04=F( +0.1111,     -10) */
{62, 0, 0,__LINE__, {{0x3f2c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0220000, 0x00000000}}},	/* +2.1701E-04=F( +0.1111,      -9) */
{62, 0, 0,__LINE__, {{0x3f3c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0200000, 0x00000000}}},	/* +4.3403E-04=F( +0.1111,      -8) */
{62, 0, 0,__LINE__, {{0x3f4c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc01c0000, 0x00000000}}},	/* +8.6806E-04=F( +0.1111,      -7) */
{62, 0, 0,__LINE__, {{0x3f5c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0180000, 0x00000000}}},	/* +1.7361E-03=F( +0.1111,      -6) */
{62, 0, 0,__LINE__, {{0x3f6c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0140000, 0x00000000}}},	/* +3.4722E-03=F( +0.1111,      -5) */
{62, 0, 0,__LINE__, {{0x3f7c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0100000, 0x00000000}}},	/* +6.9444E-03=F( +0.1111,      -4) */
{62, 0, 0,__LINE__, {{0x3f8c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0080000, 0x00000000}}},	/* +1.3889E-02=F( +0.1111,      -3) */
{62, 0, 0,__LINE__, {{0x3f9c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xc0000000, 0x00000000}}},	/* +2.7778E-02=F( +0.1111,      -2) */
{62, 0, 0,__LINE__, {{0x3fac71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0xbff00000, 0x00000000}}},	/* +5.5556E-02=F( +0.1111,      -1) */
{62, 0, 0,__LINE__, {{0x3fbc71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x00000000, 0x00000000}}},	/* +1.1111E-01=F( +0.1111,      +0) */
{62, 0, 0,__LINE__, {{0x3fcc71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x3ff00000, 0x00000000}}},	/* +2.2222E-01=F( +0.1111,      +1) */
{62, 0, 0,__LINE__, {{0x3fdc71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40000000, 0x00000000}}},	/* +4.4444E-01=F( +0.1111,      +2) */
{62, 0, 0,__LINE__, {{0x3fec71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40080000, 0x00000000}}},	/* +8.8889E-01=F( +0.1111,      +3) */
{62, 0, 0,__LINE__, {{0x3ffc71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40100000, 0x00000000}}},	/* +1.7778E+00=F( +0.1111,      +4) */
{62, 0, 0,__LINE__, {{0x400c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40140000, 0x00000000}}},	/* +3.5556E+00=F( +0.1111,      +5) */
{62, 0, 0,__LINE__, {{0x401c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40180000, 0x00000000}}},	/* +7.1111E+00=F( +0.1111,      +6) */
{62, 0, 0,__LINE__, {{0x402c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x401c0000, 0x00000000}}},	/* +1.4222E+01=F( +0.1111,      +7) */
{62, 0, 0,__LINE__, {{0x403c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40200000, 0x00000000}}},	/* +2.8444E+01=F( +0.1111,      +8) */
{62, 0, 0,__LINE__, {{0x404c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40220000, 0x00000000}}},	/* +5.6889E+01=F( +0.1111,      +9) */
{62, 0, 0,__LINE__, {{0x405c71c7, 0x1c71c71c}, {0x3fbc71c7, 0x1c71c71c}, {0x40240000, 0x00000000}}},	/* +1.1378E+02=F( +0.1111,     +10) */
{62, 0, 0,__LINE__, {{0x3f1745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0240000, 0x00000000}}},	/* +8.8778E-05=F(+0.09091,     -10) */
{62, 0, 0,__LINE__, {{0x3f2745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0220000, 0x00000000}}},	/* +1.7756E-04=F(+0.09091,      -9) */
{62, 0, 0,__LINE__, {{0x3f3745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0200000, 0x00000000}}},	/* +3.5511E-04=F(+0.09091,      -8) */
{62, 0, 0,__LINE__, {{0x3f4745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc01c0000, 0x00000000}}},	/* +7.1023E-04=F(+0.09091,      -7) */
{62, 0, 0,__LINE__, {{0x3f5745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0180000, 0x00000000}}},	/* +1.4205E-03=F(+0.09091,      -6) */
{62, 0, 0,__LINE__, {{0x3f6745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0140000, 0x00000000}}},	/* +2.8409E-03=F(+0.09091,      -5) */
{62, 0, 0,__LINE__, {{0x3f7745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0100000, 0x00000000}}},	/* +5.6818E-03=F(+0.09091,      -4) */
{62, 0, 0,__LINE__, {{0x3f8745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0080000, 0x00000000}}},	/* +1.1364E-02=F(+0.09091,      -3) */
{62, 0, 0,__LINE__, {{0x3f9745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xc0000000, 0x00000000}}},	/* +2.2727E-02=F(+0.09091,      -2) */
{62, 0, 0,__LINE__, {{0x3fa745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0xbff00000, 0x00000000}}},	/* +4.5455E-02=F(+0.09091,      -1) */
{62, 0, 0,__LINE__, {{0x3fb745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x00000000, 0x00000000}}},	/* +9.0909E-02=F(+0.09091,      +0) */
{62, 0, 0,__LINE__, {{0x3fc745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x3ff00000, 0x00000000}}},	/* +1.8182E-01=F(+0.09091,      +1) */
{62, 0, 0,__LINE__, {{0x3fd745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40000000, 0x00000000}}},	/* +3.6364E-01=F(+0.09091,      +2) */
{62, 0, 0,__LINE__, {{0x3fe745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40080000, 0x00000000}}},	/* +7.2727E-01=F(+0.09091,      +3) */
{62, 0, 0,__LINE__, {{0x3ff745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40100000, 0x00000000}}},	/* +1.4545E+00=F(+0.09091,      +4) */
{62, 0, 0,__LINE__, {{0x400745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40140000, 0x00000000}}},	/* +2.9091E+00=F(+0.09091,      +5) */
{62, 0, 0,__LINE__, {{0x401745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40180000, 0x00000000}}},	/* +5.8182E+00=F(+0.09091,      +6) */
{62, 0, 0,__LINE__, {{0x402745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x401c0000, 0x00000000}}},	/* +1.1636E+01=F(+0.09091,      +7) */
{62, 0, 0,__LINE__, {{0x403745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40200000, 0x00000000}}},	/* +2.3273E+01=F(+0.09091,      +8) */
{62, 0, 0,__LINE__, {{0x404745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40220000, 0x00000000}}},	/* +4.6545E+01=F(+0.09091,      +9) */
{62, 0, 0,__LINE__, {{0x405745d1, 0x745d1746}, {0x3fb745d1, 0x745d1746}, {0x40240000, 0x00000000}}},	/* +9.3091E+01=F(+0.09091,     +10) */
{62, 0, 0,__LINE__, {{0x3f13b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0240000, 0x00000000}}},	/* +7.5120E-05=F(+0.07692,     -10) */
{62, 0, 0,__LINE__, {{0x3f23b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0220000, 0x00000000}}},	/* +1.5024E-04=F(+0.07692,      -9) */
{62, 0, 0,__LINE__, {{0x3f33b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0200000, 0x00000000}}},	/* +3.0048E-04=F(+0.07692,      -8) */
{62, 0, 0,__LINE__, {{0x3f43b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc01c0000, 0x00000000}}},	/* +6.0096E-04=F(+0.07692,      -7) */
{62, 0, 0,__LINE__, {{0x3f53b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0180000, 0x00000000}}},	/* +1.2019E-03=F(+0.07692,      -6) */
{62, 0, 0,__LINE__, {{0x3f63b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0140000, 0x00000000}}},	/* +2.4038E-03=F(+0.07692,      -5) */
{62, 0, 0,__LINE__, {{0x3f73b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0100000, 0x00000000}}},	/* +4.8077E-03=F(+0.07692,      -4) */
{62, 0, 0,__LINE__, {{0x3f83b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0080000, 0x00000000}}},	/* +9.6154E-03=F(+0.07692,      -3) */
{62, 0, 0,__LINE__, {{0x3f93b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xc0000000, 0x00000000}}},	/* +1.9231E-02=F(+0.07692,      -2) */
{62, 0, 0,__LINE__, {{0x3fa3b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0xbff00000, 0x00000000}}},	/* +3.8462E-02=F(+0.07692,      -1) */
{62, 0, 0,__LINE__, {{0x3fb3b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x00000000, 0x00000000}}},	/* +7.6923E-02=F(+0.07692,      +0) */
{62, 0, 0,__LINE__, {{0x3fc3b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x3ff00000, 0x00000000}}},	/* +1.5385E-01=F(+0.07692,      +1) */
{62, 0, 0,__LINE__, {{0x3fd3b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40000000, 0x00000000}}},	/* +3.0769E-01=F(+0.07692,      +2) */
{62, 0, 0,__LINE__, {{0x3fe3b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40080000, 0x00000000}}},	/* +6.1538E-01=F(+0.07692,      +3) */
{62, 0, 0,__LINE__, {{0x3ff3b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40100000, 0x00000000}}},	/* +1.2308E+00=F(+0.07692,      +4) */
{62, 0, 0,__LINE__, {{0x4003b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40140000, 0x00000000}}},	/* +2.4615E+00=F(+0.07692,      +5) */
{62, 0, 0,__LINE__, {{0x4013b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40180000, 0x00000000}}},	/* +4.9231E+00=F(+0.07692,      +6) */
{62, 0, 0,__LINE__, {{0x4023b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x401c0000, 0x00000000}}},	/* +9.8462E+00=F(+0.07692,      +7) */
{62, 0, 0,__LINE__, {{0x4033b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40200000, 0x00000000}}},	/* +1.9692E+01=F(+0.07692,      +8) */
{62, 0, 0,__LINE__, {{0x4043b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40220000, 0x00000000}}},	/* +3.9385E+01=F(+0.07692,      +9) */
{62, 0, 0,__LINE__, {{0x4053b13b, 0x13b13b14}, {0x3fb3b13b, 0x13b13b14}, {0x40240000, 0x00000000}}},	/* +7.8769E+01=F(+0.07692,     +10) */
{62, 0, 0,__LINE__, {{0x3f111111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0240000, 0x00000000}}},	/* +6.5104E-05=F(+0.06667,     -10) */
{62, 0, 0,__LINE__, {{0x3f211111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0220000, 0x00000000}}},	/* +1.3021E-04=F(+0.06667,      -9) */
{62, 0, 0,__LINE__, {{0x3f311111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0200000, 0x00000000}}},	/* +2.6042E-04=F(+0.06667,      -8) */
{62, 0, 0,__LINE__, {{0x3f411111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc01c0000, 0x00000000}}},	/* +5.2083E-04=F(+0.06667,      -7) */
{62, 0, 0,__LINE__, {{0x3f511111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0180000, 0x00000000}}},	/* +1.0417E-03=F(+0.06667,      -6) */
{62, 0, 0,__LINE__, {{0x3f611111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0140000, 0x00000000}}},	/* +2.0833E-03=F(+0.06667,      -5) */
{62, 0, 0,__LINE__, {{0x3f711111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0100000, 0x00000000}}},	/* +4.1667E-03=F(+0.06667,      -4) */
{62, 0, 0,__LINE__, {{0x3f811111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0080000, 0x00000000}}},	/* +8.3333E-03=F(+0.06667,      -3) */
{62, 0, 0,__LINE__, {{0x3f911111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xc0000000, 0x00000000}}},	/* +1.6667E-02=F(+0.06667,      -2) */
{62, 0, 0,__LINE__, {{0x3fa11111, 0x11111111}, {0x3fb11111, 0x11111111}, {0xbff00000, 0x00000000}}},	/* +3.3333E-02=F(+0.06667,      -1) */
{62, 0, 0,__LINE__, {{0x3fb11111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x00000000, 0x00000000}}},	/* +6.6667E-02=F(+0.06667,      +0) */
{62, 0, 0,__LINE__, {{0x3fc11111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x3ff00000, 0x00000000}}},	/* +1.3333E-01=F(+0.06667,      +1) */
{62, 0, 0,__LINE__, {{0x3fd11111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40000000, 0x00000000}}},	/* +2.6667E-01=F(+0.06667,      +2) */
{62, 0, 0,__LINE__, {{0x3fe11111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40080000, 0x00000000}}},	/* +5.3333E-01=F(+0.06667,      +3) */
{62, 0, 0,__LINE__, {{0x3ff11111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40100000, 0x00000000}}},	/* +1.0667E+00=F(+0.06667,      +4) */
{62, 0, 0,__LINE__, {{0x40011111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40140000, 0x00000000}}},	/* +2.1333E+00=F(+0.06667,      +5) */
{62, 0, 0,__LINE__, {{0x40111111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40180000, 0x00000000}}},	/* +4.2667E+00=F(+0.06667,      +6) */
{62, 0, 0,__LINE__, {{0x40211111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x401c0000, 0x00000000}}},	/* +8.5333E+00=F(+0.06667,      +7) */
{62, 0, 0,__LINE__, {{0x40311111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40200000, 0x00000000}}},	/* +1.7067E+01=F(+0.06667,      +8) */
{62, 0, 0,__LINE__, {{0x40411111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40220000, 0x00000000}}},	/* +3.4133E+01=F(+0.06667,      +9) */
{62, 0, 0,__LINE__, {{0x40511111, 0x11111111}, {0x3fb11111, 0x11111111}, {0x40240000, 0x00000000}}},	/* +6.8267E+01=F(+0.06667,     +10) */
{62, 0, 0,__LINE__, {{0x3f0e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0240000, 0x00000000}}},	/* +5.7445E-05=F(+0.05882,     -10) */
{62, 0, 0,__LINE__, {{0x3f1e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0220000, 0x00000000}}},	/* +1.1489E-04=F(+0.05882,      -9) */
{62, 0, 0,__LINE__, {{0x3f2e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0200000, 0x00000000}}},	/* +2.2978E-04=F(+0.05882,      -8) */
{62, 0, 0,__LINE__, {{0x3f3e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc01c0000, 0x00000000}}},	/* +4.5956E-04=F(+0.05882,      -7) */
{62, 0, 0,__LINE__, {{0x3f4e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0180000, 0x00000000}}},	/* +9.1912E-04=F(+0.05882,      -6) */
{62, 0, 0,__LINE__, {{0x3f5e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0140000, 0x00000000}}},	/* +1.8382E-03=F(+0.05882,      -5) */
{62, 0, 0,__LINE__, {{0x3f6e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0100000, 0x00000000}}},	/* +3.6765E-03=F(+0.05882,      -4) */
{62, 0, 0,__LINE__, {{0x3f7e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0080000, 0x00000000}}},	/* +7.3529E-03=F(+0.05882,      -3) */
{62, 0, 0,__LINE__, {{0x3f8e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xc0000000, 0x00000000}}},	/* +1.4706E-02=F(+0.05882,      -2) */
{62, 0, 0,__LINE__, {{0x3f9e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0xbff00000, 0x00000000}}},	/* +2.9412E-02=F(+0.05882,      -1) */
{62, 0, 0,__LINE__, {{0x3fae1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x00000000, 0x00000000}}},	/* +5.8824E-02=F(+0.05882,      +0) */
{62, 0, 0,__LINE__, {{0x3fbe1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x3ff00000, 0x00000000}}},	/* +1.1765E-01=F(+0.05882,      +1) */
{62, 0, 0,__LINE__, {{0x3fce1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40000000, 0x00000000}}},	/* +2.3529E-01=F(+0.05882,      +2) */
{62, 0, 0,__LINE__, {{0x3fde1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40080000, 0x00000000}}},	/* +4.7059E-01=F(+0.05882,      +3) */
{62, 0, 0,__LINE__, {{0x3fee1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40100000, 0x00000000}}},	/* +9.4118E-01=F(+0.05882,      +4) */
{62, 0, 0,__LINE__, {{0x3ffe1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40140000, 0x00000000}}},	/* +1.8824E+00=F(+0.05882,      +5) */
{62, 0, 0,__LINE__, {{0x400e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40180000, 0x00000000}}},	/* +3.7647E+00=F(+0.05882,      +6) */
{62, 0, 0,__LINE__, {{0x401e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x401c0000, 0x00000000}}},	/* +7.5294E+00=F(+0.05882,      +7) */
{62, 0, 0,__LINE__, {{0x402e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40200000, 0x00000000}}},	/* +1.5059E+01=F(+0.05882,      +8) */
{62, 0, 0,__LINE__, {{0x403e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40220000, 0x00000000}}},	/* +3.0118E+01=F(+0.05882,      +9) */
{62, 0, 0,__LINE__, {{0x404e1e1e, 0x1e1e1e1e}, {0x3fae1e1e, 0x1e1e1e1e}, {0x40240000, 0x00000000}}},	/* +6.0235E+01=F(+0.05882,     +10) */
{62, 0, 0,__LINE__, {{0x3f0af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0240000, 0x00000000}}},	/* +5.1398E-05=F(+0.05263,     -10) */
{62, 0, 0,__LINE__, {{0x3f1af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0220000, 0x00000000}}},	/* +1.0280E-04=F(+0.05263,      -9) */
{62, 0, 0,__LINE__, {{0x3f2af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0200000, 0x00000000}}},	/* +2.0559E-04=F(+0.05263,      -8) */
{62, 0, 0,__LINE__, {{0x3f3af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc01c0000, 0x00000000}}},	/* +4.1118E-04=F(+0.05263,      -7) */
{62, 0, 0,__LINE__, {{0x3f4af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0180000, 0x00000000}}},	/* +8.2237E-04=F(+0.05263,      -6) */
{62, 0, 0,__LINE__, {{0x3f5af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0140000, 0x00000000}}},	/* +1.6447E-03=F(+0.05263,      -5) */
{62, 0, 0,__LINE__, {{0x3f6af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0100000, 0x00000000}}},	/* +3.2895E-03=F(+0.05263,      -4) */
{62, 0, 0,__LINE__, {{0x3f7af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0080000, 0x00000000}}},	/* +6.5789E-03=F(+0.05263,      -3) */
{62, 0, 0,__LINE__, {{0x3f8af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xc0000000, 0x00000000}}},	/* +1.3158E-02=F(+0.05263,      -2) */
{62, 0, 0,__LINE__, {{0x3f9af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0xbff00000, 0x00000000}}},	/* +2.6316E-02=F(+0.05263,      -1) */
{62, 0, 0,__LINE__, {{0x3faaf286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x00000000, 0x00000000}}},	/* +5.2632E-02=F(+0.05263,      +0) */
{62, 0, 0,__LINE__, {{0x3fbaf286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x3ff00000, 0x00000000}}},	/* +1.0526E-01=F(+0.05263,      +1) */
{62, 0, 0,__LINE__, {{0x3fcaf286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40000000, 0x00000000}}},	/* +2.1053E-01=F(+0.05263,      +2) */
{62, 0, 0,__LINE__, {{0x3fdaf286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40080000, 0x00000000}}},	/* +4.2105E-01=F(+0.05263,      +3) */
{62, 0, 0,__LINE__, {{0x3feaf286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40100000, 0x00000000}}},	/* +8.4211E-01=F(+0.05263,      +4) */
{62, 0, 0,__LINE__, {{0x3ffaf286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40140000, 0x00000000}}},	/* +1.6842E+00=F(+0.05263,      +5) */
{62, 0, 0,__LINE__, {{0x400af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40180000, 0x00000000}}},	/* +3.3684E+00=F(+0.05263,      +6) */
{62, 0, 0,__LINE__, {{0x401af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x401c0000, 0x00000000}}},	/* +6.7368E+00=F(+0.05263,      +7) */
{62, 0, 0,__LINE__, {{0x402af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40200000, 0x00000000}}},	/* +1.3474E+01=F(+0.05263,      +8) */
{62, 0, 0,__LINE__, {{0x403af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40220000, 0x00000000}}},	/* +2.6947E+01=F(+0.05263,      +9) */
{62, 0, 0,__LINE__, {{0x404af286, 0xbca1af28}, {0x3faaf286, 0xbca1af28}, {0x40240000, 0x00000000}}},	/* +5.3895E+01=F(+0.05263,     +10) */
{62, 0, 0,__LINE__, {{0x3f086186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0240000, 0x00000000}}},	/* +4.6503E-05=F(+0.04762,     -10) */
{62, 0, 0,__LINE__, {{0x3f186186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0220000, 0x00000000}}},	/* +9.3006E-05=F(+0.04762,      -9) */
{62, 0, 0,__LINE__, {{0x3f286186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0200000, 0x00000000}}},	/* +1.8601E-04=F(+0.04762,      -8) */
{62, 0, 0,__LINE__, {{0x3f386186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc01c0000, 0x00000000}}},	/* +3.7202E-04=F(+0.04762,      -7) */
{62, 0, 0,__LINE__, {{0x3f486186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0180000, 0x00000000}}},	/* +7.4405E-04=F(+0.04762,      -6) */
{62, 0, 0,__LINE__, {{0x3f586186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0140000, 0x00000000}}},	/* +1.4881E-03=F(+0.04762,      -5) */
{62, 0, 0,__LINE__, {{0x3f686186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0100000, 0x00000000}}},	/* +2.9762E-03=F(+0.04762,      -4) */
{62, 0, 0,__LINE__, {{0x3f786186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0080000, 0x00000000}}},	/* +5.9524E-03=F(+0.04762,      -3) */
{62, 0, 0,__LINE__, {{0x3f886186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xc0000000, 0x00000000}}},	/* +1.1905E-02=F(+0.04762,      -2) */
{62, 0, 0,__LINE__, {{0x3f986186, 0x18618618}, {0x3fa86186, 0x18618618}, {0xbff00000, 0x00000000}}},	/* +2.3810E-02=F(+0.04762,      -1) */
{62, 0, 0,__LINE__, {{0x3fa86186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x00000000, 0x00000000}}},	/* +4.7619E-02=F(+0.04762,      +0) */
{62, 0, 0,__LINE__, {{0x3fb86186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x3ff00000, 0x00000000}}},	/* +9.5238E-02=F(+0.04762,      +1) */
{62, 0, 0,__LINE__, {{0x3fc86186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40000000, 0x00000000}}},	/* +1.9048E-01=F(+0.04762,      +2) */
{62, 0, 0,__LINE__, {{0x3fd86186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40080000, 0x00000000}}},	/* +3.8095E-01=F(+0.04762,      +3) */
{62, 0, 0,__LINE__, {{0x3fe86186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40100000, 0x00000000}}},	/* +7.6190E-01=F(+0.04762,      +4) */
{62, 0, 0,__LINE__, {{0x3ff86186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40140000, 0x00000000}}},	/* +1.5238E+00=F(+0.04762,      +5) */
{62, 0, 0,__LINE__, {{0x40086186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40180000, 0x00000000}}},	/* +3.0476E+00=F(+0.04762,      +6) */
{62, 0, 0,__LINE__, {{0x40186186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x401c0000, 0x00000000}}},	/* +6.0952E+00=F(+0.04762,      +7) */
{62, 0, 0,__LINE__, {{0x40286186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40200000, 0x00000000}}},	/* +1.2190E+01=F(+0.04762,      +8) */
{62, 0, 0,__LINE__, {{0x40386186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40220000, 0x00000000}}},	/* +2.4381E+01=F(+0.04762,      +9) */
{62, 0, 0,__LINE__, {{0x40486186, 0x18618618}, {0x3fa86186, 0x18618618}, {0x40240000, 0x00000000}}},	/* +4.8762E+01=F(+0.04762,     +10) */
{62, 0, 0,__LINE__, {{0x3f0642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0240000, 0x00000000}}},	/* +4.2459E-05=F(+0.04348,     -10) */
{62, 0, 0,__LINE__, {{0x3f1642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0220000, 0x00000000}}},	/* +8.4918E-05=F(+0.04348,      -9) */
{62, 0, 0,__LINE__, {{0x3f2642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0200000, 0x00000000}}},	/* +1.6984E-04=F(+0.04348,      -8) */
{62, 0, 0,__LINE__, {{0x3f3642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc01c0000, 0x00000000}}},	/* +3.3967E-04=F(+0.04348,      -7) */
{62, 0, 0,__LINE__, {{0x3f4642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0180000, 0x00000000}}},	/* +6.7935E-04=F(+0.04348,      -6) */
{62, 0, 0,__LINE__, {{0x3f5642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0140000, 0x00000000}}},	/* +1.3587E-03=F(+0.04348,      -5) */
{62, 0, 0,__LINE__, {{0x3f6642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0100000, 0x00000000}}},	/* +2.7174E-03=F(+0.04348,      -4) */
{62, 0, 0,__LINE__, {{0x3f7642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0080000, 0x00000000}}},	/* +5.4348E-03=F(+0.04348,      -3) */
{62, 0, 0,__LINE__, {{0x3f8642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xc0000000, 0x00000000}}},	/* +1.0870E-02=F(+0.04348,      -2) */
{62, 0, 0,__LINE__, {{0x3f9642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0xbff00000, 0x00000000}}},	/* +2.1739E-02=F(+0.04348,      -1) */
{62, 0, 0,__LINE__, {{0x3fa642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x00000000, 0x00000000}}},	/* +4.3478E-02=F(+0.04348,      +0) */
{62, 0, 0,__LINE__, {{0x3fb642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x3ff00000, 0x00000000}}},	/* +8.6957E-02=F(+0.04348,      +1) */
{62, 0, 0,__LINE__, {{0x3fc642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40000000, 0x00000000}}},	/* +1.7391E-01=F(+0.04348,      +2) */
{62, 0, 0,__LINE__, {{0x3fd642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40080000, 0x00000000}}},	/* +3.4783E-01=F(+0.04348,      +3) */
{62, 0, 0,__LINE__, {{0x3fe642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40100000, 0x00000000}}},	/* +6.9565E-01=F(+0.04348,      +4) */
{62, 0, 0,__LINE__, {{0x3ff642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40140000, 0x00000000}}},	/* +1.3913E+00=F(+0.04348,      +5) */
{62, 0, 0,__LINE__, {{0x400642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40180000, 0x00000000}}},	/* +2.7826E+00=F(+0.04348,      +6) */
{62, 0, 0,__LINE__, {{0x401642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x401c0000, 0x00000000}}},	/* +5.5652E+00=F(+0.04348,      +7) */
{62, 0, 0,__LINE__, {{0x402642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40200000, 0x00000000}}},	/* +1.1130E+01=F(+0.04348,      +8) */
{62, 0, 0,__LINE__, {{0x403642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40220000, 0x00000000}}},	/* +2.2261E+01=F(+0.04348,      +9) */
{62, 0, 0,__LINE__, {{0x404642c8, 0x590b2164}, {0x3fa642c8, 0x590b2164}, {0x40240000, 0x00000000}}},	/* +4.4522E+01=F(+0.04348,     +10) */
{62, 0, 0,__LINE__, {{0x80000000, 0x00000000}, {0x80000000, 0x00000000}, {0x40900000, 0x00000000}}},	/* -0.0000E+00=F(      -0,   +1024) */
{62, 2, 0,__LINE__, {{0x80000000, 0x00000000}, {0xbff00000, 0x00000000}, {0xc09ffc00, 0x00000000}}},	/* -0.0000E+00=F(      -1,   -2047) */
{62, 2, 0,__LINE__, {{0x00000000, 0x00000000}, {0x3ff00000, 0x00000000}, {0xc09ffc00, 0x00000000}}},	/* +0.0000E+00=F(      +1,   -2047) */
{62, 2, 0,__LINE__, {{0xfff00000, 0x00000000}, {0xbff00000, 0x00000000}, {0x409ffc00, 0x00000000}}},	/* -Inf       =F(      -1,   +2047) */
{62, 2, 0,__LINE__, {{0x7ff00000, 0x00000000}, {0x3ff00000, 0x00000000}, {0x409ffc00, 0x00000000}}},	/* +Inf       =F(      +1,   +2047) */
{62, 0, 0,__LINE__, {{0x7ff00000, 0x00000000}, {0x7ff00000, 0x00000000}, {0x409ffc00, 0x00000000}}},	/* +Inf       =F(    +Inf,   +2047) */
{62, 0, 0,__LINE__, {{0xfff00000, 0x00000000}, {0xfff00000, 0x00000000}, {0xc09ffc00, 0x00000000}}},	/* -Inf       =F(    -Inf,   -2047) */
{62, 0, 0,__LINE__, {{0x7ff00000, 0x00000000}, {0x7ff00000, 0x00000000}, {0x00000000, 0x00000000}}},	/* +Inf       =F(    +Inf,      +0) */
{62, 0, 0,__LINE__, {{0xfff00000, 0x00000000}, {0xfff00000, 0x00000000}, {0x00000000, 0x00000000}}},	/* -Inf       =F(    -Inf,      +0) */
{62, 0, 0,__LINE__, {{0x7ff80000, 0x00000000}, {0x7ff80000, 0x00000000}, {0x409ffc00, 0x00000000}}},	/* +NaN       =F(    +NaN,   +2047) */
{62, 0, 0,__LINE__, {{0xfff80000, 0x00000000}, {0xfff80000, 0x00000000}, {0xc09ffc00, 0x00000000}}},	/* -NaN       =F(    -NaN,   -2047) */
{0}};
void
test_ldexp(int m)	{ run_vector_1(m, ldexp_vec,(char *)(ldexp),"ldexp","ddi");}
