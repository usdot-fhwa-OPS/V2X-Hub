#ifndef GCKRPC_LAYER_H_
#define GCKRPC_LAYER_H_

#include "pkcs11/pkcs11.h"

#include "gck-rpc-tls-psk.h"

/* ------------------------------------------------------------------
 * DISPATCHER
 */

/* Call to initialize the module and start listening, returns socket or -1 */
int gck_rpc_layer_initialize(const char *prefix, CK_FUNCTION_LIST_PTR funcs);

/* Should be called to cleanup dispatcher */
void gck_rpc_layer_uninitialize(void);

/* Accept a new connection. Should be called when above fd has read */
void gck_rpc_layer_accept(GckRpcTlsPskState *tls);

/* Run a single connection off of STDIN - call from inetd or stunnel */
void gck_rpc_layer_inetd(CK_FUNCTION_LIST_PTR funcs);

#endif /* GCKRPC_LAYER_H_ */
