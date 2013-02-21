class D2xxSerial {

public:
	D2xxSerial() {
		opened = false;
	}
	
	~D2xxSerial() {
		close();
	}
	
	
	static const char *getError(FT_STATUS err) {
		switch(err) {
			case FT_INVALID_HANDLE: return "FT_INVALID_HANDLE";
			case FT_DEVICE_NOT_FOUND: return "FT_DEVICE_NOT_FOUND";
			case FT_DEVICE_NOT_OPENED: return "FT_DEVICE_NOT_OPENED";
			case FT_IO_ERROR: return "FT_IO_ERROR";
			case FT_INSUFFICIENT_RESOURCES: return "FT_INSUFFICIENT_RESOURCES";
			case FT_INVALID_PARAMETER: return "FT_INVALID_PARAMETER";
			case FT_INVALID_BAUD_RATE: return "FT_INVALID_BAUD_RATE";
			case FT_DEVICE_NOT_OPENED_FOR_ERASE: return "FT_DEVICE_NOT_OPENED_FOR_ERASE";
			case FT_DEVICE_NOT_OPENED_FOR_WRITE: return "FT_DEVICE_NOT_OPENED_FOR_WRITE";
			case FT_FAILED_TO_WRITE_DEVICE: return "FT_FAILED_TO_WRITE_DEVICE";
			case FT_EEPROM_READ_FAILED: return "FT_EEPROM_READ_FAILED";
			case FT_EEPROM_WRITE_FAILED: return "FT_EEPROM_WRITE_FAILED";
			case FT_EEPROM_ERASE_FAILED: return "FT_EEPROM_ERASE_FAILED";
			case FT_EEPROM_NOT_PRESENT: return "FT_EEPROM_NOT_PRESENT";
			case FT_EEPROM_NOT_PROGRAMMED: return "FT_EEPROM_NOT_PROGRAMMED";
			case FT_INVALID_ARGS: return "FT_INVALID_ARGS";
			case FT_NOT_SUPPORTED: return "FT_NOT_SUPPORTED";
			case FT_OTHER_ERROR: return "FT_OTHER_ERROR";
			case FT_OK: return "FT_OK";
			default: return "unknown error";
		}
	}
	
	static int getNumDevices() {
	
		FT_STATUS ftStatus;
		DWORD numDevs;
		ftStatus = FT_ListDevices(&numDevs,NULL,FT_LIST_NUMBER_ONLY);
		if (ftStatus == FT_OK) {
			// FT_ListDevices OK, number of devices connected is in numDevs
			
			return numDevs;
		} else {
			// FT_
			printf("Couldn't list devices\n");
			return 0;
		}
	}
	

	
	static vector<string> getDeviceNames() {

		vector<string> devices;
		FT_STATUS ftStatus;
		DWORD numDevs = getNumDevices();
		
		for(int i = 0; i < numDevs; i++) {
			DWORD devIndex = i;
			char Buffer[16];
			ftStatus = FT_ListDevices((PVOID)devIndex,Buffer,FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
			if (FT_SUCCESS(ftStatus)) {
				// FT_ListDevices OK, serial number is in Buffer

				devices.push_back(string(Buffer));
			} else {
				devices.push_back("error");
				// FT_ListDevices failed
				printf("Couldn't get serial number of device [%d] - error: %s\n", i, getError(ftStatus));
			}	
		}	
		return devices;
		
	}
	bool listDevices() {
		vector<string> devices = getDeviceNames();
		printf("Found %d device%s\n", (int)devices.size(), devices.size()==1?"":"s");


		for(int i = 0; i < devices.size(); i++) {
			printf("[%d] = '%s'\n", i, devices[i].c_str());
		}		
	}
	

	bool close() {
		return !opened || FT_Close(handle)==FT_OK;		
	}
	
	bool setBaudRate(int baudRate) {
		FT_STATUS err = FT_SetBaudRate(handle, baudRate);
		if(err!=FT_OK) {
			printf("Error setting baud rate: %s\n", getError(err));
			return false;
		}
		printf("%d baud rate\n", baudRate);
		
		return true;
	}
	
	bool open(int which = 0, int baudRate = 9600) {

		FT_STATUS err = FT_Open(0,&handle);
		if (err != FT_OK) {
			// FT_OpenEx failed
			printf("Error connecting to [%d]: %s\n", which, getError(err));
			return false;
 		}
 
		setBaudRate(baudRate);
		return true;
	}
	
	bool open(string who, int baudRate = 9600) {
		FT_STATUS err = FT_OpenEx((void*)who.c_str(),FT_OPEN_BY_SERIAL_NUMBER,&handle);
		if (err != FT_OK) {
			// FT_OpenEx failed
			printf("Error connecting to '%s': %s\n", who.c_str(), getError(err));
			return false;
		}
		setBaudRate(baudRate);
		return true;
	}
	
	int available() {
		
		DWORD EventDWord;
		DWORD RxBytes;
		DWORD TxBytes;
		DWORD BytesReceived;
		FT_GetStatus(handle,&RxBytes,&TxBytes,&EventDWord);
		return RxBytes;
	}
	int read(unsigned char *buffer, int length) {
		DWORD BytesReceived = 0;
		FT_STATUS err = FT_Read(handle,buffer, length,&BytesReceived);
		if (err != FT_OK) {
			printf("Error reading: %s\n", getError(err));
			return 0;
		}
		return BytesReceived;

	}
	
	int write(unsigned char *buffer, int length) {
		DWORD outCount = 0;
		FT_STATUS err = FT_Write (handle, buffer, length, &outCount);
		if (err != FT_OK) {
			printf("Error writing: %s\n", getError(err));
			return 0;
		}
		return outCount;
	}

	int read() {
		unsigned char buffer[1];
		if(read(buffer, 1)) {
			return buffer[0];
		} else {
			return -1;
		}
	}
	
	// don't think this works
	void flush() {
		while(available()>0) {
			unsigned char inBuff[64];
			read(inBuff, 64);
		}
	}
	
private:
	bool opened;	
	FT_HANDLE handle;
	
};