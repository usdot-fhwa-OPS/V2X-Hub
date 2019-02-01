#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <istream>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

using namespace std;

//tell ramp or intersection
#define RAMP 0
#define INTERSECTION 1
//define the different phase control types
#define	PHASE_HOLD 0
#define PHASE_FORCEOFF 1
#define PHASE_OMIT 2
#define PHASE_VEH_CALL 3

#define MAX_ITEMS 20


//------------------START OF DEFINITION--------------------------------//
//ASC INTERSECTION MIB :NTCIP 1202
//1.3.6.1.4.1.1206.4.1.3.1.1.3
//Group 1 phases (1 - 8) status
#define RED_GROUP		 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1"                     //Object
#define YELLOW_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1"
#define GREEN_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1"
#define DONOTWALK_GROUP  	"1.3.6.1.4.1.1206.4.2.1.1.4.1.5.1"
#define PEDCLEAR_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.6.1"
#define WALK_GROUP 		 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1"
#define VEHICLE_CALL	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"
//#define VEHICLE_DET_CALL	"1.3.6.1.4.1.1206.3.5.2.12.2.1.3"

#define PEDES_CALL	 		"1.3.6.1.4.1.1206.4.2.1.1.4.1.9.1"  // AT this time just use vehicle call

//Group 1 Phases (1 - 8) control
#define MIB_PHASE_OMIT 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1"
#define MIB_PHASE_HOLD 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1"
#define MIB_PHASE_FORCEOFF 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.5.1"
#define MIB_PHASE_VEH_CALL 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1"

// Controller configure information
#define MAX_PHASE_NO	 	"1.3.6.1.4.1.1206.4.2.1.1.1"
#define PHASE_NUMBER	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.1.1"   // phase number: last ".X" is phase number, the return value is also X

#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

#define PHASE_ENABLED       "1.3.6.1.4.1.1206.4.2.1.1.2.1.21."  // Phase options: last ".X" is phase, the last bit of return result is "0", the phase is not enabled.
//------------The following from standard: only read PLAN 1---------//
#define PHASE_MIN_GRN	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.4."   // need last "x" is the phase number: return the minimun green of phase x
#define PHASE_MAX_GRN	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.6."
#define PHASE_RED_CLR	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.9."
#define PHASE_YLW_XGE	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.8."
//------------The following from ASC3: WILL BE USED---------//
#define PHASE_MIN_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.9."   // need last "x.p" x is the timing plan number,p is the phase number: x get from CUR_TIMING_PLAN
#define PHASE_MAX_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.15."
#define PHASE_RED_CLR_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.19."
#define PHASE_YLW_XGE_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.18."

//**********asc3PhaseStatusTiming
//T (1):       Phase Timing
//N (2):       Phase Next
//- (3):       Phase Not Enabled
//(space) (4): Phase Not Timing or Next
#define PHASE_STA_TIME_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.1."  //NEED last "p"  for the phase
//**********asc3PhaseStatusTiming2
// (1) X: XPED timing
// (2) N: Phase Next
// (3) -: Phase Not enabled
// (4) .: Phase Not Timing
// (5) R: Phase Timing RED
// (6) Y: Phase Timing YEL
// (7) G: Phase Timing GREEN

// (8) D: Phase Timing DELAY GREEN
// (9) O: Phase Timing YEL & RED
//(10) g: Phase Timing FLASHING GREEN
//(11) y: Phase Timing FLASHING YELLOW "
#define PHASE_STA_TIME2_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.6."  //NEED last "p"  for the phase

#define MIB_RING_STOPTIME	"1.3.6.1.4.1.1206.4.2.1.7.5.1.2.1"

//------------The following from ASC3: WILL BE USED---------//
//	(0) Disable
//	(2) Enable SPaT
// 	(6) Enable SPaT and Pedestrian
#define ENABLE_SPAT 	"1.3.6.1.4.1.1206.3.5.2.9.44.1.0"

/*-- 1.3.6.1.4.1.1206.4.2.1.1.1.0  maxPhases.0
            maxPhases   OBJECT-TYPE
                SYNTAX   INTEGER (0..255)
                ACCESS   read-only
            STATUS   mandatory
            DESCRIPTION
                "NTCIP 1202 maxPhases
                The Maximum Number of Phases this
                Actuated Controller Unit supports. This
                object indicates the maximum rows which
                shall appear in the phaseTable object."
            REFERENCE
                "NTCIP 1202 Clause 2.2.1"
            ::= { phase 1 }*/

#define	MAX_PHASES	"1.3.6.1.4.1.1206.4.2.1.1.1.0"
/*
            -- 1.3.6.1.4.1.1206.4.2.1.1.3.0  maxPhaseGroups.0
            maxPhaseGroups   OBJECT-TYPE
                SYNTAX   INTEGER (1..255)
                ACCESS   read-only
                STATUS   mandatory
                DESCRIPTION
                    "NTCIP 1202 maxPhaseGroups
                    The Maximum Number of Phase Groups (8
                    Phases per group) this Actuated CU
                    supports."
                REFERENCE
                    "NTCIP 1202 Clause 2.2.3"
            ::= { phase 3 }*/

#define	MAX_PHASE_GROUPS	"1.3.6.1.4.1.1206.4.2.1.1.3.0"
/*
 * 			-- 1.3.6.1.4.1.1206.4.2.1.9.1.0  maxOverlaps.0
            maxOverlaps   OBJECT-TYPE
                SYNTAX   INTEGER (0..255)
                ACCESS   read-only
            STATUS   mandatory
            DESCRIPTION
                "NTCIP 1202 maxOverlaps
                The Maximum Number of Overlaps this ASC
                CU supports. This object indicates the
                maximum number of rows which shall
                appear in the overlapTable object."
            REFERENCE
                "NTCIP 1202 Clause 2.10.1"
            ::= { overlap 1 }*/

#define	MAX_OVERLAPS	"1.3.6.1.4.1.1206.4.2.1.9.1.0"
/*
            -- 1.3.6.1.4.1.1206.4.2.1.1.2.1.8.x  phaseYellowChange.x
            phaseYellowChange   OBJECT-TYPE
                SYNTAX   INTEGER (30..255)
                ACCESS   read-write
                STATUS   mandatory
                DESCRIPTION
                    "NTCIP 1202 phaseYellowChange
                    Phase Yellow Change time in tenth sec."
                REFERENCE
                    "NTCIP 1202 Clause 2.2.2.8"
                ::= { phaseEntry 8 }
             */

            //Assign the base OID for phaseYellowChange
#define	PHASE_YELLOW_CHANGS	"1.3.6.1.4.1.1206.4.2.1.1.2.1.8."
 /*
            -- 1.3.6.1.4.1.1206.4.2.1.1.2.1.3.x  phasePedestrianClear.x
            phasePedestrianClear   OBJECT-TYPE
                SYNTAX   INTEGER (0..255)
                ACCESS   read-write
                STATUS   mandatory
                DESCRIPTION
                    "NTCIP 1202 phasePedestrianClear
                    Phase Pedestrian Clear time in seconds."
                REFERENCE
                    "NTCIP 1202 Clause 2.2.2.3"
            ::= { phaseEntry 3 }
             */

            //Assign the base OID for phasePedestrianClear
#define	PHASE_PED_CLEAR	"1.3.6.1.4.1.1206.4.2.1.1.2.1.3."
 /*
            -- 1.3.6.1.4.1.1206.4.2.1.1.2.1.9.x  phaseRedClear.x
            phaseRedClear   OBJECT-TYPE
                SYNTAX   INTEGER (0..255)
                ACCESS   read-write
                STATUS   mandatory
                DESCRIPTION
                    "NTCIP 1202 phaseRedClear
                    Phase Red Clearance time in tenth sec."
                REFERENCE
                    "NTCIP 1202 Clause 2.2.2.9"
            ::= { phaseEntry 9 }
            -- 1- maxPhases
        	*/

            //Assign the base OID for phaseRedClear
#define	PHASE_RED_CLEAR	"1.3.6.1.4.1.1206.4.2.1.1.2.1.9."
           /*
            -- 1.3.6.1.4.1.1206.4.2.1.1.2.1.10.x  phaseRedRevert.x
            -- phaseRedRevert OBJECT-TYPE
            --    SYNTAX   INTEGER (0..255)
            --    ACCESS   read-write
            --    STATUS   optional
            --    DESCRIPTION
            --       "NTCIP 1202 phaseRedRevert
            --        Phase Red Revert time in tenth sec."
            --    REFERENCE
            --       "NTCIP 1202 Clause 2.2.2.10"
            -- ::= { phaseEntry 10 }
             */
#define	PHASE_RED_REVERT	"1.3.6.1.4.1.1206.4.2.1.1.2.1.10."

//DMS OID's
#define DMS_ACCESS						"1.3.6.1.4.1.1206.4.2.3.1.1"
//dmsSignAccess OBJECT-TYPE
//SYNTAX INTEGER (0..255)
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> Indicates the access method to the sign. Methods that are
//defined are:
//<Format>
// Bit 0- Other
// Bit 1- Walk-in access
// Bit 2- Rear access
// Bit 3- Front access
//If a bit is set to one (1), then the associated feature exists; if the bit is
//set to zero (0), then the associated feature does not exist.

#define DMS_TYPE						"1.3.6.1.4.1.1206.4.2.3.1.2"
//dmsSignType OBJECT-TYPE
//SYNTAX INTEGER{
// other (1),
//bos (2),
//cms (3),
//vmsChar (4),
//vmsLine (5),
//vmsFull (6),
//portableOther (129),
//portableBOS (130),
//portableCMS (131),
//portableVMSChar (132),
//portableVMSLine (133),
//portableVMSFull (134)}
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> Indicates the type of sign. The descriptions are:
// other: Device not specified through any other definition, refer to
// device manual,
// bos: Device is a Blank-Out Sign,
// cms : Device is a Changeable Message Sign,
// vmsChar : Device is a Variable Message Sign with character matrix
// setup,
// vmsLine : Device is a Variable Message Sign with line matrix setup,
// vmsFull: Device is a Variable Message Sign with full matrix setup.
//Same is true for all portable signs.

#define DMS_HEIGHT						"1.3.6.1.4.1.1206.4.2.3.1.3"
#define DMS_WIDTH						"1.3.6.1.4.1.1206.4.2.3.1.4"
#define DMS_CHAR_HEIGHT					"1.3.6.1.4.1.1206.4.2.3.2.1"
//vmsCharacterHeightPixels OBJECT-TYPE
//SYNTAX INTEGER (0..255)
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> Indicates the height of a single character in Pixels. The value
//zero (0) indicates a variable character height, which implies a full-matrix
//sign.
//<Unit>pixel
#define DMS_CHAR_WIDTH					"1.3.6.1.4.1.1206.4.2.3.2.2"
#define DMS_PIXEL_HEIGHT  				"1.3.6.1.4.1.1206.4.2.3.2.3"
//vmsSignHeightPixels OBJECT-TYPE
//SYNTAX INTEGER (0..65535)
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> Indicates the number of rows of pixels for the entire sign.
//<Unit>pixel
//<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.2.3"
//::= { vmsCfg 3 }
//-- To determine the number of lines for a line matrix or character matrix
//-- sign, divide the vmsSignHeightPixels object value by the
//-- vmsCharacterHeightPixels object value. This should result in a whole
//-- number, the number of lines in the sign.
#define DMS_PIXEL_WIDTH					"1.3.6.1.4.1.1206.4.2.3.2.4"

#define DMS_NUM_PERMANENT_MSGS			"1.3.6.1.4.1.1206.4.2.3.5.1"
//"<Definition> Indicates the current number of Messages stored in nonvolatile,
//non-changeable memory (e.g., EPROM). For CMS and BOS, this is the
//number of different messages that can be assembled.
//See the Specifications in association with Requirement 3.6.7.1 to determine
//the messages that must be supported.
#define DMS_NUM_CHANGEABLE_MSGS			"1.3.6.1.4.1.1206.4.2.3.5.2"
//"<Definition> Indicates the current number of valid Messages stored in nonvolatile,
//changeable memory. For CMS and BOS, this number shall be zero (0).
//<Unit>message
#define DMS_NUM_VOLATILE_MSGS			"1.3.6.1.4.1.1206.4.2.3.5.5"
//"<Definition> Indicates the current number of valid Messages stored in
//volatile, changeable memory. For CMS and BOS, this number shall be zero (0).
//<Unit>message


//Section 5.6.8 Message Table Parameter
#define DMS_MSG_MEMORYTYPE				"1.3.6.1.4.1.1206.4.2.3.5.8.1.1"
#define DMS_MSG_NUMBER					"1.3.6.1.4.1.1206.4.2.3.5.8.1.2"
#define DMS_MSG_MULTISTRING				"1.3.6.1.4.1.1206.4.2.3.5.8.1.3"
#define DMS_MSG_OWNER					"1.3.6.1.4.1.1206.4.2.3.5.8.1.4"
#define DMS_MSG_CRC						"1.3.6.1.4.1.1206.4.2.3.5.8.1.5"
#define DMS_MSG_RUNTIMEPRIORITY			"1.3.6.1.4.1.1206.4.2.3.5.8.1.8"
#define DMS_MSG_STATUS					"1.3.6.1.4.1.1206.4.2.3.5.8.1.9"

//Section 5.7 Sign Control Objects
#define DMS_CONTROL_MODE				"1.3.6.1.4.1.1206.4.2.3.6.1"
//dmsControlMode OBJECT-TYPE
//SYNTAX INTEGER
//{
// --other (1), -retired
// local (2),
// --external (3), -retired
// central (4),
// centralOverride (5)
//--simulation (6) -retired
// }
//ACCESS read-write
//STATUS mandatory
//DESCRIPTION
//"<Definition> A value indicating the mode that is currently controlling the
//sign.
//The possible modes are:
// other - (deprecated) Other control mode supported by the device (refer to
//device manual);
// local - Local control mode (control is at DMS controller);
// external - (deprecated) External control mode;
// central - Central control mode;
// centralOverride - Central station took control over Local control, even
// though the control switch at the sign was set to Local;
// simulation - (deprecated) controller is in a mode where it accepts every
// command and it pretends that it would execute them but this does not
// happen because the controller only simulates.
//<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.6.1"
//DEFVAL {central}

#define DMS_ACTIVATE_MSG				"1.3.6.1.4.1.1206.4.2.3.6.3"
/*dmsControlMode OBJECT-TYPE
SYNTAX INTEGER
{
 --other (1), -retired
 local (2),
 --external (3), -retired
 central (4),
centralOverride (5)
--simulation (6) -retired
 }
ACCESS read-write
STATUS mandatory
DESCRIPTION
"<Definition> A value indicating the mode that is currently controlling the
sign.
The possible modes are:
 other - (deprecated) Other control mode supported by the device (refer to
device manual);
 local - Local control mode (control is at DMS controller);
 external - (deprecated) External control mode;
 central - Central control mode;
 centralOverride - Central station took control over Local control, even
 though the control switch at the sign was set to Local;
 simulation - (deprecated) controller is in a mode where it accepts every
 command and it pretends that it would execute them but this does not
 happen because the controller only simulates.
<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.6.1"
DEFVAL {central}
*/
#define DMS_MSG_DISPLAY_TIME_REMAINING			"1.3.6.1.4.1.1206.4.2.3.6.4.0"
/*dmsMessageTimeRemaining OBJECT-TYPE
SYNTAX INTEGER (0..65535)
ACCESS read-write
STATUS mandatory
DESCRIPTION
"<Definition> Indicates the amount of remaining time in minutes that the
current message shall be active. The time shall be accurate to the nearest
second and rounded up to the next full minute. For example, a value of 2
shall indicate that the time remaining is between 1 minute and 0.1 seconds
and 2 minutes.
When a new message is activated with a minute-based duration, or this object
NTCIP 1203 v03.04
Page 164
© 2011 AASHTO / ITE / NEMA. Copy Per MIB Distribution Notice
is directly SET, the minute-based duration value shall be multiplied by 60 to
determine the number of seconds that the message shall be active. Thus, if a
message activation is for 2 minutes, the DMS will be assured to display the
message for 120 seconds.
The value 65535 indicates an infinite duration. A value of zero (0) shall
indicate that the current message display duration has expired.
A SET operation on this object shall allow a Central Computer to extend or
shorten the duration of the message. Setting this object to zero (0) shall
result in the immediate display of the dmsEndDurationMessage.
<Unit>minute
<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.6.4"
DEFVAL {65535}*/

#define DMS_MSG_TABLE_SOURCE			"1.3.6.1.4.1.1206.4.2.3.6.5.0"	//WITHOUT SOURCE TYPE
//dmsMsgTableSource OBJECT-TYPE
//SYNTAX MessageIDCode
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> Identifies the message number used to generate the currently
//displayed message. This object is written to by the device when the new
//message is loaded into the currentBuffer of the dmsMessageTable. The value of
//this object contains the message ID code of the message that was copied into
//the 'currentBuffer'. This value can only be of message type 'permanent',
//'volatile', 'changeable', or 'blank'.
//<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.6.5"

#define DMS_MSG_REQUESTER_ID			"1.3.6.1.4.1.1206.4.2.3.6.6.0"
//dmsMsgRequesterID OBJECT-TYPE
//SYNTAX IpAddress
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> A copy of the source-address field from the dmsActivateMessageobject
//used to activate the current message. If the current message was not
//activated by the dmsActivateMessage-object, then the value of this object
//shall be zero (0).
//<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.6.6"

#define DMS_MSG_SOURCE_MODE				"1.3.6.1.4.1.1206.4.2.3.6.7.0"
/*dmsMsgSourceMode OBJECT-TYPE
SYNTAX INTEGER
{
 other (1),
 local (2),
external (3),
--otherCom1( 4), -retired
 --otherCom2 (5), -retired
 --otherCom3 (6), -retired
 --otherCom4 (7), -retired
 central (8),
 timebasedScheduler (9),
powerRecovery (10),
reset (11),
commLoss (12),
powerLoss (13),
endDuration (14)
}
ACCESS read-only
STATUS mandatory
DESCRIPTION
"<Definition> Indicates the source that initiated the currently displayed
message. The enumerations are defined as:
 other (1) - the currently displayed message was activated based on a
 condition other than the ones defined below. This would include any
 auxiliary devices.
 local (2) - the currently displayed message was activated at the sign
 controller using either an onboard terminal or a local interface.
 external (3) - the currently displayed message was activated from a locally
connected
 device using serial (or other type of) connection to the sign controller
such as a laptop or
 a PDA. This mode shall only be used, if the sign controller is capable of
distinguishing
 between a local input (see definition of 'local (2)') and a serial
connection.
 central (8) - the currently displayed message was activated from the
central
 computer.
 timebasedScheduler (9) - the currently displayed message was activated from
 the timebased scheduler as configured within the sign controller.
 powerRecovery (10) - the currently displayed message was activated based
 on the settings within the dmsLongPowerRecoveryMessage,
dmsShortPowerRecoveryMessage, and the
 dmsShortPowerLossTime objects.
 reset (11) - the currently displayed message was activated based on the
 settings within the dmsResetMessage object.
 commLoss (12) - the currently displayed message was activated based on
 the settings within the dmsCommunicationsLossMessage object.
 powerLoss (13) - the currently displayed message was activated based on
 the settings within the dmsPowerLossMessage object. Note: it may not be
 possible to point to this message depending on the technology, e.g. it
may
 not be possible to display a message on pure LED or fiber-optic signs
 DURING power loss.
 endDuration (14) - the currently displayed message was activated based on
 the settings within the dmsEndDurationMessage object.
*/

#define DMS_MEMORY_MANAGEMENT			"1.3.6.1.4.1.1206.4.2.3.6.16"
//dmsMemoryMgmt OBJECT-TYPE
//SYNTAX INTEGER {
// --other (1), -retired
// normal (2),
//clearChangeableMessages (3),
//clearVolatileMessages (4) }
//ACCESS read-write
//STATUS mandatory
//DESCRIPTION
//"<Definition> Allows the system to manage the device's memory. SNMP Get
//operations on this object should always return normal (2).
// clearChangeableMessages (3): the controller shall set dmsMessageStatus for
//all changeable messages to notUsed (1), and release all memory associated
//with changeable messages. This action does not affect any changeable
//graphics or fonts.
// clearVolatileMessages (4): the controller shall set dmsMessageStatus for
//all volatile messages to notUsed (1), and release all memory associated with
//volatile messages. This action does not affect any changeable graphics or
//fonts.
//<Object Identifier> 1.3.6.1.4.1.1206.4.2.3.6.16"
//DEFVAL {normal}

#define DMS_MSG_ACTIVATE_ERROR			"1.3.6.1.4.1.1206.4.2.3.6.17.0"
/*none (2),
priority (3),
messageStatus (4),
messageMemoryType (5),
messageNumber (6),
messageCRC (7),
syntaxMULTI (8),
localMode (9),
centralMode (10),
centralOverrideMode (11)
}
ACCESS read-only
STATUS mandatory
DESCRIPTION
"<Definition> This is an error code used to identify why a message was not
displayed. Even if multiple errors occur, only one error is indicated.
 other (1): any error not defined below.
 none (2): no error.
 priority(3): the activation priority in the MessageActivationCode is
 less than the run time priority of the currently displayed message.
 If this error occurs, the corresponding bit (message error) within
 the 'shortErrorStatus' object shall be set.
 messageStatus(4): the 'dmsMessageStatus' of the message to be
 activated is not 'valid'. If this error occurs, the corresponding bit
 (message error) within the 'shortErrorStatus' object shall be set.
 NOTE: In the 1997 version of this standard, this bit was assigned
 the name of 'underValidation'. It has been renamed to better
 reflect the fact that this bit can be set due to the message being
 in a number of different states, not just the 'validating' state.
 messageMemoryType(5): the message memory type in the
 MessageActivationCode is not supported by the device. If this
 error occurs, the corresponding bit (message error) within the
 'shortErrorStatus' object shall be set.
 messageNumber(6): the message number in the
 MessageActivationCode is not supported or is not defined
 (populated) by the device. If this error occurs, the corresponding
 bit (message error) within the 'shortErrorStatus' object shall be set.
 messageCRC(7): the checksum in the MessageActivationCode is
 different than the CRC value contained in the 'dmsMessageCRC'.
 If this error occurs, the corresponding bit (message error) within
 the 'shortErrorStatus' object shall be set.
 syntaxMULTI(8): a MULTI syntax error was detected during
 message activation. The error is further detailed in the
 'dmsMultiSyntaxError', 'dmsMultiSyntaxErrorPosition', and
'dmsMultiOtherErrorDescription' objects. If this error occurs, the
corresponding bit (message error)
 within the 'shortErrorStatus' object shall be set.
 localMode(9): the central system attempted to activate a message
 while the 'dmsControlMode' object is 'local'. This error shall NOT
 be set if the value of the 'dmsControlMode' is set to
 'central', or 'centralOverride'. If this error occurs, the
 corresponding bit (message error) within the 'shortErrorStatus'
 object shall be set.
 centralMode (10): a locally connected system attempted to activate
 a message while the 'dmsControlMode' object is 'central'.
 This error shall NOT be set if the value of the 'dmsControlMode'
 is set to 'local'. If this error occurs, the corresponding
 bit (message error) within the 'shortErrorStatus'
 object shall be set.
 centralOverrideMode (11): a locally connected system attempted to activate
 a message while the 'dmsControlMode' object is 'centralOverride', even
 though the local switch is set to local control.
 If this error occurs, the corresponding bit (message error)
 within the 'shortErrorStatus' object shall be set.
A 'criticalTemperature' alarm shall have no effect on the 'activation' of a
message, it only effects the display of the active message. Thus, a message
activation may occur during a 'criticalTemperature' alarm and the sign
controller behaves as if the message is displayed. However, the
shortErrorStatus indicates a criticalTemperature alarm and the sign face
illumination is off. As soon as the DMS determines that the
'criticalTemperature' alarm is no longer present, the DMS shall display the
message stored in the currentBuffer.
*/
#define DMS_MSG_ACTIVATE_ERROR_CODE		"1.3.6.1.4.1.1206.4.2.3.6.24"
//dmsActivateErrorMsgCode OBJECT-TYPE
//SYNTAX MessageActivationCode
//ACCESS read-only
//STATUS mandatory
//DESCRIPTION
//"<Definition> Indicates the MessageActivationCode that resulted in the
//current value of the dmsActivateMsgError object.

#define DMS_MSG_MULTI_SYNTAX_ERROR		"1.3.6.1.4.1.1206.4.2.3.6.18.0"
/*dmsMultiSyntaxError OBJECT-TYPE
SYNTAX INTEGER
{
 	other (1),
 	none (2),
	unsupportedTag (3),
	unsupportedTagValue (4),
	textTooBig (5),
	fontNotDefined (6),
	characterNotDefined (7),
	fieldDeviceNotExist (8),
	fieldDeviceError (9),
	flashRegionError (10),
	tagConflict (11),
	tooManyPages (12),
	fontVersionID (13),
	graphicID (14),
 	graphicNotDefined (15)
 }
ACCESS read-only
STATUS mandatory
DESCRIPTION
"<Definition> This is an error code used to identify the first detected
syntax error within the MULTI message.
 other (1): An error other than one of those listed.
 none (2): No error detected.
 unsupportedTag (3): The tag is not supported by this device.
 unsupportedTagValue (4): The tag value is not supported by this
 device.
 textTooBig (5): Too many characters on a line, too many lines for a
 page, or font is too large for the display.
 fontNotDefined (6): The font is not defined in this device.
 characterNotDefined (7): The character is not defined in the
 selected font.
 fieldDeviceNotExist (8): The field device does not exist / is not
 connected to this device.
 fieldDeviceError (9): This device is not receiving input from the
 referenced field device and/or the field device has a fault.
 flashRegionError (10): The flashing region cannot be flashed by this
 device.
 tagConflict (11): The message cannot be displayed with the
 combination of tags and/or tag implementation cannot be resolved.
 tooManyPages (12): Too many pages of text exists in the message.
 fontVersionID (13): The fontVersionID contained in the MULTI tag
 [fox,cccc] does not match the fontVersionID for the fontNumber
 indicated.
 graphicID (14): The dmsGraphicID contained in the
 MULTI tag [gx,cccc] does not match the dmsGraphicID for the
 dmsGraphicIndex indicated.
 graphicNotDefined (15): The graphic is not defined in this device.
*/
int GetSignalColor(int PhaseStatus);
int outputlog(char *output);



class SignalControllerNTCIP
{
public:
//	SignalPhase Phases;
//	PhaseStatus phase_read;
	int CurTimingPlan;


public:
	SignalControllerNTCIP(void);
	~SignalControllerNTCIP(void);

	// phase_control is the CMD applied to some phases, AS defined above PHASE_HOLD, FORCEOFF, etc.
	// The Total is the number for commanded phases: if a phase is CMD, SET it to 1, otherwise 0
	// For Example: CALL phase 2&6: 00100010--> TOTAL=34
	void PhaseControl(int phase_control, int Total,char YES);

	//read phase information through NTCIP: group=Red or Yellow or Green
	void PhaseRead();

	//
	int CurTimingPlanRead();

	//
	void setConfigs(std::string snmpIP, char* snmpPort);
	void setConfigs(std::string snmpIP, int snmpPort);

	// Updating the member "Phases"
	void UpdatePhase();

	int  getMaxPhases();
	int  getMaxPhaseGroups();
	int  getMaxOverlaps();
	// Enable/Disable SPaT/Pedestrian message push
	void EnableSpaTPush();
	void EnableSpaTPedestrianPush();
	void DisableSpaTPush();

	//DMS functions
	int  getDMSAccess();
	int  getDMSType();
	int  getDMSHeight();
	int  getDMSWidth();
	int  getDMSCharHeight();
	int  getDMSCharWidth();
	int  getDMSPixelHeight();
	int  getDMSPixelWidth();

	int  getDMSNumberOfPermanentMsgs();
	int  getDMSNumberofChangeableMsgs();
	int  getDMSNumberofVolatileMsgs();


	int  getDMSMsgMemoryType();
    int  getDMSMsgNumber();
	char *  getDMSMsgMultiString();
	int  getDMSMsgOwner();
	int  getDMSMsgCRC();
	int  getDMSMsgRunTimePriority();
	int  getDMSMsgStatus(const char *msgID);

	int  getDMSMsgMemoryTypeCurr(const char *msgID);
	char *  getDMSMsgMultiStringCurr(const char *msgID);
    int  getDMSMsgNumberCurr(const char *msgID);
	int  getDMSMsgOwnerCurr(const char *msgID);
	int  getDMSMsgCRCCurr(const char *msgID);
	int  getDMSMsgRunTimePriorityCurr(const char *msgID);
	int  getDMSMsgStatusCurr(const char *msgID);


	int  getDMSControlMode();
	int  getDMSMsgDisplayTimeRemaining();
	//int  getDMSMsgTableSource();
	//int  getDMSMsgRequesterID();
	char *  getDMSMsgTableSource();
	char *  getDMSMsgRequesterID();
	int  getDMSMsgSourceMode();
	int  getDMSMsgActivateError();
	int  getDMSMsgActivateErrorCode();
	int  getDMSMsgMultiSyntaxError();

	void setDMSControlMode(const char *Value);
	void setDMSMsgStatus(const char *Status, const char * MsgID);
	void setDMSMsgMultiString(const char *TxtMsg, const char *msgID);
	void setDMSMsgOwner(const char *Owner, const char *msgID);
	void setDMSMsgRunTimePriority(const char *Priority, const char *msgID);
	//void setDMSMsgCRC(const char *CRC);

	int  getDMSMsgActivate();
	bool setDMSMsgActivate(const char *ActivationString);


	int  getDMSActiveMessageNumber();
	int  getDMSMsgPriority();

private:
	char* INTip;
	char* INTport;


	void sendSpatPush(int command);
	netsnmp_pdu* getSNMP(netsnmp_pdu *pdu, const char *community);
	//netsnmp_pdu* getSNMPActivateMsg(netsnmp_pdu *pdu, const char *community);

	netsnmp_pdu* singleGetSNMP(const char* getOID, const char *community);
	int  getSingleINT(const char* getOID, const char *community);
	char * getSingleString(const char* getOID, const char *community);
	int  getOctetString(const char* getOID, const char *community);

	netsnmp_pdu* setSNMPTCP(const char* getOID, const char *community, const char *Value);
	netsnmp_pdu* setSNMP(const char* getOID, const char *community, const char *Value);
	netsnmp_pdu* setSNMPInt(const char* getOID, const char *community, const char *Value);
	netsnmp_pdu* setSNMPText(const char* getOID, const char *community, const char *Value);
	bool  setSNMPActivateMsg(const char* getOID, const char *community, const char *ActivationString);

};
