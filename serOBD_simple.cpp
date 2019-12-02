/*
  serOBD_simple.h - library for simple OBD shit - implementation
  started from Test.cpp example
  Copyright (c) 2019 Trip-g.com LLC.  All right reserved.
  tg@trip-g.com
*/

// include this library's description file
#include "serOBD_simple.h"

// include description files for other libraries used (if any)


// Constructor /////////////////////////////////////////////////////////////////



// Public Methods //////////////////////////////////////////////////////////////

/*
void serOBD_simple::begin(uint32_t baudRate, HardwareSerial& log_device) {
    hwStream->begin(baudRate);
    //Log.begin(LOG_LEVEL_VERBOSE, &Serial);
    Log.begin(LOG_LEVEL_WARNING, log_device);
    //Log.begin(LOG_LEVEL_WARNING, &obd_nlogger);
    Log.warning("OBD Class Logging begin completed" CR);
    
}
*/

uint8_t serOBD_simple::elm_setup(void) {
    snd_cmd(_reset);
    if (!verify_resp("ELM327", "Command",1)) {
      return 0;
    }
    snd_cmd(_echo_off); //echo off
    if (!verify_resp("OK", "Command",1)) {
      return 0;
    }
    snd_cmd(_hdrs_off); //headers off
    if (!verify_resp("OK", "Command",1)) {
      return 0;
    }
    snd_cmd(_lnfd_off); //linesfeeds off
    if (!verify_resp("OK", "Command",1)) {
      return 0;
    }
    snd_cmd(_space_off); //Space Characters off
    if (!verify_resp("OK", "Command",1)) {
      return 0;
    }
    snd_cmd( _set_adaptive_timing); //adaptive timing (timeout)
    if (!verify_resp("OK", "Command",1)) {
      return 0;
    }
    if (!set_proto()) {
      return 0;
    }
    _setup_complete=1;
    return 1;
}





// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library

uint8_t serOBD_simple::set_proto(void) {
    snd_cmd(_ford_protocol); //Try the ford protocol
    if (!verify_resp("OK", "Command",1)) {
      return 0;
    }
    snd_PID(_connection_test,0); //Try 0100 pid with this protocol
    if (verify_resp("NO DATA", "PID",1)) {
      f_log("Wrong Protocol Switching to Auto Detect", TG_WARN_STR, __func__);
      snd_cmd("ATTP0"); //Auto Protocol Detection
      if (!verify_resp("OK", "Command",1)) {
        return 0;
      }
      snd_PID(_connection_test,0); //Try the auto dect function out this causes some "Searching...." type of thing before the response
      //TODO: Pause here without blocking somehow
      delay(2000); //Give OBD bus auto detect time to work
      if (verify_resp("SEARCH", "PID",1)) {
        return 1;
      } else {
        return 0;
      }
    }
}


uint8_t serOBD_simple::get_hungbit(void){
  return _obd_hung;
}

int serOBD_simple::get_err_rate(void){
  return _err_counter;
}

uint8_t serOBD_simple::clr_err_rate(void){
  _err_counter = 0;
  return 1;
}

int serOBD_simple::get_mafbuf_size(void){
  return _mafgps.size();
}

int serOBD_simple::get_spdbuf_size(void){
  return _spd.size();
}

long serOBD_simple::get_mafbuf_data(void){
  return _mafgps.shift();

}
int serOBD_simple::get_spdbuf_data(void){
  return _spd.shift();
}

void serOBD_simple::flush_buffer(void){
  if (_pidreadcounter > 5) {
    f_log("Flush Buffer", TG_NOTICE_STR, __func__);
    _logger->warning("Pid Read Counter: %d" CR, _pidreadcounter);
      while (!_mafgps.isEmpty()) {
        //f_log(_readings.shift(), TG_WAR_NOCR, __func__);
        Serial.print(_mafgps.shift());
        //_mafgps.shift();
      }
      Serial.println(" MAF BUFFER_END");
      while (!_spd.isEmpty()){
        Serial.print(_spd.shift());
      }
      f_log(" BUFFER END", TG_WARN_STR, __func__);
      Serial.println(" SPD BUFFER_END");
  } else {
    _pidzero_err_cnt += 1;
  }
  if (_pidzero_err_cnt > 5) {
    f_log("FAILURE NEED TO RESET CPU", TG_WARN_STR, __func__);
    _obd_hung = 1;
  }
    _pidreadcounter = 0;
}

void serOBD_simple::f_log(char err_str[], tgerr_type etype,const char c_from[]){
  switch(etype) {
    case (TG_NOTICE_STR): _logger->notice(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s--->>%s!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" CR), c_from, err_str) ; 
      break;
    case (TG_WARN_STR):_logger->warning(F("###############################################%s--->>%s################################################" CR), c_from, err_str);
      break;
    case (TG_ERROR_STR):_logger->error(F("************************************************%s--->>%s************************************************" CR), c_from, err_str) ;
      break;
    case (TG_NOT_NOCR):_logger->notice(F("%s"), c_from, err_str) ;
      break;
    case (TG_WAR_NOCR):_logger->warning(F("%s"), c_from, err_str) ;
      break;
    case (TG_ERR_NOCR):_logger->error(F("%s"), c_from, err_str) ;
      break;
    default:_logger->error(F("f_log Fucked Up missed all case statements!!!" CR) );
  }
  
}
uint8_t serOBD_simple::snd_cmd(const char cmd[]) {
  f_log((char*)cmd, TG_NOTICE_STR, __func__);
  _stream->println(cmd);
}

uint8_t serOBD_simple::snd_PID(const char gpid[], uint8_t bresp) {
  f_log((char*)gpid, TG_NOTICE_STR, __func__);
  _stream->println(gpid);  
  if (bresp) {
    uint8_t rlen = verify_resp("STOPPED", "PID", 0);
    if (rlen > 0) {
      f_log("Buffering Response", TG_NOTICE_STR, __func__);
      buff_resp(rlen);
      return 1;
    }
    return 0;
  }
  return 1;
}
void serOBD_simple::buff_resp(uint8_t rdlen){
  _pidreadcounter += 1;
  char lresponse[4];
  if (_response[2] == '1' && _response[3] == '0'){ //chars 2&3 are 10 then we have MAF
    _mafgps.push(calc_gpers());
  } else if (_response[2] == '0' && _response[3] == 'D') { //chars 2&3 are 0D then we have MAF
    f_log("Is SPD", TG_NOTICE_STR, __func__);
    int spd = calc_spd(); 
    _spd.push((int)spd);
  }

}

int serOBD_simple::calc_spd() {
  char *pEnd = NULL;
  char tmpstr[4];
  sprintf(tmpstr, "0x%c%c", _response[4], _response[5]);
  long int a = strtol(tmpstr, &pEnd, 0);
  return a * MPHfactor;
}

float serOBD_simple::calc_gpers() {
  char *pEnd = NULL;
  char tmpstr[4];
  sprintf(tmpstr, "0x%c%c", _response[4], _response[5]);
  long int a = strtol(tmpstr, &pEnd, 0);
  sprintf(tmpstr, "0x%c%c", _response[6], _response[7]);
  long int b = strtol(tmpstr, &pEnd, 0);
  float gps = (((256*a)+b)/100);
  return gps;
}

//Is matching the chkval good?  match = 1, If matching chkval is bad... match=0
uint8_t serOBD_simple::verify_resp(char chkval[], char read_type[], uint8_t match) {  
  uint8_t readlen = get_response(__func__, read_type);
  char e_msg[32];
  String compstr=chkval;
  String readingstr(_response);
  if (readingstr.indexOf(compstr) >=0 && match) { // if we match the chckval and we want to then return readlen
      f_log("Returning rlen", TG_NOTICE_STR, __func__);
      return readlen;
    } else if (readingstr.indexOf(compstr) >=0 && !match){  //if we match the chckval and we don't want to return an error
      _err_counter += 1;
      sprintf(e_msg, "%s Read ERROR", read_type);
      f_log(e_msg, TG_ERROR_STR, __func__);
      return 0;
    } 
    return readlen;  //finally return the read len if all the above are ok
}
int serOBD_simple::get_response(const char c_from[], char msg[]) {
  //Log.notice("Serial Has Overrun?: %d" CR, Serial.hasOverrun());
  //Log.notice("Serial Has READ ERRROR?: %d" CR, Serial.hasRxError());
  char local_response[32];
  //if (_stream->peek() != -1) {
    int rlen = _stream->readBytesUntil('>', local_response,32);
    strcpy(_response, local_response);
    if (_setup_complete) {
      f_log(local_response, TG_NOTICE_STR, __func__);
    } else {
      f_log(local_response, TG_WARN_STR, __func__);
    }
    f_log("Returning rlen", TG_NOTICE_STR, __func__);
    return rlen;
  //}
  return 0;
  
}
/*
uint8_t serOBD_simple::verify_pidresp() {
  int readlen = get_response(__func__, "PID read response");
  String compstr="STOPPED";
  String readingstr(_response);
  if (readingstr.indexOf(compstr) >=0) {
    f_log("Data Read ERROR", TG_ERROR, __func__)
    return 0;
  }
  return 1
}
*/