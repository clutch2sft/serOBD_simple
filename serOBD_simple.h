/*
  serOBD_simple.h - library for simple OBD shit - implementation
  started from Test.cpp example
  Copyright (c) 2019 Trip-g.com LLC.  All right reserved.
*/

// ensure this library description is only included once
#ifndef serOBD_simple_h
#define serOBD_simple_h


// include types & constants
#include <CircularBuffer.h>
#include <ArduinoLog.h>
//#include <LDudp.h>


typedef enum
{
	TG_NOTICE_STR = 0,
	TG_WARN_ST,  
	TG_ERROR_STR, 
  TG_NOT_NOCR,
  TG_WAR_NOCR,
  TG_ERR_NOCR,
} tgerr_type;

// library interface description

class serOBD_simple
{
  // user-accessible "public" interface
  public:
    /*    
    serOBD_simple(HardwareSerial& device) {
      hwStream = &device; 
    }

    serOBD_simple( SoftwareSerial& device) {
      swStream = &device;
    }
    */    
    serOBD_simple(Stream &serialPort) {
      _stream = &serialPort;
    }

    void begin(uint32_t baudRate, Print  &log_device){
      //Log.begin(LOG_LEVEL_VERBOSE, &Serial);
      Log.begin(LOG_LEVEL_WARNING, &log_device);
      //Log.begin(LOG_LEVEL_WARNING, &obd_nlogger);
      Log.warning("OBD Class Logging begin completed" CR);    
}
    uint8_t getPID(char gpid[]);
    void flush_buffer(void);
    uint8_t elm_setup(void);
  
  // library-accessible "private" interface
  private:
    //int value;
    HardwareSerial* hwStream;
    //SoftwareSerial* swStream;
    Stream* _stream;
    const char _reset = 'ATZ';
    const char _echo_off = 'ATE0';
    const char _hdrs_off = 'ATH0';
    const char _lnfd_off = 'ATL0';
    const char _space_off = 'ATS0';
    const char _set_adaptive_timing = 'ATAT2';
    const char _ford_protocol = 'ATTP1';
    const char _connection_test = '0100';
    char _response[32];
    int pidreadcounter = 0;
    CircularBuffer<char,150> _readings;
    void f_log(char err_str[], tgerr_type etype, const char c_from[]="Oh FUCK"){
    void set_proto(void);
    uint8_t snd_cmd(char cmd[]);
    uint8_t verify_cmdresp(char chkval[]="OK");
    uint8_t verify_pidresp();
    void buff_resp();
    int get_response(const char c_from[] = "Oh FUCK", char msg[] = "Not Provided");
};

#endif
