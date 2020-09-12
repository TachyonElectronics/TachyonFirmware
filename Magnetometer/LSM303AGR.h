

#ifndef LSM303AGR_H_
#define LSM303AGR_H_

#include "stdint.h"
#include "Vector3.h"

#define byte unsigned short

#define LSM303AGR_MAG_ADDRES 0b0011110
#define LSM303AGR_ACC_ADDRESS 0b0011001

#define LSM303AGR_OUTX_L_REG_M 0x68
#define LSM303AGR_OUT_X_L_A 0x28

#define MAG 0
#define ACC 1

#define AINC 0b10000000 //Auto increment flag

class LSM303AGR
{
  public:
	Vector3f m_max = Vector3f(INT32_MIN, INT32_MIN, INT32_MIN);
	Vector3f m_min = Vector3f(INT32_MAX, INT32_MAX, INT32_MAX);
	Vector3f offset = Vector3f(0, 0, 0);

	Vector3f mag;
	Vector3f acc;
	
	float interp_m = 0.5F;

	void writeRegMag(byte addr, byte data);
	void writeRegAcc(byte addr, byte data);

	void read3Axis(uint8_t sensor);

	void init();

	uint16_t getMagneticBearing();
};

#endif /* LSM303AGR_H_ */
