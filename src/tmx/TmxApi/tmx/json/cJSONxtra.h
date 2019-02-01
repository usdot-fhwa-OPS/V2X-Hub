/*
 * cJSONxtra.h
 *
 *  Created on: Jul 17, 2014
 *      Author: ivp
 */

#ifndef CJSONXTRA_H_
#define CJSONXTRA_H_

#include "cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif

int cJSONxtra_tryGetStr(cJSON *root, const char *fieldName, char **out);
int cJSONxtra_tryGetUnsignedInt(cJSON *root, const char *fieldName, unsigned int *out);
int cJSONxtra_tryGetInt(cJSON *root, const char *fieldName, int *out);
int cJSONxtra_tryGetInt64(cJSON *root, const char *fieldName, uint64_t *out);
int cJSONxtra_tryGetDouble(cJSON *root, const char *fieldName, double *out);

#ifdef __cplusplus
}
#endif

#endif /* CJSONXTRA_H_ */
