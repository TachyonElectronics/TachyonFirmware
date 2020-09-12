#include "LSM303AGR.h"
#include "Arduino.h"

#include "../FirmwareCore/TachyonFirmware.h"
#include <Wire.h>

extern struct __TachyonSettings settings;

void LSM303AGR::writeRegMag(byte addr, byte data)
{
	Wire.beginTransmission(LSM303AGR_MAG_ADDRES);
	Wire.write(addr);
	Wire.write(data);
	Wire.endTransmission();
}
void LSM303AGR::writeRegAcc(byte addr, byte data)
{

	Wire.beginTransmission(LSM303AGR_ACC_ADDRESS);
	Wire.write(addr);
	Wire.write(data);
	Wire.endTransmission();
}

void LSM303AGR::read3Axis(uint8_t sensor)
{
	uint8_t address = sensor ? LSM303AGR_ACC_ADDRESS : LSM303AGR_MAG_ADDRES;
	Wire.beginTransmission(address);
	Wire.write((sensor ? LSM303AGR_OUT_X_L_A : LSM303AGR_OUTX_L_REG_M) | AINC);
	Wire.endTransmission();
	Wire.requestFrom(address, (uint8_t)6);
	int x = Wire.read() | (Wire.read() << 8);
	int y = Wire.read() | (Wire.read() << 8);
	int z = Wire.read() | (Wire.read() << 8);
	*(&mag + sensor) = Vector3f(x,y,z);
}

void LSM303AGR::init()
{
	writeRegMag(0x60, 0b10000000); //Setup magnetometer
	writeRegMag(0x62, 0b00010000); //Enable BDU

	writeRegAcc(0x20, 0b00100111); //Set Acc to 10Hz mode and enable all axes
	writeRegAcc(0x23, 0b10001000); //Set Acc to HiRes mode, enable Block Data Update (BDU)
}

uint16_t LSM303AGR::getMagneticBearing()
{
	read3Axis(MAG);
	read3Axis(ACC);
	Vector3f nmag = mag - offset;
	nmag.Normalize();

	Vector3f nacc = acc;
	nacc.Normalize();

	Vector3f m_a_plane_vect = Vector3f::CrossProduct(nmag, nacc);
	m_a_plane_vect.Normalize();
	Vector3f perp_m_vect = Vector3f::CrossProduct(nacc, m_a_plane_vect);

	Vector3f front_vect = Vector3f(1, 0, 0);

	perp_m_vect.Normalize();

	return (atan2(Vector3f::DotProduct(m_a_plane_vect, front_vect), -Vector3f::DotProduct(perp_m_vect, front_vect)) + PI) * RAD_TO_DEG;
}
