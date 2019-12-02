/*
  serOBD_simple.h - library for simple OBD shit - implementation
  started from Test.cpp example
  Copyright (c) 2019 Trip-g.com LLC.  All right reserved.
  tg@trip-g.com
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
	TG_WARN_STR,  
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

    void begin(uint32_t baudRate, Logging &logger){
      //Log.begin(LOG_LEVEL_VERBOSE, &log_device);
      //Log.begin(LOG_LEVEL_WARNING, &log_device);
      //Log.begin(LOG_LEVEL_SILENT, &log_device);
      _logger = &logger;
      _logger->warning("OBD Class Logging begin completed" CR);    
}
    void f_log(char err_str[], tgerr_type etype,const char c_from[]="Oh FUCK");
    uint8_t getPID(char gpid[]);
    void flush_buffer(void);
    uint8_t elm_setup(void);
    uint8_t snd_PID(const char gpid[], uint8_t bresp = 1);
    uint8_t get_hungbit(void);
    int get_err_rate(void);
    uint8_t clr_err_rate(void);
    int get_mafbuf_size(void);
    int get_spdbuf_size(void);
    long get_mafbuf_data(void);
    int get_spdbuf_data(void);
  // library-accessible "private" interface
  private:
    //int value;
    //HardwareSerial* hwStream;
    //SoftwareSerial* swStream;
    Stream* _stream;
    Logging* _logger;
    char const* _reset = "ATZ";
    char const* _echo_off = "ATE0";
    char const* _hdrs_off = "ATH0";
    char const* _lnfd_off = "ATL0";
    char const* _space_off = "ATS0";
    char const* _set_adaptive_timing = "ATAT2";
    char const* _ford_protocol = "ATTP1";
    char const* _connection_test = "0100";
    const float MPHfactor = 0.6213711922; //used to convert km/h to mph
    char _response[32];
    int _pidreadcounter = 0;
    uint8_t _setup_complete=0;
    uint8_t _obd_hung = 0;
    //CircularBuffer<char,150> _readings;
    CircularBuffer<float,30> _mafgps;
    CircularBuffer<int,30> _spd;
    uint8_t set_proto(void);
    uint8_t snd_cmd(const char cmd[]);
    uint8_t verify_resp(char chkval[]="OK", char read_type[]="Command", uint8_t match=0);
    //uint8_t verify_pidresp();
    void buff_resp(uint8_t rdlen);
    int get_response(const char c_from[] = "Oh FUCK", char msg[] = "Not Provided");
    float calc_gpers();
    int calc_spd();
    int _pidzero_err_cnt = 0;
    int _err_counter = 0;
};

#endif

