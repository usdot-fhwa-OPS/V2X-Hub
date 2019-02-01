/*************************************************************************************
*
*  common.cpp : Common Procedures
*
*  purpose    : Provide common procedures used by the SPAT and MAP messages
*               for the CRC error checking and blob memory management.
*
*************************************************************************************/

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
static int crc_table_initialized = 0;
static unsigned short crc_table[256];

namespace MapPlugin
{

/* CRC Calculation Procedures */

/*************************************************************************************
*  Name       : initialize_crc_table 
*  Purpose    : Initialize the look-up table for the CRC calculation
*  Parameters : none
*************************************************************************************/
static void initialize_crc_table(void) 
    {
    int i, j;
    unsigned short crc, c;

    for (i=0; i<256; i++) 
        {
        crc = 0;
        c = ((unsigned short) i) << 8;
        for (j=0; j<8; j++) 
            {
            if ((crc^c) & 0x8000 ) crc = (crc << 1)^0x1021;
            else                   crc = (crc << 1);
            c = (c << 1);
            }
        crc_table[i] = crc;
        }
        
    crc_table_initialized = 1;
    } 
      
/*************************************************************************************
*  Name       : crc_update
*  Purpose    : Update the CRC calculation
*  Parameters : crc  - Current CRC value
*               c    - Next byte to be included in the CRC calculation
*************************************************************************************/
unsigned short crc_update(unsigned short crc, char c) 
    {
    unsigned short tmp, short_c;

    short_c  = 0x00ff & (unsigned short) c;

    if (!crc_table_initialized) initialize_crc_table();

    tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ crc_table[tmp];

    return crc;
    } 
    
/*************************************************************************************
*  Name       : crc_ccitt
*  Purpose    : Calculate the CRC value
*  Parameters : pblob    - Pointer to the blob binary byte array
*               size     - Size of the array
*************************************************************************************/
unsigned short crc_ccitt(unsigned char *pblob, int size)
    {
    unsigned short crc;
    
    crc=0;
    while (size) {crc = crc_update(crc, (*pblob)); pblob++; size--;}
    return crc;
    }

    
/* Blob Procedures */

/*************************************************************************************
*  Name       : blob_initialize
*  Purpose    : Initialize the payload blob structure.  This procedure must be 
*               called after a new instance of the payload structure is created.
*  Parameters : pblob  - Pointer to the blob binary byte array
*************************************************************************************/
bblob blob_initialize(unsigned char *pblob)
    {
    bblob payload;
    unsigned short crc;
    int i, j;

/*  Initialize Payload Structure */
    payload.size=0;
    payload.pblob = NULL;
    payload.messageid = 0;
    payload.version = 0;
    
/*  Exit if there is no Message */
    if ((*pblob) == 0)
        { 
        return payload;
        }  

/*  Get the Message Information */
    i = 0;
    payload.pblob = pblob;
    payload.messageid = pblob[i++];
    payload.version = pblob[i++];
    for (j=0; j<2; j++) payload.size += pblob[i++] * (0x0100 >> (8*j));
    payload.size += 6;

/*  Validate the Message Integrity */
    crc = crc_ccitt(pblob, payload.size);
    if (crc != 0) 
        {
        payload.messageid = 0;
        payload.version = 0;
        }

/*  Return the Blob Structure */
    return payload;
    }
      
/*************************************************************************************
*  Name       : blob_free
*  Purpose    : Free up any data allocated from the system for the blob.  This 
*               procedure must be called before an existing instance of a payload
*               structure goes out of scope.
*  Parameters : pblob  - Pointer to the payload blob structure
*************************************************************************************/
void blob_free(bblob *payload)
    {
    if ((*payload).pblob != NULL) free((*payload).pblob);
    (*payload).pblob = NULL;
    (*payload).size = 0;
    (*payload).messageid = 0;
    (*payload).version = 0;
    }

} /* End namespace MapPlugin */
