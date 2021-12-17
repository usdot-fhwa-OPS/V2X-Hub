/***** Implimentation of pkcs11 proxy class for integrating softhsm module with V2X hub */ 
/* Author: Anjan Rayamajhi, leidos Inc */ 


#include "config.h"

#include <pkcs11.h>


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <dlfcn.h>
#include <pthread.h>

#include <syslog.h>



#define SOCKET_PATH "tcp://127.0.0.1"

#ifdef SECCOMP
#include <seccomp.h>
//#include "seccomp-bpf.h"
#ifdef DEBUG_SECCOMP
# include "syscall-reporter.h"
#endif /* DEBUG_SECCOMP */
#include <fcntl.h> /* for seccomp init */
#endif /* SECCOMP */




struct hsmtoken
{    
    int slotID; 
    char* slotLabel; 
    char* userID;
    char* userpin; 
};



namespace tmx {

    class softhsmHelper
    {
        public:

        char *path, *tsk_file_path, *socket_url;

        int socket;
        fd_set filedesc; 

        hsmtoken tk; 



        protected: 

        int get_slot_info(char *path, int slotID, char* label, char* userPIN, char* username); // returns the success/failure of slot access and info
        hsmtoken create_token(char *path, int slotID, char* label, char* userPIN, char* username);
        char*  psk_keygen_helper(char* passcode, char* encryption);  
        int encrypt(char* input, char* output); 
        int decrypt(char* input, char* output);
        int get_slot0_token(); // this might be necessary if the install process sets up a slot 0 token for general use

        int sign_messages(char* input, char* output);  

    }

};

