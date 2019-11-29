/*
  Test.h - Test library for Wiring - implementation
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// includeAPI

// include this library's description file
#include "serOBD_simple.h"

// include description files for other libraries used (if any)
//LDudp obd_nlogger = LDudp();

// setup a serial buffer for me to used



// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances


// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries
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
    if (!verify_cmdresp()) {
      return 0;
    }
    snd_cmd(_echo_off); //echo off
    if (!verify_cmdresp()) {
      return 0;
    }
    snd_cmd(_hdrs_off); //headers off
    if (!verify_cmdresp()) {
      return 0;
    }
    snd_cmd(_lnfd_off); //linesfeeds off
    if (!verify_cmdresp()) {
      return 0;
    }
    snd_cmd(_space_off); //Space Characters off
    if (!verify_cmdresp()) {
      return 0;
    }
    snd_cmd( _set_adaptive_timing); //adaptive timing (timeout)
    if (!verify_cmdresp()) {
      return 0;
    }
    if (!set_proto()) {
      return 0;
    }
    return 1;
}





// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library

uint8_t serOBD_simple::set_proto(void) {
    snd_cmd(_ford_protocol); //Try the ford protocol
    if (!verify_cmdresp()) {
      return 0;
    }
    snd_PID("0100"); //Try the ford protocol
    if (verify_cmdresp("NO DATA")) {
      f_log("Wrong Protocol Switching to Auto Detect", TG_WARN_STR, __func__);
      snd_cmd("ATTP0"); //Auto Protocol Detection
      if (!verify_cmdresp()) {
        return 0;
      }
      snd_PID("0100"); //Try the auto dect function out this causes some "Searching...." type of thing before the response
      //TODO: Pause here without blocking somehow
      delay(2000); //Give OBD bus auto detect time to work
      if (verify_cmdresp("SEARCH")) {
        return 1
      } else {
        return 0
      }
    }
}

int serOBD_simple::get_response(const char c_from[], char msg[]) {
  //Log.notice("Serial Has Overrun?: %d" CR, Serial.hasOverrun());
  //Log.notice("Serial Has READ ERRROR?: %d" CR, Serial.hasRxError());
  char local_response[32];
  String calling(c_from);
  int rlen = _stream->readBytesUntil('>', local_response,32);
  strcpy(_response, local_response);
  if (calling.indexOf("getPID") >=0) {
    f_log("Response Value", TG_NOTICE_STR, c_from);
  } else {
    f_log("Response Value", TG_WARN_STR, c_from);
  }
  return rlen;
  
}
void serOBD_simple::buff_resp(){
  pidreadcounter += 1;
  for(int i=2; i<readlen-2; i++){
    _readings.push(_response[i]);
  }
  _readings.push('~');
}

uint8_t serOBD_simple::snd_PID(char gpid[]) {
  _stream->println(gpid);  // Send MAF PID request 0110
  if (verify_pidresp()) {
    buff_resp();
    return 1;
  }
  return 0;
}

void serOBD_simple::flush_buffer(void){
  f_log("Flush Buffer", TG_NOTICE, __func__);
  Log.warning("Pid Read Counter: %d" CR, pidreadcounter);
    while (!_readings.isEmpty()) {
      f_log(_readings.shift(), TG_WAR_NOCR)
    }
    f_log(" BUFFER END", TG_WARN_STR, __func__)
    Serial.println(" BUFFER_END");
    pidreadcounter = 0;
}

void f_log(char err_str[], tgerr_type etype, char c_from[]){
  switch(etype) {
    case (TG_NOTICE):Log.notice(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s--->>%s!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" CR), c_from, err_str) ;
    case (TG_WARN):Log.warning(F("###############################################%s--->>%s################################################" CR), c_from, err_str);
    case (TG_ERROR):Log.error(F("************************************************%s--->>%s************************************************" CR), c_from, err_str) ;
    case (TG_NOT_NOCR):Log.notice(F("%s"), c_from, err_str) ;
    case (TG_WAR_NOCR):Log.warning(F("%s"), c_from, err_str) ;
    case (TG_ERR_NOCR):Log.error(F("%s"), c_from, err_str) ;
  }
  
}
uint8_t serOBD_simple::snd_cmd(char cmd[]) {
  _stream->println(cmd);
}

uint8_t serOBD_simple::verify_cmdresp(char chkval[]) {
  int readlen = get_response(__func__, "CMD read response");
  String compstr=chkval;
  String readingstr(_response);
  if (readingstr.indexOf(compstr) >=0) {
    f_log("Command Read ERROR", TG_ERROR, __func__);
    return 0;
  }
  return 1
}

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