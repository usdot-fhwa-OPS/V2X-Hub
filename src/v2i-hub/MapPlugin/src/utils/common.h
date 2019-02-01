#ifndef COMMON_H_
#define COMMON_H_
/*************************************************************************************
*
*  common.h : Common Procedure Header File
*
*************************************************************************************/

namespace MapPlugin
{

/**
 *\ingroup MAPPlugin
 * structure used to transport Battelle SPaT messages
 */
struct bblob
    {
    unsigned int messageid;
    unsigned int version;
    unsigned char *pblob;
    unsigned int size;
    };

/**
 * \ingroup MAPPlugin
 * Movements for lanes in a map
 */
enum map_movements {straight = 1, leftturn = 2, rightturn = 4, uturn = 8, softleft = 16, softright = 32, mergeleft = 64, mergeright = 128};

/* Common Function Prototypes */  
unsigned short crc_ccitt(unsigned char*, int);
bblob blob_initialize(unsigned char*);
void blob_free(bblob*);

} /* End namespace MapPlugin */

#endif /* COMMON_H_ */
