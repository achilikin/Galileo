/*	Apache 2.0 License
	
	Copyright (c) 2015 Andrey Chilikin https://github.com/achilikin

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/
#include <Arduino.h>

#ifdef __ARDUINO_X86__

#include <stdio.h>

#include "netif.h"

int get_netif_info(const char *name, netif_t *netif)
{
	iostat_t *pstat;
	char *val, str[512];
	int i, ifound = 0;

	if (name == NULL || netif == NULL)
		return -1;

	memset(netif, 0, sizeof(netif_t));

	sprintf(str, "ifconfig %s", name);
	FILE *pf = popen(str, "r");
	if (pf == NULL)
		return -1;

	while(fgets(str, 512, pf)) {
		if (strstr(str, name)) {
			strcpy(netif->name, name);
			ifound = 1;
		}

		if (!ifound)
			continue;

		if ((val = strstr(str, "HWaddr ")) != NULL) {
			val = strchr(val, ' ') + 1;
			for(i = 0; val[i] != '\0' && i < 17; i++)
				netif->hwas[i] = val[i];
			netif->hwas[i] = 0;
			continue;
		}

		if ((val = strstr(str, " inet addr:")) != NULL) {
			val = strchr(val, ':') + 1;
			while(*val == ' ') val++;
			for(i = 0; val[i] != ' '; i++)
				netif->ip4as[i] = val[i];
			netif->ip4as[i] = 0;
			continue;
		}

		if ((val = strstr(str, " inet6 addr:")) != NULL) {
			val = strchr(val, ':') + 1;
			while(*val == ' ') val++;
			for(i = 0; val[i] != ' ' && val[i] != '/'; i++)
				netif->ip6as[i] = val[i];
			netif->ip6as[i] = 0;
			continue;
		}
			
		if (strstr(str, "RX "))
			pstat = &netif->rx;

		if (strstr(str, "TX "))
			pstat = &netif->tx;

		if ((val = strstr(str, " packets:")) != NULL) {
			val = strchr(val, ':') + 1;
			pstat->packets = strtoul(val, &val, 10);
		}
		if ((val = strstr(str, " errors:")) != NULL) {
			val = strchr(val, ':') + 1;
			pstat->errors = strtoul(val, &val, 10);
		}
		if ((val = strstr(str, " dropped:")) != NULL) {
			val = strchr(val, ':') + 1;
			pstat->dropped = strtoul(val, &val, 10);
		}

		if ((val = strstr(str, "RX bytes:")) != NULL) {
			val = strchr(val, ':') + 1;
			netif->rx.bytes = strtoul(val, &val, 10);
		}
		if ((val = strstr(str, "TX bytes:")) != NULL) {
			val = strchr(val, ':') + 1;
			netif->tx.bytes = strtoul(val, &val, 10);
			break;
		}
	}

	fclose(pf);
	return ifound ? 0 : -1;
}

#endif
