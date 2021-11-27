// Device Address (Modbus)
#define ID_SENSOR 1     

// 7 in 1 Register Address (Modbus)
#define _MOISTURE 0x0000
#define _TEMP     0x0001
#define _EC       0x0002
#define _PH       0x0003
#define _Nit      0x0004
#define _Pho      0x0005
#define _Pot      0x0006

uint16_t const Address[7] = {
  _MOISTURE,
  _TEMP,
  _EC,
  _PH,
  _Nit,
  _Pho,
  _Pot
};
