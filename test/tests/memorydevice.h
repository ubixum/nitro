#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

#include <map>
#include <iostream>

#include <cstdlib>

#include <nitro.h>

// simple device for testing device stuff
// only supports int data

class MemoryDevice : public Nitro::Device {
    private:
       // reg->val 
       // only supporting one terminal for test
       std::map<int,int> m_data; 
       bool m_err_mode;
       int m_err_step;
       int m_cur_step;
       int status;
       int checksum;

       int inc_error_step() {
         ++ m_cur_step;
         if (m_cur_step > m_err_step) 
            m_cur_step=0;
         return m_cur_step;
       }

    protected:
       int _transfer_status() { return status; }
       uint16 _transfer_checksum() { return checksum; };
       Nitro::DataType _get( uint32 term_addr, uint32 reg_addr, uint32 timeout ) {
            if (m_err_mode) {
                if (!inc_error_step()) {
                    status=1;
                    return (uint32) (rand() % 100);
                }
            }
            status=0;
            checksum = m_data[reg_addr];
            return m_data[reg_addr];
       }
       void _set(uint32 term_addr, uint32 reg_addr, const Nitro::DataType& val, uint32 timeout ) {
           if (m_err_mode) {
              if (!inc_error_step()) {
                status=1;
                m_data[reg_addr] = rand() % 100;
                return; // set didn't work.
              }
           }
           status=0;
           checksum=val;
           m_data[reg_addr] = (uint32)val; 
       }
       // read and write just happen to pack things in 16 bit data 
       void _read ( uint32 term_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout ) {
            if (m_err_mode) {
                if (!inc_error_step()) {
                    status=1; // read didn't work
                    return;
                }
            }
            uint16* data2 = (uint16*)data;
            checksum=0;
            for (uint32 i=0;i<length/2;++i) {
                data2[i] = (uint16)m_data[reg_addr++];
                checksum+=data2[i];
            }
        }
       void _write ( uint32 term_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) {
            if (m_err_mode) {
                if (!inc_error_step()) {
                    status=1; // write didn't work
                    return;
                }
            }
            const uint16* data2 = (const uint16*)data;
            checksum=0;
            for (uint32 i=0;i<length/2;++i) {
                m_data[reg_addr++] = data2[i];
                checksum += data2[i];
            }
        }
       void _close () throw() {}

    public:
       MemoryDevice() : m_err_mode(false), m_err_step(1), m_cur_step(0), status(0) {} 
       ~MemoryDevice() throw () {}
       void clear() { m_data.clear(); m_err_mode=false; }
       void debug ( ) {
         std::cout << "****" << std::endl;
         for (std::map<int,int>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr ) {
            std::cout << itr->first << "->" << itr->second << std::endl; 
         }
         std::cout << "****" << std::endl;
       }
       // every other status is bad.
       // every other get returns 0
       void set_error_mode(bool err) {
        m_err_mode=err;
       }
       void set_error_step(int step) {
        m_err_step=step;
       }
        
};

//// a device that doesn't do anything
//class BrokenDevice : public Nitro::Device {
//    protected:
//       Nitro::DataType _get( uint32 term_addr, uint32 reg_addr, uint32 timeout ) {
//        return 0;
//       }
//       void _set(uint32 term_addr, uint32 reg_addr, const Nitro::DataType& val, uint32 timeout ) {
//       }
//       void _read ( uint32 term_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout ) {
//       }
//       void _write ( uint32 term_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) {
//       }
//       void _close () throw() {}
//};


#endif
