/*
 * ToF.c
 *
 *  Created on: 14 mai 2020
 *      Author: Alexis DEVLEESCHAUWER
 */

#include "ToF.h"


//#define VL53L_ADDR 0xD0 << 1

/* Private variables ---------------------------------------------------------*/

extern I2C_HandleTypeDef I2CHANDLE;
extern uint16_t trigger;

uint16_t preMeasure = 0;
uint16_t triggered = 0;

bool ToF_init(VL53L0X_DEV device, bool debug){

	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	uint8_t VhvSettings;
	uint8_t PhaseCal;

	VL53L0X_Error Status;

	device->I2cDevAddr      = 0x29 << 1;
	device->comms_type      = 1;
	device->comms_speed_khz = 400;

	VL53L0X_DeviceInfo_t Info;
	VL53L0X_DeviceInfo_t *pInfo = &Info;

	VL53L0X_RangingMeasurementData_t measure;
	VL53L0X_RangingMeasurementData_t *pMeasure = &measure;

	Status = VL53L0X_DataInit(device);

	if (Status == VL53L0X_ERROR_NONE) {

	Status = VL53L0X_GetDeviceInfo(device, pInfo);
		if (debug){
			dbg_printfln("VL53L0X Info:");
			dbg_printfln("Device Name: %s", Info.Name);
			dbg_printfln("Type: %s", Info.Type);
			dbg_printfln("ID: %s", Info.ProductId);
			dbg_printfln("Rev Major: %u Minor: %u, %u\r\n", Info.ProductRevisionMajor, Info.ProductRevisionMinor, Info.ProductType);
		}

	}

	if (Status == VL53L0X_ERROR_NONE) {
		if (debug) {
			dbg_printfln("VL53L0X: StaticInit");
		}

		Status = VL53L0X_StaticInit(device);

	}

	if (Status == VL53L0X_ERROR_NONE) {
	        if (debug) {
	        	dbg_printfln("VL53L0X: PerformRefSpadManagement");
	        }

	        Status = VL53L0X_PerformRefSpadManagement(device, &refSpadCount, &isApertureSpads); // Device Initialization

	        if (debug) {
	        	dbg_printfln("refSpadCount = %lu, isApertureSpads %u", refSpadCount, isApertureSpads);
	        }
	    }

	if (Status == VL53L0X_ERROR_NONE) {
		if (debug) {
			dbg_printfln("VL53L0X: PerformRefCalibration");
		}

		Status = VL53L0X_PerformRefCalibration(device, &VhvSettings, &PhaseCal); // Device Initialization
	}

    if (Status == VL53L0X_ERROR_NONE) {
        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        if (debug) {
        	dbg_printfln("VL53L0X: SetDeviceMode");
        }

        Status = VL53L0X_SetDeviceMode(device, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
    }

    if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetLimitCheckEnable(device, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }

    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(device, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }

    if (Status == VL53L0X_ERROR_NONE) {
        Status = VL53L0X_SetLimitCheckEnable(device, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
    }

    if (Status == VL53L0X_ERROR_NONE) {
    	Status = VL53L0X_SetLimitCheckValue(device, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,(FixPoint1616_t) (1.5 * 0.023 * 65536));
    }

    if (Status == VL53L0X_ERROR_NONE) {
    	Status = VL53L0X_SetLimitCheckValue(device, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,(FixPoint1616_t) (1.5 * 0.023 * 65536));
    }

    VL53L0X_PerformSingleRangingMeasurement(device, pMeasure);

    trigger = pMeasure->RangeMilliMeter;

	if (debug){
		print_pal_error(Status);
	}


    if (Status == VL53L0X_ERROR_NONE) {
		return true;
    } else {

    	return false;
    }
}

VL53L0X_Error getSingleRanging(VL53L0X_DEV device, uint16_t* Measure, bool debug) {
	//dbg_printfln("\r\ngetSingleRanging() has started\r\n");

	VL53L0X_RangingMeasurementData_t measure;
	VL53L0X_RangingMeasurementData_t *pMeasure = &measure;

	VL53L0X_Error Status;
	FixPoint1616_t LimitCheckCurrent;

	Status = VL53L0X_PerformSingleRangingMeasurement(device, pMeasure);

	*Measure = pMeasure->RangeMilliMeter;

if (Status == VL53L0X_ERROR_NONE) {

		if (debug) {
			Status = VL53L0X_GetLimitCheckCurrent(device, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &LimitCheckCurrent);

			if (Status == VL53L0X_ERROR_NONE) {
				dbg_printfln("RANGE IGNORE THRESHOLD: %f", LimitCheckCurrent / 65536.0);
				print_pal_error(Status);
			}

			dbg_printfln("Measured distance : %d", pMeasure->RangeMilliMeter);
			print_pal_error(Status);
		}
	}

	return Status;
}


void smartLight(uint16_t Measure){
	//getSingleRanging(device, Measure, false);

	if (Measure == 0 || Measure == 8190){
		Measure = 0;
	}


	if (Measure > 0) {

		if (Measure > trigger + 25 || Measure < trigger - 25){
			if (triggered){

			} else {
				triggered = true;
				dbg_printfln("Tof : triggered");
				esp8266_print("red", "triggered");
			}

		} else {
			dbg_printfln("Distance (mm): %d", Measure);

			if (triggered){
				triggered = false;
			}
		}


	}



}

void print_range_status(VL53L0X_RangingMeasurementData_t* pRangingMeasurementData){
    char buf[VL53L0X_MAX_STRING_LENGTH];
    uint8_t RangeStatus;

    /*
     * New Range Status: data is valid when pRangingMeasurementData->RangeStatus = 0
     */

    RangeStatus = pRangingMeasurementData->RangeStatus;

    VL53L0X_GetRangeStatusString(RangeStatus, buf);
    dbg_printfln("Range Status: %i : %s", RangeStatus, buf);

}

void print_pal_error(VL53L0X_Error Status){
    char buf[VL53L0X_MAX_STRING_LENGTH];
    VL53L0X_GetPalErrorString(Status, buf);
    dbg_printfln("API Status: %i : %s", Status, buf);
}


/*
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t Device;
	VL53L0X_Dev_t *device = &Device;

	device->I2cDevAddr      = 0x29 << 1;
	device->comms_type      =  1;
	device->comms_speed_khz =  400;

	uint8_t byteData;
	uint16_t wordData;

	uint8_t ProductRevisionMajor, ProductRevisionMinor;

	uint8_t *pProductRevisionMajor = &ProductRevisionMajor;
	uint8_t *pProductRevisionMinor = &ProductRevisionMinor;

	dbg_printfln("Status : %u", Status);
	Status = VL53L0X_GetProductRevision(device, pProductRevisionMajor, pProductRevisionMinor);


	dbg_printfln("Status : %u", Status);

	dbg_printfln("Major : %u", ProductRevisionMajor);
	dbg_printfln("Minor : %u", ProductRevisionMinor);
 */
