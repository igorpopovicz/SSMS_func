#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

int snmpGet(char *oid_string, char *ipPort_string)
{
    int value = 0;

    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;

    init_snmp("getAPI");

    snmp_sess_init( &session );                   
    session.peername = strdup(ipPort_string);

    session.version = SNMP_VERSION_1;

    session.community = (u_char *)"public";
    session.community_len = strlen((char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);

    if (!ss) {
      snmp_sess_perror("ack", &session);
      SOCK_CLEANUP;
      exit(1);
    }
    
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;
    if (!snmp_parse_oid(oid_string, anOID, &anOID_len)) {
      snmp_perror(oid_string);
      SOCK_CLEANUP;
      exit(1);
    }

    snmp_add_null_var(pdu, anOID, anOID_len);

    status = snmp_synch_response(ss, pdu, &response);

    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {

      for(vars = response->variables; vars; vars = vars->next_variable) {
        if (vars->type == ASN_OCTET_STR) {
	        char *sp = (char *)malloc(1 + vars->val_len);
	        memcpy(sp, vars->val.string, vars->val_len);
	        sp[vars->val_len] = '\0';
	        free(sp);
	      }
        else if (vars->type == ASN_INTEGER) {
          long sp = *vars->val.integer;
          value = sp;

        }
      }
    }else {

      if (status == STAT_SUCCESS)
      {
        fprintf(stderr, "Error in packet\nReason: %s\n",
        snmp_errstring(response->errstat));
      }
      else if (status == STAT_TIMEOUT)
      {
        fprintf(stderr, "Timeout: No response from %s.\n",
        session.peername);
        return -1;
      }
      else
        snmp_sess_perror("snmpdemoapp", ss);

    }

    if (response)
      snmp_free_pdu(response);
    snmp_close(ss);

    SOCK_CLEANUP;
    return value;
}

