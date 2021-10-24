#include "dmsNTCIP.h"

const char* DMS_MSG_MEMORYTYPE_CURR =		"1.3.6.1.4.1.1206.4.2.3.5.8.1.1";
const char* DMS_MSG_NUMBER_CURR =			"1.3.6.1.4.1.1206.4.2.3.5.8.1.2";
const char* DMS_MSG_MULTISTRING_CURR =		"1.3.6.1.4.1.1206.4.2.3.5.8.1.3";
const char* DMS_MSG_OWNER_CURR =			"1.3.6.1.4.1.1206.4.2.3.5.8.1.4";
const char* DMS_MSG_CRC_CURR =				"1.3.6.1.4.1.1206.4.2.3.5.8.1.5";
const char* DMS_MSG_RUNTIMEPRIORITY_CURR =	"1.3.6.1.4.1.1206.4.2.3.5.8.1.8";
const char* DMS_MSG_STATUS_CURR =			"1.3.6.1.4.1.1206.4.2.3.5.8.1.9";

char* concat(char *s1, char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = (char *) malloc(len1+len2+1);

    strcpy(result, s1);
    strcat(result, s2);
    //memcpy(result, s1, len1);
    //memcpy(result+len1, s2, len2+1);//+1 to copy the null-terminator
    return result;
}
char* concat(const char *s1, const char *s2)
{
	return concat((char*)s1, (char*)s2);
}
char* concat(const char *s1, char *s2)
{
	return concat((char*)s1, s2);
}

void SignalControllerNTCIP::setConfigs(std::string snmpIP, char* snmpPort)
{
	INTip = strdup(snmpIP.c_str());
	INTport = strdup(snmpPort);
	//printf("NTCIP Configuration Set %s:%s\n", INTip, INTport);
}

void SignalControllerNTCIP::setConfigs(std::string snmpIP, int snmpPort)
{
	INTip = strdup(snmpIP.c_str());

	char port[6];
	snprintf(port, 6, "%d", snmpPort);

	INTport = strdup(port);
}

/**
 * Send SMNP messages from the supplied pdu
 */

netsnmp_pdu* SignalControllerNTCIP::getSNMP(netsnmp_pdu *pdu, const char *community)
{
	netsnmp_session session, *ss;
	netsnmp_pdu *response;
	int status;

	printf("getSNMP Function - \tIP:   %s   Port:   %s   Community: %s\n", INTip, INTport, community);
	//printf("getSNMP %s %s\n", INTip, INTport);
	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to
	/* set up defaults */
	char ipwithport[64];
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	//printf("SNMP IP %s\n", ipwithport);

	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1; //for ASC intersection  /* set the SNMP version number */
	/* set the SNMPv1 community name used for authentication */
	//session.community = (u_char *) "Public";
	session.community = (u_char *) community;
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss) {
		// Could not open session
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		return NULL;
	}

	/*
	 * Send the messages and get responses
	 */
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Check response for errors.
	 */
	if (!response)
	{
		printf("SignalControllerNTCIP::getSNMP Response is null\n");
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		return NULL;
	}

	if (status != STAT_SUCCESS && response->errstat != SNMP_ERR_NOERROR) {
//	if (status != STAT_SUCCESS) { // && response->errstat != SNMP_ERR_NOERROR) {
		printf("SignalControllerNTCIP::getSNMP Status not successful\n");
		if (status == STAT_SUCCESS)
		{
			fprintf(stderr, "Error in packet\nReason: %s\n", snmp_errstring(response->errstat));
		}
		else if (status == STAT_TIMEOUT)
		{
			fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
		}
		else
		{
			fprintf(stderr, "Error %s.\n", session.peername);
			snmp_sess_perror("SendSNMP", ss);
		}
		return NULL;
	}

	snmp_close(ss);
	SOCK_CLEANUP;

	return response;
}

netsnmp_pdu*  SignalControllerNTCIP::singleGetSNMP(const char* getOID, const char* community)
{
    netsnmp_pdu *pdu;
    netsnmp_pdu *response = NULL;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    pdu = snmp_pdu_create(SNMP_MSG_GET);

    anOID_len = MAX_OID_LEN;

    if (!snmp_parse_oid(getOID, anOID, &anOID_len))
    {
        snmp_perror(getOID);
        SOCK_CLEANUP;
        return NULL;
    }
    snmp_add_null_var(pdu, anOID, anOID_len);

    response = getSNMP(pdu, community);
    return response;

}

int  SignalControllerNTCIP::getSingleINT(const char* getOID, const char *community)
{
    netsnmp_pdu *response;
    netsnmp_variable_list *vars;
    int *returnInt;

	//printf("getSingleINT Function -\tOID: %s \t Community: %s\n", getOID, community);
    response = singleGetSNMP(getOID, community);
    if (response)
    {
        for(vars = response->variables; vars; vars = vars->next_variable)
        {
        	// Should only be one integer variable
        	returnInt = (int *) vars->val.integer;
        }
    }
    else
    {
    	// No response
        return -1;
    }

    if (response)
    {
    	snmp_free_pdu(response);
    }
    return *returnInt;

}

char * SignalControllerNTCIP::getSingleString(const char* getOID, const char *community)
{
    netsnmp_pdu *response;
    netsnmp_variable_list *vars;
    char *returnInt;
    //int *NumVars;

    //printf("getSingleString\n");
    response = singleGetSNMP(getOID, community);
    if (response)
    {
		printf("getSingleString Function -\tOID: %s \t Community: %s\t", getOID, community);
    	//------SUCCESS: Print the result variables
	    int *out = new int[MAX_ITEMS];
	    int i =0;
	    for(vars = response->variables; vars; vars = vars->next_variable)
	        print_variable(vars->name, vars->name_length, vars);

	    for(vars = response->variables; vars; vars = vars->next_variable)
	    {
			//case ASN_INTEGER:
			//case ASN_COUNTER:
			//case ASN_GAUGE:
			//case ASN_TIMETICKS:
			//case ASN_PRIV_IMPLIED_OBJECT_ID:
			//case ASN_OBJECT_ID:
			//case ASN_PRIV_IMPLIED_OCTET_STR:
			//case ASN_OPAQUE:
			//case ASN_OCTET_STR:
			//	var->name_length = var->val_len + 1;
			//	var->name = (oid *) malloc(sizeof(oid) * (var->name_length));
	        if (vars->type == ASN_OCTET_STR)
	        {
	            printf("ASN-OCTET-String Variable: \n");
	            char *sp = (char *)malloc(1 + vars->val_len);
	            memcpy(sp, vars->val.string, vars->val_len);
	            sp[vars->val_len] = '\0';
	            //printf("value #%d is a string: %s\n", *NumVars++, sp);
	            printf("%s\n", sp);
	            free(sp);
	        }
	        else if (vars->type == ASN_INTEGER)
	        {
	            printf("ASN-INTEGER Variable: \n");
	            int *aa;
	            aa =(int *)vars->val.integer;
	            out[i++] = * aa;
	            printf("value #%d is NOT a string! Ack!\n", *returnInt++);
	        }
	    }
       for(vars = response->variables; vars; vars = vars->next_variable)
        {
        	//NumChars = vars->val_len;
        	returnInt = (char *) vars->val.bitstring;
        }
	    // freed the block of allocated memory
	    delete[] out;
    }
    else
    {
    	// No response
        return NULL;
    }

    if (response)
    {
    	snmp_free_pdu(response);
    }
    return returnInt;
}

int  SignalControllerNTCIP::getOctetString(const char* getOID,  const char *community)
{
    netsnmp_pdu *response;
    netsnmp_variable_list *vars;
    int *returnInt;

    response = singleGetSNMP(getOID, community);
    if (response)
    {
		printf("getOctetString Function -\tOID: %s \t Community: %s\n", getOID, community);
	    //------SUCCESS: Print the result variables
	    int *out = new int[MAX_ITEMS];
	    int i =0;
	    for(vars = response->variables; vars; vars = vars->next_variable)
	        print_variable(vars->name, vars->name_length, vars);

	    /* manipuate the information ourselves */
	    for(vars = response->variables; vars; vars = vars->next_variable)
	    {
	        if (vars->type == ASN_OCTET_STR)
	        {
	        	printf("ASN_OCTET_String Variable\n");
	            char *sp = (char *)malloc(1 + vars->val_len);
	            memcpy(sp, vars->val.string, vars->val_len);
	            sp[vars->val_len] = '\0';
	            printf("value #%d is an Octet string: %s\n", *returnInt++, sp);
	            free(sp);
	        }
	        else
	        {
	            int *aa;
	            aa =(int *)vars->val.integer;
	            out[i++] = * aa;
	            printf("value #%d is NOT a string! Ack!\n", *returnInt++);
	        }
		    // freed the block of allocated memory
		    delete[] out;
		    
	    }
	    return *returnInt;
    }
    else
    {
    	// No response
        return -1;
    }

    if (response)
    {
    	snmp_free_pdu(response);
    }
    return *returnInt;
}



int  SignalControllerNTCIP::getMaxPhases()
{
	return getSingleINT(MAX_PHASES, "public");
}
int  SignalControllerNTCIP::getMaxPhaseGroups()
{
	return getSingleINT(MAX_PHASE_GROUPS, "public");
}
int  SignalControllerNTCIP::getMaxOverlaps()
{
	return getSingleINT(MAX_OVERLAPS, "public");
}

//---------------------------------------------------------------------------------------

int  SignalControllerNTCIP::CurTimingPlanRead()
{
    // USING the standard oid : ONLY read timing plan 1.
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    //int count=1;
    //int currentTimePlan; // return value

    snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
    /* set up defaults */
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
   char ipwithport[64];
       strcpy(ipwithport,INTip);
       strcat(ipwithport,":");
       strcat(ipwithport,INTport);
    session.peername = strdup(ipwithport);
    //session.version = SNMP_VERSION_2c; //for ASC intersection  /* set the SNMP version number */
    session.version = SNMP_VERSION_1;    //for ASC/3 software  /* set the SNMP version number */
    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

    char ctemp[50];

    sprintf(ctemp,"%s",CUR_TIMING_PLAN);   // WORK
    // sprintf(ctemp,"%s",PHASE_MIN_GRN_ASC_TEST); // WORK
    // sprintf(ctemp,"%s%d.%d",PHASE_MIN_GRN_ASC,2,1);        //  WORK

    if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
    {
        snmp_perror(ctemp);
        SOCK_CLEANUP;
        exit(1);
    }

    snmp_add_null_var(pdu, anOID, anOID_len);


    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        /*
        * SUCCESS: Print the result variables
        */
        int *out = new int[MAX_ITEMS];
        int i =0;
        for(vars = response->variables; vars; vars = vars->next_variable)
            print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
        {
            if (vars->type == ASN_OCTET_STR)
            {
                char *sp = (char *)malloc(1 + vars->val_len);
                memcpy(sp, vars->val.string, vars->val_len);
                sp[vars->val_len] = '\0';
                //printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
            }
            else
            {

                int *aa;
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }

		// ----------get the current timing plan----------------//
        CurTimingPlan=out[0];
	    // freed the block of allocated memory
	    delete[] out;

    }
    else
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("signalControllerSNMP", ss);

    }

    /*
    * Clean up:    *  1) free the response.   *  2) close the session.
    */
    if (response)        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;

    return CurTimingPlan;

}

SignalControllerNTCIP::SignalControllerNTCIP(void)
{
	// Initialize SNMP libraries
	init_snmp("ASC");
}

SignalControllerNTCIP::~SignalControllerNTCIP(void)
{
}

void SignalControllerNTCIP::PhaseControl(int phase_control, int Total,char YES)
    {
    char tmp_log[64];
    char tmp_int[16];
    //char buffer[16];
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
	int verbose=0;
	if(YES=='y' || YES=='Y') verbose=1;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    int  failures = 0;

    //int number=pow(2.0,phaseNO-1);
    int number=Total;

    //itoa(number,buffer,2);

    sprintf(tmp_int,"%d",number);

    cout<<"  "<<tmp_int<<"  "<<YES<<endl;

    /*
    * Initialize a "session" that defines who we're going to talk to
    */
    snmp_sess_init( &session );                   /* set up defaults */

    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport);
    session.peername = strdup(ipwithport);

    /* set the SNMP version number */
    //session.version = SNMP_VERSION_2c;
    session.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
        {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
        }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to SET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    anOID_len = MAX_OID_LEN;
    if (PHASE_HOLD==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_HOLD, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_HOLD);
            failures++;
            }


		if (verbose)
		{
			sprintf(tmp_log,"HOLD control! Number (%d)\n",Total);
		}

        }
    else if (PHASE_FORCEOFF==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_FORCEOFF, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_FORCEOFF);
            failures++;
            }
		if (verbose)
		{
			sprintf(tmp_log,"FORCEOFF control! Number (%d)\n",Total);
			std::cout <<tmp_log;
		}


        }
    else if (PHASE_OMIT==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_OMIT, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_OMIT);
            failures++;
            }
		if (verbose)
		{
        sprintf(tmp_log,"OMIT control! Number (%d)\n",Total);
        std::cout <<tmp_log;
		}
        }
    else if (PHASE_VEH_CALL==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_VEH_CALL, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_VEH_CALL);
            failures++;
            }
		if (verbose)
		{
        sprintf(tmp_log,"VEH CALL to ASC controller! Number (%d)\n",Total);
        std::cout <<tmp_log;
		}
        }


    //snmp_add_var() return 0 if success
    if (snmp_add_var(pdu, anOID, anOID_len,'i', tmp_int))
        {
        snmp_perror(MIB_PHASE_HOLD);
        failures++;
        }

    if (failures)
        {
        snmp_close(ss);
        SOCK_CLEANUP;
        exit(1);
        }

    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
        {
        //------SUCCESS: Print the result variables
          int *out = new int[MAX_ITEMS];
        int i =0;
        for(vars = response->variables; vars; vars = vars->next_variable)
            print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
            {
            if (vars->type == ASN_OCTET_STR)
                {
                char *sp = (char *)malloc(1 + vars->val_len);
                memcpy(sp, vars->val.string, vars->val_len);
                sp[vars->val_len] = '\0';
                printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
                }
            else
                {
                int *aa;
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                printf("value #%d is NOT a string! Ack!\n", count++);
                }
            }
	    // freed the block of allocated memory
	    delete[] out;
        }
    else
        {
        // FAILURE: print what went wrong!
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("signalControllerSNMP", ss);
        }
    //------Clean up:1) free the response. 2) close the session.
    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);

    SOCK_CLEANUP;

    }

void SignalControllerNTCIP::PhaseRead()
    {
    netsnmp_session session, *ss;
	    netsnmp_pdu *pdu;
	    netsnmp_pdu *response;

	    oid anOID[MAX_OID_LEN];
	    size_t anOID_len;

	    netsnmp_variable_list *vars;
	    int status;
	    int count=1;

	    snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
	    /* set up defaults */
	    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
	    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
	    char ipwithport[64];
		    strcpy(ipwithport,INTip);
		    strcat(ipwithport,":");
		    strcat(ipwithport,INTport);
	    session.peername = strdup(ipwithport);
	    //session.version = SNMP_VERSION_2c; //for ASC intersection  /* set the SNMP version number */
	    session.version = SNMP_VERSION_1; //for ASC intersection  /* set the SNMP version number */
	    /* set the SNMPv1 community name used for authentication */
	    session.community = (u_char *)"public";
	    session.community_len = strlen((const char *)session.community);

	    SOCK_STARTUP;
	    ss = snmp_open(&session);                     /* establish the session */

	    if (!ss)
	    {
	        snmp_sess_perror("ASC", &session);
	        SOCK_CLEANUP;
	        exit(1);
	    }

	    /*
	    * Create the PDU for the data for our request.
	    *   1) We're going to GET the system.sysDescr.0 node.
	    */
	    pdu = snmp_pdu_create(SNMP_MSG_GET);
	    anOID_len = MAX_OID_LEN;

	    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

	    char ctemp[50];

	    for(int i=1;i<=8;i++)
	    {
	        sprintf(ctemp,"%s%d",PHASE_STA_TIME2_ASC,i);

	        if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	        {
	            snmp_perror(ctemp);
	            SOCK_CLEANUP;
	            exit(1);
	        }

	        snmp_add_null_var(pdu, anOID, anOID_len);

	    }


	    /*
	    * Send the Request out.
	    */
	    status = snmp_synch_response(ss, pdu, &response);

	    /*
	    * Process the response.
	    */
	    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	    {
	        /*
	        * SUCCESS: Print the result variables
	        */
	        int *out = new int[MAX_ITEMS];
	        int i =0;
	        for(vars = response->variables; vars; vars = vars->next_variable)
	            print_variable(vars->name, vars->name_length, vars);

	        /* manipuate the information ourselves */
	        for(vars = response->variables; vars; vars = vars->next_variable)
	        {
	        	printf("Manipulating\n");
	            if (vars->type == ASN_OCTET_STR)
	            {
	                char *sp = (char *)malloc(1 + vars->val_len);
	                memcpy(sp, vars->val.string, vars->val_len);
	                sp[vars->val_len] = '\0';
	                printf("value #%d is a string: %s\n", count++, sp);
	                free(sp);
	            }
	            else
	            {

	                int *aa;
	                aa =(int *)vars->val.integer;
	                out[i++] = * aa;
	                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
	            }
	        }
	        //****** GET the results from controller *************//
//	        for(int i=0;i<8;i++)
//	        {
//	            phase_read.phaseColor[i]=GetSignalColor(out[i]);
//	            phase_read.phaseColor[i]=out[i];

	            //if(out[i]==3)       PhaseDisabled[i]=1;  // Phase i is not enabled.
//	        }
	    }
	    else
	    {
	        if (status == STAT_SUCCESS)
	            fprintf(stderr, "Error in packet\nReason: %s\n",
	            snmp_errstring(response->errstat));
	        else if (status == STAT_TIMEOUT)
	            fprintf(stderr, "Timeout: No response from %s.\n",
	            session.peername);
	        else
	            snmp_sess_perror("signalControllerSNMP", ss);

	    }
	    /*
	    * Clean up:    *  1) free the response.   *  2) close the session.
	    */
	    if (response)        snmp_free_pdu(response);

	    snmp_close(ss);
	    SOCK_CLEANUP;
    }

 // Spat push Control

void SignalControllerNTCIP::EnableSpaTPush()
{
	sendSpatPush(2);
}
void SignalControllerNTCIP::EnableSpaTPedestrianPush()
{
	sendSpatPush(6);
}
void SignalControllerNTCIP::DisableSpaTPush()
{
	sendSpatPush(0);
}
void SignalControllerNTCIP::sendSpatPush(int command)
{
	// USING the standard oid : Set the Spat Push value
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	//netsnmp_variable_list *vars;
	int status;

	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to
	/* set up defaults */
	char ipwithport[64];
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1;
	/* set the SNMPv1 community name used for authentication */
	session.community = (u_char *) "public";
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss) {
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		// TODO: add failure
		return;
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	anOID_len = MAX_OID_LEN;


	if (!snmp_parse_oid(ENABLE_SPAT, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		printf("Error parse failed\n");
		snmp_perror(ENABLE_SPAT);
		SOCK_CLEANUP;
		return;
	}
	snmp_add_var(pdu, anOID, anOID_len, 'i', "6");

	/*
	 * Send the Request out.
	 */
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Process the response.
	 */
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
		/*
		 * SUCCESS: Print the result variables
		 */
		//int *out = new int[MAX_ITEMS];
		//int i = 0;
//		for (vars = response->variables; vars; vars = vars->next_variable)
//			print_variable(vars->name, vars->name_length, vars);
/*
		// manipuate the information ourselves
		for (vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OCTET_STR) {
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				//printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			} else {

				int *aa;
				aa = (int *) vars->val.integer;
				out[i++] = *aa;
				//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
			}l
		}

		// ----------get the current timing plan----------------//
		CurTimingPlan = out[0];
		*/

	} else {
		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",
					snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",
					session.peername);
		else
			snmp_sess_perror("signalControllerSNMP", ss);

	}

	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;
}





//DMS Sign Parameters Functions

int  SignalControllerNTCIP::getDMSAccess()
{
	//printf("\ngetDMSAccess Function\n");
	return getSingleINT(DMS_ACCESS, "public");
}
int  SignalControllerNTCIP::getDMSType()
{
	//printf("\ngetDMSType Function\n");
	return getSingleINT(DMS_TYPE, "public");
}
int  SignalControllerNTCIP::getDMSHeight()
{
	//printf("\ngetDMSHeight Function\n");
	return getSingleINT(DMS_HEIGHT, "public");
}
int  SignalControllerNTCIP::getDMSWidth()
{
	//printf("\ngetDMSWidth Function\n");
	return getSingleINT(DMS_WIDTH, "public");
}
int  SignalControllerNTCIP::getDMSCharHeight()
{
	//printf("\ngetDMSCharHeight Function\n");
	return getSingleINT(DMS_CHAR_HEIGHT, "public");
}
int  SignalControllerNTCIP::getDMSCharWidth()
{
	//printf("\ngetDMSCharWidth Function\n");
	return getSingleINT(DMS_CHAR_WIDTH, "public");
}
int  SignalControllerNTCIP::getDMSPixelHeight()
{
	//printf("\ngetDMSPixelHeight Function\n");
	return getSingleINT(DMS_PIXEL_HEIGHT, "public");
}
int  SignalControllerNTCIP::getDMSPixelWidth()
{
	//printf("\ngetDMSPixelWidth Function\n");
	return getSingleINT(DMS_PIXEL_WIDTH, "public");
}

int  SignalControllerNTCIP::getDMSNumberOfPermanentMsgs()
{
	//printf("\ngetDMSNumberOfPermanentMsgs Function\n");
	return getSingleINT(DMS_NUM_PERMANENT_MSGS, "public");
}
int  SignalControllerNTCIP::getDMSNumberofChangeableMsgs()
{
	//printf("\ngetDMSNumberofChangeableMsgs Function\n");
	return getSingleINT(DMS_NUM_CHANGEABLE_MSGS, "public");
}
int  SignalControllerNTCIP::getDMSNumberofVolatileMsgs()
{
	//printf("\ngetDMSNumberofVolatileMsgs Function\n");
	return getSingleINT(DMS_NUM_VOLATILE_MSGS, "public");
}


int  SignalControllerNTCIP::getDMSMsgMemoryType()
{
	//printf("\ngetDMSMsgMemoryType Function\n");
	return getSingleINT(DMS_MSG_MEMORYTYPE, "public");
}
int  SignalControllerNTCIP::getDMSMsgNumber()
{
	//printf("\ngetDMSMsgNumber Function\n");
	return getSingleINT(DMS_MSG_NUMBER, "public");
}
char *  SignalControllerNTCIP::getDMSMsgMultiString()
{
	//printf("\ngetDMSMsgMultiString Function\n");
	return getSingleString(DMS_MSG_MULTISTRING, "public");
}
int  SignalControllerNTCIP::getDMSMsgOwner()
{
	//printf("\ngetDMSMsgOwner Function\n");
	return getSingleINT(DMS_MSG_OWNER, "public");
}
int  SignalControllerNTCIP::getDMSMsgCRC()
{
	//printf("\ngetDMSMsgCRCCurr Function\n");
	return getSingleINT(DMS_MSG_CRC, "public");
}
int  SignalControllerNTCIP::getDMSMsgRunTimePriority()
{
	//printf("\ngetDMSMsgRunTimePriority Function\n");
	return getSingleINT(DMS_MSG_RUNTIMEPRIORITY, "public");
}
int  SignalControllerNTCIP::getDMSMsgStatus(const char *MsgID)
{
	//printf("\ngetDMSMsgStatus Function\n");
	return getSingleINT(concat(DMS_MSG_STATUS, MsgID), "public");
}

int  SignalControllerNTCIP::getDMSMsgMemoryTypeCurr(const char *msgID)
{
	//printf("\ngetDMSMsgMemoryType_Curr Function\n");
	return getSingleINT(concat(DMS_MSG_MEMORYTYPE_CURR, msgID), "public");
}
int  SignalControllerNTCIP::getDMSMsgNumberCurr(const char *msgID)
{
	//printf("\ngetDMSMsgNumber_Curr Function\n");
	return getSingleINT(concat(DMS_MSG_NUMBER_CURR, msgID), "public");
}
char *  SignalControllerNTCIP::getDMSMsgMultiStringCurr(const char *msgID)
{
	//printf("\ngetDMSMsgMultiString_Curr Function\n");
	return getSingleString(concat(DMS_MSG_MULTISTRING_CURR, msgID), "public");
}
int  SignalControllerNTCIP::getDMSMsgOwnerCurr(const char *msgID)
{
	//printf("\ngetDMSMsgOwner_Curr Function\n");
	return getSingleINT(concat(DMS_MSG_OWNER_CURR, msgID), "public");
}
int  SignalControllerNTCIP::getDMSMsgCRCCurr(const char *msgID)
{
	//printf("\ngetDMSMsgCRC_Curr Function\n");
	return getSingleINT(concat(DMS_MSG_CRC_CURR, msgID), "public");
}
int  SignalControllerNTCIP::getDMSMsgRunTimePriorityCurr(const char *msgID)
{
	//printf("\ngetDMSMsgRunTimePriority_Curr Function\n");
	return getSingleINT(concat(DMS_MSG_RUNTIMEPRIORITY_CURR, msgID), "public");
}
int  SignalControllerNTCIP::getDMSMsgStatusCurr(const char *msgID)
{
	//printf("\ngetDMSMsgStatus_Curr Function\n");
	return getSingleINT(concat(DMS_MSG_STATUS_CURR, msgID), "public");
}


int  SignalControllerNTCIP::getDMSControlMode()
{
	//printf("\ngetDMSControlMode Function\n");
	return getSingleINT(DMS_CONTROL_MODE, "public");
}
int  SignalControllerNTCIP::getDMSMsgDisplayTimeRemaining()
{
	//printf("\ngetDMSMsgDisplayTimeRemaining Function\n");
	return getSingleINT(DMS_MSG_DISPLAY_TIME_REMAINING, "public");
}
//int  SignalControllerNTCIP::getDMSMsgTableSource()
char *  SignalControllerNTCIP::getDMSMsgTableSource()
{
	printf("\ngetDMSMsgTableSource Function\n");
	//return getSingleINT(DMS_MSG_TABLE_SOURCE, "public");
	return getSingleString(DMS_MSG_TABLE_SOURCE, "public");
}
//int  SignalControllerNTCIP::getDMSMsgRequesterID()
char *  SignalControllerNTCIP::getDMSMsgRequesterID()
{
	printf("\ngetDMSMsgRequesterID Function\n");
	//return getSingleINT(DMS_MSG_REQUESTER_ID, "public");
	return getSingleString(DMS_MSG_REQUESTER_ID, "public");
}
int  SignalControllerNTCIP::getDMSMsgSourceMode()
{
	//printf("\ngetDMSMsgSourceMode Function\n");
	return getSingleINT(DMS_MSG_SOURCE_MODE, "public");
}
int  SignalControllerNTCIP::getDMSMsgActivateError()
{
	//printf("\ngetDMSMsgActivateError Function\n");
	return getSingleINT(DMS_MSG_ACTIVATE_ERROR, "public");
}
int  SignalControllerNTCIP::getDMSMsgActivateErrorCode()
{
	//printf("\ngetDMSMsgActivateErrorCode Function\n");
	return getSingleINT(DMS_MSG_ACTIVATE_ERROR_CODE, "public");
}
int  SignalControllerNTCIP::getDMSMsgMultiSyntaxError()
{
	//printf("\ngetDMSMsgMultiSyntaxError Function\n");
	return getSingleINT(DMS_MSG_MULTI_SYNTAX_ERROR, "public");
}


void  SignalControllerNTCIP::setDMSControlMode(const char *Value)
{
	//printf("\n setDMSControlMode Function\n");
	setSNMP(DMS_CONTROL_MODE, "public", Value);
}
void SignalControllerNTCIP::setDMSMsgStatus(const char *Value, const char * MsgID)
{
	//printf("\n setDMSMsgStatus Function\n");
	setSNMP(concat(DMS_MSG_STATUS, MsgID), "public", Value);
}
void SignalControllerNTCIP::setDMSMsgMultiString(const char *Msg, const char *MsgID)
{
	//printf("\n setDMSMsgMultiString Function\n");
	setSNMPText(concat(DMS_MSG_MULTISTRING, MsgID), "public", Msg);
}
void SignalControllerNTCIP::setDMSMsgOwner(const char *Owner, const char *MsgID)
{
	//printf("\n setDMSMsgOwner Function\n");
	setSNMPText(concat(DMS_MSG_OWNER, MsgID), "public", Owner);
}

void SignalControllerNTCIP::setDMSMsgRunTimePriority(const char *Priority, const char *MsgID)
{
	setSNMP(concat(DMS_MSG_RUNTIMEPRIORITY, MsgID), "public", Priority);
	//printf("\n setDMSMsgRunTimePriority Function\n");
}


int  SignalControllerNTCIP::getDMSMsgActivate()
{
	//printf("\n getDMSMsgActivate Function\n");
	return getOctetString(DMS_ACTIVATE_MSG, "public");
}

// Returns true on success; false on error.
bool SignalControllerNTCIP::setDMSMsgActivate(const char *ActivationString)
{
	//printf("\n setDMSMsgActivate Function\n");
	return setSNMPActivateMsg(DMS_ACTIVATE_MSG, "public", ActivationString);
}

netsnmp_pdu*  SignalControllerNTCIP::setSNMPTCP(const char* setOID, const char *community, const char *Value)
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	//netsnmp_variable_list *vars;
	int status;

	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to
	/* set up defaults */
	char ipwithport[64];
	strcpy(ipwithport, "tcp:");
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1;

	/* set the SNMPv1 community name used for authentication */
	//session.community = (u_char *) "public";
	session.community = (u_char *)community;
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss)
	{
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		// TODO: add failure
		return response;
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	anOID_len = MAX_OID_LEN;


	if (!snmp_parse_oid(setOID, anOID, &anOID_len))
	{
		printf("Error parse OID failed\n");
		snmp_perror(ENABLE_SPAT);
		SOCK_CLEANUP;
		return response;
	}
	//snmp_add_var(pdu, anOID, anOID_len, 'i', "6");
	snmp_add_var(pdu, anOID, anOID_len, 'i', Value);

	/*
	 * Send the Request out.
	 */
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Process the response.
	 */
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		/*
		 * SUCCESS: Print the result variables
		 */
		//int *out = new int[MAX_ITEMS];
		//int i = 0;
//		for (vars = response->variables; vars; vars = vars->next_variable)
//			print_variable(vars->name, vars->name_length, vars);
/*
		// manipuate the information ourselves
		for (vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OCTET_STR) {
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				//printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			} else {

				int *aa;
				aa = (int *) vars->val.integer;
				out[i++] = *aa;
				//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
			}l
		}

		// ----------get the current timing plan----------------//
		CurTimingPlan = out[0];
		*/
	}
	else
	{
		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",session.peername);
		else
			snmp_sess_perror("signalControllerSNMP", ss);
	}

	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;

	return response;
}

netsnmp_pdu*  SignalControllerNTCIP::setSNMP(const char* setOID, const char *community, const char *Value)
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	//netsnmp_variable_list *vars;
	int status;

	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to
	/* set up defaults */
	char ipwithport[64];
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1;

	/* set the SNMPv1 community name used for authentication */
	//session.community = (u_char *) "public";
	session.community = (u_char *)community;
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss)
	{
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		// TODO: add failure
		return response;
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	anOID_len = MAX_OID_LEN;


	if (!snmp_parse_oid(setOID, anOID, &anOID_len))
	{
		printf("Error parse OID failed\n");
		snmp_perror(ENABLE_SPAT);
		SOCK_CLEANUP;
		return response;
	}
	//snmp_add_var(pdu, anOID, anOID_len, 'i', "6");
	snmp_add_var(pdu, anOID, anOID_len, 'i', Value);

	/*
	 * Send the Request out.
	 */
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Process the response.
	 */
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		/*
		 * SUCCESS: Print the result variables
		 */
		//int *out = new int[MAX_ITEMS];
		//int i = 0;
//		for (vars = response->variables; vars; vars = vars->next_variable)
//			print_variable(vars->name, vars->name_length, vars);
/*
		// manipuate the information ourselves
		for (vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OCTET_STR) {
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				//printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			} else {

				int *aa;
				aa = (int *) vars->val.integer;
				out[i++] = *aa;
				//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
			}l
		}

		// ----------get the current timing plan----------------//
		CurTimingPlan = out[0];
		*/
	}
	else
	{
		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",session.peername);
		else
			snmp_sess_perror("signalControllerSNMP", ss);
	}

	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;

	return response;
}

netsnmp_pdu*  SignalControllerNTCIP::setSNMPInt(const char* setOID, const char *community, const char *Value)
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	netsnmp_variable_list *vars;
	int status;

	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to
	/* set up defaults */
	char ipwithport[64];
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1;

	/* set the SNMPv1 community name used for authentication */
	//session.community = (u_char *) "public";
	session.community = (u_char *)community;
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss)
	{
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		// TODO: add failure
		return response;
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	anOID_len = MAX_OID_LEN;


	if (!snmp_parse_oid(setOID, anOID, &anOID_len))
	{
		printf("Error parse OID failed\n");
		//Check with greg if we need to set the variable in the next commented line
		//based on the object we are trying to set its value
		//snmp_perror(ENABLE_SPAT);
		SOCK_CLEANUP;
		return response;
	}
	//snmp_add_var(pdu, anOID, anOID_len, 'i', "6");
	snmp_add_var(pdu, anOID, anOID_len, 'i', Value);

	/*
	 * Send the Request out.
	 */
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Process the response.
	 */

	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		printf("SetSNMPInt Function - OID: %s   \nCommunity: %s   \nValue: %s", setOID, community, Value);
		/*
		 * SUCCESS: Print the result variables
		 */
		int *out = new int[MAX_ITEMS];
		int i = 0;
		int count = 0;

		for (vars = response->variables; vars; vars = vars->next_variable)
			print_variable(vars->name, vars->name_length, vars);

		// manipuate the information ourselves
		for (vars = response->variables; vars; vars = vars->next_variable)
		{
			if (vars->type == ASN_OCTET_STR)
			{
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			}
			else if (vars->type == ASN_INTEGER)
			{
				int *aa;
				aa = (int *) vars->val.integer;
				out[i++] = *aa;
				//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
			}
		}

		// ----------get the current timing plan----------------//
		CurTimingPlan = out[0];
		// freed the block of allocated memory
		delete[] out;

	}
	else
	{
		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",session.peername);
		else
			snmp_sess_perror("signalControllerSNMP", ss);
	}

	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;

	return response;
}

netsnmp_pdu*  SignalControllerNTCIP::setSNMPText(const char* getOID, const char *community, const char *Value)
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	//netsnmp_variable_list *vars;
	int status;

	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to

	/* set up defaults */
	char ipwithport[64];

	//Original code to add ip address and port to the peername
	//strcpy(ipwithport, INTip);
	//strcat(ipwithport, ":");
	//strcat(ipwithport, INTport);

	//Modified code for adding ip address and port by adding the transport as tcp
	//strcpy(ipwithport, "tcp:");
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1;

	/* set the SNMPv1 community name used for authentication */
	//session.community = (u_char *) "public";
	session.community = (u_char *)community;
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss)
	{
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		// TODO: add failure
		return response;
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	anOID_len = MAX_OID_LEN;


	if (!snmp_parse_oid(getOID, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		printf("Error parse failed\n");
		snmp_perror(ENABLE_SPAT);
		SOCK_CLEANUP;
		return response;
	}
	//snmp_add_var(pdu, anOID, anOID_len, 'i', "6");
	snmp_add_var(pdu, anOID, anOID_len, 's', Value);

	/*
	 * Send the Request out.
	 */
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Process the response.
	 */
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		/*
		 * SUCCESS: Print the result variables
		 */
		//int *out = new int[MAX_ITEMS];
		//int i = 0;
//		for (vars = response->variables; vars; vars = vars->next_variable)
//			print_variable(vars->name, vars->name_length, vars);
/*
		// manipuate the information ourselves
		for (vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OCTET_STR) {
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				//printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			} else {

				int *aa;
				aa = (int *) vars->val.integer;
				out[i++] = *aa;
				//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
			}l
		}

		// ----------get the current timing plan----------------//
		CurTimingPlan = out[0];
		*/
	}
	else
	{
		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",session.peername);
		else
			snmp_sess_perror("signalControllerSNMP", ss);
	}

	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;

	return response;
}

// Return true on success.  False on error.
bool SignalControllerNTCIP::setSNMPActivateMsg(const char* getOID, const char *community, const char * ActivationString)
{
	bool isSuccess = false;
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;
	//int j = 0;

	netsnmp_variable_list *vars;
	int status;

	snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to

	/* set up defaults */
	char ipwithport[64];

	//Original code to add ip address and port to the peername
	//strcpy(ipwithport, INTip);
	//strcat(ipwithport, ":");
	//strcat(ipwithport, INTport);

	//Modified code for adding ip address and port by adding the transport as tcp
	//strcpy(ipwithport, "tcp:");
	strcpy(ipwithport, INTip);
	strcat(ipwithport, ":");
	strcat(ipwithport, INTport);
	session.peername = strdup(ipwithport);
	session.version = SNMP_VERSION_1;

	/* set the SNMPv1 community name used for authentication */
	//session.community = (u_char *) "public";
	session.community = (u_char *)community;
	session.community_len = strlen((const char *) session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session); /* establish the session */

	if (!ss)
	{
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		return false;
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_SET);
	anOID_len = MAX_OID_LEN;


	if (!snmp_parse_oid(getOID, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	{
		printf("Error parse failed\n");
		snmp_perror(ENABLE_SPAT);
		SOCK_CLEANUP;
		return false;
	}
	//snmp_add_var(pdu, anOID, anOID_len, 'i', "6");
	//char* SignActivationInfo="FFFFFF030001057B12345678";

	//snmp_add_var(pdu, anOID, anOID_len, 'x', "FFFFFF030001057B12345678");
	snmp_add_var(pdu, anOID, anOID_len, 'x', ActivationString);

	//snmp_add_var(pdu, anOID, anOID_len, 'x', "FF FF FF 03 00 01 53 54 12 34 56 78");
	status = snmp_synch_response(ss, pdu, &response);

	/*
	 * Process the response.
	 */
	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{
		isSuccess = true;

		/*
		 * SUCCESS: Print the result variables
		 */
		int *out = new int[MAX_ITEMS];
		int i = 0;
		for (vars = response->variables; vars; vars = vars->next_variable)
			print_variable(vars->name, vars->name_length, vars);

		// manipuate the information ourselves
		for (vars = response->variables; vars; vars = vars->next_variable)
		{
			if (vars->type == ASN_OCTET_STR)
			{
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				//printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
				//j++;
			}
			else
			{

				int *aa;
				aa = (int *) vars->val.integer;
				out[i++] = *aa;
				//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
			}
		}

		// ----------get the current timing plan----------------//
		//CurTimingPlan = out[0];
		// freed the block of allocated memory
		delete[] out;

	}
	else
	{
		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",session.peername);
		else
			snmp_sess_perror("signalControllerSNMP", ss);
	}

	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;

	return isSuccess;
}
