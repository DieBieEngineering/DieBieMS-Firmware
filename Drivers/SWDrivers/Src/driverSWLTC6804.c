#include "driverSWLTC6804.h"

uint8_t driverSWLTC6804TotalNumerOfICs = 0;
driverLTC6804ConfigStructTypedef driverSWLTC6804ConfigStruct;

uint16_t voltageUnder;
uint16_t voltageOver;
uint32_t voltageLimitReg;

void driverSWLTC6804DelayMS(uint32_t delayMS) {
	uint32_t currentTick = HAL_GetTick();
	while(!modDelayTick1ms(&currentTick,delayMS)){};
}

void driverSWLTC6804Init(driverLTC6804ConfigStructTypedef configStruct, uint8_t totalNumberOfLTCs) {		
	driverSWLTC6804ConfigStruct = configStruct;
	driverSWLTC6804TotalNumerOfICs = totalNumberOfLTCs;
	
	uint8_t rxConfig [driverSWLTC6804TotalNumerOfICs][8];
	int8_t returnPEC = -1;
	uint8_t LTCScanCount = 0;
	
	driverHWSPI1Init(LTC6804_CS_GPIO_Port,LTC6804_CS_Pin);
	driverSWLTC6804WakeIC();
	
	while((LTCScanCount < 5) && (returnPEC == -1)){
	  returnPEC =	driverSWLTC6804ReadConfigRegister(driverSWLTC6804TotalNumerOfICs,rxConfig);
		driverSWLTC6804WriteConfigRegister(driverSWLTC6804TotalNumerOfICs);
		LTCScanCount++;
	}
}

void driverSWLTC6804ResetCellVoltageRegisters(void) {
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  cmd[0] = 0x07;
  cmd[1] = 0x11;
  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec );
	
	driverSWLTC6804WakeIC();
  driverSWLTC6804Write(cmd,4);
}

void driverSWLTC6804ResetAuxRegisters(void) {
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  cmd[0] = 0x07;
  cmd[1] = 0x12;
  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec );
	
	driverSWLTC6804WakeIC();
  driverSWLTC6804Write(cmd,4);
}

void driverSWLTC6804ResetStatusRegisters(void) {
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  cmd[0] = 0x07;
  cmd[1] = 0x13;
  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec );
	
	driverSWLTC6804WakeIC();
  driverSWLTC6804Write(cmd,4);
}

void driverSWLTC6804ResetAuxVoltageRegisters(void) {
  uint8_t cmd[4];
  uint16_t cmd_pec;

  cmd[0] = 0x07;
  cmd[1] = 0x12;
  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
  driverSWLTC6804WakeIC();     //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  driverSWLTC6804Write(cmd,4);
}

void driverSWLTC6804StartCellVoltageConversion(uint8_t MD,uint8_t DCP, uint8_t CH) {	
  uint8_t cmd[4];
  uint16_t cmd_pec;
	uint8_t ADCV[2]; //!< Cell Voltage conversion command.
	
  ADCV[0] = ((MD & 0x02) >> 1) + 0x02;
  ADCV[1] = ((MD & 0x01) << 7) + 0x60 + (DCP<<4) + CH;

  cmd[0] = ADCV[0];
  cmd[1] = ADCV[1];
  cmd_pec = driverSWLTC6804CalcPEC15(2, ADCV);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
	driverSWLTC6804WakeIC();
  driverSWLTC6804Write(cmd,4);
}

void driverSWLTC6804StartLoadedCellVoltageConversion(uint8_t MD,uint8_t DCP, uint8_t CH,uint8_t PUP) {	
  uint8_t cmd[4];
  uint16_t cmd_pec;
	uint8_t ADCV[2]; //!< Cell Voltage conversion command.
	
  ADCV[0] = ((MD & 0x02) >> 1) + 0x02;
  ADCV[1] = ((MD & 0x01) << 7) + 0x28 + (DCP<<4) + CH + (PUP<<6);

  cmd[0] = ADCV[0];
  cmd[1] = ADCV[1];
  cmd_pec = driverSWLTC6804CalcPEC15(2, ADCV);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
	driverSWLTC6804WakeIC();
  driverSWLTC6804Write(cmd,4);
}

void driverSWLTC6804StartAuxVoltageConversion(uint8_t MD, uint8_t CHG) {
  uint8_t cmd[4];
  uint16_t cmd_pec;
	uint8_t ADAX[2]; //!< GPIO conversion command.
	
  ADAX[0] = ((MD & 0x02) >> 1) + 0x04;
  ADAX[1] = ((MD & 0x01) << 7) + 0x60 + CHG ;

  cmd[0] = ADAX[0];
  cmd[1] = ADAX[1];
  cmd_pec = driverSWLTC6804CalcPEC15(2, ADAX);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);

	driverSWLTC6804WakeIC();
  driverSWLTC6804Write(cmd,4);
}

bool driverSWLTC6804ReadCellVoltages(cellMonitorCellsTypedef *cellVoltages) {
	bool dataValid = true;
	static uint16_t cellVoltageCodes[1][12]; 
	
	driverSWLTC6804ReadCellVoltageRegisters(CELL_CH_ALL,driverSWLTC6804TotalNumerOfICs,cellVoltageCodes);
	
	if(driverSWLTC6804TotalNumerOfICs == 1) {
		uint8_t cellPointer;
		for(cellPointer = 0; cellPointer < 12; cellPointer++){
			if(cellVoltageCodes[0][cellPointer]*0.0001f < 10.0f)
			  cellVoltages[cellPointer].cellVoltage = cellVoltageCodes[0][cellPointer]*0.0001f;
			else
				dataValid = false;
			cellVoltages[cellPointer].cellNumber = cellPointer;
		}
	}else{
		dataValid = false;
	}
	
	return dataValid;
}

uint8_t driverSWLTC6804ReadCellVoltageRegisters(uint8_t reg, uint8_t total_ic, uint16_t cell_codes[][12] ) {
  const uint8_t NUM_RX_BYT = 8;
  const uint8_t BYT_IN_REG = 6;
  const uint8_t CELL_IN_REG = 3;
  
  uint8_t  *cell_data;
  int8_t  pec_error = 0;
  uint16_t parsed_cell;
  uint16_t received_pec;
  uint16_t data_pec;
  uint8_t data_counter=0; //data counter
  cell_data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));

  if (reg == 0) {
    for(uint8_t cell_reg = 1; cell_reg<5; cell_reg++) {        			 			                 //executes once for each of the LTC6804 cell voltage registers
      data_counter = 0;
      driverSWLTC6804ReadCellVoltageGroups(cell_reg, total_ic,cell_data );								                     //Reads a single Cell voltage register
      for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) { 			           // executes for every LTC6804 in the daisy chain current_ic is used as the IC counter
		    for(uint8_t current_cell = 0; current_cell<CELL_IN_REG; current_cell++) {	 	       // This loop parses the read back data into cell voltages, it loops once for each of the 3 cell voltage codes in the register 
          parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);      //Each cell code is received as two bytes and is combined to create the parsed cell voltage code													 
          cell_codes[current_ic][current_cell  + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
          data_counter = data_counter + 2;											                           //Because cell voltage codes are two bytes the data counter must increment by two for each parsed cell code
        }
        received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter+1];         //The received PEC for the current_ic is transmitted as the 7th and 8th after the 6 cell voltage data bytes
        data_pec = driverSWLTC6804CalcPEC15(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
        if(received_pec != data_pec) {
          pec_error = -1;															                                     //The pec_error variable is simply set negative if any PEC errors are detected in the serial data
        }
        data_counter=data_counter+2;												                               //Because the transmitted PEC code is 2 bytes long the data_counter must be incremented by 2 bytes to point to the next ICs cell voltage data
      }
    }
  }else{
    driverSWLTC6804ReadCellVoltageGroups(reg, total_ic,cell_data);
    for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) { 				           // executes for every LTC6804 in the daisy chain current_ic is used as the IC counter
		  for(uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++) {          // This loop parses the read back data into cell voltages, it loops once for each of the 3 cell voltage codes in the register 
			  parsed_cell = cell_data[data_counter] + (cell_data[data_counter+1]<<8);            //Each cell code is received as two bytes and is combined to create the parsed cell voltage code													
			  cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)] = 0x0000FFFF & parsed_cell;
			  data_counter= data_counter + 2;     									                             //Because cell voltage codes are two bytes the data counter must increment by two for each parsed cell code
		  }
	    received_pec = (cell_data[data_counter] << 8 )+ cell_data[data_counter + 1];         //The received PEC for the current_ic is transmitted as the 7th and 8th after the 6 cell voltage data bytes
      data_pec = driverSWLTC6804CalcPEC15(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
		  
			if(received_pec != data_pec) {
			  pec_error = -1;															                                       //The pec_error variable is simply set negative if any PEC errors are detected in the serial data
		  }
		  data_counter= data_counter + 2; 											                               //Because the transmitted PEC code is 2 bytes long the data_counter must be incremented by 2 bytes to point to the next ICs cell voltage data
	  }
  }

  free(cell_data);
  return(pec_error);
}

void driverSWLTC6804ReadCellVoltageGroups(uint8_t reg, uint8_t total_ic, uint8_t *data ) {
  const uint8_t REG_LEN = 8; //number of bytes in each ICs register + 2 bytes for the PEC
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  if (reg == 1) {      //1: RDCVA
    cmd[1] = 0x04;
    cmd[0] = 0x00;
  }else if(reg == 2) { //2: RDCVB
    cmd[1] = 0x06;
    cmd[0] = 0x00;
  }else if(reg == 3) { //3: RDCVC
    cmd[1] = 0x08;
    cmd[0] = 0x00;
  }else if(reg == 4) { //4: RDCVD
    cmd[1] = 0x0A;
    cmd[0] = 0x00;
  }

  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec); 
  
  driverSWLTC6804WakeIC(); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
	driverSWLTC6804WriteRead(cmd,4,data,(REG_LEN*total_ic));
}

bool driverSWLTC6804ReadVoltageFlags(uint16_t *underVoltageFlags, uint16_t *overVoltageFlags) {
	driverSWLTC6804StatusStructTypedef driverSWLTC6804StatusStruct[driverSWLTC6804TotalNumerOfICs];
	
	driverSWLTC6804ReadStatusValues(driverSWLTC6804TotalNumerOfICs,driverSWLTC6804StatusStruct);
	*underVoltageFlags = voltageUnder = driverSWLTC6804StatusStruct[0].underVoltage;
	*overVoltageFlags  = voltageOver = driverSWLTC6804StatusStruct[0].overVoltage;
  return false;
}

uint8_t driverSWLTC6804ReadStatusValues(uint8_t total_ic, driverSWLTC6804StatusStructTypedef statusArray[]) {
  const uint8_t NUM_RX_BYT = 8;
  const uint8_t BYT_IN_REG = 6;
  
	uint32_t registersCombined;
  uint32_t registersCombinedTemp;
  int8_t  pec_error = 0;
  uint16_t received_pec;
  uint16_t data_pec;
  uint8_t data_counter = 0; //data counter
  uint8_t *status_data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));
	
	driverSWLTC6804ReadStatusGroups(1,total_ic,status_data);
	data_counter = 0;
	
	for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) {
		// Extract DATA
		statusArray[current_ic].sumOfCells          = ((status_data[data_counter+1] << 8) | status_data[data_counter]) * 0.0001f;
		statusArray[current_ic].dieTemperature      = (((status_data[data_counter+3] << 8) | status_data[data_counter+2]) * 0.0001f)/0.0075f - 273.0f;
		statusArray[current_ic].voltageAnalogSupply = ((status_data[data_counter+5] << 8) | status_data[data_counter+4]) * 0.0001f;
		data_counter = data_counter + 6;
		// Calculate PEC
		received_pec = (status_data[data_counter] << 8) + status_data[data_counter+1];         //The received PEC for the current_ic is transmitted as the 7th and 8th after the 6 cell voltage data bytes
		data_pec = driverSWLTC6804CalcPEC15(BYT_IN_REG, &status_data[current_ic * NUM_RX_BYT]);
		if(received_pec != data_pec) {
			pec_error = -1;															                                     //The pec_error variable is simply set negative if any PEC errors are detected in the serial data
		}
	}
	
	driverSWLTC6804ReadStatusGroups(2,total_ic,status_data);
	data_counter = 0;
	
	for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) {
		// Reset vlag registers
		statusArray[current_ic].underVoltage = 0;
	  statusArray[current_ic].overVoltage  = 0;
		
		// Extract DATA
		statusArray[current_ic].voltageDigitalSupply = ((status_data[data_counter+1] << 8) | status_data[data_counter]) * 0.0001f;
		
		registersCombined = voltageLimitReg = (status_data[data_counter+4] << 16) | (status_data[data_counter+3] << 8) | status_data[data_counter+2];	// Combine all registers in one variable
		registersCombinedTemp = registersCombined & 0x00555555;																							// Filter out only the undervoltage bits
		
		for(int bitPointer = 0; bitPointer < driverSWLTC6804ConfigStruct.noOfCells; bitPointer++)
			statusArray[current_ic].underVoltage |= (registersCombinedTemp & (1 << bitPointer*2)) ? (1 << bitPointer) : 0;			// Shift undervoltage bits closer together and store them in *underVoltageFlags
		
		registersCombinedTemp = registersCombined & 0x00AAAAAA;																							// Filter out only the overvoltage bits
		registersCombinedTemp = registersCombinedTemp >> 1;																									// Move everything one bit to the right

		for(int bitPointer = 0; bitPointer < driverSWLTC6804ConfigStruct.noOfCells; bitPointer++)
			statusArray[current_ic].overVoltage |= (registersCombinedTemp & (1 << bitPointer*2)) ? (1 << bitPointer) : 0;				// And do the same for the overvoltage bits
		
		statusArray[current_ic].muxFail  = 0;		
		data_counter = data_counter + 6;
		// Calculate PEC
		received_pec = (status_data[data_counter] << 8) + status_data[data_counter+1];         //The received PEC for the current_ic is transmitted as the 7th and 8th after the 6 cell voltage data bytes
		data_pec = driverSWLTC6804CalcPEC15(BYT_IN_REG, &status_data[current_ic * NUM_RX_BYT]);
		if(received_pec != data_pec) {
			pec_error = -1;															                                     //The pec_error variable is simply set negative if any PEC errors are detected in the serial data
		}
	}
	
	free(status_data);
	return pec_error;
}

void driverSWLTC6804ReadStatusGroups(uint8_t reg, uint8_t total_ic, uint8_t *data ) {
  const uint8_t REG_LEN = 6; //number of bytes in each ICs register + 2 bytes for the PEC
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  if (reg == 1) {      //1: RDSTATA
    cmd[1] = 0x10;
    cmd[0] = 0x00;
  }else if(reg == 2) { //2: RDSTATB
    cmd[1] = 0x12;
    cmd[0] = 0x00;
  }

  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec); 
  
	driverSWLTC6804WriteRead(cmd,4,data,(REG_LEN*total_ic));
}

int8_t driverSWLTC6804ReadAuxVoltages(uint8_t reg, uint8_t total_ic, uint16_t aux_codes[][6]) {
  const uint8_t NUM_RX_BYT = 8;
  const uint8_t BYT_IN_REG = 6;
  const uint8_t GPIO_IN_REG = 3;
  
  uint8_t *data;
  uint8_t data_counter = 0; 
  int8_t pec_error = 0;
  uint16_t parsed_aux;
  uint16_t received_pec;
  uint16_t data_pec;
  data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));
  //1.a
  if (reg == 0) {
    for(uint8_t gpio_reg = 1; gpio_reg<3; gpio_reg++) {		 	   		 			                   //executes once for each of the LTC6804 aux voltage registers
      data_counter = 0;
      driverSWLTC6804ReadAuxGroups(gpio_reg, total_ic,data);									             //Reads the raw auxiliary register data into the data[] array
      for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) { 			           // executes for every LTC6804 in the daisy chain current_ic is used as the IC counter
		    for(uint8_t current_gpio = 0; current_gpio< GPIO_IN_REG; current_gpio++) {	       // This loop parses the read back data into GPIO voltages, it loops once for each of the 3 gpio voltage codes in the register 
		      parsed_aux = data[data_counter] + (data[data_counter+1]<<8);                     //Each gpio codes is received as two bytes and is combined to create the parsed gpio voltage code
          aux_codes[current_ic][current_gpio +((gpio_reg-1)*GPIO_IN_REG)] = parsed_aux;
          data_counter=data_counter+2;												                             //Because gpio voltage codes are two bytes the data counter must increment by two for each parsed gpio voltage code
        }
				
        received_pec = (data[data_counter]<<8)+ data[data_counter+1]; 				             //The received PEC for the current_ic is transmitted as the 7th and 8th after the 6 gpio voltage data bytes
        data_pec = driverSWLTC6804CalcPEC15(BYT_IN_REG, &data[current_ic*NUM_RX_BYT]);
        if(received_pec != data_pec) {
          pec_error = -1;															                                     //The pec_error variable is simply set negative if any PEC errors are detected in the received serial data
        }
        data_counter=data_counter+2;												                               //Because the transmitted PEC code is 2 bytes long the data_counter must be incremented by 2 bytes to point to the next ICs gpio voltage data
      }
    }
  }else{
    driverSWLTC6804ReadAuxGroups(reg, total_ic, data);
    for (int current_ic = 0 ; current_ic < total_ic; current_ic++) { 			  		           // executes for every LTC6804 in the daisy chain current_ic is used as an IC counter
	
		  for(int current_gpio = 0; current_gpio<GPIO_IN_REG; current_gpio++) {  	 	           // This loop parses the read back data. Loops once for each aux voltage in the register 
			  parsed_aux = (data[data_counter] + (data[data_counter+1]<<8));    		             //Each gpio codes is received as two bytes and is combined to create the parsed gpio voltage code
			  aux_codes[current_ic][current_gpio +((reg-1)*GPIO_IN_REG)] = parsed_aux;
			  data_counter=data_counter+2;									 		                                 //Because gpio voltage codes are two bytes the data counter must increment by two for each parsed gpio voltage code
		  }
		  received_pec = (data[data_counter]<<8) + data[data_counter+1]; 				               //The received PEC for the current_ic is transmitted as the 7th and 8th after the 6 gpio voltage data bytes
      data_pec = driverSWLTC6804CalcPEC15(BYT_IN_REG, &data[current_ic*NUM_RX_BYT]);
      
			if(received_pec != data_pec) {
        pec_error = -1;													   		                                     //The pec_error variable is simply set negative if any PEC errors  are detected in the received serial data
      } 
		  
			data_counter=data_counter+2;												                                 //Because the transmitted PEC code is 2 bytes long the data_counter must be incremented by 2 bytes to point to the next ICs gpio voltage data
	  }
  }
  free(data);
  return (pec_error);
}

void driverSWLTC6804ReadAuxGroups(uint8_t reg, uint8_t total_ic, uint8_t *data) {
  const uint8_t REG_LEN = 8; // number of bytes in the register + 2 bytes for the PEC
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  //1
  if (reg == 1) {			//Read back auxiliary group A
    cmd[1] = 0x0C;
    cmd[0] = 0x00;
  }else if(reg == 2) {		//Read back auxiliary group B 
    cmd[1] = 0x0e;
    cmd[0] = 0x00;
  }else{					      //Read back auxiliary group A
     cmd[1] = 0x0C;		
     cmd[0] = 0x00;
  }

  cmd_pec = driverSWLTC6804CalcPEC15(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
  driverSWLTC6804WakeIC(); //This will guarantee that the LTC6804 isoSPI port is awake, this command can be removed.
	driverSWLTC6804WriteRead(cmd,4,data,(REG_LEN*total_ic));
}

void driverSWLTC6804WriteConfigRegister(uint8_t totalNumberOfLTCs) {
  const uint8_t BYTES_IN_REG = 6;
  const uint8_t CMD_LEN = 4+(8*totalNumberOfLTCs);
  uint8_t *cmd;
  uint16_t cfg_pec;
  uint8_t cmd_index; //command counter
	uint8_t tx_cfg[totalNumberOfLTCs][6];
	uint16_t VuV = driverSWLTC6804ConfigStruct.CellUnderVoltageLimit/(16*0.0001);
	uint16_t VoV = driverSWLTC6804ConfigStruct.CellOverVoltageLimit/(16*0.0001);
	
  for(int i = 0; i<totalNumberOfLTCs;i++) {
    tx_cfg[i][0] = (driverSWLTC6804ConfigStruct.GPIO5 << 7) | (driverSWLTC6804ConfigStruct.GPIO4 << 6) | (driverSWLTC6804ConfigStruct.GPIO3 << 5) | (driverSWLTC6804ConfigStruct.GPIO2 << 4) | (driverSWLTC6804ConfigStruct.GPIO1 << 3) | (driverSWLTC6804ConfigStruct.ReferenceON << 2) | (driverSWLTC6804ConfigStruct.ADCOption);
    tx_cfg[i][1] = (VuV & 0xFF) ;
    tx_cfg[i][2] = ((VoV & 0x0F) << 4) | (VuV >> 8) ;
    tx_cfg[i][3] = (VoV >> 4) ; 
    tx_cfg[i][4] = (driverSWLTC6804ConfigStruct.DisChargeEnableMask & 0xFF) ;
    tx_cfg[i][5] = ((driverSWLTC6804ConfigStruct.DischargeTimout & 0x0F) << 4) | (driverSWLTC6804ConfigStruct.DisChargeEnableMask >> 8) ;
  }
  
  cmd = (uint8_t *)malloc(CMD_LEN*sizeof(uint8_t));
  cmd[0] = 0x00; // config register command
  cmd[1] = 0x01; // config register command
  cmd[2] = 0x3d; // PEC
  cmd[3] = 0x6e; // PEC
  cmd_index = 4;
	
  for (uint8_t current_ic = totalNumberOfLTCs; current_ic > 0; current_ic--) { 			// executes for each LTC6804 in daisy chain, this loops starts with the last IC on the stack. The first configuration written is received by the last IC in the daisy chain																		
    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) { // executes for each of the 6 bytes in the CFGR register current_byte is the byte counter
      cmd[cmd_index] = tx_cfg[current_ic-1][current_byte]; 						//adding the config data to the array to be sent 
      cmd_index = cmd_index + 1;                
    }
    cfg_pec = (uint16_t)driverSWLTC6804CalcPEC15(BYTES_IN_REG, &tx_cfg[current_ic-1][0]);		// calculating the PEC for each ICs configuration register data
    cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
    cmd[cmd_index + 1] = (uint8_t)cfg_pec;
    cmd_index = cmd_index + 2;
  }

	driverSWLTC6804Write(cmd,CMD_LEN);
  free(cmd);
}

void driverSWLTC6804EnableBalanceResistors(uint16_t enableMask) {
	driverSWLTC6804ConfigStruct.DisChargeEnableMask = enableMask;
	
	if(driverSWLTC6804TotalNumerOfICs == 1) {
		driverSWLTC6804WriteConfigRegister(driverSWLTC6804TotalNumerOfICs);
	}
}

uint16_t driverSWLTC6804CalcPEC15(uint8_t len, uint8_t *data) {
	uint16_t remainder,addr;
	
	remainder = 16;//initialize the PEC
	for(uint8_t i = 0; i<len;i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address 
		remainder = (remainder<<8)^crc15Table[addr];
	}
	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}

int8_t driverSWLTC6804ReadConfigRegister(uint8_t total_ic, uint8_t r_config[][8]) {
  const uint8_t BYTES_IN_REG = 8;
  
  uint8_t cmd[4];
  uint8_t *rx_data;
  int8_t pec_error = 0; 
  uint16_t data_pec;
  uint16_t received_pec;
  
  rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));

  cmd[0] = 0x00;
  cmd[1] = 0x02;
  cmd[2] = 0x2b;
  cmd[3] = 0x0A;

	driverSWLTC6804WriteRead(cmd, 4, rx_data, (BYTES_IN_REG*total_ic));
 
  for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) { 			//executes for each LTC6804 in the daisy chain and packs the data into the r_config array as well as check the received Config data for any bit errors																
    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)	{
      r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
    }
    received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
    data_pec = driverSWLTC6804CalcPEC15(6, &r_config[current_ic][0]);
    if(received_pec != data_pec) {
      pec_error = -1;
    }  
  }
  free(rx_data);
  return(pec_error);
}

// Coupling of drivers
void driverSWLTC6804Write(uint8_t *writeBytes, uint8_t writeLength) {
	driverHWSPI1Write(writeBytes,writeLength,LTC6804_CS_GPIO_Port,LTC6804_CS_Pin);
};

// Coupling of drivers
void driverSWLTC6804WriteRead(uint8_t *writeBytes, uint8_t writeLength, uint8_t *readBytes, uint8_t readLength) {
	driverHWSPI1WriteRead(writeBytes,writeLength,readBytes,readLength,LTC6804_CS_GPIO_Port,LTC6804_CS_Pin);
};

void driverSWLTC6804WakeIC(void){
	HAL_GPIO_WritePin(LTC6804_CS_GPIO_Port,LTC6804_CS_Pin,GPIO_PIN_RESET);
	driverSWLTC6804DelayMS(10);
	HAL_GPIO_WritePin(LTC6804_CS_GPIO_Port,LTC6804_CS_Pin,GPIO_PIN_SET);	
}




