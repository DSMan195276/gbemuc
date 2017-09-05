
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <signal.h>

#include <jit/jit.h>

#include "debug.h"
#include "gb_internal.h"
#include "gb/cpu.h"
#include "cpu_internal.h"
#include "cpu_jit_helpers.h"

/* From Mednafen (GPLv2) - table of DAA result values */
static const uint16_t gb_daa_table[] = {
    0x0080,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,
    0x0800,0x0900,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,
    0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,
    0x1800,0x1900,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,
    0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,
    0x2800,0x2900,0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,
    0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,
    0x3800,0x3900,0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,
    0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,
    0x4800,0x4900,0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,
    0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,
    0x5800,0x5900,0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,
    0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,
    0x6800,0x6900,0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,
    0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,
    0x7800,0x7900,0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,
    0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,
    0x8800,0x8900,0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,
    0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,0x9600,0x9700,
    0x9800,0x9900,0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,
    0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
    0x0810,0x0910,0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,
    0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
    0x1810,0x1910,0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,
    0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
    0x2810,0x2910,0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,
    0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
    0x3810,0x3910,0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,
    0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
    0x4810,0x4910,0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,
    0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
    0x5810,0x5910,0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,
    0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,0x6610,0x6710,
    0x6810,0x6910,0x7010,0x7110,0x7210,0x7310,0x7410,0x7510,
    0x7010,0x7110,0x7210,0x7310,0x7410,0x7510,0x7610,0x7710,
    0x7810,0x7910,0x8010,0x8110,0x8210,0x8310,0x8410,0x8510,
    0x8010,0x8110,0x8210,0x8310,0x8410,0x8510,0x8610,0x8710,
    0x8810,0x8910,0x9010,0x9110,0x9210,0x9310,0x9410,0x9510,
    0x9010,0x9110,0x9210,0x9310,0x9410,0x9510,0x9610,0x9710,
    0x9810,0x9910,0xA010,0xA110,0xA210,0xA310,0xA410,0xA510,
    0xA010,0xA110,0xA210,0xA310,0xA410,0xA510,0xA610,0xA710,
    0xA810,0xA910,0xB010,0xB110,0xB210,0xB310,0xB410,0xB510,
    0xB010,0xB110,0xB210,0xB310,0xB410,0xB510,0xB610,0xB710,
    0xB810,0xB910,0xC010,0xC110,0xC210,0xC310,0xC410,0xC510,
    0xC010,0xC110,0xC210,0xC310,0xC410,0xC510,0xC610,0xC710,
    0xC810,0xC910,0xD010,0xD110,0xD210,0xD310,0xD410,0xD510,
    0xD010,0xD110,0xD210,0xD310,0xD410,0xD510,0xD610,0xD710,
    0xD810,0xD910,0xE010,0xE110,0xE210,0xE310,0xE410,0xE510,
    0xE010,0xE110,0xE210,0xE310,0xE410,0xE510,0xE610,0xE710,
    0xE810,0xE910,0xF010,0xF110,0xF210,0xF310,0xF410,0xF510,
    0xF010,0xF110,0xF210,0xF310,0xF410,0xF510,0xF610,0xF710,
    0xF810,0xF910,0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,
    0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
    0x0810,0x0910,0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,
    0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
    0x1810,0x1910,0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,
    0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
    0x2810,0x2910,0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,
    0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
    0x3810,0x3910,0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,
    0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
    0x4810,0x4910,0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,
    0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
    0x5810,0x5910,0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,
    0x0600,0x0700,0x0800,0x0900,0x0A00,0x0B00,0x0C00,0x0D00,
    0x0E00,0x0F00,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,
    0x1600,0x1700,0x1800,0x1900,0x1A00,0x1B00,0x1C00,0x1D00,
    0x1E00,0x1F00,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,
    0x2600,0x2700,0x2800,0x2900,0x2A00,0x2B00,0x2C00,0x2D00,
    0x2E00,0x2F00,0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,
    0x3600,0x3700,0x3800,0x3900,0x3A00,0x3B00,0x3C00,0x3D00,
    0x3E00,0x3F00,0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,
    0x4600,0x4700,0x4800,0x4900,0x4A00,0x4B00,0x4C00,0x4D00,
    0x4E00,0x4F00,0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,
    0x5600,0x5700,0x5800,0x5900,0x5A00,0x5B00,0x5C00,0x5D00,
    0x5E00,0x5F00,0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,
    0x6600,0x6700,0x6800,0x6900,0x6A00,0x6B00,0x6C00,0x6D00,
    0x6E00,0x6F00,0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,
    0x7600,0x7700,0x7800,0x7900,0x7A00,0x7B00,0x7C00,0x7D00,
    0x7E00,0x7F00,0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,
    0x8600,0x8700,0x8800,0x8900,0x8A00,0x8B00,0x8C00,0x8D00,
    0x8E00,0x8F00,0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,
    0x9600,0x9700,0x9800,0x9900,0x9A00,0x9B00,0x9C00,0x9D00,
    0x9E00,0x9F00,0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,
    0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
    0x0E10,0x0F10,0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,
    0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
    0x1E10,0x1F10,0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,
    0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
    0x2E10,0x2F10,0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,
    0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
    0x3E10,0x3F10,0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,
    0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
    0x4E10,0x4F10,0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,
    0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
    0x5E10,0x5F10,0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,
    0x6610,0x6710,0x6810,0x6910,0x6A10,0x6B10,0x6C10,0x6D10,
    0x6E10,0x6F10,0x7010,0x7110,0x7210,0x7310,0x7410,0x7510,
    0x7610,0x7710,0x7810,0x7910,0x7A10,0x7B10,0x7C10,0x7D10,
    0x7E10,0x7F10,0x8010,0x8110,0x8210,0x8310,0x8410,0x8510,
    0x8610,0x8710,0x8810,0x8910,0x8A10,0x8B10,0x8C10,0x8D10,
    0x8E10,0x8F10,0x9010,0x9110,0x9210,0x9310,0x9410,0x9510,
    0x9610,0x9710,0x9810,0x9910,0x9A10,0x9B10,0x9C10,0x9D10,
    0x9E10,0x9F10,0xA010,0xA110,0xA210,0xA310,0xA410,0xA510,
    0xA610,0xA710,0xA810,0xA910,0xAA10,0xAB10,0xAC10,0xAD10,
    0xAE10,0xAF10,0xB010,0xB110,0xB210,0xB310,0xB410,0xB510,
    0xB610,0xB710,0xB810,0xB910,0xBA10,0xBB10,0xBC10,0xBD10,
    0xBE10,0xBF10,0xC010,0xC110,0xC210,0xC310,0xC410,0xC510,
    0xC610,0xC710,0xC810,0xC910,0xCA10,0xCB10,0xCC10,0xCD10,
    0xCE10,0xCF10,0xD010,0xD110,0xD210,0xD310,0xD410,0xD510,
    0xD610,0xD710,0xD810,0xD910,0xDA10,0xDB10,0xDC10,0xDD10,
    0xDE10,0xDF10,0xE010,0xE110,0xE210,0xE310,0xE410,0xE510,
    0xE610,0xE710,0xE810,0xE910,0xEA10,0xEB10,0xEC10,0xED10,
    0xEE10,0xEF10,0xF010,0xF110,0xF210,0xF310,0xF410,0xF510,
    0xF610,0xF710,0xF810,0xF910,0xFA10,0xFB10,0xFC10,0xFD10,
    0xFE10,0xFF10,0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,
    0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
    0x0E10,0x0F10,0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,
    0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
    0x1E10,0x1F10,0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,
    0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
    0x2E10,0x2F10,0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,
    0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
    0x3E10,0x3F10,0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,
    0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
    0x4E10,0x4F10,0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,
    0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
    0x5E10,0x5F10,0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,
    0x00C0,0x0140,0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,
    0x0840,0x0940,0x0A40,0x0B40,0x0C40,0x0D40,0x0E40,0x0F40,
    0x1040,0x1140,0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,
    0x1840,0x1940,0x1A40,0x1B40,0x1C40,0x1D40,0x1E40,0x1F40,
    0x2040,0x2140,0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,
    0x2840,0x2940,0x2A40,0x2B40,0x2C40,0x2D40,0x2E40,0x2F40,
    0x3040,0x3140,0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,
    0x3840,0x3940,0x3A40,0x3B40,0x3C40,0x3D40,0x3E40,0x3F40,
    0x4040,0x4140,0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,
    0x4840,0x4940,0x4A40,0x4B40,0x4C40,0x4D40,0x4E40,0x4F40,
    0x5040,0x5140,0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,
    0x5840,0x5940,0x5A40,0x5B40,0x5C40,0x5D40,0x5E40,0x5F40,
    0x6040,0x6140,0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,
    0x6840,0x6940,0x6A40,0x6B40,0x6C40,0x6D40,0x6E40,0x6F40,
    0x7040,0x7140,0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,
    0x7840,0x7940,0x7A40,0x7B40,0x7C40,0x7D40,0x7E40,0x7F40,
    0x8040,0x8140,0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,
    0x8840,0x8940,0x8A40,0x8B40,0x8C40,0x8D40,0x8E40,0x8F40,
    0x9040,0x9140,0x9240,0x9340,0x9440,0x9540,0x9640,0x9740,
    0x9840,0x9940,0x9A40,0x9B40,0x9C40,0x9D40,0x9E40,0x9F40,
    0xA040,0xA140,0xA240,0xA340,0xA440,0xA540,0xA640,0xA740,
    0xA840,0xA940,0xAA40,0xAB40,0xAC40,0xAD40,0xAE40,0xAF40,
    0xB040,0xB140,0xB240,0xB340,0xB440,0xB540,0xB640,0xB740,
    0xB840,0xB940,0xBA40,0xBB40,0xBC40,0xBD40,0xBE40,0xBF40,
    0xC040,0xC140,0xC240,0xC340,0xC440,0xC540,0xC640,0xC740,
    0xC840,0xC940,0xCA40,0xCB40,0xCC40,0xCD40,0xCE40,0xCF40,
    0xD040,0xD140,0xD240,0xD340,0xD440,0xD540,0xD640,0xD740,
    0xD840,0xD940,0xDA40,0xDB40,0xDC40,0xDD40,0xDE40,0xDF40,
    0xE040,0xE140,0xE240,0xE340,0xE440,0xE540,0xE640,0xE740,
    0xE840,0xE940,0xEA40,0xEB40,0xEC40,0xED40,0xEE40,0xEF40,
    0xF040,0xF140,0xF240,0xF340,0xF440,0xF540,0xF640,0xF740,
    0xF840,0xF940,0xFA40,0xFB40,0xFC40,0xFD40,0xFE40,0xFF40,
    0xA050,0xA150,0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,
    0xA850,0xA950,0xAA50,0xAB50,0xAC50,0xAD50,0xAE50,0xAF50,
    0xB050,0xB150,0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,
    0xB850,0xB950,0xBA50,0xBB50,0xBC50,0xBD50,0xBE50,0xBF50,
    0xC050,0xC150,0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,
    0xC850,0xC950,0xCA50,0xCB50,0xCC50,0xCD50,0xCE50,0xCF50,
    0xD050,0xD150,0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,
    0xD850,0xD950,0xDA50,0xDB50,0xDC50,0xDD50,0xDE50,0xDF50,
    0xE050,0xE150,0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,
    0xE850,0xE950,0xEA50,0xEB50,0xEC50,0xED50,0xEE50,0xEF50,
    0xF050,0xF150,0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,
    0xF850,0xF950,0xFA50,0xFB50,0xFC50,0xFD50,0xFE50,0xFF50,
    0x00D0,0x0150,0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,
    0x0850,0x0950,0x0A50,0x0B50,0x0C50,0x0D50,0x0E50,0x0F50,
    0x1050,0x1150,0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,
    0x1850,0x1950,0x1A50,0x1B50,0x1C50,0x1D50,0x1E50,0x1F50,
    0x2050,0x2150,0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,
    0x2850,0x2950,0x2A50,0x2B50,0x2C50,0x2D50,0x2E50,0x2F50,
    0x3050,0x3150,0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,
    0x3850,0x3950,0x3A50,0x3B50,0x3C50,0x3D50,0x3E50,0x3F50,
    0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
    0x4850,0x4950,0x4A50,0x4B50,0x4C50,0x4D50,0x4E50,0x4F50,
    0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
    0x5850,0x5950,0x5A50,0x5B50,0x5C50,0x5D50,0x5E50,0x5F50,
    0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
    0x6850,0x6950,0x6A50,0x6B50,0x6C50,0x6D50,0x6E50,0x6F50,
    0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
    0x7850,0x7950,0x7A50,0x7B50,0x7C50,0x7D50,0x7E50,0x7F50,
    0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
    0x8850,0x8950,0x8A50,0x8B50,0x8C50,0x8D50,0x8E50,0x8F50,
    0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
    0x9850,0x9950,0x9A50,0x9B50,0x9C50,0x9D50,0x9E50,0x9F50,
    0xFA40,0xFB40,0xFC40,0xFD40,0xFE40,0xFF40,0x00C0,0x0140,
    0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
    0x0A40,0x0B40,0x0C40,0x0D40,0x0E40,0x0F40,0x1040,0x1140,
    0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
    0x1A40,0x1B40,0x1C40,0x1D40,0x1E40,0x1F40,0x2040,0x2140,
    0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
    0x2A40,0x2B40,0x2C40,0x2D40,0x2E40,0x2F40,0x3040,0x3140,
    0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
    0x3A40,0x3B40,0x3C40,0x3D40,0x3E40,0x3F40,0x4040,0x4140,
    0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
    0x4A40,0x4B40,0x4C40,0x4D40,0x4E40,0x4F40,0x5040,0x5140,
    0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
    0x5A40,0x5B40,0x5C40,0x5D40,0x5E40,0x5F40,0x6040,0x6140,
    0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
    0x6A40,0x6B40,0x6C40,0x6D40,0x6E40,0x6F40,0x7040,0x7140,
    0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
    0x7A40,0x7B40,0x7C40,0x7D40,0x7E40,0x7F40,0x8040,0x8140,
    0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
    0x8A40,0x8B40,0x8C40,0x8D40,0x8E40,0x8F40,0x9040,0x9140,
    0x9240,0x9340,0x9440,0x9540,0x9640,0x9740,0x9840,0x9940,
    0x9A40,0x9B40,0x9C40,0x9D40,0x9E40,0x9F40,0xA040,0xA140,
    0xA240,0xA340,0xA440,0xA540,0xA640,0xA740,0xA840,0xA940,
    0xAA40,0xAB40,0xAC40,0xAD40,0xAE40,0xAF40,0xB040,0xB140,
    0xB240,0xB340,0xB440,0xB540,0xB640,0xB740,0xB840,0xB940,
    0xBA40,0xBB40,0xBC40,0xBD40,0xBE40,0xBF40,0xC040,0xC140,
    0xC240,0xC340,0xC440,0xC540,0xC640,0xC740,0xC840,0xC940,
    0xCA40,0xCB40,0xCC40,0xCD40,0xCE40,0xCF40,0xD040,0xD140,
    0xD240,0xD340,0xD440,0xD540,0xD640,0xD740,0xD840,0xD940,
    0xDA40,0xDB40,0xDC40,0xDD40,0xDE40,0xDF40,0xE040,0xE140,
    0xE240,0xE340,0xE440,0xE540,0xE640,0xE740,0xE840,0xE940,
    0xEA40,0xEB40,0xEC40,0xED40,0xEE40,0xEF40,0xF040,0xF140,
    0xF240,0xF340,0xF440,0xF540,0xF640,0xF740,0xF840,0xF940,
    0x9A50,0x9B50,0x9C50,0x9D50,0x9E50,0x9F50,0xA050,0xA150,
    0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
    0xAA50,0xAB50,0xAC50,0xAD50,0xAE50,0xAF50,0xB050,0xB150,
    0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
    0xBA50,0xBB50,0xBC50,0xBD50,0xBE50,0xBF50,0xC050,0xC150,
    0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
    0xCA50,0xCB50,0xCC50,0xCD50,0xCE50,0xCF50,0xD050,0xD150,
    0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
    0xDA50,0xDB50,0xDC50,0xDD50,0xDE50,0xDF50,0xE050,0xE150,
    0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
    0xEA50,0xEB50,0xEC50,0xED50,0xEE50,0xEF50,0xF050,0xF150,
    0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
    0xFA50,0xFB50,0xFC50,0xFD50,0xFE50,0xFF50,0x00D0,0x0150,
    0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
    0x0A50,0x0B50,0x0C50,0x0D50,0x0E50,0x0F50,0x1050,0x1150,
    0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
    0x1A50,0x1B50,0x1C50,0x1D50,0x1E50,0x1F50,0x2050,0x2150,
    0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
    0x2A50,0x2B50,0x2C50,0x2D50,0x2E50,0x2F50,0x3050,0x3150,
    0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
    0x3A50,0x3B50,0x3C50,0x3D50,0x3E50,0x3F50,0x4050,0x4150,
    0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
    0x4A50,0x4B50,0x4C50,0x4D50,0x4E50,0x4F50,0x5050,0x5150,
    0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
    0x5A50,0x5B50,0x5C50,0x5D50,0x5E50,0x5F50,0x6050,0x6150,
    0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
    0x6A50,0x6B50,0x6C50,0x6D50,0x6E50,0x6F50,0x7050,0x7150,
    0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
    0x7A50,0x7B50,0x7C50,0x7D50,0x7E50,0x7F50,0x8050,0x8150,
    0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
    0x8A50,0x8B50,0x8C50,0x8D50,0x8E50,0x8F50,0x9050,0x9150,
    0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
};

/* A map used by instructions to convert a numeral in an opcode to actual
 * register numbers */
static const uint8_t reg_map_8bit[8] = {
    GB_REG_B,
    GB_REG_C,
    GB_REG_D,
    GB_REG_E,
    GB_REG_H,
    GB_REG_L,
    0, /* (HL) */
    GB_REG_A,
};

/* Returns whether or not the provided value maps to (HL) */
#define IS_HL(x) ((x) == b8(110))

/* Handles the special case of (HL).
 * Returns the value that should be used, and modifies cycles if necessary. */
static inline jit_value_t read_8bit_reg(struct gb_cpu_jit_context *ctx, int reg)
{
    if (IS_HL(reg)) {
        gb_jit_clock_tick(ctx);
        jit_value_t hl = gb_jit_load_reg16(ctx, GB_REG_HL);
        return gb_jit_read8(ctx, hl);
    } else {
        return gb_jit_load_reg8(ctx, reg_map_8bit[reg]);
    }
}

static void write_8bit_reg(struct gb_cpu_jit_context *ctx, int reg, jit_value_t val)
{
    if (IS_HL(reg)) {
        gb_jit_clock_tick(ctx);
        jit_value_t hl = gb_jit_load_reg16(ctx, GB_REG_HL);
        gb_jit_write8(ctx, hl, val);
    } else {
        gb_jit_store_reg8(ctx, reg_map_8bit[reg], val);
    }
}

/* A map used by instructions to convert numerals in an opcode into a register
 * number.
 *
 * Some instructions make-use of SP instead of AF, so there are two tables. */
static const uint8_t reg_map_16bit_sp[4] = {
    GB_REG_BC,
    GB_REG_DE,
    GB_REG_HL,
    GB_REG_SP,
};

static const uint8_t reg_map_16bit_af[4] = {
    GB_REG_BC,
    GB_REG_DE,
    GB_REG_HL,
    GB_REG_AF,
};

/*
 *
 * 8-bit loads
 *
 */

/* LD reg, imm */
static void load8_reg_imm(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* Pull Register number out of bits 4 through 6. */
    uint8_t reg = ((opcode & b8(00111000)) >> 3);

    /* Reading PC takes a clock-tick */
    gb_jit_clock_tick(ctx);
    jit_value_t imm = gb_jit_next_pc8(ctx);
    ctx->addr++;

    write_8bit_reg(ctx, reg, imm);

    return ;
}

/* LD reg, reg. */
static void load8_reg_reg(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* Dest is bits 4 through 6
     * Src is bits 1 through 3 */
    uint8_t dest = ((opcode & b8(00111000)) >> 3);
    uint8_t src = (opcode & b8(00000111));
    jit_value_t val;

    val = read_8bit_reg(ctx, src);
    write_8bit_reg(ctx, dest, val);

    return ;
}

/* LD A, (n)
 * LD A, #
 * LD A, (C)
 * LDD A, (HL)
 * LDI A, (HL)
 */
static void load8_a_extra(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg;
    jit_value_t val = NULL;
    jit_value_t src;

    switch (opcode) {
    case 0x0A: /* LD A, (BC) */
    case 0x1A: /* LD A, (DE) */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;

        val = gb_jit_read8(ctx, gb_jit_load_reg16(ctx, reg));
        break;

    case 0xFA: /* LD A, (nn) */
        /* Two clock ticks for reading a 16-bit from PC */
        gb_jit_clock_tick(ctx);
        gb_jit_clock_tick(ctx);

        src = gb_jit_next_pc16(ctx);
        val = gb_jit_read8(ctx, src);
        ctx->addr += 2;
        break;

    case 0x3E: /* LD A, # */
        val = gb_jit_next_pc8(ctx);
        ctx->addr++;
        break;

    case 0xF2: /* LD A, (C) */
        src = jit_insn_add(ctx->func, gb_jit_load_reg8(ctx, GB_REG_C),
                                 GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        val = gb_jit_read8(ctx, src);
        break;

    case 0xF0:
        /* 8-bit read from PC */
        gb_jit_clock_tick(ctx);
        src = jit_insn_add(ctx->func, gb_jit_next_pc8(ctx),
                                 GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        val = gb_jit_read8(ctx, src);
        ctx->addr++;
        break;

    case 0x3A: /* LDD A, (HL) */
    case 0x2A: /* LDI A, (HL) */
        src = gb_jit_load_reg16(ctx, GB_REG_HL);
        if (opcode == 0x3A) {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                jit_insn_sub(ctx->func, src, GB_JIT_CONST_USHORT(ctx->func, 1)));
        } else {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                jit_insn_add(ctx->func, src, GB_JIT_CONST_USHORT(ctx->func, 1)));
        }

        val = gb_jit_read8(ctx, src);
        break;
    }

    /* Accounts for 8-bit read in every path. */
    gb_jit_clock_tick(ctx);

    gb_jit_store_reg8(ctx, GB_REG_A, val);

    return ;
}

/* LD (n), A
 * LDD (HL), A
 * LDI (HL), A
 */
static void load8_extra_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg;
    jit_value_t dest = NULL;

    switch (opcode) {
    case 0x02: /* LD (BC), A */
    case 0x12: /* LD (DE), A */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;

        dest = gb_jit_load_reg16(ctx, reg);
        break;

    case 0xEA: /* LD (nn), A */
        gb_jit_clock_tick(ctx);
        gb_jit_clock_tick(ctx);
        dest = gb_jit_next_pc16(ctx);
        ctx->addr += 2;
        break;

    case 0xE2: /* LD (0xFF00 + C), A */
        dest = jit_insn_add(ctx->func, gb_jit_load_reg8(ctx, GB_REG_C),
                                  GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        break;

    case 0xE0: /* LD (0xFF00 + n), A */
        gb_jit_clock_tick(ctx);
        jit_value_t val = gb_jit_next_pc8(ctx);
        dest = jit_insn_add(ctx->func, val,
                                  GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        ctx->addr++;
        break;

    case 0x32: /* LDD (HL), A */
    case 0x22: /* LDI (HL), A */
        dest = gb_jit_load_reg16(ctx, GB_REG_HL);
        if (opcode == 0x32) {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                    jit_insn_sub(ctx->func, dest, GB_JIT_CONST_USHORT(ctx->func, 1)));
        } else {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                    jit_insn_add(ctx->func, dest, GB_JIT_CONST_USHORT(ctx->func, 1)));
        }
        break;
    }

    gb_jit_clock_tick(ctx);
    gb_jit_write8(ctx, dest, gb_jit_load_reg8(ctx, GB_REG_A));

    return ;
}

/*
 *
 * 16-bit Loads
 *
 */

/* LD n, nn */
static void load16_reg_imm(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int dest = reg_map_16bit_sp[(opcode & 0x30) >> 4];

    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    jit_value_t val = gb_jit_next_pc16(ctx);
    ctx->addr += 2;

    gb_jit_store_reg16(ctx, dest, val);

    return ;
}

/* LD SP, HL */
static void load_sp_hl(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* Extra clock-tick for 16-bit loads */
    gb_jit_clock_tick(ctx);

    gb_jit_store_reg16(ctx, GB_REG_SP, gb_jit_load_reg16(ctx, GB_REG_HL));

    return ;
}

/* LDHL SP, n */
static void load_hl_sp_n(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags;
    jit_value_t val;
    jit_label_t tmp_label;

    /* Read 8-bit from PC */
    gb_jit_clock_tick(ctx);
    jit_value_t off = gb_jit_next_pc8(ctx);
    ctx->addr++;

    /* Extra clock tick for 16-bit load */
    gb_jit_clock_tick(ctx);

    val = jit_insn_add(ctx->func, gb_jit_load_reg16(ctx, GB_REG_SP), off);

    flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    jit_value_t flag_check_value;

    jit_value_t tmp;
    tmp = gb_jit_load_reg16(ctx, GB_REG_SP);
    tmp = jit_insn_xor(ctx->func, tmp, off);
    flag_check_value = jit_insn_xor(ctx->func, tmp, val);

    tmp = jit_insn_and(ctx->func, flag_check_value, GB_JIT_CONST_USHORT(ctx->func, 0x100));
    tmp_label = jit_label_undefined;

    tmp = jit_insn_eq(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0));
    jit_insn_branch_if(ctx->func, tmp, &tmp_label);
    jit_insn_store(ctx->func, flags, jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY)));
    jit_insn_label(ctx->func, &tmp_label);

    tmp = jit_insn_and(ctx->func, flag_check_value, GB_JIT_CONST_UBYTE(ctx->func, 0x10));
    tmp_label = jit_label_undefined;

    tmp = jit_insn_eq(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0));
    jit_insn_branch_if(ctx->func, tmp, &tmp_label);
    jit_insn_store(ctx->func, flags, jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_HCARRY)));
    jit_insn_label(ctx->func, &tmp_label);

    gb_jit_store_reg16(ctx, GB_REG_HL, val);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* LD (nn), SP */
static void load_mem_sp(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* 16-bit read */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    jit_value_t dest = gb_jit_next_pc16(ctx);
    ctx->addr += 2;

    /* 16-bit write */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    gb_jit_write16(ctx, dest, gb_jit_load_reg16(ctx, GB_REG_SP));

    return ;
}

static void push_val(struct gb_cpu_jit_context *ctx, jit_value_t val)
{
    /* Extra clock-tick for 16-bit */
    gb_jit_clock_tick(ctx);
    jit_value_t tmp = gb_jit_load_reg16(ctx, GB_REG_SP);
    tmp = jit_insn_sub(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 2));
    gb_jit_store_reg16(ctx, GB_REG_SP, tmp);

    /* 16-bit write */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    gb_jit_write16(ctx, tmp, val);

    return ;
}

static jit_value_t pop_val(struct gb_cpu_jit_context *ctx)
{
    /* 16-bit read */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    jit_value_t dest = gb_jit_read16(ctx, gb_jit_load_reg16(ctx, GB_REG_SP));

    /* Extra clock-tick for 16-bit */
    gb_jit_clock_tick(ctx);
    jit_value_t tmp = gb_jit_load_reg16(ctx, GB_REG_SP);
    tmp = jit_insn_add(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 2));
    gb_jit_store_reg16(ctx, GB_REG_SP, tmp);

    return dest;
}

/* PUSH reg
 * reg = AF, BC, DE, HL */
static void push(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int dest = reg_map_16bit_af[(opcode & 0x30) >> 4];

    jit_value_t tmp = gb_jit_load_reg16(ctx, dest);

    push_val(ctx, tmp);

    return ;
}

/* POP reg
 * reg = AF, BC, DE, HL */
static void pop(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int dest = reg_map_16bit_af[(opcode & 0x30) >> 4];

    jit_value_t tmp = pop_val(ctx);
    gb_jit_store_reg16(ctx, dest, tmp);

    return ;
}

/*
 *
 * 8-bit ALU
 *
 */

/* Performs an 8-bit addition, with optional carry.
 *
 * Returns the result of the add */
static jit_value_t adc(struct gb_cpu_jit_context *ctx, jit_value_t val1, jit_value_t val2, jit_value_t carry)
{
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    jit_value_t val1_result = jit_insn_and(ctx->func, val1, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    jit_value_t val2_result = jit_insn_and(ctx->func, val2, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    jit_value_t add_result = jit_insn_add(ctx->func, val1_result, jit_insn_add(ctx->func, val2_result, carry));

    gb_jit_set_flag_if_gt(ctx->func, add_result, GB_JIT_CONST_UBYTE(ctx->func, 0x0F), flags, GB_FLAG_HCARRY);

    val1_result = jit_insn_convert(ctx->func, val1, jit_type_ushort, 0);
    val2_result = jit_insn_convert(ctx->func, val2, jit_type_ushort, 0);
    add_result = jit_insn_add(ctx->func, val1_result, jit_insn_add(ctx->func, val2_result, carry));

    gb_jit_set_flag_if_gt(ctx->func, add_result, GB_JIT_CONST_USHORT(ctx->func, 0xFF), flags, GB_FLAG_CARRY);

    jit_value_t result = jit_insn_and(ctx->func, jit_insn_add(ctx->func, val1_result, jit_insn_add(ctx->func, val2_result, carry)), GB_JIT_CONST_USHORT(ctx->func, 0xFF));

    gb_jit_set_flag_if_zero(ctx->func, result, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return result;
}

/* ADD A, reg
 * ADD A, #
 *
 * ADC A, reg
 * ADC A, #
 */
static void add_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t carry = GB_JIT_CONST_UBYTE(ctx->func, 0);
    jit_value_t tmp;

    /* 0xC0 means it's an immediate value */
    if ((opcode & 0xF0) != 0xC0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    /* ADC commands add the carry flag */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xCE) {
        carry = gb_jit_load_reg8(ctx, GB_REG_F);
        carry = jit_insn_and(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        /* Shifts carry from 0x10 to 0x01 */
        carry = jit_insn_shr(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 4));
    }

    gb_jit_clock_tick(ctx);
    tmp = adc(ctx, gb_jit_load_reg8(ctx, GB_REG_A), tmp, carry);
    gb_jit_store_reg8(ctx, GB_REG_A, tmp);

    return ;
}

static jit_value_t sbc(struct gb_cpu_jit_context *ctx, jit_value_t val1, jit_value_t val2, jit_value_t carry)
{
    jit_value_t result;
    jit_value_t tmp;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB));

    tmp = jit_insn_sub(ctx->func, val1, val2);
    tmp = jit_insn_sub(ctx->func, tmp, carry);
    result = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xFF));

    jit_value_t val1_16 = jit_insn_convert(ctx->func, val1, jit_type_ushort, 0);
    jit_value_t val2_16 = jit_insn_convert(ctx->func, val2, jit_type_ushort, 0);
    tmp = jit_insn_sub(ctx->func, val1_16, val2_16);
    tmp = jit_insn_sub(ctx->func, tmp, carry);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0xFF00));

    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_CARRY);

    tmp = jit_insn_xor(ctx->func, val1, val2);
    tmp = jit_insn_xor(ctx->func, tmp, result);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x10));

    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_zero(ctx->func, result, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return result;
}

/* SUB reg
 * SUB #
 */
static void sub_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t tmp, carry;

    /* 0xD0 means it's am immediate value */
    if ((opcode & 0xF0) != 0xD0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    /* SBC commands add the carry flag to the front of 'a'. */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xDE) {
        carry = gb_jit_load_reg8(ctx, GB_REG_F);
        carry = jit_insn_and(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        /* Shifts carry from 0x10 to 0x01 */
        carry = jit_insn_shr(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 4));
    } else {
        carry = GB_JIT_CONST_UBYTE(ctx->func, 0);
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    gb_jit_store_reg8(ctx, GB_REG_A, sbc(ctx, a, tmp, carry));

    return ;
}

/* AND reg
 * AND n */
static void and_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t tmp;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_HCARRY));

    /* Check if this is not the immediate value opcode 0xE6 */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_and(ctx->func, a, tmp);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    gb_jit_set_flag_if_zero(ctx->func, a, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* OR reg
 * OR n */
static void or_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* Check if this is not the immediate value opcode 0xF6 */
    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_or(ctx->func, a, tmp);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    gb_jit_set_flag_if_zero(ctx->func, a, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* XOR reg
 * XOR n */
static void xor_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* Check if this is not the immediate value opcode 0xEE */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_xor(ctx->func, a, tmp);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    gb_jit_set_flag_if_zero(ctx->func, a, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* CP reg
 * CP n */
static void cp_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);

    gb_jit_set_flag_if_eq(ctx->func, a, tmp, flags, GB_FLAG_ZERO);

    jit_value_t a_low = jit_insn_and(ctx->func, a, GB_JIT_CONST_UBYTE(ctx->func, 0xF));
    jit_value_t tmp_low = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xF));
    gb_jit_set_flag_if_lt(ctx->func, a_low, tmp_low, flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_lt(ctx->func, a, tmp, flags, GB_FLAG_CARRY);

    jit_insn_store(ctx->func, flags, jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB)));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* INC reg */
static void inc_reg(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x38) >> 3;
    jit_value_t flags, tmp;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));

    tmp = read_8bit_reg(ctx, src);

    jit_value_t tmp_low = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    gb_jit_set_flag_if_eq(ctx->func, tmp_low, GB_JIT_CONST_UBYTE(ctx->func, 0x0F), flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_eq(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xFF), flags, GB_FLAG_ZERO);

    jit_value_t tmp_16bit = jit_insn_convert(ctx->func, tmp, jit_type_ushort, 0);
    tmp_16bit = jit_insn_add(ctx->func, tmp_16bit, GB_JIT_CONST_USHORT(ctx->func, 1));
    tmp = jit_insn_convert(ctx->func, tmp_16bit, jit_type_ubyte, 0);

    write_8bit_reg(ctx, src, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* DEC reg */
static void dec_reg(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x38) >> 3;
    jit_value_t flags, tmp;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
    flags = jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB));

    tmp = read_8bit_reg(ctx, src);

    jit_value_t tmp_low = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    gb_jit_set_flag_if_zero(ctx->func, tmp_low, flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_eq(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1), flags, GB_FLAG_ZERO);

    jit_value_t tmp_16bit = jit_insn_convert(ctx->func, tmp, jit_type_ushort, 0);
    tmp_16bit = jit_insn_sub(ctx->func, tmp_16bit, GB_JIT_CONST_USHORT(ctx->func, 1));
    tmp = jit_insn_convert(ctx->func, tmp_16bit, jit_type_ubyte, 0);

    write_8bit_reg(ctx, src, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/*
 *
 * 16-bit ALU
 *
 */

/* ADD HL, reg */
static void add_hl_reg16(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];
    jit_value_t flags;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));

    jit_value_t hl = gb_jit_load_reg16(ctx, GB_REG_HL);
    jit_value_t reg16 = gb_jit_load_reg16(ctx, reg);
    jit_value_t hl_low = jit_insn_and(ctx->func, hl, GB_JIT_CONST_USHORT(ctx->func, 0x0FFF));
    jit_value_t reg16_low = jit_insn_and(ctx->func, reg16, GB_JIT_CONST_USHORT(ctx->func, 0x0FFF));
    jit_value_t tmp = jit_insn_add(ctx->func, hl_low, reg16_low);

    gb_jit_set_flag_if_gt(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0x0FFF), flags, GB_FLAG_HCARRY);

    jit_value_t hl_32bit = jit_insn_convert(ctx->func, hl, jit_type_uint, 0);
    jit_value_t reg_32bit = jit_insn_convert(ctx->func, reg16, jit_type_uint, 0);
    jit_value_t add_32bit = jit_insn_add(ctx->func, hl_32bit, reg_32bit);

    gb_jit_set_flag_if_gt(ctx->func, add_32bit, GB_JIT_CONST_UINT(ctx->func, 0xFFFF), flags, GB_FLAG_CARRY);

    gb_jit_clock_tick(ctx);
    jit_value_t add_16bit = jit_insn_convert(ctx->func, add_32bit, jit_type_ushort, 0);
    gb_jit_store_reg16(ctx, GB_REG_HL, add_16bit);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* ADD SP, # */
static void add_sp_imm(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    gb_jit_clock_tick(ctx);
    tmp = gb_jit_next_pc8(ctx);
    ctx->addr++;

    jit_value_t sp = gb_jit_load_reg16(ctx, GB_REG_SP);
    jit_value_t result = jit_insn_add(ctx->func, sp, tmp);

    /* From Mednafem - magic to do the carry and hcarry flags */
    jit_value_t xor_value = jit_insn_xor(ctx->func, sp, jit_insn_xor(ctx->func, tmp, result));

    gb_jit_set_flag_if_nonzero(ctx->func, jit_insn_and(ctx->func, xor_value, GB_JIT_CONST_USHORT(ctx->func, 0x100)), flags, GB_FLAG_CARRY);
    gb_jit_set_flag_if_nonzero(ctx->func, jit_insn_and(ctx->func, xor_value, GB_JIT_CONST_USHORT(ctx->func, 0x10)), flags, GB_FLAG_HCARRY);

    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    gb_jit_store_reg16(ctx, GB_REG_SP, result);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* INC nn */
static void inc_reg16(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];

    gb_jit_clock_tick(ctx);
    jit_value_t reg_val = gb_jit_load_reg16(ctx, reg);
    reg_val = jit_insn_add(ctx->func, reg_val, GB_JIT_CONST_USHORT(ctx->func, 1));
    gb_jit_store_reg16(ctx, reg, reg_val);

    return ;
}

/* DEC nn */
static void dec_reg16(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];

    gb_jit_clock_tick(ctx);
    jit_value_t reg_val = gb_jit_load_reg16(ctx, reg);
    reg_val = jit_insn_sub(ctx->func, reg_val, GB_JIT_CONST_USHORT(ctx->func, 1));
    gb_jit_store_reg16(ctx, reg, reg_val);

    return ;
}

/*
 *
 * Misc ops
 *
 */

/* SWAP reg */
static void swap(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x7);
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    tmp = read_8bit_reg(ctx, src);

    gb_jit_set_flag_if_zero(ctx->func, tmp, flags, GB_FLAG_ZERO);

    jit_value_t left_half = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xF0));
    jit_value_t right_half = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));

    left_half = jit_insn_shr(ctx->func, left_half, GB_JIT_CONST_UBYTE(ctx->func, 4));
    right_half = jit_insn_shl(ctx->func, right_half, GB_JIT_CONST_UBYTE(ctx->func, 4));

    tmp = jit_insn_or(ctx->func, left_half, right_half);

    write_8bit_reg(ctx, src, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* DAA */
static void daa(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t lookup;

    lookup = gb_jit_load_reg8(ctx, GB_REG_A);
    lookup = jit_insn_convert(ctx->func, lookup, jit_type_ushort, 0);

    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY | GB_FLAG_HCARRY | GB_FLAG_SUB));
    flags = jit_insn_convert(ctx->func, flags, jit_type_ushort, 0);
    flags = jit_insn_shl(ctx->func, flags, GB_JIT_CONST_USHORT(ctx->func, 4));
    lookup = jit_insn_or(ctx->func, lookup, flags);

    jit_value_t af = jit_insn_load_elem(ctx->func, GB_JIT_CONST_PTR(ctx->func, gb_daa_table), lookup, jit_type_ushort);
    gb_jit_store_reg16(ctx, GB_REG_AF, af);

    return ;
}

/* CPL */
static void cpl(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_not(ctx->func, a);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB | GB_FLAG_HCARRY));
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* CCF */
static void ccf(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);

    flags = jit_insn_xor(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, ~(GB_FLAG_SUB | GB_FLAG_HCARRY)));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* SCF */
static void scf(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);

    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));
    flags = jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* NOP */
static void nop(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_nop(ctx->func);
    return ;
}

/* HALT */
static void halt(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.halted), GB_JIT_CONST_UBYTE(ctx->func, 1));
    return ;
}

/* STOP */
static void stop(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    if (!gb_emu_is_cgb(ctx->gb_emu)) {
        jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.stopped), GB_JIT_CONST_UBYTE(ctx->func, 1));
        return ;
    }

    jit_label_t tmp_label = jit_label_undefined;
    jit_label_t end_label = jit_label_undefined;

    jit_value_t do_speed = jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.do_speed_switch), jit_type_ubyte);
    jit_value_t cmp = jit_insn_eq(ctx->func, do_speed, GB_JIT_CONST_UBYTE(ctx->func, 0));

    jit_insn_branch_if_not(ctx->func, cmp, &tmp_label);
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.stopped), GB_JIT_CONST_UBYTE(ctx->func, 1));
    jit_insn_branch(ctx->func, &end_label);

    jit_insn_label(ctx->func, &tmp_label);
    jit_value_t ds = jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.double_speed), jit_type_ubyte);
    ds = jit_insn_xor(ctx->func, ds, GB_JIT_CONST_UBYTE(ctx->func, 1));
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.double_speed), ds);

    jit_insn_label(ctx->func, &end_label);

    return ;
}

/* DI */
static void di(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.next_ime), GB_JIT_CONST_UBYTE(ctx->func, 0));
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.int_count), GB_JIT_CONST_UBYTE(ctx->func, 1));

    return ;
}

/* EI */
static void ei(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.next_ime), GB_JIT_CONST_UBYTE(ctx->func, 1));
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.int_count), GB_JIT_CONST_UBYTE(ctx->func, 2));

    return ;
}

/*
 *
 * Rotates and shifts
 *
 */

/* RLA
 * RLCA
 * RL reg
 * RLC reg */
static void rla(struct gb_cpu_jit_context *ctx, uint8_t opcode, int is_cb)
{
    int reg = opcode & 0x07;
    jit_value_t flags, carry, tmp;

    tmp = read_8bit_reg(ctx, reg);

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* RLA - use carry flag */
    if ((opcode & 0x10) == 0x10) {
        jit_value_t f = gb_jit_load_reg8(ctx, GB_REG_F);
        f = jit_insn_and(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        carry = jit_insn_shr(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SHIFT_CARRY));
    } else {
        carry = jit_insn_shr(ctx->func, jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x80)), GB_JIT_CONST_UBYTE(ctx->func, 7));
    }

    jit_value_t res = jit_insn_convert(ctx->func, tmp, jit_type_ushort, 0);
    res = jit_insn_shl(ctx->func, res, GB_JIT_CONST_USHORT(ctx->func, 1));
    res = jit_insn_or(ctx->func, res, carry);

    tmp = jit_insn_and(ctx->func, res, GB_JIT_CONST_USHORT(ctx->func, 0x100));
    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_CARRY);

    res = jit_insn_convert(ctx->func, res, jit_type_ubyte, 0);

    if (is_cb)
        gb_jit_set_flag_if_zero(ctx->func, res, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, res);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* RRA
 * RRCA
 * RR reg
 * RRC reg */
static void rra(struct gb_cpu_jit_context *ctx, uint8_t opcode, int is_cb)
{
    int reg = opcode & 0x07;
    jit_value_t flags, carry, tmp;

    tmp = read_8bit_reg(ctx, reg);

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* RRA uses carry */
    if ((opcode & 0x10) == 0x10) {
        jit_value_t f = gb_jit_load_reg8(ctx, GB_REG_F);
        f = jit_insn_and(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        carry = jit_insn_shr(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SHIFT_CARRY));
        carry = jit_insn_shl(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 7));
    } else {
        carry = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x01));
        carry = jit_insn_shl(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 7));
    }

    jit_value_t res = jit_insn_shr(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));
    res = jit_insn_or(ctx->func, res, carry);

    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x01));
    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_CARRY);

    if (is_cb)
        gb_jit_set_flag_if_zero(ctx->func, res, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, res);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* SLA reg */
static void sla(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    jit_value_t tmp, flags;

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    tmp = read_8bit_reg(ctx, reg);

    jit_value_t test = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x80));
    gb_jit_set_flag_if_nonzero(ctx->func, test, flags, GB_FLAG_CARRY);

    tmp = jit_insn_shl(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));

    gb_jit_set_flag_if_zero(ctx->func, tmp, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* SRA reg
 * SRL reg */
static void sra(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    jit_value_t tmp, flags;

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    tmp = read_8bit_reg(ctx, reg);

    jit_value_t test = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x01));
    gb_jit_set_flag_if_nonzero(ctx->func, test, flags, GB_FLAG_CARRY);

    /* Replicate the top bit if this is a SRA */
    if ((opcode & 0x10) == 0x00)
        tmp = jit_insn_sshr(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));
    else
        tmp = jit_insn_shr(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));

    gb_jit_set_flag_if_zero(ctx->func, tmp, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* BIT b, reg */
static void bit(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    jit_value_t flags, tmp;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));

    tmp = read_8bit_reg(ctx, reg);

    jit_value_t test = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, (1 << bit)));
    gb_jit_set_flag_if_zero(ctx->func, test, flags, GB_FLAG_ZERO);

    jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_HCARRY));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    /* The (HL) version takes twice as long as normal */
    if (IS_HL(reg))
        gb_jit_clock_tick(ctx);

    return ;
}

/* SET b, reg */
static void set(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    jit_value_t tmp;

    tmp = read_8bit_reg(ctx, reg);
    tmp = jit_insn_or(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, (1 << bit)));

    write_8bit_reg(ctx, reg, tmp);

    return ;
}

/* RES b, reg */
static void res(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    jit_value_t tmp;

    tmp = read_8bit_reg(ctx, reg);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, ~(1 << bit)));

    write_8bit_reg(ctx, reg, tmp);

    return ;
}

/*
 *
 * jumps
 *
 */

enum jump_type {
    JUMP_TYPE_DIRECT,
    JUMP_TYPE_RELATIVE,
    JUMP_TYPE_CALL,
    JUMP_TYPE_RET,
};

static void jp_all(struct gb_cpu_jit_context *ctx, uint8_t opcode, enum jump_type jump_type)
{
    jit_value_t flags, test;
    jit_value_t jump = jit_value_create(ctx->func, jit_type_ubyte);;
    jit_label_t tmp_label, end_label;

    jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 0));

    if (jump_type != JUMP_TYPE_RET) {
        gb_jit_clock_tick(ctx);
        if (jump_type != JUMP_TYPE_RELATIVE)
            gb_jit_clock_tick(ctx);
    }

    flags = gb_jit_load_reg8(ctx, GB_REG_F);

    tmp_label = jit_label_undefined;
    switch (opcode) {
    case 0xC3: /* unconditional */
    case 0x18:
    case 0xCD:
    case 0xC9:
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        break;

    case 0xC2: /* Not-zero */
    case 0x20:
    case 0xC4:
    case 0xC0:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));
        jit_insn_branch_if(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;

    case 0xCA: /* zero */
    case 0x28:
    case 0xCC:
    case 0xC8:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));
        jit_insn_branch_if_not(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;

    case 0xD2: /* not-carry */
    case 0x30:
    case 0xD4:
    case 0xD0:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        jit_insn_branch_if(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;

    case 0xDA: /* carry */
    case 0x38:
    case 0xDC:
    case 0xD8:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        jit_insn_branch_if_not(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;
    }

    tmp_label = jit_label_undefined;
    end_label = jit_label_undefined;

    jit_insn_branch_if_not(ctx->func, jump, &tmp_label);
    {
        jit_value_t tmp;
        switch (jump_type) {
        case JUMP_TYPE_DIRECT:
            gb_jit_clock_tick(ctx);
            tmp = gb_jit_next_pc16(ctx);
            gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
            ctx->addr += 2;
            break;

        case JUMP_TYPE_RELATIVE:
            gb_jit_clock_tick(ctx);
            jit_value_t tmp = gb_jit_next_pc8(ctx);
            tmp = jit_insn_convert(ctx->func, tmp, jit_type_sbyte, 0);
            tmp = jit_insn_add(ctx->func, gb_jit_load_reg16(ctx, GB_REG_PC), tmp);
            gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
            ctx->addr++;
            break;

        case JUMP_TYPE_CALL:
            tmp = gb_jit_next_pc16(ctx);
            push_val(ctx, gb_jit_load_reg16(ctx, GB_REG_PC));
            gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
            ctx->addr += 2;
            break;

        case JUMP_TYPE_RET:
            gb_jit_store_reg16(ctx, GB_REG_PC, pop_val(ctx));
            break;
        }
    }
    jit_insn_branch(ctx->func, &end_label);
    jit_insn_label(ctx->func, &tmp_label);
    if (jump_type != JUMP_TYPE_RET) {
        jit_value_t tmp = gb_jit_load_reg16(ctx, GB_REG_PC);

        switch (jump_type) {
        case JUMP_TYPE_DIRECT:
        case JUMP_TYPE_CALL:
            tmp = jit_insn_add(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 2));
            break;

        case JUMP_TYPE_RELATIVE:
            tmp = jit_insn_add(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 1));
            break;

        case JUMP_TYPE_RET:
            /* Can't happen */
            break;
        }

        gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
    }
    jit_insn_label(ctx->func, &end_label);
}

/* JP nn
 * JP cc, nn */
static void jp(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_DIRECT);
}

/* JP (HL) */
static void jp_hl(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    gb_jit_store_reg16(ctx, GB_REG_PC, gb_jit_load_reg16(ctx, GB_REG_HL));
}

/* JP n (8-bits).
 * JP cc, n */
static void jp_rel(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_RELATIVE);
}

/*
 *
 * calls
 *
 */

/* CALL nn,
 * CALL cc, nn */
static void call(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_CALL);
}

/*
 *
 * Restarts
 *
 */

/* RST n
 * n = 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 */
static void rst(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    uint16_t addr = ((opcode & 0x38) >> 3);
    addr *= 0x08;

    push_val(ctx, gb_jit_load_reg16(ctx, GB_REG_PC));
    gb_jit_store_reg16(ctx, GB_REG_PC, GB_JIT_CONST_USHORT(ctx->func, addr));
}

/*
 *
 * Returns
 *
 */

/* RET
 * RET cc */
static void ret(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_RET);
}

/* RETI */
static void reti(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    gb_jit_store_reg16(ctx, GB_REG_PC, pop_val(ctx));

    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.ime), GB_JIT_CONST_UBYTE(ctx->func, 1));
}

/* 0xCB prefix */
static void z80_emu_run_cb_inst(struct gb_cpu_jit_context *ctx)
{
    uint8_t opcode;

    gb_jit_clock_tick(ctx);
    gb_jit_next_pc8(ctx);
    opcode = gb_emu_read8(ctx->gb_emu, ctx->addr);
    ctx->addr++;

    switch (opcode) {
    case 0x30 ... 0x37:
        swap(ctx, opcode);
        break;

    case 0x00 ... 0x07:
    case 0x10 ... 0x17:
        rla(ctx, opcode, 1);
        break;

    case 0x08 ... 0x0F:
    case 0x18 ... 0x1F:
        rra(ctx, opcode, 1);
        break;

    case 0x20 ... 0x27:
        sla(ctx, opcode);
        break;

    case 0x28 ... 0x2F:
    case 0x38 ... 0x3F:
        sra(ctx, opcode);
        break;

    case 0x40 ... 0x7F:
        bit(ctx, opcode);
        break;

    case 0xC0 ... 0xFF:
        set(ctx, opcode);
        break;

    case 0x80 ... 0xBF:
        res(ctx, opcode);
        break;
    }

    return ;
}

static void check_int_count(struct gb_emu *emu)
{
    if (emu->cpu.int_count > 0) {
        emu->cpu.int_count--;
        if (emu->cpu.int_count == 0)
            emu->cpu.ime = emu->cpu.next_ime;
    }
}

/* Returns true if instruction was a jump */
int gb_emu_jit_run_inst(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int jump = 0;

    switch (opcode) {
    case 0x06:
    case 0x0E:
    case 0x16:
    case 0x1E:
    case 0x26:
    case 0x2E:
    case 0x36:
        load8_reg_imm(ctx, opcode);
        break;

    case 0x40 ... 0x75: /* 0x76 isn't valid */
    case 0x77 ... 0x7F: /* It's used as HALT down below */
        load8_reg_reg(ctx, opcode);
        break;

    case 0x0A:
    case 0x1A:
    case 0xFA:
    case 0x3E:
    case 0xF2:
    case 0x3A:
    case 0x2A:
    case 0xF0:
        load8_a_extra(ctx, opcode);
        break;

    case 0x02:
    case 0x12:
    case 0xEA:
    case 0xE2:
    case 0x32:
    case 0x22:
    case 0xE0:
        load8_extra_a(ctx, opcode);
        break;

    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
        load16_reg_imm(ctx, opcode);
        break;

    case 0xF9:
        load_sp_hl(ctx, opcode);
        break;

    case 0xF8:
        load_hl_sp_n(ctx, opcode);
        break;

    case 0x08:
        load_mem_sp(ctx, opcode);
        break;

    case 0xC5:
    case 0xD5:
    case 0xE5:
    case 0xF5:
        push(ctx, opcode);
        break;

    case 0xC1:
    case 0xD1:
    case 0xE1:
    case 0xF1:
        pop(ctx, opcode);
        break;

    case 0x80 ... 0x87:
    case 0xC6:
    case 0x88 ... 0x8F:
    case 0xCE:
        add_a(ctx, opcode);
        break;

    case 0x90 ... 0x97:
    case 0xD6:
    case 0x98 ... 0x9F:
    case 0xDE:
        sub_a(ctx, opcode);
        break;

    case 0xA0 ... 0xA7:
    case 0xE6:
        and_a(ctx, opcode);
        break;

    case 0xB0 ... 0xB7:
    case 0xF6:
        or_a(ctx, opcode);
        break;

    case 0xA8 ... 0xAF:
    case 0xEE:
        xor_a(ctx, opcode);
        break;

    case 0xB8 ... 0xBF:
    case 0xFE:
        cp_a(ctx, opcode);
        break;

    case 0x04:
    case 0x0C:
    case 0x14:
    case 0x1C:
    case 0x24:
    case 0x2C:
    case 0x34:
    case 0x3C:
        inc_reg(ctx, opcode);
        break;

    case 0x05:
    case 0x0D:
    case 0x15:
    case 0x1D:
    case 0x25:
    case 0x2D:
    case 0x35:
    case 0x3D:
        dec_reg(ctx, opcode);
        break;

    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:
        add_hl_reg16(ctx, opcode);
        break;

    case 0xE8:
        add_sp_imm(ctx, opcode);
        break;

    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
        inc_reg16(ctx, opcode);
        break;

    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:
        dec_reg16(ctx, opcode);
        break;

    case 0x27:
        daa(ctx, opcode);
        break;

    case 0x2F:
        cpl(ctx, opcode);
        break;

    case 0x3F:
        ccf(ctx, opcode);
        break;

    case 0x37:
        scf(ctx, opcode);
        break;

    case 0x00:
        nop(ctx, opcode);
        break;

    case 0x76:
        halt(ctx, opcode);
        break;

    case 0x10:
        stop(ctx, opcode);
        break;

    case 0xF3:
        di(ctx, opcode);
        break;

    case 0xFB:
        ei(ctx, opcode);
        break;

    case 0x07:
    case 0x17:
        rla(ctx, opcode, 0);
        break;

    case 0x0F:
    case 0x1F:
        rra(ctx, opcode, 0);
        break;

    case 0xC3:
    case 0xC2:
    case 0xCA:
    case 0xD2:
    case 0xDA:
        jp(ctx, opcode);
        jump = 1;
        break;

    case 0xE9:
        jp_hl(ctx, opcode);
        jump = 1;
        break;

    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
        jp_rel(ctx, opcode);
        jump = 1;
        break;

    case 0xCD:
    case 0xC4:
    case 0xCC:
    case 0xD4:
    case 0xDC:
        call(ctx, opcode);
        jump = 1;
        break;

    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
        rst(ctx, opcode);
        jump = 1;
        break;

    case 0xC9:
    case 0xC0:
    case 0xC8:
    case 0xD0:
    case 0xD8:
        ret(ctx, opcode);
        jump = 1;
        break;

    case 0xD9:
        reti(ctx, opcode);
        jump = 1;
        break;

    case 0xCB:
        z80_emu_run_cb_inst(ctx);
        break;
    }

    /* No matter what the code does, the lower bits of F are always zero. */
    jit_value_t tmp = gb_jit_load_reg8(ctx, GB_REG_F);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xF0));
    gb_jit_store_reg8(ctx, GB_REG_F, tmp);

    /* Check if we should enable interrupts */
    jit_type_t params[] = { jit_type_void_ptr };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu };
    jit_insn_call_native(ctx->func, "check_int_count", check_int_count, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);

    return jump;
}

/* Returns true when we hit a jump - and the end of a block */
int gb_emu_cpu_jit_run_next_inst(struct gb_cpu_jit_context *ctx)
{
    jit_type_t check_int_params[] = { jit_type_void_ptr };
    jit_type_t check_int_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_sys_int, check_int_params, ARRAY_SIZE(check_int_params), 1);
    jit_value_t check_int_args[] = { ctx->emu };

    jit_type_t dispatch_params[] = { jit_type_void_ptr, jit_type_void_ptr };
    jit_type_t dispatch_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, dispatch_params, ARRAY_SIZE(dispatch_params), 1);
    jit_value_t dispatch_args[] = { GB_JIT_CONST_PTR(ctx->func, ctx->dispatcher), ctx->emu };
    jit_value_t interrupt_check;

    jit_label_t inst_end = jit_label_undefined;
    jit_label_t not_halted = jit_label_undefined;
    jit_label_t run_again = jit_label_undefined;

    jit_insn_label(ctx->func, &run_again);

    jit_value_t halted = jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.halted), jit_type_ubyte);

    /* When we're halted, we keep running the instruction again until we're not longer halted */
    jit_insn_branch_if_not(ctx->func, halted, &not_halted);

        /* When we're halted, the clock still ticks */
        gb_jit_clock_tick(ctx);
        interrupt_check = jit_insn_call_native(ctx->func, "gb_emu_check_interrupt", gb_emu_check_interrupt, check_int_signature, check_int_args, ARRAY_SIZE(check_int_args), JIT_CALL_NOTHROW);
        jit_value_t halt_check_int = jit_insn_eq(ctx->func, interrupt_check, GB_JIT_CONST_INT(ctx->func, 0));

        /* 
         *
         *
         * FIXME: DEFINITELY AN ERROR
         * INTERRUPTS CAN CAUSE STACK OVERFLOWS IF THEY DON'T HAVE A CORESPONDING RETI
         *
         * Happens in Pokemon Gold
         *
         */
        jit_insn_branch_if(ctx->func, halt_check_int, &run_again);
        jit_insn_default_return(ctx->func);
        jit_insn_call_native(ctx->func, "gb_emu_run_dispatcher", gb_emu_run_dispatcher, dispatch_signature, dispatch_args, ARRAY_SIZE(dispatch_args), JIT_CALL_NOTHROW);
        jit_insn_branch(ctx->func, &run_again);
    jit_insn_label(ctx->func, &not_halted);

    jit_type_t hdma_check_params[] = { jit_type_void_ptr };
    jit_type_t hdma_check_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_sys_int, hdma_check_params, ARRAY_SIZE(hdma_check_params), 1);

    jit_value_t hdma_check_args[] = { ctx->emu };
    jit_value_t hdma_check_flag = jit_insn_call_native(ctx->func, "gb_emu_hdma_check", gb_emu_hdma_check, hdma_check_signature, hdma_check_args, ARRAY_SIZE(hdma_check_args), JIT_CALL_NOTHROW);

    jit_insn_branch_if(ctx->func, hdma_check_flag, &run_again);

    gb_jit_clock_tick(ctx);

    gb_jit_next_pc8(ctx);
    uint8_t opcode = gb_emu_read8(ctx->gb_emu, ctx->addr);
    ctx->addr++;

    int jump = gb_emu_jit_run_inst(ctx, opcode);

    jit_insn_label(ctx->func, &inst_end);

    interrupt_check = jit_insn_call_native(ctx->func, "gb_emu_check_interrupt", gb_emu_check_interrupt, check_int_signature, check_int_args, ARRAY_SIZE(check_int_args), JIT_CALL_NOTHROW);

    jit_label_t dispatch_label = jit_label_undefined;

    jit_insn_branch_if(ctx->func, jit_insn_eq(ctx->func, interrupt_check, GB_JIT_CONST_INT(ctx->func, 0)), &dispatch_label);
    jit_insn_default_return(ctx->func);
    jit_insn_call_native(ctx->func, "gb_emu_run_dispatcher", gb_emu_run_dispatcher, dispatch_signature, dispatch_args, ARRAY_SIZE(dispatch_args), JIT_CALL_NOTHROW);

    jit_insn_label(ctx->func, &dispatch_label);

    return jump;
}

static struct gb_emu *signal_emu;

static void debugger_run_sigint(int signum)
{
    signal_emu->stop_emu = 1;
    signal_emu->reason = GB_EMU_STOP;
}

enum gb_emu_stop gb_run_jit(struct gb_emu *emu)
{
    int i;
    uint64_t cycles = 0;
    struct sigaction new_act, old_act;

    emu->stop_emu = 0;

    if (emu->sound.driver) {
        gb_sound_start(&emu->sound);
        gb_sound_set_sound_rate(&emu->sound, GB_APU_SAMPLE_RATE);

        (emu->sound.driver->play) (emu->sound.driver);
    }

    memset(&new_act, 0, sizeof(new_act));
    new_act.sa_handler = debugger_run_sigint;

    sigaction(SIGINT, &new_act, &old_act);

    signal_emu = emu;

    while (!emu->stop_emu) {
        cycles = 0;

        for (i = 0; i < 20000; i++) {
            cycles += gb_emu_cpu_run_next_inst(emu);
            if (emu->stop_emu)
                break;
        }
    }

    sigaction(SIGINT, &old_act, NULL);

    if (emu->sound.driver) {
        (emu->sound.driver->pause) (emu->sound.driver);

        gb_sound_finish(&emu->sound);
    }

    return emu->reason;
}

