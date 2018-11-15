#ifndef SX1508_H
#define SX1508_H

#include <Arduino.h>
#include "E32-TTL-100.h"

#define M0_PIN	23
#define M1_PIN	19
#define AUX_PIN	33
#define SOFT_RX	5
#define SOFT_TX 4

class SX1508
{
public:
    SX1508();
    ~SX1508();

    void begin();
    RET_STATUS SleepModeCmd(uint8_t CMD, void* pBuff);
    RET_STATUS SettingModule(struct CFGstruct *pCFG);
    void SwitchMode(MODE_TYPE mode);
    RET_STATUS WaitAUX_H();
    RET_STATUS SendMsg(uint8_t mode, uint8_t id);
    RET_STATUS ReceiveMsg(uint8_t *pdatabuf, uint8_t *data_len);
private:
    bool ReadAUX();
    void Reset_module();
    RET_STATUS Read_module_version(struct MVerstruct* MVer);
    RET_STATUS Read_CFG(struct CFGstruct* pCFG);
    RET_STATUS Write_CFG_PDS(struct CFGstruct* pCFG);
    RET_STATUS Module_info(uint8_t* pReadbuf, uint8_t buf_len);
    void triple_cmd(SLEEP_MODE_CMD_TYPE Tcmd);
    void cleanUARTBuf();
    bool chkModeSame(MODE_TYPE mode);


    bool AUX_HL;
};




#endif