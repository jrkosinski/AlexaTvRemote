#ifndef __WEMO_CALLBACK_HANDLER_H__
#define __WEMO_CALLBACK_HANDLER_H__

#include "Wemulator.h"
#include "debug.h"

/****************************************
 * WemoCallbackHandler
 * --------------------
 */
class WemoCallbackHandler : public IWemoCallbackHandler 
{
  private: 
    bool* _pCmdReceived;
    
  public: 
    WemoCallbackHandler(bool*); 
 
    virtual void handleCallback(int param); 
}; 
/****************************************/


// ************************************************************************************
// constructor; pass reference to a flag
// 
 WemoCallbackHandler::WemoCallbackHandler(bool* pCmdReceived)
{
  this->_pCmdReceived = pCmdReceived;
}

// ************************************************************************************
// handles the callback; param is ignored 
// 
void WemoCallbackHandler::handleCallback(int param)
{
  debugPrintln("got callback");
  *(this->_pCmdReceived) = true;
}

#endif 
