/*************************************************************************************
*
*  map.cpp    : MAP Message Procedures
*
*  purpose    : Provide procedures for Initialization, Encoding, and Decoding
*               MAP message structure
*
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "map.h"

namespace MapPlugin
{

/* Functions */

/*************************************************************************************
*  Name       : map_clear 
*  Purpose    : Clear the contents of the MAP structure
*  Parameters : message  - Pointer to the MAP message structure
*************************************************************************************/
void map_clear(map *message)
    {
    int g, l, n, c, b, d;
    map_group *pg;
    map_barrier *pb;
    
/*  Initialize the Map Message */
	(*message).attributes=0;
	(*message).intersectionid= 0;
	for (g=0; g<map_maxgeometries; g++)
	    {

/*      Initialize the Geometry Structure */
    	(*message).geometry[g].refpoint.latitude=0;
	    (*message).geometry[g].refpoint.longitude=0;
	    (*message).geometry[g].refpoint.elevation=0;
	    for (d=1; d<3; d++)
	        {
            switch (d) {
                case approach: pg = &(*message).geometry[g].approach; break;
                case egress:   pg = &(*message).geometry[g].egress; break;
                }
            (*pg).width=0;
            
/*         Initialize the Lane Structure */
            for (l=0; l<map_maxlanes; l++)
                {
	            (*pg).lane[l].attributes=0;
	            (*pg).lane[l].number=0;
	            (*pg).lane[l].type=vehicle;
	            (*pg).lane[l].width=0;
	            (*pg).lane[l].referencelane.lanenumber=0;
	            (*pg).lane[l].referencelane.lateraloffset=0;
                (*pg).lane[l].referencelane.xoffset=0;
                (*pg).lane[l].referencelane.yoffset=0;
                for (n=0; n<map_maxnodes; n++)
                    {
	                (*pg).lane[l].node[n].eastern=0;
	                (*pg).lane[l].node[n].northern=0;
	                (*pg).lane[l].node[n].elevation=0;
	                (*pg).lane[l].node[n].width=0;
	                }
                for (c=0; c<map_maxconnections; c++)
                    {
	                (*pg).lane[l].connection[c].lanenumber=0;    
	                (*pg).lane[l].connection[c].maneuver=0;
	                //not sure if this works
	                (*pg).lane[l].connection[c].connectionID=0;
	                }
	            }
	        }

/*      Initialize the Barrier Structure */
        for (b=0; b<map_maxbarriers; b++)
	        {
	        pb = &(*message).geometry[g].barrier[b];
	        (*pb).attributes=0;
	        (*pb).width=0;
            for (n=0; n<map_maxnodes; n++)
                {
	            (*pb).node[n].eastern=0;
	            (*pb).node[n].northern=0;
	            (*pb).node[n].elevation=0;
	            (*pb).node[n].width=0;
	            }
            }
        }
	}

/*************************************************************************************
*  Name       : encode_nodelist 
*  Purpose    : Determine the encoding format of a node list and encode it.
*  Parameters : pnode           - Pointer to the node list
*               pblob           - Pointer to the blob payload
*               i               - Index to the blob payload byte array
*               msg_attributes  - Message attributes to direct encoding process
*************************************************************************************/
int encode_nodelist(map_node *pnode, unsigned char *pblob, int i, char msg_attributes)
    {
    int count, n, k, j;
    int nodelist[map_maxnodes*4];
    char node_attributes;
    map_node *pn;

/*  Determine if the Node List has Width Data */
    pn = pnode;
    node_attributes = 0;
    for (n=0; n<map_maxnodes; n++) {if ((*pn).width !=0) {node_attributes = node_attributes | width; break;} pn++;}

/*  Create the Node List */
    pn = pnode;
    count = 0;
    for (n=0; n<map_maxnodes; n++)
        {
        if (((*pn).northern !=0) && ((*pn).eastern !=0))
            {
            nodelist[count++] = (*pn).eastern;
            nodelist[count++] = (*pn).northern;
            if (msg_attributes & elevation) {nodelist[count++] = (*pn).elevation;}
            if (node_attributes & width) {nodelist[count++] = (*pn).width;}
            }
        pn++;
        }     

/*  Determine if the Node List can be Packed */
    node_attributes = node_attributes | packed;
    for (k=0; k<count; k++) {if ((nodelist[k] > 2047) | (nodelist[k] < -2048)) {node_attributes = (node_attributes & (~packed)); break;}}
    pblob[i++] = node_attributes;

/*  Limit the Magnitude of the Node Coordinates */
    for (k=0; k<count; k++) 
        {
        if (nodelist[k] > 32767)  {nodelist[k] = 32767;}
        if (nodelist[k] < -32768) {nodelist[k] = -32768;}
        }

/*  Encode an Unpacked Node List */
    if (!(node_attributes & packed))
        {
        for (k=0; k<count; k++) {for (j=0; j<2; j++) pblob[i++] = (nodelist[k] & (0xff00 >> (8*j))) >> (8*(1-j));}
        }                            
    
/*  Encode a Packed Node List */
    else
        {
        for (k=0; k<count; k++) 
            {
            pblob[i++] = ((nodelist[k] & 0x0ff0) >> 4);
            if (k < (count-1)) 
                {
                pblob[i++] = ((nodelist[k] & 0x000f) << 4) | ((nodelist[k+1] & 0x0f00) >> 8);
                pblob[i++] = (nodelist[k+1] & 0x00ff);
                k++;
                }
            else { pblob[i++] = ((nodelist[k] & 0x000f) << 4); }
            }
        }                            
    return i;
    }           
 
/*************************************************************************************
*  Name       : decode_nodelist 
*  Purpose    : Determine the encoding format of a node list and decode it.
*  Parameters : pnode           - Pointer to the node list
*               pblob           - Pointer to the blob payload
*               i               - Index to the blob payload byte array
*               msg_attributes  - Message attributes to direct encoding process
*************************************************************************************/
int decode_nodelist(map_node *pnode, unsigned char *pblob, int i, char msg_attributes)
    {
    int end, node_attributes, mode;
    
/*  Initialize Variables */
    end = i + pblob[i++];
    node_attributes = pblob[i++];
    mode = 1;

/*  Decode an Unpacked Node List */
    if (!(node_attributes & packed))
        {
        do 
        {
        (*pnode).eastern = ((signed char)pblob[i++] * 0x100) | pblob[i+1]; i++;
        (*pnode).northern = ((signed char)pblob[i++] * 0x100) | pblob[i+1]; i++;
        if (msg_attributes & elevation) {(*pnode).elevation = ((signed char)pblob[i++] * 0x100) | pblob[i+1]; i++;}
        if (node_attributes & width)    {(*pnode).width = ((signed char)pblob[i++] * 0x100) | pblob[i+1]; i++;}
        pnode++;
        }
        while (i < end);
        }

/*  Decode a Packed Node List */
    else
        {
        do 
        {            
        if (mode == 1)
            {    
            (*pnode).eastern = ((signed char)pblob[i++] * 0x10) | ((pblob[i+1] & 0xf0) >> 4);
            (*pnode).northern = ((signed char)((pblob[i++] & 0x0f) << 4) * 0x10) | pblob[i+1]; i++;
            if (msg_attributes & elevation) 
                {
                (*pnode).elevation = ((signed char)pblob[i++] * 0x10) | ((pblob[i+1] & 0xf0) >> 4); mode = 2;
                if (node_attributes & width) {(*pnode).width = ((signed char)((pblob[i++] & 0x0f) << 4) * 0x10) | pblob[i+1]; i++; mode = 1;}
                } 
            else
                {
                if (node_attributes & width) {(*pnode).width = ((signed char)pblob[i++] * 0x10) | ((pblob[i+1] & 0xf0) >> 4); mode = 2;}
               }
            }
        else 
            {
            (*pnode).eastern = ((signed char)((pblob[i++] & 0x0f) << 4) * 0x10) | pblob[i+1]; i++;
            (*pnode).northern = ((signed char)pblob[i++] * 0x10) | ((pblob[i+1] & 0xf0) >> 4);
            if (msg_attributes & elevation) 
                {
                (*pnode).elevation = ((signed char)((pblob[i++] & 0x0f) << 4) * 0x10) | pblob[i+1]; i++; mode = 1;
                if (node_attributes & width) {(*pnode).width = ((signed char)pblob[i++] * 0x10) | ((pblob[i+1] & 0xf0) >> 4);  mode = 2;}
                } 
            else
                {
                if (node_attributes & width) {(*pnode).width = ((signed char)((pblob[i++] & 0x0f) << 4) * 0x10) | pblob[i+1]; i++; mode = 1;}
               }            
            }
        pnode++;
        }
        while (i < end);
        }

    return i;
    }
       
/* MAP Procedures */

/*************************************************************************************
*  Name       : map_initialize 
*  Purpose    : Initialize the MAP structure
*  Parameters : message  - Pointer to the MAP message structure
*************************************************************************************/
int map_initialize(map *message)
    {
    map_clear(message);
    (*message).payload.pblob = NULL;
    (*message).payload.size = 0;
    (*message).payload.messageid = 0;
    (*message).payload.version = 0;
	return 1;
    }
        
/*************************************************************************************
*  Name       : map_encode 
*  Purpose    : Encode the MAP message into the blob payload
*  Parameters : message          - Pointer to the MAP message structure
*************************************************************************************/
int map_encode(map *message, unsigned char content_version)
    {
    int value, i, j, g, l, p, c, b, d, s;
    unsigned char *pblob;
    map_lane *pl;
    map_node *pn;
    map_referencelane *prl;
    map_barrier *pb;
    map_group *pg;
    unsigned short crc;

/*  Free any Previously Allocated Memory */
    blob_free(&(*message).payload);
    
/*  Create the MAP Blob */
    pblob = (unsigned char*) malloc(1024);
    if (pblob == NULL) 
        { 
        return 0;
        }  
    for (i=0; i<1024; i++) pblob[i] = 0;

/*  Encode the Message Header */
    i = 0;

    pblob[i++] = 0x87;
    pblob[i++] = content_version;
    s = i++;
    i++;
    
/*  Encode the Message Attributes */
    pblob[i++] = 1;
    pblob[i++] = 1; 
    value=0;   
    if ((*message).attributes & elevation)    {value = value | 0x01;} 
    if ((*message).attributes & decimeter)    {value = value | 0x02;} 
    if ((*message).attributes & geometric)    {value = value | 0x04;} 
    if ((*message).attributes & navigational) {value = value | 0x08;} 
    pblob[i++] = value;

/*  Encode the Intersection ID */
    pblob[i++] = 2;
    pblob[i++] = 4;    
    for (j=0; j<4; j++) pblob[i++] = (unsigned char)((*message).intersectionid & (0xff000000 >> (8*j))) >> (8*(3-j));

/*  Encode the Intersection Geometry */
	for (g=0; g<map_maxgeometries; g++)
	    {
	    if ((*message).geometry[g].refpoint.latitude != 0) 
	        {
	    
/*          Encode the Reference Point */
            pblob[i++] = 3;

            if ((*message).attributes & elevation) {pblob[i++] = 10;} else {pblob[i++] = 8;}

            for(j=0; j<4; j++)
            {

            	int lat = (*message).geometry[g].refpoint.latitude;
            	int latAfterBitShift = lat >> (8 * (3-j));
            	pblob[i++] = (unsigned char) latAfterBitShift;
            }

            for(j=0; j<4; j++)
			{

				int lat = (*message).geometry[g].refpoint.longitude;
				int latAfterBitShift = lat >> (8 * (3-j));
				pblob[i++] = (unsigned char) latAfterBitShift;
			}

	        if ((*message).attributes & elevation)
	            {
                for (j=0; j<2; j++) pblob[i++] = ((*message).geometry[g].refpoint.elevation & (0xff00 >> (8*j))) >> (8*(1-j));
	            }



/*          Encode the Approach or Egress Flag */
	        for (d=1; d<3; d++)
	            {
                switch (d) 
                    {
                    case approach: 
                    pg = &(*message).geometry[g].approach; break;
                    case egress:   
                    pg = &(*message).geometry[g].egress; break;
                    }
                if ((*pg).lane[0].number != 0)
                    {
                    pblob[i++] = 4;
                    pblob[i++] = 1;
                    pblob[i++] = d;                        

/*                  Encode the Approach or Egress Width */
                    if ((*pg).width != 0)
                        {
                        pblob[i++] = 8;
                        pblob[i++] = 2;
                        for (j=0; j<2; j++) pblob[i++] = ((*pg).width & (0xff00 >> (8*j))) >> (8*(1-j));
                        }
                                
/*                  Encode Each Lane in the Group */
                    for (l=0; l<map_maxlanes; l++)
                        {
                        pl = &(*pg).lane[l];
                        if ((*pl).number != 0)
                            {

/*                          Encode the Lane Number and Type */
                            pblob[i++] = 5;
                            pblob[i++] = 2;
                            pblob[i++] = (*pl).number;
                            pblob[i++] = (*pl).type;
                                                
/*                          Encode the Lane Attributes */
                            pblob[i++] = 6;
                            pblob[i++] = 2;
                            for (j=0; j<2; j++) pblob[i++] = ((*pl).attributes & (0xff00 >> (8*j))) >> (8*(1-j));
                        
/*                          Encode the Lane Width */
                            if ((*pl).width != 0)
                                {
                                pblob[i++] = 8;
                                pblob[i++] = 2;
                                for (j=0; j<2; j++) pblob[i++] = ((*pl).width & (0xff00 >> (8*j))) >> (8*(1-j));
                                }
                        
/*                          Encode the Reference Lane */
                            prl = &(*pl).referencelane;
                            if ((*prl).lanenumber != 0)
                                {
                                pblob[i++] = 11;
                                pblob[i++] = 3;
                                pblob[i++] = (*prl).lanenumber;
                                for (j=0; j<2; j++) pblob[i++] = (((*prl).lateraloffset & (0xff00 >> (8*j))) >> (8*(1-j)));
                                }
                        
/*                          Encode the Node List */
                            else
                                {
                                pblob[i++] = 9;
                                pblob[i++] = 0;
                                p = (i - 1);
                                i = encode_nodelist(&(*pl).node[0], pblob, i, (*message).attributes);
                                pblob[p] = (i-p-1);
                                }

/*                          Encode the Lane Connections */
                            for (c=0; c<map_maxconnections; c++) 
                                {
                                if ((*pl).connection[c].lanenumber !=0)
                                    {
                                    pblob[i++] = 10;
                                    pblob[i++] = 2;
                                    pblob[i++] = (*pl).connection[c].lanenumber;
                                    pblob[i++] = (*pl).connection[c].maneuver;
                                    //might not work
                                    pblob[i++] = (*pl).connection[c].connectionID;
                                    }
                                }
                            }
                        }
                    }
                }
                
/*          Encode the Barrier Flag */
            for (b=0; b<map_maxbarriers; b++)
                {
                pb = &(*message).geometry[g].barrier[b];
                pn = &(*pb).node[0];
                if (((*pn).northern !=0) && ((*pn).eastern !=0))
                    {
                    pblob[i++] = 4;
                    pblob[i++] = 1;
                    pblob[i++] = 3;

/*                  Encode the Barrier Attributes */
                    pblob[i++] = 7;
                    pblob[i++] = 2;
                    for (j=0; j<2; j++) pblob[i++] = ((*pb).attributes & (0xff00 >> (8*j))) >> (8*(1-j));

/*                  Encode the Barrier Width */
                    if ((*pb).width != 0)
                        {
                        pblob[i++] = 8;
                        pblob[i++] = 2;
                        for (j=0; j<2; j++) pblob[i++] = ((*pb).width & (0xff00 >> (8*j))) >> (8*(1-j));
                        }                

/*                  Encode the Barrier Node List */
                    pblob[i++] = 9;
                    pblob[i++] = 0;
                    p = (i - 1);
                    i = encode_nodelist(&(*pb).node[0], pblob, i, (*message).attributes);
                    pblob[p] = (i-p-1);
                    }
                }
            }
        }

/*  End of Blob */
    pblob[i++] = 255;
    
/*  Record the Message Size */
    for (j=0; j<2; j++) pblob[s++] = ((i-4) & (0xff00 >> (8*j))) >> (8*(1-j));

/*  Calculate the CRC Value */
    crc = crc_ccitt(pblob, i);    
    for (j=0; j<2; j++) pblob[i++] = (crc & (0xff00 >> (8*j))) >> (8*(1-j));

/*  Resize the MAP Blob  */
    pblob = (unsigned char*) realloc(pblob, i);
    
/*  Return the Blob and Size */        
    (*message).payload.pblob = pblob;
    (*message).payload.size = i;
    
/*  Return TRUE */
    return 1;    
    }
    
/*************************************************************************************
*  Name       : map_decode 
*  Purpose    : Decode the blob payload into the MAP message
*  Parameters : message  - Pointer to the MAP message structure
*************************************************************************************/
int map_decode(map *message)
    {
     unsigned char *pblob;
	 unsigned int i;
	 int j, g, l, b, c;
     map_geometry *geometry = NULL;
     map_group *group = NULL;
     map_barrier *barrier = NULL;
     map_connection *connection = NULL;
     map_lane *lane = NULL;
   
/*  Exit if there is no Blob to Decode */
    if ((*message).payload.pblob == NULL) 
        { 
        return 0;
        }  
        
/*  Clear any Previous Data */
    map_clear(message);
    pblob = (*message).payload.pblob;
    
/*  Initialize Variables */
    g = 0;
    b = 0;
    
/*  Parse the MAP Blob */
    i = 4;    
    while ((pblob[i] != 255) && (i<(*message).payload.size-2))
        {
        switch (pblob[i++]) 
            {
            
/*          Decode the Message Attributes */
            case 1:
            i++;
            (*message).attributes = pblob[i++];
            break;       

/*          Decode the Intersection ID */
            case 2:
            i++;
            for (j=0; j<4; j++) (*message).intersectionid |= pblob[i++] << (8*(3-j));
            break;       
            
/*          Decode the Reference Point */
            case 3:
            i++;
            geometry = &(*message).geometry[g++];            
            (*geometry).refpoint.latitude  = ((char)pblob[i] * 0x1000000) | (pblob[i+1] << 16) |(pblob[i+2] << 8) | pblob[i+3]; i += 4;
            (*geometry).refpoint.longitude = ((char)pblob[i] * 0x1000000) | (pblob[i+1] << 16) |(pblob[i+2] << 8) | pblob[i+3]; i += 4;
	        if ((*message).attributes & elevation)
	            {
                (*geometry).refpoint.elevation = ((char)pblob[i++] * 0x100) | pblob[i+1]; i++;
	            }            
            break;
           
/*          Decode the Approach/Egress/Barrier Flag */
            case 4:
            i++;
            switch (pblob[i++]) 
                {
                case 1: {group   = &(*geometry).approach;     barrier = NULL; break;}         
                case 2: {group   = &(*geometry).egress;       barrier = NULL; break;}         
                case 3: {barrier = &(*geometry).barrier[b++]; group = NULL;   break;}
                }         
            break;
                
/*          Decode the Lane Number and Type */
            case 5:
            lane = NULL;
            if (group != NULL)
                {
                for (l=0; l<map_maxlanes; l++) {if ((*group).lane[l].number == 0) {lane = &(*group).lane[l]; break;}}
                if (lane != NULL)
                    {
                    i++;
                    (*lane).number = pblob[i++]; 
                    (*lane).type = pblob[i++]; 
                    }
                else {i += pblob[i++];}
                }
            break; 
                  
/*          Decode the Lane Attributes */
            case 6:
            if (lane != NULL)
                {
                i++;
                for (j=0; j<2; j++) (*lane).attributes |= pblob[i++] << (8*(1-j));
                }
            else {i += pblob[i++];}
            break; 

/*          Decode the Barrier Attributes */
            case 7:
            if (barrier != NULL)
                {
                i++;
                for (j=0; j<2; j++) (*barrier).attributes |= pblob[i++] << (8*(1-j));
                }
            else {i += pblob[i++];}
            break; 
                        
/*          Decode the Lane Width */
            case 8:
            if (lane != NULL)
                {
                i++;
                for (j=0; j<2; j++) (*lane).width |= pblob[i++] << (8*(1-j));
                }
            else { 
            
/*          Decode the Barrier Width */
            if (barrier != NULL)
                {
                i++;
                for (j=0; j<2; j++) (*barrier).width |= pblob[i++] << (8*(1-j));
                }
            else {i += pblob[i++];}} 
            break; 
            
/*          Decode the Node List for a Lane */
            case 9:
            if (lane != NULL)
                {
                if ((*lane).type != computed)
                    {
                    i = decode_nodelist(&(*lane).node[0], pblob, i, (*message).attributes);
                    }
                else {i += pblob[i++];}
                }
            else {
            
/*          Decode the Node List for a Barrier */
            if (barrier != NULL)
                {
                i = decode_nodelist(&(*barrier).node[0], pblob, i, (*message).attributes);
                }
            else {i += pblob[i++];}}
            break; 

/*          Decode the Connection */
            case 10:
            if (lane != NULL)
                {
                if ((*lane).type != pedestrian)
                    {
                    connection = NULL;
                    for (c=0; c<map_maxconnections; c++) {if ((*lane).connection[c].lanenumber == 0) {connection = &(*lane).connection[c]; break;}}
                    if (connection != NULL)
                        {
                        i++;
                        (*connection).lanenumber = pblob[i++]; 
                        (*connection).maneuver = pblob[i++]; 
                        (*connection).connectionID = pblob[i++];
                        }
                    else {i += pblob[i++];}
                    }
                else {i += pblob[i++];}
                }
            else {i += pblob[i++];}
            break;            

/*          Decode the Reference Lane and Lateral Offset */
            case 11:
            if (lane != NULL)
                {
                if ((*lane).type == computed)
                    {
                    i++;
                    (*lane).referencelane.lanenumber = pblob[i++];
                    (*lane).referencelane.lateraloffset = ((char)pblob[i++] * 0x100) | pblob[i+1]; i++;
                    }
                else {i += pblob[i++];}
                }
            else {i += pblob[i++];}
            break;            
            }
        }    

/*  Return TRUE */
    return 1;   
    }

} /* End namespace MapPlugin */
