
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <signal.h>

#include "debug.h"
#include "gb/disasm.h"
#include "gb_internal.h"
#include "gb/cpu.h"

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
static inline uint8_t read_8bit_reg(struct gb_emu *emu, int reg, int *cycles)
{
    if (IS_HL(reg)) {
        gb_emu_clock_tick(emu);
        *cycles += 4;
        return gb_emu_read8(emu, emu->cpu.r.w[GB_REG_HL]);
    } else {
        return emu->cpu.r.b[reg_map_8bit[reg]];
    }
}

static inline void write_8bit_reg(struct gb_emu *emu, int reg, uint8_t val, int *cycles)
{
    if (IS_HL(reg)) {
        gb_emu_clock_tick(emu);
        *cycles += 4;
        gb_emu_write8(emu, emu->cpu.r.w[GB_REG_HL], val);
    } else {
        emu->cpu.r.b[reg_map_8bit[reg]] = val;
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
static int load8_reg_imm(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    /* Pull Register number out of bits 4 through 6. */
    uint8_t reg = ((opcode & b8(00111000)) >> 3);

    /* Reading PC takes a clock-tick */
    gb_emu_clock_tick(emu);
    cycles += 4;
    uint8_t imm = gb_emu_next_pc8(emu);

    write_8bit_reg(emu, reg, imm, &cycles);
    /* emu->cpu.r.b[reg_map_8bit[reg]] = imm; */

    /* 4 cycles for our read from PC */
    return cycles;
}

/* LD reg, reg. */
static int load8_reg_reg(struct gb_emu *emu, uint8_t opcode)
{
    /* Dest is bits 4 through 6
     * Src is bits 1 through 3 */
    uint8_t dest = ((opcode & b8(00111000)) >> 3);
    uint8_t src = (opcode & b8(00000111));
    int cycles = 0;
    uint8_t val;

    val = read_8bit_reg(emu, src, &cycles);
    write_8bit_reg(emu, dest, val, &cycles);

    return cycles;
}

/* LD A, (n)
 * LD A, #
 * LD A, (C)
 * LDD A, (HL)
 * LDI A, (HL)
 */
static int load8_a_extra(struct gb_emu *emu, uint8_t opcode)
{
    int reg;
    uint8_t val = 0;
    uint16_t src;
    int cycles = 0;

    switch (opcode) {
    case 0x0A: /* LD A, (BC) */
    case 0x1A: /* LD A, (DE) */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;
        val = gb_emu_read8(emu, emu->cpu.r.w[reg]);
        break;

    case 0xFA: /* LD A, (nn) */
        /* Two clock ticks for reading a 16-bit from PC */
        gb_emu_clock_tick(emu);
        gb_emu_clock_tick(emu);
        src = gb_emu_next_pc16(emu);
        val = gb_emu_read8(emu, src);
        cycles += 8;
        break;

    case 0x3E: /* LD A, # */
        val = gb_emu_next_pc8(emu);
        break;

    case 0xF2: /* LD A, (C) */
        src = 0xFF00 + emu->cpu.r.b[GB_REG_C];
        val = gb_emu_read8(emu, src);
        break;

    case 0xF0:
        /* 8-bit read from PC */
        gb_emu_clock_tick(emu);
        src = 0xFF00 + gb_emu_next_pc8(emu);
        val = gb_emu_read8(emu, src);
        cycles += 4;
        break;

    case 0x3A: /* LDD A, (HL) */
    case 0x2A: /* LDI A, (HL) */
        src = emu->cpu.r.w[GB_REG_HL];
        if (opcode == 0x3A)
            emu->cpu.r.w[GB_REG_HL]--;
        else
            emu->cpu.r.w[GB_REG_HL]++;

        val = gb_emu_read8(emu, src);
        break;
    }

    /* Accounts for 8-bit read in every path. */
    gb_emu_clock_tick(emu);

    emu->cpu.r.b[GB_REG_A] = val;

    return cycles;
}

/* LD (n), A
 * LDD (HL), A
 * LDI (HL), A
 */
static int load8_extra_a(struct gb_emu *emu, uint8_t opcode)
{
    int reg;
    uint16_t dest = 0;
    int cycles = 0;

    switch (opcode) {
    case 0x02: /* LD (BC), A */
    case 0x12: /* LD (DE), A */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;

        dest = emu->cpu.r.w[reg];
        break;

    case 0xEA: /* LD (nn), A */
        gb_emu_clock_tick(emu);
        gb_emu_clock_tick(emu);
        dest = gb_emu_next_pc16(emu);
        cycles += 8;
        break;

    case 0xE2: /* LD (0xFF00 + C), A */
        dest = 0xFF00 + emu->cpu.r.b[GB_REG_C];
        break;

    case 0xE0: /* LD (0xFF00 + n), A */
        gb_emu_clock_tick(emu);
        dest = 0xFF00 + gb_emu_next_pc8(emu);
        cycles += 4;
        break;

    case 0x32: /* LDD (HL), A */
    case 0x22: /* LDI (HL), A */
        dest = emu->cpu.r.w[GB_REG_HL];
        if (opcode == 0x32)
            emu->cpu.r.w[GB_REG_HL]--;
        else
            emu->cpu.r.w[GB_REG_HL]++;
        break;
    }

    gb_emu_clock_tick(emu);
    gb_emu_write8(emu, dest, emu->cpu.r.b[GB_REG_A]);
    cycles += 4;

    return cycles;
}

/*
 *
 * 16-bit Loads
 *
 */

/* LD n, nn */
static int load16_reg_imm(struct gb_emu *emu, uint8_t opcode)
{
    int dest = reg_map_16bit_sp[(opcode & 0x30) >> 4];

    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    uint16_t src = gb_emu_next_pc16(emu);

    emu->cpu.r.w[dest] = src;

    /* Always takes 8 cycles */
    return 8;
}

/* LD SP, HL */
static int load_sp_hl(struct gb_emu *emu, uint8_t opcode)
{
    /* Extra clock-tick for 16-bit loads */
    gb_emu_clock_tick(emu);

    emu->cpu.r.w[GB_REG_SP] = emu->cpu.r.w[GB_REG_HL];

    return 4;
}

/* LDHL SP, n */
static int load_hl_sp_n(struct gb_emu *emu, uint8_t opcode)
{
    uint8_t flags = 0;
    uint16_t val;

    /* Read 8-bit from PC */
    gb_emu_clock_tick(emu);
    int8_t off = gb_emu_next_pc8(emu);

    /* Extra clock tick for 16-bit load */
    gb_emu_clock_tick(emu);

    val = emu->cpu.r.w[GB_REG_SP] + off;

    flags |= ((emu->cpu.r.w[GB_REG_SP] ^ off ^ val) & 0x100)? GB_FLAG_CARRY: 0;
    flags |= ((emu->cpu.r.w[GB_REG_SP] ^ off ^ val) &  0x10)? GB_FLAG_HCARRY: 0;

    emu->cpu.r.w[GB_REG_HL] = val;
    emu->cpu.r.b[GB_REG_F] = flags;

    return 8;
}

/* LD (nn), SP */
static int load_mem_sp(struct gb_emu *emu, uint8_t opcode)
{
    /* 16-bit read */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    uint16_t dest = gb_emu_next_pc16(emu);

    /* 16-bit write */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    gb_emu_write16(emu, dest, emu->cpu.r.w[GB_REG_SP]);

    return 16;
}

static int push_val(struct gb_emu *emu, uint16_t val)
{
    /* Extra clock-tick for 16-bit */
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_SP] -= 2;

    /* 16-bit write */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    gb_emu_write16(emu, emu->cpu.r.w[GB_REG_SP], val);

    return 12;
}

static int pop_val(struct gb_emu *emu, uint16_t *dest)
{
    /* 16-bit read */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    *dest = gb_emu_read16(emu, emu->cpu.r.w[GB_REG_SP]);

    /* Extra clock-tick for 16-bit */
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_SP] += 2;

    return 12;
}

/* PUSH reg
 * reg = AF, BC, DE, HL */
static int push(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int dest = reg_map_16bit_af[(opcode & 0x30) >> 4];

    cycles += push_val(emu, emu->cpu.r.w[dest]);

    return cycles;
}

/* POP reg
 * reg = AF, BC, DE, HL */
static int pop(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int dest = reg_map_16bit_af[(opcode & 0x30) >> 4];

    cycles += pop_val(emu, emu->cpu.r.w + dest);

    return cycles;
}

/*
 *
 * 8-bit ALU
 *
 */

/* Performs an 8-bit addition, with optional carry.
 *
 * Returns the result of the add */
static uint8_t adc(uint8_t val1, uint8_t val2, uint8_t *flags, int carry)
{
    uint8_t result;

    *flags = 0;

    if ((val1 & 0xF) + (val2 & 0xF) + carry > 0xF)
        *flags |= GB_FLAG_HCARRY;

    if ((uint16_t)val1 + (uint16_t)val2 + carry > 0xFF)
        *flags |= GB_FLAG_CARRY;

    /* Overflow is undefined in C - We avoid it by upcasting to uint16_t, and
     * then masking out the higher bits after we do the addition */
    result = ((uint16_t)val1 + (uint16_t)val2 + carry) & 0xFF;

    if (result == 0)
        *flags |= GB_FLAG_ZERO;

    return result;
}

/* ADD A, reg
 * ADD A, #
 *
 * ADC A, reg
 * ADC A, #
 */
static int add_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t carry = 0;
    uint8_t tmp;
    uint8_t flags = 0;

    /* 0xC0 means it's an immediate value */
    if ((opcode & 0xF0) != 0xC0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    /* ADC commands add the carry flag */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xCE)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY);

    gb_emu_clock_tick(emu);
    emu->cpu.r.b[GB_REG_A] = adc(emu->cpu.r.b[GB_REG_A], tmp, &flags, carry);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles + 4;
}

static uint8_t sbc(uint8_t val1, uint8_t val2, uint8_t *flags, int carry)
{
    uint8_t result;

    *flags = GB_FLAG_SUB;

    result = (val1 - val2 - carry) & 0xFF;

    if (((uint16_t)val1 - (uint16_t)val2 - carry) & 0xFF00)
        *flags |= GB_FLAG_CARRY;

    if ((val1 ^ val2 ^ result) & 0x10)
        *flags |= GB_FLAG_HCARRY;

    if (result == 0)
        *flags |= GB_FLAG_ZERO;

    return result;
}

/* SUB reg
 * SUB #
 */
static int sub_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t tmp, carry;
    uint8_t flags = 0;

    /* 0xD0 means it's am immediate value */
    if ((opcode & 0xF0) != 0xD0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    /* SBC commands add the carry flag to the front of 'a'. */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xDE)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY);
    else
        carry = 0;

    emu->cpu.r.b[GB_REG_A] = sbc(emu->cpu.r.b[GB_REG_A], tmp, &flags, carry);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* AND reg
 * AND n */
static int and_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = GB_FLAG_HCARRY; /* AND always sets HCARRY */
    uint8_t tmp;

    /* Check if this is not the immediate value opcode 0xE6 */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    emu->cpu.r.b[GB_REG_A] &= tmp;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* OR reg
 * OR n */
static int or_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = 0;
    uint8_t tmp;

    /* Check if this is not the immediate value opcode 0xF6 */
    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    emu->cpu.r.b[GB_REG_A] |= tmp;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* XOR reg
 * XOR n */
static int xor_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = 0;
    uint8_t tmp;

    /* Check if this is not the immediate value opcode 0xEE */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    emu->cpu.r.b[GB_REG_A] ^= tmp;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* CP reg
 * CP n */
static int cp_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = 0;
    uint8_t tmp, a;

    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    a = emu->cpu.r.b[GB_REG_A];

    if (a == tmp)
        flags |= GB_FLAG_ZERO;

    if ((a & 0xF) < (tmp & 0xF))
        flags |= GB_FLAG_HCARRY;

    if (a < tmp)
        flags |= GB_FLAG_CARRY;

    flags |= GB_FLAG_SUB;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* INC reg */
static int inc_reg(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x38) >> 3;
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, src, &cycles);

    if ((tmp & 0x0F) == 0xF)
        flags |= GB_FLAG_HCARRY;

    if (tmp == 0xFF)
        flags |= GB_FLAG_ZERO;

    tmp = ((uint16_t)tmp + 1) & 0xFF;

    write_8bit_reg(emu, src, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* DEC reg */
static int dec_reg(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x38) >> 3;
    uint8_t flags = (emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY) | GB_FLAG_SUB;
    uint8_t tmp;


    tmp = read_8bit_reg(emu, src, &cycles);

    if ((tmp & 0x0F) == 0)
        flags |= GB_FLAG_HCARRY;

    if (tmp == 1)
        flags |= GB_FLAG_ZERO;

    tmp = ((uint16_t)tmp - 1) & 0xFF;

    write_8bit_reg(emu, src, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/*
 *
 * 16-bit ALU
 *
 */

/* ADD HL, reg */
static int add_hl_reg16(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_ZERO;

    if ((emu->cpu.r.w[GB_REG_HL] & 0x0FFF) + (emu->cpu.r.w[reg] & 0x0FFF) > 0x0FFF)
        flags |= GB_FLAG_HCARRY;

    if (((uint32_t)emu->cpu.r.w[GB_REG_HL] + (uint32_t)emu->cpu.r.w[reg]) > 0xFFFF)
        flags |= GB_FLAG_CARRY;

    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_HL] = ((uint32_t)emu->cpu.r.w[GB_REG_HL] + (uint32_t)emu->cpu.r.w[reg]) & 0xFFFF;

    emu->cpu.r.b[GB_REG_F] = flags;

    cycles = 4;

    return cycles;
}

/* ADD SP, # */
static int add_sp_imm(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int8_t tmp;
    uint8_t flags = 0;
    uint16_t temp_val;

    gb_emu_clock_tick(emu);
    tmp = gb_emu_next_pc8(emu);

    temp_val = emu->cpu.r.w[GB_REG_SP] + tmp;

    /* From Mednafem - magic to do the carry and hcarry flags */
    flags |= ((emu->cpu.r.w[GB_REG_SP] ^ tmp ^ temp_val) & 0x100)? GB_FLAG_CARRY: 0;
    flags |= ((emu->cpu.r.w[GB_REG_SP] ^ tmp ^ temp_val) &  0x10)? GB_FLAG_HCARRY: 0;

    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_SP] = temp_val;
    emu->cpu.r.b[GB_REG_F] = flags;

    cycles = 12;

    return cycles;
}

/* INC nn */
static int inc_reg16(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];

    gb_emu_clock_tick(emu);
    emu->cpu.r.w[reg] = ((uint32_t)emu->cpu.r.w[reg] + 1) & 0xFFFF;

    cycles = 4;

    return cycles;
}

/* DEC nn */
static int dec_reg16(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];

    gb_emu_clock_tick(emu);
    emu->cpu.r.w[reg] = ((uint32_t)emu->cpu.r.w[reg] - 1) & 0xFFFF;

    cycles = 4;

    return cycles;
}

/*
 *
 * Misc ops
 *
 */

/* SWAP reg */
static int swap(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x7);
    uint8_t tmp, flags = 0;

    tmp = read_8bit_reg(emu, src, &cycles);

    if (tmp == 0)
        flags |= GB_FLAG_ZERO;

    tmp = ((tmp & 0xF0) >> 4) + ((tmp & 0x0F) << 4);

    write_8bit_reg(emu, src, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* DAA */
static int daa(struct gb_emu *emu, uint8_t opcode)
{
    uint16_t lookup;

    lookup = emu->cpu.r.b[GB_REG_A];
    lookup |= (emu->cpu.r.b[GB_REG_F] & (GB_FLAG_CARRY | GB_FLAG_HCARRY | GB_FLAG_SUB)) << 4;

    emu->cpu.r.w[GB_REG_AF] = gb_daa_table[lookup];

    return 0;
}

/* CPL */
static int cpl(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    emu->cpu.r.b[GB_REG_A] = ~emu->cpu.r.b[GB_REG_A];
    emu->cpu.r.b[GB_REG_F] |= GB_FLAG_SUB | GB_FLAG_HCARRY;

    return cycles;
}

/* CCF */
static int ccf(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.r.b[GB_REG_F] ^= GB_FLAG_CARRY;
    emu->cpu.r.b[GB_REG_F] &= ~(GB_FLAG_SUB | GB_FLAG_HCARRY);
    return 0;
}

/* SCF */
static int scf(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.r.b[GB_REG_F] = (emu->cpu.r.b[GB_REG_F] & GB_FLAG_ZERO) | GB_FLAG_CARRY;

    return 0;
}

/* NOP */
static int nop(struct gb_emu *emu, uint8_t opcode)
{
    return 0;
}

/* HALT */
static int halt(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.halted = 1;
    return 0;
}

/* STOP */
static int stop(struct gb_emu *emu, uint8_t opcode)
{
    if (!gb_emu_is_cgb(emu) || !emu->cpu.do_speed_switch) {
        emu->cpu.stopped = 1;
    } else {
        /* Toggle double speed mode */
        emu->cpu.double_speed ^= 1;
        printf("Entered double-speed mode\n");
    }

    return 0;
}

/* DI */
static int di(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.next_ime = 0;
    emu->cpu.int_count = 1;

    return 0;
}

/* EI */
static int ei(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.next_ime = 1;
    emu->cpu.int_count = 2;

    return 0;
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
static int rla(struct gb_emu *emu, uint8_t opcode, int is_cb)
{
    int cycles = 0;
    uint8_t flags = 0;
    uint8_t carry = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    /* RLA - use carry flag */
    if ((opcode & 0x10) == 0x10)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY);
    else
        carry = (tmp & 0x80) >> 7;

    uint16_t res = ((uint16_t)tmp << 1) | carry;

    if (res & 0x100)
        flags |= GB_FLAG_CARRY;

    if (is_cb && (res & 0xFF) == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, res & 0xFF, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* RRA
 * RRCA
 * RR reg
 * RRC reg */
static int rra(struct gb_emu *emu, uint8_t opcode, int is_cb)
{
    int cycles = 0;
    uint8_t flags = 0;
    uint8_t carry = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    /* RRA uses carry */
    if ((opcode & 0x10) == 0x10)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY) << 7;
    else
        carry = (tmp & 0x01) << 7;

    uint8_t res = carry | (tmp >> 1);

    if (tmp & 0x01)
        flags |= GB_FLAG_CARRY;

    if (is_cb && res == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, res, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* SLA reg */
static int sla(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    if (tmp & 0x80)
        flags |= GB_FLAG_CARRY;

    tmp <<= 1;

    if (tmp == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* SRA reg
 * SRL reg */
static int sra(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    if (tmp & 0x01)
        flags |= GB_FLAG_CARRY;

    tmp >>= 1;

    /* Replicate the top bit if this is a SRA */
    if ((opcode & 0x10) == 0x00 && (tmp & 0x40))
        tmp |= 0x80;

    if (tmp == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* BIT b, reg */
static int bit(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY;
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    if (!(tmp & (1 << bit)))
        flags |= GB_FLAG_ZERO;

    flags |= GB_FLAG_HCARRY;

    emu->cpu.r.b[GB_REG_F] = flags;

    /* The (HL) version takes twice as long as normal */
    if (IS_HL(reg)) {
        gb_emu_clock_tick(emu);
        cycles += 4;
    }

    return cycles;
}

/* SET b, reg */
static int set(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    tmp |= (1 << bit);

    write_8bit_reg(emu, reg, tmp, &cycles);

    return cycles;
}

/* RES b, reg */
static int res(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    tmp &= ~(1 << bit);

    write_8bit_reg(emu, reg, tmp, &cycles);

    return cycles;
}

/*
 *
 * jumps
 *
 */

/* JP nn
 * JP cc, nn */
static int jp(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags;

    cycles += 8;
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0xC3: /* unconditional */
        jump = 1;
        break;

    case 0xC2: /* Not-zero */
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        break;

    case 0xCA: /* zero */
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        break;

    case 0xD2: /* not-carry */
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        break;

    case 0xDA: /* carry */
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        break;
    }

    if (jump) {
        uint16_t addr = gb_emu_next_pc16(emu);
        gb_emu_clock_tick(emu);
        emu->cpu.r.w[GB_REG_PC] = addr;
        cycles += 4;
    } else {
        emu->cpu.r.w[GB_REG_PC] += 2;
    }

    return cycles;
}

/* JP (HL) */
static int jp_hl(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    emu->cpu.r.w[GB_REG_PC] = emu->cpu.r.w[GB_REG_HL];

    return cycles;
}

/* JP n (8-bits).
 * JP cc, n */
static int jp_rel(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags;

    cycles += 4;
    gb_emu_clock_tick(emu);

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0x18:
        jump = 1;
        break;

    case 0x20: /* not-zero */
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        break;

    case 0x28: /* zero */
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        break;

    case 0x30: /* not-carry */
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        break;

    case 0x38: /* carry */
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        break;
    }

    if (jump) {
        gb_emu_clock_tick(emu);
        int8_t tmp = gb_emu_next_pc8(emu);
        emu->cpu.r.w[GB_REG_PC] += tmp;
        cycles += 4;
    } else {
        emu->cpu.r.w[GB_REG_PC] += 1;
    }

    return cycles;
}

/*
 *
 * calls
 *
 */

/* CALL nn,
 * CALL cc, nn */
static int call(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags;

    cycles += 8;
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0xCD:
        jump = 1;
        break;

    case 0xC4: /* non-zero */
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        break;

    case 0xCC: /* zero */
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        break;

    case 0xD4: /* non-carry */
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        break;

    case 0xDC: /* carry */
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        break;
    }

    if (jump) {
        uint16_t addr = gb_emu_next_pc16(emu);

        cycles += push_val(emu, emu->cpu.r.w[GB_REG_PC]);
        emu->cpu.r.w[GB_REG_PC] = addr;
    } else {
        emu->cpu.r.w[GB_REG_PC] += 2;
    }

    return cycles;
}

/*
 *
 * Restarts
 *
 */

/* RST n
 * n = 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 */
static int rst(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint16_t addr = ((opcode & 0x38) >> 3);

    addr *= 0x08;

    cycles += push_val(emu, emu->cpu.r.w[GB_REG_PC]);

    emu->cpu.r.w[GB_REG_PC] = addr;

    return cycles;
}

/*
 *
 * Returns
 *
 */

/* RET
 * RET cc */
static int ret(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags = 0;

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0xC9:
        jump = 1;
        break;

    case 0xC0:
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        cycles += 4;
        break;

    case 0xC8:
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        cycles += 4;
        break;

    case 0xD0:
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        cycles += 4;
        break;

    case 0xD8:
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        cycles += 4;
        break;
    }

    if (jump) {
        cycles += pop_val(emu, emu->cpu.r.w + GB_REG_PC);
    }

    return cycles;
}

/* RETI */
static int reti(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    cycles += pop_val(emu, emu->cpu.r.w + GB_REG_PC);

    emu->cpu.ime = 1;

    return cycles;
}

/* 0xCB prefix */
static int z80_emu_run_cb_inst(struct gb_emu *emu)
{
    int cycles = 0;
    uint8_t opcode;

    gb_emu_clock_tick(emu);
    opcode = gb_emu_next_pc8(emu);

    switch (opcode) {
    case 0x30 ... 0x37:
        cycles = swap(emu, opcode);
        break;

    case 0x00 ... 0x07:
    case 0x10 ... 0x17:
        cycles = rla(emu, opcode, 1);
        break;

    case 0x08 ... 0x0F:
    case 0x18 ... 0x1F:
        cycles = rra(emu, opcode, 1);
        break;

    case 0x20 ... 0x27:
        cycles = sla(emu, opcode);
        break;

    case 0x28 ... 0x2F:
    case 0x38 ... 0x3F:
        cycles = sra(emu, opcode);
        break;

    case 0x40 ... 0x7F:
        cycles = bit(emu, opcode);
        break;

    case 0xC0 ... 0xFF:
        cycles = set(emu, opcode);
        break;

    case 0x80 ... 0xBF:
        cycles = res(emu, opcode);
        break;
    }

    cycles += 4;

    return cycles;
}

/* Returns cycle count for this instruction */
int gb_emu_run_inst(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    switch (opcode) {
    case 0x06:
    case 0x0E:
    case 0x16:
    case 0x1E:
    case 0x26:
    case 0x2E:
    case 0x36:
        cycles = load8_reg_imm(emu, opcode);
        break;

    case 0x40 ... 0x75: /* 0x76 isn't valid */
    case 0x77 ... 0x7F: /* It's used as HALT down below */
        cycles = load8_reg_reg(emu, opcode);
        break;

    case 0x0A:
    case 0x1A:
    case 0xFA:
    case 0x3E:
    case 0xF2:
    case 0x3A:
    case 0x2A:
    case 0xF0:
        cycles = load8_a_extra(emu, opcode);
        break;

    case 0x02:
    case 0x12:
    case 0xEA:
    case 0xE2:
    case 0x32:
    case 0x22:
    case 0xE0:
        cycles = load8_extra_a(emu, opcode);
        break;

    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
        cycles = load16_reg_imm(emu, opcode);
        break;

    case 0xF9:
        cycles = load_sp_hl(emu, opcode);
        break;

    case 0xF8:
        cycles = load_hl_sp_n(emu, opcode);
        break;

    case 0x08:
        cycles = load_mem_sp(emu, opcode);
        break;

    case 0xC5:
    case 0xD5:
    case 0xE5:
    case 0xF5:
        cycles = push(emu, opcode);
        break;

    case 0xC1:
    case 0xD1:
    case 0xE1:
    case 0xF1:
        cycles = pop(emu, opcode);
        break;

    case 0x80 ... 0x87:
    case 0xC6:
    case 0x88 ... 0x8F:
    case 0xCE:
        cycles = add_a(emu, opcode);
        break;

    case 0x90 ... 0x97:
    case 0xD6:
    case 0x98 ... 0x9F:
    case 0xDE:
        cycles = sub_a(emu, opcode);
        break;

    case 0xA0 ... 0xA7:
    case 0xE6:
        cycles = and_a(emu, opcode);
        break;

    case 0xB0 ... 0xB7:
    case 0xF6:
        cycles = or_a(emu, opcode);
        break;

    case 0xA8 ... 0xAF:
    case 0xEE:
        cycles = xor_a(emu, opcode);
        break;

    case 0xB8 ... 0xBF:
    case 0xFE:
        cycles = cp_a(emu, opcode);
        break;

    case 0x04:
    case 0x0C:
    case 0x14:
    case 0x1C:
    case 0x24:
    case 0x2C:
    case 0x34:
    case 0x3C:
        cycles = inc_reg(emu, opcode);
        break;

    case 0x05:
    case 0x0D:
    case 0x15:
    case 0x1D:
    case 0x25:
    case 0x2D:
    case 0x35:
    case 0x3D:
        cycles = dec_reg(emu, opcode);
        break;

    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:
        cycles = add_hl_reg16(emu, opcode);
        break;

    case 0xE8:
        cycles = add_sp_imm(emu, opcode);
        break;

    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
        cycles = inc_reg16(emu, opcode);
        break;

    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:
        cycles = dec_reg16(emu, opcode);
        break;

    case 0x27:
        cycles = daa(emu, opcode);
        break;

    case 0x2F:
        cycles = cpl(emu, opcode);
        break;

    case 0x3F:
        cycles = ccf(emu, opcode);
        break;

    case 0x37:
        cycles = scf(emu, opcode);
        break;

    case 0x00:
        cycles = nop(emu, opcode);
        break;

    case 0x76:
        cycles = halt(emu, opcode);
        break;

    case 0x10:
        cycles = stop(emu, opcode);
        break;

    case 0xF3:
        cycles = di(emu, opcode);
        break;

    case 0xFB:
        cycles = ei(emu, opcode);
        break;

    case 0x07:
    case 0x17:
        cycles = rla(emu, opcode, 0);
        break;

    case 0x0F:
    case 0x1F:
        cycles = rra(emu, opcode, 0);
        break;

    case 0xC3:
    case 0xC2:
    case 0xCA:
    case 0xD2:
    case 0xDA:
        cycles = jp(emu, opcode);
        break;

    case 0xE9:
        cycles = jp_hl(emu, opcode);
        break;

    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
        cycles = jp_rel(emu, opcode);
        break;

    case 0xCD:
    case 0xC4:
    case 0xCC:
    case 0xD4:
    case 0xDC:
        cycles = call(emu, opcode);
        break;

    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
        cycles = rst(emu, opcode);
        break;

    case 0xC9:
    case 0xC0:
    case 0xC8:
    case 0xD0:
    case 0xD8:
        cycles = ret(emu, opcode);
        break;

    case 0xD9:
        cycles = reti(emu, opcode);
        break;

    case 0xCB:
        cycles = z80_emu_run_cb_inst(emu);
        break;
    }

    /* No matter what the code does, the lower bits of F are always zero. */
    emu->cpu.r.b[GB_REG_F] &= 0xF0;

    /* Check if we should enable interrupts */
    if (emu->cpu.int_count > 0) {
        emu->cpu.int_count--;
        if (emu->cpu.int_count == 0)
            emu->cpu.ime = emu->cpu.next_ime;
    }

    return cycles;
}

int gb_emu_check_interrupt(struct gb_emu *emu)
{
    uint8_t int_check = emu->cpu.int_enabled & emu->cpu.int_flags;
    int i;
    int ret = 0;

    /* Lower number interrupts have priority */
    for (i = 0; i < GB_INT_TOTAL; i++) {
        if (int_check & (1 << i)) {

            /* And interrupt unhalts the CPU regardless of the IME state */
            emu->cpu.halted = 0;
            if (!emu->cpu.ime)
                continue;

            push_val(emu, emu->cpu.r.w[GB_REG_PC]);

            emu->cpu.r.w[GB_REG_PC] = GB_INT_BASE_ADDR + i * 0x8;
            emu->cpu.ime = 0;

            emu->cpu.int_flags &= ~(1 << i); /* Reset this interrupt's bit, since we're servicing it */
            ret = 1;
            break;
        }
    }

    return ret;
}

int gb_emu_hdma_check(struct gb_emu *emu)
{
    if (emu->mmu.hdma_active && emu->gpu.mode == GB_GPU_MODE_HBLANK) {
        int i;

        for (i = 0; i < 16 && emu->mmu.hdma_length_left; i++) {
            gb_emu_write8(emu, emu->mmu.hdma_dest, gb_emu_read8(emu, emu->mmu.hdma_source));

            emu->mmu.hdma_dest++;
            emu->mmu.hdma_source++;
            emu->mmu.hdma_length_left--;
        }

        if (!emu->mmu.hdma_length_left) {
            emu->mmu.hdma_length_left = 0xFF;
            emu->mmu.hdma_active = 0;
        }

        while (emu->gpu.mode == GB_GPU_MODE_HBLANK)
            gb_emu_clock_tick(emu);

        return 1;
    }

    return 0;
}

void gb_emu_cpu_next_inst_hook(struct gb_emu *emu)
{
    if (emu->hook_flag) {
        uint8_t bytes[3];

        bytes[0] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC]);
        bytes[1] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 1);
        bytes[2] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 2);

        if (emu->cpu.hooks && emu->cpu.hooks->next_inst)
            (emu->cpu.hooks->next_inst) (emu->cpu.hooks, emu, bytes);
    }
}

void gb_emu_cpu_breakpoint_check(struct gb_emu *emu)
{
    if (emu->break_flag) {
        int k;
        uint16_t pc = emu->cpu.r.w[GB_REG_PC];
        for (k = 0; k < emu->breakpoint_count; k++) {
            if (pc == emu->breakpoints[k]) {
                emu->stop_emu = 1;
                emu->reason = GB_EMU_BREAK;
            }
        }
   }
}

int gb_emu_cpu_run_next_inst(struct gb_emu *emu)
{
    int cycles = 0;

    if (emu->cpu.halted) {
        /* When we're halted, the clock still ticks */
        gb_emu_clock_tick(emu);
        cycles += 4;
        goto inst_end;
    }

    if (gb_emu_hdma_check(emu))
        return cycles;

    if (emu->hook_flag) {
        uint8_t bytes[3];

        bytes[0] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC]);
        bytes[1] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 1);
        bytes[2] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 2);

        if (emu->cpu.hooks && emu->cpu.hooks->next_inst)
            (emu->cpu.hooks->next_inst) (emu->cpu.hooks, emu, bytes);
    }


    gb_emu_clock_tick(emu);
    uint8_t opcode = gb_emu_next_pc8(emu);

    /* The extra four accounts for the read from PC above */
    cycles = gb_emu_run_inst(emu, opcode) + 4;

    if (emu->hook_flag)
        if (emu->cpu.hooks && emu->cpu.hooks->end_inst)
            (emu->cpu.hooks->end_inst) (emu->cpu.hooks, emu);

inst_end:

    gb_emu_check_interrupt(emu);

    if (emu->break_flag) {
        int k;
        uint16_t pc = emu->cpu.r.w[GB_REG_PC];
        for (k = 0; k < emu->breakpoint_count; k++) {
            if (pc == emu->breakpoints[k]) {
                emu->stop_emu = 1;
                emu->reason = GB_EMU_BREAK;
            }
        }
   }

    return cycles;
}

void gb_emu_run_interpreter(struct gb_emu *emu)
{
    int i;
    while (!emu->stop_emu) {
        for (i = 0; i < 20000; i++) {
            gb_emu_cpu_run_next_inst(emu);
            if (emu->stop_emu)
                break;
        }
    }
}

uint8_t gb_cpu_int_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->cpu.int_enabled;
}

void gb_cpu_int_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    emu->cpu.int_enabled = byte;
}

void gb_emu_dump_regs(struct gb_emu *emu, char *output_buf)
{
    struct gb_cpu *cpu = &emu->cpu;

    struct reg {
        char id;
        int reg;
    } regs[] = {
        { 'A', GB_REG_A },
        { 'F', GB_REG_F },
        { 'B', GB_REG_B },
        { 'C', GB_REG_C },
        { 'D', GB_REG_D },
        { 'E', GB_REG_E },
        { 'H', GB_REG_H },
        { 'L', GB_REG_L },
        { '\0', 0 },
    };

    struct reg16 {
        const char *id;
        int reg;
    } regs16[] = {
        { "AF", GB_REG_AF },
        { "BC", GB_REG_BC },
        { "DE", GB_REG_DE },
        { "HL", GB_REG_HL },
        { "SP", GB_REG_SP },
        { "PC", GB_REG_PC },
        { NULL, 0 },
    };

    struct reg *reg;
    struct reg16 *reg16;
    char buf[80];
    size_t len = 0;

    memset(buf, 0, sizeof(buf));

    len = sprintf(buf, "8REG: ");
    for (reg = regs; reg->id; reg++)
        len += sprintf(buf + len, "%c: 0x%02x ", reg->id, cpu->r.b[reg->reg]);
    len += sprintf(buf + len, "\n");

    output_buf += sprintf(output_buf, "%s", buf);

    memset(buf, 0, sizeof(buf));

    len = 0;

    len = sprintf(buf, "16REG: ");
    for (reg16 = regs16; reg16->id; reg16++)
        len += sprintf(buf + len, "%s: 0x%04x ", reg16->id, cpu->r.w[reg16->reg]);
    len += sprintf(buf + len, "\n");

    output_buf += sprintf(output_buf, "%s", buf);

    output_buf += sprintf(output_buf, "IME: %d, IE: 0x%02x, IF: 0x%02x, HALT: %d, STOP: %d\n", cpu->ime, cpu->int_enabled, cpu->int_flags, cpu->halted, cpu->stopped);
}

