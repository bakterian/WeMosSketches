
// ====================================================================
// INCLUDS
//
#include <FS.h>
#include <ESP.h>

// ====================================================================
// SETUP
//
void setup() {
  Serial.begin(115200);

  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();
  
  SPIFFS.begin();
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  
  Serial.printf("########## SPI FLASH INFOS ###########\n");
  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u\n", realSize);
  Serial.printf("Flash ide  size: %u\n", ideSize);
  Serial.printf("Flash ide speed: %u\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
  Serial.printf("######################################\n\n");
  
  Serial.printf("### SPI FLASH FILING SYSYTEM INFOS ###\n");
  Serial.printf("SPIFFS.totalBytes = %u\n", fs_info.totalBytes);
  Serial.printf("SPIFFS.usedBytes = %u\n", fs_info.usedBytes);
  Serial.printf("SPIFFS.blockSize = %u\n", fs_info.blockSize);
  Serial.printf("SPIFFS.pageSize = %u\n", fs_info.pageSize);
  Serial.printf("SPIFFS.maxOpenFiles = %u\n", fs_info.maxOpenFiles);
  Serial.printf("SPIFFS.maxPathLength = %u\n", fs_info.maxPathLength);
  Serial.printf("######################################\n\n");
}

// ================================================================
// LOOP
//
void loop() {
	while(1) {
		yield();
	}
}
