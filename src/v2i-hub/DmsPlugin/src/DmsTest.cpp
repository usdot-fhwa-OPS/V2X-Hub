/*
 * DmsTest.cpp
 *
 *  Created on: Oct 12, 2015
 *      Author: ivp
 */

#include "DmsTest.h"

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <array>

DmsTest::DmsTest()
{
}

DmsTest::~DmsTest()
{
}

// This is old DMS test code used by Hassan.
//
void DmsTest::Test(SignalControllerNTCIP sc)
{
	//int MemoryType = 3;
	std::array<char, 28> MsgActivationCode;
	static int DMSMsgNumber = 0;
	int DMSMsgStatus = 0;

    //Get DMS parameters
/*
	int dmsAccess = sc.getDMSAccess();
	printf("DMSAccess:= %d\n", dmsAccess);

	int dmsType = sc.getDMSType();
	if (dmsType == 1)
		printf("DMSType:= %d = Other\n", dmsType);
	else if (dmsType == 2)
		printf("DMSType:= %d = bos\n", dmsType);
	else if (dmsType == 3)
		printf("DMSType:= %d = cms\n", dmsType);
	else if (dmsType == 4)
		printf("DMSType:= %d = vmsChar\n", dmsType);
	else if (dmsType == 5)
		printf("DMSType:= %d = vmsLine\n", dmsType);
	else if (dmsType == 6)
		printf("DMSType:= %d = vmsFull\n", dmsType);
	else if (dmsType == 129)
		printf("DMSType:= %d = portableOther\n", dmsType);
	else if (dmsType == 130)
		printf("DMSType:= %d = portableBOS\n", dmsType);
	else if (dmsType == 131)
		printf("DMSType:= %d = portableCMS\n", dmsType);
	else if (dmsType == 132)
		printf("DMSType:= %d = portableVMSChar\n", dmsType);
	else if (dmsType == 133)
		printf("DMSType:= %d = portableVMSLine\n", dmsType);
	else if (dmsType == 134)
		printf("DMSType:= %d = portableVMSFull\n", dmsType);

	int signHeight = sc.getDMSHeight();
	printf("DMSHeight:= %d\n", signHeight);
	int signWidth = sc.getDMSWidth();
	printf("DMSWidth:= %d\n", signWidth);
	int signCharHeight = sc.getDMSCharHeight();
	printf("DMSCharacterHeight:= %d\n", signCharHeight);
	int signCharWidth = sc.getDMSCharWidth();
	printf("DMSCharacterWidth:= %d\n", signCharWidth);
	int signPixelHeight = sc.getDMSPixelHeight();
	printf("DMSCharacterHeight:= %d\n", signPixelHeight);
	int signPixelWidth = sc.getDMSPixelWidth();
	printf("DMSCharacterWidth:= %d\n", signPixelWidth);

	int NumberOfPermanenetMessages = sc.getDMSNumberOfPermanentMsgs();
	printf("NumberOfPermanenetMessages:= %d\n", NumberOfPermanenetMessages);
	int NumberOfChangeableMsgs = sc.getDMSNumberofChangeableMsgs();
	printf("NumberOfChangeableMsgs:= %d\n", NumberOfChangeableMsgs);
	int NumberOfVolatileMsgs = sc.getDMSNumberofVolatileMsgs();
	printf("NumberOfVolatileMsgs:= %d\n", NumberOfVolatileMsgs);


	//Get current displayed message parameters
	int dmsControlMode = sc.getDMSControlMode();
	printf("dmsControlMode:= %d\n", dmsControlMode);

	int dmsMsgTimeRemaining = sc.getDMSMsgDisplayTimeRemaining();
	printf("DMSMsgTimeRemaining:= %d\n", dmsMsgTimeRemaining);
	//int DMSMsgTableSource = sc.getDMSMsgTableSource();
	//printf("DMSMsgTableSource:= %d\n", DMSMsgTableSource);
	char * DMSMsgTableSource = sc.getDMSMsgTableSource();
	printf("DMSMsgTableSource:= %d\n", DMSMsgTableSource);
	printf("DMSMsgTableSource:= %s\n", DMSMsgTableSource);
	printf("DMSMsgTableSource:= %x\n", DMSMsgTableSource);
	for (int i = 0; i < sizeof DMSMsgTableSource; i ++)
	{
	        printf(" %2x\n", DMSMsgTableSource[i]);
	}

	//int DMSMsgRequesterID = sc.getDMSMsgRequesterID();
	//printf("DMSMsgRequesterID:= %d\n", DMSMsgRequesterID);
	char * DMSMsgRequesterID = sc.getDMSMsgRequesterID();
	printf("DMSMsgRequesterID:= %d\n", DMSMsgRequesterID);
	printf("DMSMsgRequesterID:= %s\n", DMSMsgRequesterID);
	printf("DMSMsgRequesterID:= %x\n", DMSMsgRequesterID);
	for (int i = 0; i < sizeof DMSMsgRequesterID; i ++)
	{
	        printf(" %2x\n", DMSMsgRequesterID[i]);
	}
	int DMSMsgSourceMode = sc.getDMSMsgSourceMode();
	printf("DMSMsgSourceMode:= %d\n", DMSMsgSourceMode);
	int DMSMsgActivateError = sc.getDMSMsgActivateError();
	printf("DMSMsgActivateError:= %d\n", DMSMsgActivateError);
	int DMSMsgMultiSyntaxError = sc.getDMSMsgMultiSyntaxError();
	printf("DMSMsgMultiSyntaxError:= %d\n", DMSMsgMultiSyntaxError);


	//Get parameters for the first 4 messages in Changeable memory
	//Message 1		.3.1
	int dmsMsgMemoryTypeCurr = sc.getDMSMsgMemoryTypeCurr(".3.1");
	printf("dmsMsgMemoryTypeCurr:= %d\n", dmsMsgMemoryTypeCurr);
	int DMSMsgNumberCurr = sc.getDMSMsgNumberCurr(".3.1");
	printf("DMSMsgNumberCurr:= %d\n", DMSMsgNumberCurr);
	char * DMSMsgMultiStringCurr = sc.getDMSMsgMultiStringCurr(".3.1");
	printf("DMSMsgMultiStringCurr:= %d\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %s\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %x\n", DMSMsgMultiStringCurr);
	for (int i = 0; i < sizeof DMSMsgMultiStringCurr; i ++)
	{
	        printf(" %2x\n", DMSMsgMultiStringCurr[i]);
	}
	int DMSMsgOwnerCurr = sc.getDMSMsgOwnerCurr(".3.1");
	printf("DMSMsgMsgOwnerCurr:= %d\n", DMSMsgOwnerCurr);
	int DMSMsgCRCCurr = sc.getDMSMsgCRCCurr(".3.1");
	printf("DMSMsgCRCCurr:= %d\n", DMSMsgCRCCurr);
	int DMSMsgRunTimePriorityCurr = sc.getDMSMsgRunTimePriorityCurr(".3.1");
	printf("DMSMsgRunTimePriorityCurr:= %d\n", DMSMsgRunTimePriorityCurr);
	int DMSMsgStatus = sc.getDMSMsgStatusCurr(".3.1");
	printf("DMSMsgStatus-3-1:= %d\n", DMSMsgStatus);

	//Message 2 		.3.2
	dmsMsgMemoryTypeCurr = sc.getDMSMsgMemoryTypeCurr(".3.2");
	printf("dmsMsgMemoryTypeCurr:= %d\n", dmsMsgMemoryTypeCurr);
	DMSMsgNumberCurr = sc.getDMSMsgNumberCurr(".3.2");
	printf("DMSMsgNumberCurr:= %d\n", DMSMsgNumberCurr);
	DMSMsgMultiStringCurr = sc.getDMSMsgMultiStringCurr(".3.2");
	printf("DMSMsgMultiStringCurr:= %d\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %s\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %x\n", DMSMsgMultiStringCurr);
	for (int i = 0; i < sizeof DMSMsgMultiStringCurr; i ++)
	{
	        printf(" %2x\n", DMSMsgMultiStringCurr[i]);
	}
	DMSMsgOwnerCurr = sc.getDMSMsgOwnerCurr(".3.2");
	printf("DMSMsgMsgOwnerCurr:= %d\n", DMSMsgOwnerCurr);
	DMSMsgCRCCurr = sc.getDMSMsgCRCCurr(".3.2");
	printf("DMSMsgCRCCurr:= %d\n", DMSMsgCRCCurr);
	DMSMsgRunTimePriorityCurr = sc.getDMSMsgRunTimePriorityCurr(".3.2");
	printf("DMSMsgRunTimePriorityCurr:= %d\n", DMSMsgRunTimePriorityCurr);
	DMSMsgStatus = sc.getDMSMsgStatusCurr(".3.2");
	printf("DMSMsgStatus-3-2:= %d\n", DMSMsgStatus);

	//Message 3		.3.3
	dmsMsgMemoryTypeCurr = sc.getDMSMsgMemoryTypeCurr(".3.3");
	printf("dmsMsgMemoryTypeCurr:= %d\n", dmsMsgMemoryTypeCurr);
	DMSMsgNumberCurr = sc.getDMSMsgNumberCurr(".3.3");
	printf("DMSMsgNumberCurr:= %d\n", DMSMsgNumberCurr);
	DMSMsgMultiStringCurr = sc.getDMSMsgMultiStringCurr(".3.3");
	printf("DMSMsgMultiStringCurr:= %d\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %s\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %x\n", DMSMsgMultiStringCurr);
	for (int i = 0; i < sizeof DMSMsgMultiStringCurr; i ++)
	{
	        printf(" %2x\n", DMSMsgMultiStringCurr[i]);
	}
	DMSMsgOwnerCurr = sc.getDMSMsgOwnerCurr(".3.3");
	printf("DMSMsgMsgOwnerCurr:= %d\n", DMSMsgOwnerCurr);
	DMSMsgCRCCurr = sc.getDMSMsgCRCCurr(".3.3");
	printf("DMSMsgCRCCurr:= %d\n", DMSMsgCRCCurr);
	DMSMsgRunTimePriorityCurr = sc.getDMSMsgRunTimePriorityCurr(".3.3");
	printf("DMSMsgRunTimePriorityCurr:= %d\n", DMSMsgRunTimePriorityCurr);
	DMSMsgStatus = sc.getDMSMsgStatusCurr(".3.3");
	printf("DMSMsgStatus-3-3:= %d\n", DMSMsgStatus);

	//Message 4		.3.4
	dmsMsgMemoryTypeCurr = sc.getDMSMsgMemoryTypeCurr(".3.4");
	printf("dmsMsgMemoryTypeCurr:= %d\n", dmsMsgMemoryTypeCurr);
	DMSMsgNumberCurr = sc.getDMSMsgNumberCurr(".3.4");
	printf("DMSMsgNumberCurr:= %d\n", DMSMsgNumberCurr);
	DMSMsgMultiStringCurr = sc.getDMSMsgMultiStringCurr(".3.4");
	printf("DMSMsgMultiStringCurr:= %d\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %s\n", DMSMsgMultiStringCurr);
	printf("DMSMsgMultiStringCurr:= %x\n", DMSMsgMultiStringCurr);
	for (int i = 0; i < sizeof DMSMsgMultiStringCurr; i ++)
	{
	        printf(" %2x\n", DMSMsgMultiStringCurr[i]);
	}
	DMSMsgOwnerCurr = sc.getDMSMsgOwnerCurr(".3.4");
	printf("DMSMsgMsgOwnerCurr:= %d\n", DMSMsgOwnerCurr);
	DMSMsgCRCCurr = sc.getDMSMsgCRCCurr(".3.4");
	printf("DMSMsgCRCCurr:= %d\n", DMSMsgCRCCurr);
	DMSMsgRunTimePriorityCurr = sc.getDMSMsgRunTimePriorityCurr(".3.4");
	printf("DMSMsgRunTimePriorityCurr:= %d\n", DMSMsgRunTimePriorityCurr);
	DMSMsgStatus = sc.getDMSMsgStatusCurr(".3.4");
	printf("DMSMsgStatus-3-4:= %d\n", DMSMsgStatus);

	sc.getDMSMsgActivate();

	//Activate a message
	sc.setDMSMsgActivate("FFFFFF030001057B12345678");
	//sc.setDMSMsgActivate("FF FF FF 03 00 01 53 54 12 34 56 78");
*/

	//sc.setDMSControlMode("4");

	DMSMsgStatus = 0;

	// DMC added check for DMSMsgStatus != -1, so local loop will exit on error
	// until this method is called again from main loop.

	while (DMSMsgNumber < 10 && DMSMsgStatus != -1)
	{
		sc.setDMSMsgStatus("8", ".3.2");
		DMSMsgStatus = sc.getDMSMsgStatus(".3.2");
		printf("DMSMsgStatus-3.2:= %d\n", DMSMsgStatus);
		if (DMSMsgStatus == 1)
		{
			sc.setDMSMsgStatus("6", ".3.2");
			DMSMsgStatus = sc.getDMSMsgStatus(".3.2");
			printf("DMSMsgStatus-3.2:= %d\n", DMSMsgStatus);
			if (DMSMsgStatus == 2)
			{
				const char* message;
				if (DMSMsgNumber == 0)
					message = "MSG0";
				else if (DMSMsgNumber == 1)
					message = "MSG1";
				else if (DMSMsgNumber == 2)
					message = "MSG2";
				else if (DMSMsgNumber == 3)
					message = "MSG3";
				else if (DMSMsgNumber == 4)
					message = "MSG4";
				else if (DMSMsgNumber == 5)
					message = "MSG5";
				else if (DMSMsgNumber == 6)
					message = "MSG6";
				else if (DMSMsgNumber == 7)
					message = "MSG7";
				else if (DMSMsgNumber == 8)
					message = "MSG8";
				else if (DMSMsgNumber == 9)
					message = "MSG9";

				printf("***** Sending Message: %s\n", message);

				sc.setDMSMsgMultiString(message, ".3.2");

				sc.setDMSMsgOwner("TTI", ".3.2");
				sc.setDMSMsgRunTimePriority("1", ".3.2");

				sc.setDMSMsgStatus("7", ".3.2");
				DMSMsgStatus = sc.getDMSMsgStatus(".3.2");
				printf("DMSMsgStatus-3.2:= %d\n", DMSMsgStatus);
				if (DMSMsgStatus == 4)
				{
					int DMSMsgCRCCurr = sc.getDMSMsgCRCCurr(".3.2");
					printf("DMSMsgCRCCurr:= %d\n", DMSMsgCRCCurr);
					printf("DMSMsgCRC:= %x\n", DMSMsgCRCCurr);
					sprintf(MsgActivationCode.data(),"ffffff030002%04xc0a0010a",DMSMsgCRCCurr);
					printf("MsgActivationCode:= %s\n", MsgActivationCode.data());
					//usleep(500000);
					sc.setDMSMsgActivate(MsgActivationCode.data());
					//sc.setDMSMsgActivate("FFFFFF0300027ef6c0a0010a");
					//Checksum 2 bytes
					//Owner 4 bytes
				}
				DMSMsgNumber++;
			}
		}
	}

	/*sc.setDMSMsgStatus("8", ".3.2");

	DMSMsgStatus = sc.getDMSMsgStatus(".3.2");
	printf("DMSMsgStatus-3.2:= %d\n", DMSMsgStatus);

	if (DMSMsgStatus == 1)
	{
		sc.setDMSMsgStatus("6", ".3.2");

		DMSMsgStatus = sc.getDMSMsgStatus(".3.2");
		printf("DMSMsgStatus-3.2:= %d\n", DMSMsgStatus);
		if (DMSMsgStatus == 2)
		{
			sc.setDMSMsgMultiString("TEST", ".3.2");

			sc.setDMSMsgOwner("TTI", ".3.2");

			sc.setDMSMsgRunTimePriority("1", ".3.2");

			sc.setDMSMsgStatus("7", ".3.2");

			DMSMsgStatus = sc.getDMSMsgStatus(".3.2");
			printf("DMSMsgStatus-3.2:= %d\n", DMSMsgStatus);
			if (DMSMsgStatus == 4)
			{
				int DMSMsgCRCCurr = sc.getDMSMsgCRCCurr(".3.2");
				printf("DMSMsgCRCCurr:= %d\n", DMSMsgCRCCurr);
				printf("DMSMsgCRC:= %x\n", DMSMsgCRCCurr);
				sc.setDMSMsgActivate("FFFFFF0300026d50c0a0010a");
				//Checksum 2 bytes
				//Owner 4 bytes
			}
		}
	}*/
}
